#include "utils.hpp"
#include "../native/imports.hpp"
#include "../utils/mem.hpp"

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
    auto thread_list_head = static_cast<w2004::EPROCESS *>( process )->Pcb.ThreadListHead;
    
    auto iter = thread_list_head.Flink;
    while (iter != &thread_list_head) {
        auto thread = CONTAINING_RECORD( iter, w2004::ETHREAD, ThreadListEntry );
        if (thread->Cid.UniqueThread == static_cast<HANDLE>(thread_id)) {
            RemoveEntryList( iter );

            // spoof thread info
            thread->Win32StartAddress = reinterpret_cast<void *>( -1 );
            thread->Cid.UniqueProcess = reinterpret_cast<void *>( -1 );
            thread->Cid.UniqueThread = reinterpret_cast<void *>( -1 );

            break;
        }

        iter = iter->Flink;
    }
}