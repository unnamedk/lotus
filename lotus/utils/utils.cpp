#include "utils.hpp"
#include "../native/imports.hpp"
#include "../utils/mem.hpp"
#include "../native/smbios_types.hpp"

#include "../utils/log.hpp"

native::system_driver &utils::get_kernel_base()
{
    static native::system_driver kernel { "\\SystemRoot\\system32\\ntoskrnl.exe" };
    return kernel;
}

void *utils::query_system_information( std::int32_t info_code ) noexcept
{
    auto size = 0ul;

    auto status = ZwQuerySystemInformation( static_cast<SYSTEM_INFORMATION_CLASS>( info_code ), nullptr, size, &size );
    if ( status != STATUS_INFO_LENGTH_MISMATCH ) {
        return nullptr;
    }

    auto memory = utils::mem::alloc( size );
    if ( !memory ) {
        return nullptr;
    }

    if ( !NT_SUCCESS( ZwQuerySystemInformation( static_cast<SYSTEM_INFORMATION_CLASS>( info_code ), memory, size, &size ) ) ) {
        ExFreePoolWithTag( memory, 'enoN' );
        return nullptr;
    }

    return memory;
}

void utils::unlink_thread( void* process, void* thread_id ) noexcept
{
    auto thread_list_head = static_cast<w2004::EPROCESS *>( process )->ThreadListHead;

    auto iter = thread_list_head.Flink;
    while (iter != &thread_list_head) {
        auto thread = CONTAINING_RECORD( iter, w2004::ETHREAD, ThreadListEntry );

        auto id = PsGetThreadId( reinterpret_cast<PETHREAD>( thread ) );
        if ( id == static_cast<HANDLE>( thread_id ) ) {
            RemoveEntryList( iter );

            // spoof thread info
            thread->Win32StartAddress = reinterpret_cast<void *>( -1 );
            thread->Cid.UniqueProcess = reinterpret_cast<void *>( -1 );
            thread->Cid.UniqueThread = reinterpret_cast<void *>( -1 );
            
            // bypass PsLookupThreadByThreadId
            // not sure if this has any side-effects we'll see, it works at least
            // if ( !(*(_DWORD *)(thread_cid_entry + 0x510) & 2) ) {
            //     HalPutDmaAdapter(thread_cid_entry);
            //     return 0xC000000Bi64;
            // }
            thread->ThreadInserted = 0;


            break;
        }

        iter = iter->Flink;
    }
}

std::uint64_t utils::hash( void *buffer, int len, int seed )
{
    // murmur hash
    const uint64_t m = 0xc6a4a7935bd1e995;
    const int r = 47;

    uint64_t h = seed ^ ( len * m );

    const uint64_t *data = ( const uint64_t * ) buffer;
    const uint64_t *end = data + ( len / 8 );

    while ( data != end ) {
        uint64_t k = *data++;

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    const unsigned char *data2 = ( const unsigned char * ) data;

    switch ( len & 7 ) {
        case 7:
            h ^= uint64_t( data2[ 6 ] ) << 48;
        case 6:
            h ^= uint64_t( data2[ 5 ] ) << 40;
        case 5:
            h ^= uint64_t( data2[ 4 ] ) << 32;
        case 4:
            h ^= uint64_t( data2[ 3 ] ) << 24;
        case 3:
            h ^= uint64_t( data2[ 2 ] ) << 16;
        case 2:
            h ^= uint64_t( data2[ 1 ] ) << 8;
        case 1:
            h ^= uint64_t( data2[ 0 ] );
            h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}