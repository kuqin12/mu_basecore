==============================
Project Mu Basecore Repository
==============================

============================= ================= =============== ===================
 Host Type & Toolchain        Build Status      Test Status     Code Coverage
============================= ================= =============== ===================
Windows_VS2022_               |WindowsCiBuild|  |WindowsCiTest| |WindowsCiCoverage|
Ubuntu_GCC5_                  |UbuntuCiBuild|   |UbuntuCiTest|  |UbuntuCiCoverage|
============================= ================= =============== ===================

This repository is part of Project Mu.  Please see Project Mu for details https://microsoft.github.io/mu

For more details about the repository, refer to `RepoDetails.md`_.

.. _`RepoDetails.md`: https://github.com/microsoft/mu_basecore/blob/HEAD/RepoDetails.md

Branch Status - release/202308
==============================

:Status:
  In Development

:Entered Development:
  2023/10/16

:Anticipated Stabilization:
  Feb 2024

Branch Changes - release/202308
===============================

Breaking Changes-dev
--------------------

- UefiCpuPkg/ResetVector/Bin/IA32/ResetVector.inf was removed along with all pre-built reset vector binaries.
The Readme.txt was updated to describe the new way to integrate the rest vector.

Main Changes-dev
----------------

- Moved to openssl 3.0.9 from 1.1.1t.  This comes with a large size increase.
- Added TraceHubLib suport
- Added suport FDT library in MdePkg
- Updated PEI to use restricted memory mappings
- Added SmmCpuFeaturesLib implementation for AMD processor family
- Cleaned up and removed definitions and tool chains in BaseTools

Bug Fixes-dev
-------------

- A bunch of Codeql fixes for changes from edk2
- Fixed AMD hardware specific bugs

2308_RefBoot Changes
--------------------

- Incomplete

2308_CIBuild Changes
--------------------

- Fixed markdown errors in multiple readme files from edk2
- Updated MdeModulePkg Pcd indentifier values to work with tracehub additions
- Disabled Vtf0.inf to be able to build CI
- Updated PiSmmCpuCpuDxe and DxeMpLib.c to work with edk2 Changes
- Removed some rebase artifacts from CryptoPkg.dsc
- Updated tools_def.template to use the new MdePkg GnuNoteBti.bin
- Fixed header file issue in StandaloneMmPeCoffExtraActionLib.c
- Updated Crypto tests to not use native instructions for non IA32 and X64 build_status_windows

2308_Rebase Changes
-------------------

| Starting commit: 1f4f0a4243 ("Repo File Sync: Update to Mu DevOps 7.0.1 and Rust 1.73.0", 2023-10-13)
| Destination Commit from upstream edk2: 819cfc6b42 ("OvmfPkg/RiscVVirt: Fix issues in VarStore Blockmap config", 2023-08-24)

- In PcRtc.c the default year was changed.
  - Keeping our default year but using new logic
- BdsEntry.c default boot behavior slightly changed.
  - Staying with MU_CHANGE that prevents loading the default boot option.


Code of Conduct
===============

This project has adopted the Microsoft Open Source Code of Conduct https://opensource.microsoft.com/codeofconduct/

For more information see the Code of Conduct FAQ https://opensource.microsoft.com/codeofconduct/faq/
or contact `opencode@microsoft.com <mailto:opencode@microsoft.com>`_. with any additional questions or comments.

Contributions
=============

Contributions are always welcome and encouraged!
Please open any issues in the Project Mu GitHub tracker and read https://microsoft.github.io/mu/How/contributing/

For documentation:

Copyright & License
===================

| Copyright (C) Microsoft Corporation
| SPDX-License-Identifier: BSD-2-Clause-Patent

Upstream License (TianoCore)
============================

Copyright (c) 2019, TianoCore and contributors.  All rights reserved.

SPDX-License-Identifier: BSD-2-Clause-Patent

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

Subject to the terms and conditions of this license, each copyright holder
and contributor hereby grants to those receiving rights under this license
a perpetual, worldwide, non-exclusive, no-charge, royalty-free, irrevocable
(except for failure to satisfy the conditions of this license) patent
license to make, have made, use, offer to sell, sell, import, and otherwise
transfer this software, where such license applies only to those patent
claims, already acquired or hereafter acquired, licensable by such copyright
holder or contributor that are necessarily infringed by:

(a) their Contribution(s) (the licensed copyrights of copyright holders and
    non-copyrightable additions of contributors, in source or binary form)
    alone; or

(b) combination of their Contribution(s) with the work of authorship to
    which such Contribution(s) was added by such copyright holder or
    contributor, if, at the time the Contribution is added, such addition
    causes such combination to be necessarily infringed. The patent license
    shall not apply to any other combinations which include the
    Contribution.

Except as expressly stated above, no rights or licenses from any copyright
holder or contributor is granted under this license, whether expressly, by
implication, estoppel or otherwise.

DISCLAIMER

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

.. ===================================================================
.. This is a bunch of directives to make the README file more readable
.. ===================================================================

.. CoreCI

.. _Windows_VS2019: https://dev.azure.com/projectmu/mu/_build/latest?definitionId=39&&branchName=release%2F202308
.. |WindowsCiBuild| image:: https://dev.azure.com/projectmu/mu/_apis/build/status/CI/Mu%20Basecore%20CI%20VS2019?branchName=release%2F202308
.. |WindowsCiTest| image:: https://img.shields.io/azure-devops/tests/projectmu/mu/39.svg
.. |WindowsCiCoverage| image:: https://img.shields.io/badge/coverage-coming_soon-blue

.. _Ubuntu_GCC5: https://dev.azure.com/projectmu/mu/_build/latest?definitionId=40&branchName=release%2F202308
.. |UbuntuCiBuild| image:: https://dev.azure.com/projectmu/mu/_apis/build/status/CI/Mu%20Basecore%20CI%20Ubuntu%20GCC5?branchName=release%2F202308
.. |UbuntuCiTest| image:: https://img.shields.io/azure-devops/tests/projectmu/mu/40.svg
.. |UbuntuCiCoverage| image:: https://img.shields.io/badge/coverage-coming_soon-blue

.. |build_status_windows| image:: https://dev.azure.com/projectmu/mu/_apis/build/status/CI/Mu%20Basecore%20CI%20VS2019?branchName=release%2F202308
