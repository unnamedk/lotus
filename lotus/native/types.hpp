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

struct KPROCESS
{
    pad( 0x30 );
    LIST_ENTRY ThreadListHead;
    pad( 0x3f8 );
};

#define offsetof( s, m ) ( ( size_t ) & ( ( ( s * ) 0 )->m ) )
namespace w2004
{
    struct KTHREAD
    {
        pad( 0xe8 );
        void *Teb;
        pad( 0x130 );
        void *Process;
        pad( 0x208);
    };

    struct ETHREAD
    {
        KTHREAD Tcb; // 0x0
        LARGE_INTEGER CreateTime; // 0x430
        pad( 0x38 );
        CLIENT_ID Cid; // 0x478
        pad( 0x50 );
        void *Win32StartAddress; // 0x4d0
        pad( 0x10 );
        LIST_ENTRY ThreadListEntry; // 0x4e8
    };

    struct EPROCESS
    {
        KPROCESS Pcb;
        pad( 0x8 );
        void *UniqueProcessId;
        LIST_ENTRY ActiveProcessLinks;
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
        pad( 0x40 );
        CLIENT_ID Cid; // 0x630
        pad( 0x48 );
        void *Win32StartAddress; // 0x4d0
        pad( 0x10 );
        LIST_ENTRY ThreadListEntry; // 0x4e8
    };
}

namespace w1809
{
    using w1803::KTHREAD;

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
    };
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
    };
}

namespace w1909
{
    using w1903::KTHREAD;

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
    };
}