"""Package and replace the SEA firmware files in an existing UEFI image."""

import argparse
import hashlib
import json
import logging
import os
import platform
import shutil
import subprocess
import sys
import tempfile
import time
import uuid
from dataclasses import dataclass
from pathlib import Path
from typing import Optional


SCRIPT_DIR = Path(__file__).resolve().parent
DEFAULT_BASETOOLS = (
    SCRIPT_DIR.parents[1]
    if (SCRIPT_DIR / "FMMT").is_dir()
    else SCRIPT_DIR / "MU_BASECORE" / "BaseTools"
)
DEFAULT_MANIFEST = SCRIPT_DIR / "sea_firmware_manifest.json"

SECTION_TYPES = {
    "PE32": ("EFI_SECTION_PE32", 0x10),
    "RAW": ("EFI_SECTION_RAW", 0x19),
}

FFS_TYPES = {
    "FREEFORM": ("EFI_FV_FILETYPE_FREEFORM", 0x02),
    "MM_CORE_STANDALONE": ("EFI_FV_FILETYPE_MM_CORE_STANDALONE", 0x0F),
}


@dataclass(frozen=True)
class SeaFile:
    name: str
    guid: str
    payload_path: str
    section_type: str
    section_type_value: int
    ffs_type: str
    ffs_type_value: int


class SwapError(RuntimeError):
    pass


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Replace manifest-defined FFS files in a UEFI firmware image.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument("input", type=Path, help="Input FD/BIN firmware image")
    parser.add_argument("output", type=Path, help="Output firmware image")
    parser.add_argument(
        "-m",
        "--manifest",
        type=Path,
        default=DEFAULT_MANIFEST,
        help="JSON replacement manifest; relative payload paths use the launch directory",
    )
    parser.add_argument(
        "-b",
        "--basetools",
        type=Path,
        default=DEFAULT_BASETOOLS,
        help="EDK II BaseTools directory containing Python sources and built tools",
    )
    parser.add_argument(
        "--replace-all",
        action="store_true",
        help="Replace every instance when a target GUID occurs more than once",
    )
    parser.add_argument(
        "--keep-work-dir",
        action="store_true",
        help="Keep generated SEC/FFS files for inspection",
    )
    return parser.parse_args()


def _sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as file:
        for block in iter(lambda: file.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def _load_manifest(path: Path) -> tuple[SeaFile, ...]:
    try:
        with path.open("r", encoding="utf-8") as manifest_file:
            manifest = json.load(manifest_file)
    except (OSError, json.JSONDecodeError) as error:
        raise SwapError(f"Unable to load manifest {path}: {error}") from error

    if not isinstance(manifest, dict) or manifest.get("schema_version") != 1:
        raise SwapError(f"Manifest {path} must have schema_version 1")
    entries = manifest.get("files")
    if not isinstance(entries, list) or not entries:
        raise SwapError(f"Manifest {path} must contain a non-empty files array")

    files = []
    seen_guids = set()
    required_fields = {
        "name",
        "guid",
        "payload",
        "section_type",
        "ffs_type",
    }
    for index, entry in enumerate(entries):
        if not isinstance(entry, dict):
            raise SwapError(f"Manifest files[{index}] must be an object")
        missing = required_fields - entry.keys()
        if missing:
            raise SwapError(
                f"Manifest files[{index}] is missing: {', '.join(sorted(missing))}"
            )
        try:
            guid = str(uuid.UUID(entry["guid"])).upper()
        except (AttributeError, TypeError, ValueError) as error:
            raise SwapError(
                f"Manifest files[{index}] has invalid GUID: {entry['guid']!r}"
            ) from error
        if guid in seen_guids:
            raise SwapError(f"Manifest contains duplicate GUID: {guid}")
        seen_guids.add(guid)

        if not isinstance(entry["section_type"], str):
            raise SwapError(
                f"Manifest files[{index}] section_type must be a string"
            )
        if not isinstance(entry["ffs_type"], str):
            raise SwapError(f"Manifest files[{index}] ffs_type must be a string")
        section_key = entry["section_type"].upper()
        ffs_key = entry["ffs_type"].upper()
        if section_key not in SECTION_TYPES:
            raise SwapError(
                f"Manifest files[{index}] has unsupported section_type: "
                f"{entry['section_type']!r}"
            )
        if ffs_key not in FFS_TYPES:
            raise SwapError(
                f"Manifest files[{index}] has unsupported ffs_type: "
                f"{entry['ffs_type']!r}"
            )
        if not isinstance(entry["name"], str) or not entry["name"]:
            raise SwapError(f"Manifest files[{index}] name must be a non-empty string")
        if not isinstance(entry["payload"], str) or not entry["payload"]:
            raise SwapError(
                f"Manifest files[{index}] payload must be a non-empty string"
            )

        section_type, section_type_value = SECTION_TYPES[section_key]
        ffs_type, ffs_type_value = FFS_TYPES[ffs_key]
        files.append(
            SeaFile(
                entry["name"],
                guid,
                entry["payload"],
                section_type,
                section_type_value,
                ffs_type,
                ffs_type_value,
            )
        )
    return tuple(files)


def _resolve_payloads(
    sea_files: tuple[SeaFile, ...],
) -> dict[str, Path]:
    payloads = {}
    for sea_file in sea_files:
        path = Path(sea_file.payload_path).resolve()
        if not path.is_file():
            raise SwapError(f"Missing {sea_file.name} payload: {path}")
        payloads[sea_file.guid] = path
    return payloads


def _find_tools_dir(basetools: Path) -> Path:
    system = platform.system()
    machine = platform.machine()
    architecture = "ARM-64" if machine.lower() in ("arm64", "aarch64") else "x86"
    candidates = [basetools / "Bin" / "Mu-Basetools_extdep" / f"{system}-{architecture}"]
    if system == "Windows":
        build_folders = ("Win32",) if machine.lower() in ("x86", "i386", "i686") else ("Win64", "Win32")
        candidates.extend(basetools / "Bin" / folder for folder in build_folders)
        suffix = ".exe"
    else:
        candidates.extend((basetools / "Bin" / f"{system}-{machine}", basetools / "Source" / "C" / "bin"))
        suffix = ""

    for directory in candidates:
        if all((directory / f"{name}{suffix}").is_file() for name in ("GenSec", "GenFfs")):
            return directory.resolve()
    searched = ", ".join(str(directory) for directory in candidates)
    raise SwapError(
        f"Cannot find GenSec and GenFfs under {basetools}. "
        "Build/download BaseTools for this host or use --basetools to select another installation. "
        f"Searched: {searched}"
    )


def _find_tool(name: str, tools_dir: Optional[Path]) -> str:
    names = (f"{name}.exe", name) if os.name == "nt" else (name,)
    if tools_dir:
        for candidate_name in names:
            candidate = tools_dir / candidate_name
            if candidate.is_file():
                return str(candidate.resolve())
    for candidate_name in names:
        candidate = shutil.which(candidate_name)
        if candidate:
            return candidate
    location = f" in {tools_dir}" if tools_dir else " on PATH"
    raise SwapError(f"Cannot find {name}{location}")


def _run(command: list[str]) -> None:
    result = subprocess.run(command, text=True, capture_output=True)
    if result.returncode != 0:
        detail = (result.stderr or result.stdout).strip()
        raise SwapError(f"Command failed ({result.returncode}): {' '.join(command)}\n{detail}")


def _load_fmmt(basetools: Path, tools_dir: Optional[Path]):
    python_source = basetools / "Source" / "Python"
    fmmt_source = python_source / "FMMT"
    if not fmmt_source.is_dir():
        raise SwapError(f"FMMT source directory not found: {fmmt_source}")

    os.environ.setdefault("FmmtConfPath", "")
    if tools_dir:
        os.environ["PATH"] = str(tools_dir.resolve()) + os.pathsep + os.environ["PATH"]
    sys.path.insert(0, str(fmmt_source))
    sys.path.insert(0, str(python_source))

    original_directory = Path.cwd()
    log_directory = tempfile.TemporaryDirectory(prefix="swap-sea-fmmt-log-")
    try:
        os.chdir(log_directory.name)
        try:
            from core.BiosTree import FFS_TREE, ROOT_TREE, SECTION_TREE
            from core.FMMTOperation import ReplaceFfs
            from core.FMMTParser import FMMTParser
        finally:
            os.chdir(original_directory)
    except ImportError as error:
        raise SwapError(f"Unable to import FMMT from {fmmt_source}: {error}") from error
    finally:
        fmmt_logger = logging.getLogger("FMMT")
        for handler in list(fmmt_logger.handlers):
            if isinstance(handler, logging.FileHandler):
                fmmt_logger.removeHandler(handler)
                handler.close()
        log_directory.cleanup()

    return FMMTParser, ReplaceFfs, ROOT_TREE, FFS_TREE, SECTION_TREE


def _parse_image(path: Path, fmmt_parser, root_tree):
    parser = fmmt_parser(str(path), root_tree)
    try:
        parser.ParserFromRoot(parser.WholeFvTree, path.read_bytes())
    except Exception as error:
        raise SwapError(
            f"FMMT could not parse {path}: {error}\n"
            "Ensure the compression utilities are available in the selected BaseTools installation."
        ) from error
    return parser


def _find_targets(
    root,
    sea_file: SeaFile,
    ffs_tree: str,
    section_tree: str,
    replace_all: bool,
):
    matches = []
    root.FindNode(uuid.UUID(sea_file.guid), matches)
    matches = [node for node in matches if node.type == ffs_tree]
    if not matches:
        raise SwapError(
            f"{sea_file.name} FFS ({sea_file.guid}) was not found"
        )
    if len(matches) > 1 and not replace_all:
        raise SwapError(
            f"Found {len(matches)} instances of {sea_file.name} FFS ({sea_file.guid}); "
            "use --replace-all to replace every instance"
        )

    for target in matches:
        sections = [
            child
            for child in target.Child
            if child.type == section_tree
            and child.Data.Type == sea_file.section_type_value
        ]
        if len(sections) != 1:
            raise SwapError(
                f"Expected one {sea_file.section_type} in {sea_file.guid}, "
                f"found {len(sections)}"
            )
    return matches


def _verify_payloads(
    root,
    sea_file: SeaFile,
    payload: Path,
    ffs_tree: str,
    section_tree: str,
    replace_all: bool,
) -> None:
    expected = payload.read_bytes()
    targets = _find_targets(
        root, sea_file, ffs_tree, section_tree, replace_all
    )
    for target in targets:
        actual_type = target.Data.Header.Type
        if actual_type != sea_file.ffs_type_value:
            raise SwapError(
                f"{sea_file.guid} has FFS type 0x{actual_type:02X}; "
                f"expected 0x{sea_file.ffs_type_value:02X}"
            )
        section = next(
            child
            for child in target.Child
            if child.type == section_tree
            and child.Data.Type == sea_file.section_type_value
        )
        if section.Data.Data != expected:
            raise SwapError(
                f"Payload verification failed for {sea_file.guid} at "
                f"FFS offset 0x{target.Data.HOffset:X}"
            )


def _targets_match_payload(
    targets,
    sea_file: SeaFile,
    payload: Path,
    section_tree: str,
) -> bool:
    expected = payload.read_bytes()
    return all(
        target.Data.Header.Type == sea_file.ffs_type_value
        and next(
            child
            for child in target.Child
            if child.type == section_tree
            and child.Data.Type == sea_file.section_type_value
        ).Data.Data
        == expected
        for target in targets
    )


def _build_ffs(
    sea_file: SeaFile,
    payload: Path,
    work_dir: Path,
    gen_sec: str,
    gen_ffs: str,
) -> Path:
    stem = sea_file.guid.lower()
    section = work_dir / f"{stem}.sec"
    ffs = work_dir / f"{stem}.ffs"
    _run([gen_sec, "-s", sea_file.section_type, "-o", str(section), str(payload)])
    _run(
        [
            gen_ffs,
            "-t",
            sea_file.ffs_type,
            "-g",
            sea_file.guid,
            "-o",
            str(ffs),
            "-i",
            str(section),
        ]
    )
    return ffs


def _swap(args: argparse.Namespace) -> None:
    start_time = time.perf_counter()
    input_path = args.input.resolve()
    output_path = args.output.resolve()
    basetools = args.basetools.resolve()

    if not input_path.is_file():
        raise SwapError(f"Input image not found: {input_path}")
    if input_path == output_path:
        raise SwapError("Input and output must be different files")
    if output_path.exists():
        raise SwapError(f"Output already exists: {output_path}")

    manifest_path = args.manifest.resolve()
    sea_files = _load_manifest(manifest_path)
    payloads = _resolve_payloads(sea_files)
    tools_dir = _find_tools_dir(basetools)
    gen_sec = _find_tool("GenSec", tools_dir)
    gen_ffs = _find_tool("GenFfs", tools_dir)
    fmmt_parser, replace_ffs, root_tree, ffs_tree, section_tree = _load_fmmt(
        basetools, tools_dir
    )

    print(f"Input:  {input_path}", flush=True)
    print(f"SHA256: {_sha256(input_path)}", flush=True)
    print(f"Manifest: {manifest_path} ({len(sea_files)} files)", flush=True)
    print("Parsing and checking input image...", flush=True)
    parser = _parse_image(input_path, fmmt_parser, root_tree)
    files_to_replace = []
    for sea_file in sea_files:
        targets = _find_targets(
            parser.WholeFvTree,
            sea_file,
            ffs_tree,
            section_tree,
            args.replace_all,
        )
        locations = ", ".join(
            f"0x{target.Data.HOffset:X} ({target.Data.Size} bytes)"
            for target in targets
        )
        print(f"Found {sea_file.guid} ({len(targets)}): {locations}", flush=True)
        print(
            f"  Payload SHA256: {_sha256(payloads[sea_file.guid])}",
            flush=True,
        )
        if _targets_match_payload(
            targets,
            sea_file,
            payloads[sea_file.guid],
            section_tree,
        ):
            print("  Already matches; replacement skipped", flush=True)
        else:
            files_to_replace.append(sea_file)

    temporary = Path(tempfile.mkdtemp(prefix="swap-sea-"))
    try:
        replacements = {}
        for index, sea_file in enumerate(files_to_replace, 1):
            print(
                f"[{index}/{len(files_to_replace)}] Packaging {sea_file.name}...",
                flush=True,
            )
            replacements[sea_file.guid] = _build_ffs(
                sea_file,
                payloads[sea_file.guid],
                temporary,
                gen_sec,
                gen_ffs,
            )

        current = input_path
        for index, sea_file in enumerate(files_to_replace):
            stage_start = time.perf_counter()
            print(
                f"[{index + 1}/{len(files_to_replace)}] Replacing {sea_file.name}...",
                flush=True,
            )
            next_image = temporary / f"step-{index + 1}.bin"
            try:
                replace_ffs(
                    str(current),
                    uuid.UUID(sea_file.guid),
                    str(replacements[sea_file.guid]),
                    str(next_image),
                )
            except Exception as error:
                raise SwapError(f"Failed to replace {sea_file.guid}: {error}") from error
            if not next_image.is_file():
                raise SwapError(f"FMMT did not create output while replacing {sea_file.guid}")
            current = next_image
            print(
                f"[{index + 1}/{len(files_to_replace)}] Replaced in "
                f"{time.perf_counter() - stage_start:.1f}s",
                flush=True,
            )

        if current.stat().st_size != input_path.stat().st_size:
            raise SwapError(
                f"Image size changed from {input_path.stat().st_size} to {current.stat().st_size} bytes"
            )

        verify_start = time.perf_counter()
        print("Verifying final image and all replaced payloads...", flush=True)
        verified = _parse_image(current, fmmt_parser, root_tree)
        for sea_file in sea_files:
            _verify_payloads(
                verified.WholeFvTree,
                sea_file,
                payloads[sea_file.guid],
                ffs_tree,
                section_tree,
                args.replace_all,
            )

        output_path.parent.mkdir(parents=True, exist_ok=True)
        with tempfile.NamedTemporaryFile(
            dir=output_path.parent,
            prefix=f".{output_path.name}.",
            suffix=".tmp",
            delete=False,
        ) as publish_file:
            publish_path = Path(publish_file.name)
        try:
            shutil.copyfile(current, publish_path)
            os.replace(publish_path, output_path)
        finally:
            publish_path.unlink(missing_ok=True)
        print(
            f"Verification completed in {time.perf_counter() - verify_start:.1f}s",
            flush=True,
        )
        print(f"Output: {output_path}", flush=True)
        print(f"SHA256: {_sha256(output_path)}", flush=True)
        print(f"Completed in {time.perf_counter() - start_time:.1f}s", flush=True)
    finally:
        if args.keep_work_dir:
            print(f"Work directory retained: {temporary}")
        else:
            shutil.rmtree(temporary, ignore_errors=True)


def main() -> int:
    try:
        _swap(_parse_args())
    except SwapError as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
