#ifndef FFAPERF_H_
#define FFAPERF_H_

#define FFA_PERF_DATA_BUFFER_BASE 0x10F000000
#define FFA_PERF_DATA_BUFFER_SIZE 0x2400000

#define FFA_PERF_DATA_SIG SIGNATURE_32('F', 'F', 'A', 'P')
#define FFA_PERF_DATA_VERSION 0x00000001

#define MAX_ENTRY_COUNT ((FFA_PERF_DATA_BUFFER_SIZE - sizeof(FFA_PERF_DATA_BUFFER)) / sizeof(FFA_PERF_ENTRY))

#define ARM_FFA_PERF_DATA_BUFFER_GUID \
  { 0x3f5c1e7b, 0x8d9e, 0x4a2c, { 0x9f, 0x6b, 0x1a, 0x2b, 0x3c, 0x4d, 0x5e, 0x6f } }

typedef enum {
  FFA_PERF_ENTRY_TYPE_RAW = 0,
  FFA_PERF_ENTRY_TYPE_DIRECT = 1,
  FFA_PERF_ENTRY_TYPE_RT = 2,
} FFA_PERF_ENTRY_TYPE;

#pragma pack(push, 1)

typedef struct {
  FFA_PERF_ENTRY_TYPE Type;
  UINT64 FunctionId;
  UINT64 StartTick;
  UINT64 EndTick;
  UINT64 Target;
  UINT64 ReturnValue;
} FFA_PERF_ENTRY;

typedef struct {
  UINT32 Signature;
  UINT32 Version;
  UINT64 TotalCalls;
  UINT64 LongestDurationNs;
  UINT64 TotalDurationNs;
  UINT64 TimerTickFrequency;
  UINT64 NextIndex;
  FFA_PERF_ENTRY CallEntries[];
} FFA_PERF_DATA_BUFFER;

#pragma pack(pop)

extern EFI_GUID gArmFfaPerfDataBufferGuid;

#endif // FFAPERF_H_
