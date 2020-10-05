#pragma once

#include <ntifs.h>
#include <ntimage.h>

#define PHNT_MODE 0
#include <phnt.h>

#include <cstdint>

#define pad_impl2( x, y ) x##y
#define pad_impl( x, y ) pad_impl2( x, y )
#define pad( x ) pad_impl( std::uint8_t __pad_, __COUNTER__ )[ x ];

typedef struct _MMPTE_HARDWARE
{
    ULONGLONG Valid : 1;
    ULONGLONG Writable : 1;
    ULONGLONG Owner : 1;
    ULONGLONG WriteThrough : 1;
    ULONGLONG CacheDisable : 1;
    ULONGLONG Accessed : 1;
    ULONGLONG Dirty : 1;
    ULONGLONG LargePage : 1;
    ULONGLONG Global : 1;
    ULONGLONG CopyOnWrite : 1;
    ULONGLONG Prototype : 1;
    ULONGLONG Write : 1;
    ULONGLONG PageFrameNumber : 40;
    ULONGLONG Reserved : 11;
    ULONGLONG NoExecute : 1;
} MMPTE_HARDWARE, *PMMPTE_HARDWARE;

typedef union _MMPTE
{
    ULONG_PTR Long;
    MMPTE_HARDWARE Hardware;
} MMPTE, *PMMPTE;

#pragma pack( push, 1 )

    typedef struct _IDINFO
{
    USHORT wGenConfig;
    USHORT wNumCyls;
    USHORT wReserved;
    USHORT wNumHeads;
    USHORT wBytesPerTrack;
    USHORT wBytesPerSector;
    USHORT wNumSectorsPerTrack;
    USHORT wVendorUnique[ 3 ];
    CHAR sSerialNumber[ 20 ];
    USHORT wBufferType;
    USHORT wBufferSize;
    USHORT wECCSize;
    CHAR sFirmwareRev[ 8 ];
    CHAR sModelNumber[ 40 ];
    USHORT wMoreVendorUnique;
    USHORT wDoubleWordIO;
    struct
    {
        USHORT Reserved : 8;
        USHORT DMA : 1;
        USHORT LBA : 1;
        USHORT DisIORDY : 1;
        USHORT IORDY : 1;
        USHORT SoftReset : 1;
        USHORT Overlap : 1;
        USHORT Queue : 1;
        USHORT InlDMA : 1;
    } wCapabilities;
    USHORT wReserved1;
    USHORT wPIOTiming;
    USHORT wDMATiming;
    struct
    {
        USHORT CHSNumber : 1;
        USHORT CycleNumber : 1;
        USHORT UnltraDMA : 1;
        USHORT Reserved : 13;
    } wFieldValidity;
    USHORT wNumCurCyls;
    USHORT wNumCurHeads;
    USHORT wNumCurSectorsPerTrack;
    USHORT wCurSectorsLow;
    USHORT wCurSectorsHigh;
    struct
    {
        USHORT CurNumber : 8;
        USHORT Multi : 1;
        USHORT Reserved : 7;
    } wMultSectorStuff;
    ULONG dwTotalSectors;
    USHORT wSingleWordDMA;
    struct
    {
        USHORT Mode0 : 1;
        USHORT Mode1 : 1;
        USHORT Mode2 : 1;
        USHORT Reserved1 : 5;
        USHORT Mode0Sel : 1;
        USHORT Mode1Sel : 1;
        USHORT Mode2Sel : 1;
        USHORT Reserved2 : 5;
    } wMultiWordDMA;
    struct
    {
        USHORT AdvPOIModes : 8;
        USHORT Reserved : 8;
    } wPIOCapacity;
    USHORT wMinMultiWordDMACycle;
    USHORT wRecMultiWordDMACycle;
    USHORT wMinPIONoFlowCycle;
    USHORT wMinPOIFlowCycle;
    USHORT wReserved69[ 11 ];
    struct
    {
        USHORT Reserved1 : 1;
        USHORT ATA1 : 1;
        USHORT ATA2 : 1;
        USHORT ATA3 : 1;
        USHORT ATA4 : 1;
        USHORT ATA5 : 1;
        USHORT ATA6 : 1;
        USHORT ATA7 : 1;
        USHORT ATA8 : 1;
        USHORT ATA9 : 1;
        USHORT ATA10 : 1;
        USHORT ATA11 : 1;
        USHORT ATA12 : 1;
        USHORT ATA13 : 1;
        USHORT ATA14 : 1;
        USHORT Reserved2 : 1;
    } wMajorVersion;
    USHORT wMinorVersion;
    USHORT wReserved82[ 6 ];
    struct
    {
        USHORT Mode0 : 1;
        USHORT Mode1 : 1;
        USHORT Mode2 : 1;
        USHORT Mode3 : 1;
        USHORT Mode4 : 1;
        USHORT Mode5 : 1;
        USHORT Mode6 : 1;
        USHORT Mode7 : 1;
        USHORT Mode0Sel : 1;
        USHORT Mode1Sel : 1;
        USHORT Mode2Sel : 1;
        USHORT Mode3Sel : 1;
        USHORT Mode4Sel : 1;
        USHORT Mode5Sel : 1;
        USHORT Mode6Sel : 1;
        USHORT Mode7Sel : 1;
    } wUltraDMA;
    USHORT wReserved89[ 167 ];
} IDINFO, *PIDINFO;


namespace w2004
{
    struct KTHREAD
    {
        pad( 0xf0 );
        void *Teb;
        pad( 0x128 );
        void *Process;
        pad( 0x208 );
    };

    struct ETHREAD
    {
        KTHREAD Tcb; // 0x0
        LARGE_INTEGER CreateTime; // 0x430
        pad( 0x40 );
        CLIENT_ID Cid; // 0x478
        pad( 0x48 );
        void *Win32StartAddress; // 0x4d0
        pad( 0x10 );
        LIST_ENTRY ThreadListEntry; // 0x4e8
        pad(0x18);
        union
        {
            ULONG CrossThreadFlags; //0x510
            struct
            {
                ULONG Terminated : 1; //0x510
                ULONG ThreadInserted : 1; //0x510
                ULONG HideFromDebugger : 1; //0x510
                ULONG ActiveImpersonationInfo : 1; //0x510
                ULONG HardErrorsAreDisabled : 1; //0x510
                ULONG BreakOnTermination : 1; //0x510
                ULONG SkipCreationMsg : 1; //0x510
                ULONG SkipTerminationMsg : 1; //0x510
                ULONG CopyTokenOnOpen : 1; //0x510
                ULONG ThreadIoPriority : 3; //0x510
                ULONG ThreadPagePriority : 3; //0x510
                ULONG RundownFail : 1; //0x510
                ULONG UmsForceQueueTermination : 1; //0x510
                ULONG IndirectCpuSets : 1; //0x510
                ULONG DisableDynamicCodeOptOut : 1; //0x510
                ULONG ExplicitCaseSensitivity : 1; //0x510
                ULONG PicoNotifyExit : 1; //0x510
                ULONG DbgWerUserReportActive : 1; //0x510
                ULONG ForcedSelfTrimActive : 1; //0x510
                ULONG SamplingCoverage : 1; //0x510
                ULONG ReservedCrossThreadFlags : 8; //0x510
            };
        };
    };

    struct KPROCESS
    {
        pad( 0x30 );
        LIST_ENTRY ThreadListHead;
        pad( 0x3f8 );
    };

    struct EPROCESS
    {
        KPROCESS Pcb;
        pad( 0x8 );
        void *UniqueProcessId; // 0x440
        LIST_ENTRY ActiveProcessLinks; // 0x448
        pad( 0x188 );
        LIST_ENTRY ThreadListHead; // 0x5e0
    };
}

namespace w1803
{
    struct KTHREAD
    {
        pad( 0xf0 );
        void *Teb; // 0xf0
        pad( 0x128 );
        void *Process; // 0x220
        pad( 0x3c8 );
    };

    struct ETHREAD
    {
        KTHREAD Tcb;
        LARGE_INTEGER CreateTime; // 0x5f0
        pad( 0x48 );
        CLIENT_ID Cid; // 0x638
        pad( 0x48 );
        void *Win32StartAddress; // 0x690
        pad( 0x10 );
        LIST_ENTRY ThreadListEntry; // 0x6a8
        pad( 0x18 );
        union
        {
            ULONG CrossThreadFlags; // 0x6d0
            struct
            {
                ULONG Terminated : 1; 
                ULONG ThreadInserted : 1; 
                ULONG HideFromDebugger : 1; 
                ULONG ActiveImpersonationInfo : 1; 
                ULONG HardErrorsAreDisabled : 1; 
                ULONG BreakOnTermination : 1; 
                ULONG SkipCreationMsg : 1; 
                ULONG SkipTerminationMsg : 1; 
                ULONG CopyTokenOnOpen : 1; 
                ULONG ThreadIoPriority : 3; 
                ULONG ThreadPagePriority : 3; 
                ULONG RundownFail : 1; 
                ULONG UmsForceQueueTermination : 1; 
                ULONG IndirectCpuSets : 1; 
                ULONG DisableDynamicCodeOptOut : 1; 
                ULONG ExplicitCaseSensitivity : 1; 
                ULONG PicoNotifyExit : 1; 
                ULONG DbgWerUserReportActive : 1; 
                ULONG ForcedSelfTrimActive : 1; 
                ULONG SamplingCoverage : 1; 
                ULONG ReservedCrossThreadFlags : 8; 
            };
        };
    };

    struct KPROCESS
    {
        pad( 0x2d8 );
    };

    struct EPROCESS
    {
        KPROCESS Pcb;
        pad( 0x8 );
        void *UniqueProcessId; // 0x2e0
        LIST_ENTRY ActiveProcessLinks; // 0x2e8
        pad( 0x190 );
        LIST_ENTRY ThreadListHead; // 0x488
    };
}

namespace w1809
{
    using w1803::KTHREAD;
    using w1803::ETHREAD;
    using w1803::KPROCESS;
    using w1803::EPROCESS;
}

namespace w1903
{
    struct KTHREAD
    {
        pad( 0xf0 );
        void *Teb; // 0xf0
        pad( 0x128 );
        void *Process; // 0x220
        pad( 0x3d8 );
    };

    struct ETHREAD
    {
        KTHREAD Tcb;
        LARGE_INTEGER CreateTime; // 0x600
        pad( 0x40 );
        CLIENT_ID Cid; // 0x648
        pad( 0x48 );
        void *Win32StartAddress; // 0x6a0
        pad( 0x10 );
        LIST_ENTRY ThreadListEntry; // 0x6b8
        pad( 0x18 );
        union
        {
            ULONG CrossThreadFlags; //0x6e0
            struct
            {
                ULONG Terminated : 1; 
                ULONG ThreadInserted : 1; 
                ULONG HideFromDebugger : 1; 
                ULONG ActiveImpersonationInfo : 1; 
                ULONG HardErrorsAreDisabled : 1; 
                ULONG BreakOnTermination : 1; 
                ULONG SkipCreationMsg : 1; 
                ULONG SkipTerminationMsg : 1; 
                ULONG CopyTokenOnOpen : 1; 
                ULONG ThreadIoPriority : 3; 
                ULONG ThreadPagePriority : 3; 
                ULONG RundownFail : 1; 
                ULONG UmsForceQueueTermination : 1; 
                ULONG IndirectCpuSets : 1; 
                ULONG DisableDynamicCodeOptOut : 1; 
                ULONG ExplicitCaseSensitivity : 1; 
                ULONG PicoNotifyExit : 1; 
                ULONG DbgWerUserReportActive : 1; 
                ULONG ForcedSelfTrimActive : 1; 
                ULONG SamplingCoverage : 1; 
                ULONG ReservedCrossThreadFlags : 8;
            };
        };
    };

    struct KPROCESS
    {
        pad( 0x2e0 );
    };

    struct EPROCESS
    {
        KPROCESS Pcb;
        pad( 0x8 );
        void *UniqueProcessId; // 0x2e8
        LIST_ENTRY ActiveProcessLinks; // 0x2f0
        pad( 0x188 );
        LIST_ENTRY ThreadListHead; // 0x488
    };
}

namespace w1909
{
    using namespace w1903;
}

#pragma pack( pop )