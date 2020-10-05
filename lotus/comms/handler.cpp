#include "handler.hpp"
#include "../utils/utils.hpp"
#include "../utils/mem.hpp"

#include "../native/system_process.hpp"
#include "../native/imports.hpp"

#include <memory>
#include "../utils/log.hpp"


HANDLE handler::g_section_handle = nullptr;

struct unloaded_driver_t
{
    UNICODE_STRING name;
    void *start_address;
    void *end_address;
    LARGE_INTEGER current_time;
};

struct piddb_cache_entry_t
{
    LIST_ENTRY list;
    UNICODE_STRING driver_name;
    ULONG time_date_stamp;
    NTSTATUS load_status;
};

void clear_mm_drivers( native::system_driver &ntoskrnl, const UNICODE_STRING &name, unsigned long *last_unloaded, unloaded_driver_t *unloaded_driver_list )
{
    constexpr auto MI_UNLOADED_DRIVERS = 50;

    // based on https://www.unknowncheats.me/forum/anti-cheat-bypass/302508-deleting-entry-mmunloadeddrivers-cleaning.html

    // first check if MmUnloadedDrivers is filled
    bool is_filled = true;
    for ( int i = 0; i < MI_UNLOADED_DRIVERS; ++i ) {
        auto unloaded_entry = unloaded_driver_list[ i ];
        if ( ( unloaded_entry.name.Length == 0 ) || ( unloaded_entry.name.Buffer == nullptr ) ) {
            is_filled = false;
            break;
        }
    }

    bool modified = false;
    for ( int i = 0; i < MI_UNLOADED_DRIVERS; ++i ) {
        auto unloaded_entry = &unloaded_driver_list[ i ];

        if ( modified ) {
            auto prev_entry = &unloaded_driver_list[ i - 1 ];
            RtlCopyMemory( prev_entry, unloaded_entry, sizeof( unloaded_driver_t ) );

            if ( i == ( MI_UNLOADED_DRIVERS - 1 ) ) {
                RtlFillMemory( unloaded_entry, sizeof( unloaded_driver_t ), 0 );
            }
        } else if ( RtlEqualUnicodeString( &unloaded_entry->name, &name, TRUE ) ) {
            // remove driver from list
            auto buffer = unloaded_entry->name.Buffer;
            RtlFillMemory( unloaded_entry, sizeof( unloaded_driver_t ), 0 );
            utils::mem::free( buffer, 'TDmM' );
            // update last loaded driver index
            ( *last_unloaded ) = ( is_filled ? MI_UNLOADED_DRIVERS : *last_unloaded ) - 1;

            // set modified flag
            modified = true;
        }
    }

    if ( modified ) {
        std::int64_t prev_time = 0;
        for ( auto i = MI_UNLOADED_DRIVERS - 2; i >= 0; --i ) {
            auto unloaded_entry = &unloaded_driver_list[ i ];
            if ( ( unloaded_entry->name.Length == 0 ) || ( unloaded_entry->name.Buffer == nullptr ) ) {
                continue;
            }

            if ( ( prev_time != 0 ) && ( unloaded_entry->current_time.QuadPart > prev_time ) ) {
                unloaded_entry->current_time.QuadPart = prev_time - 100;
            }
        }

        clear_mm_drivers( ntoskrnl, name, last_unloaded, unloaded_driver_list );
    }
}

void handler::clear_loaded_drivers()
{
    auto &ntoskrnl = utils::get_kernel_base();
    auto purge_db_cache = [ &ntoskrnl ]() {
        auto cache_table = utils::mem::find_pattern( ntoskrnl, "48 8D 0D ? ? ? ? 45 33 F6 48 89 44 24 50", 'EGAP' )
                               .skip_bytes( 3 )
                               .resolve_rip()
                               .get<RTL_AVL_TABLE *>();

        auto db_lock = utils::mem::find_pattern( ntoskrnl, "48 8D 0D ? ? ? ? E8 ? ? ? ? 4C 8B 8C 24 88 00 00 00", 'EGAP' )
                           .skip_bytes( 3 )
                           .resolve_rip()
                           .get<ERESOURCE *>();

        ExAcquireResourceExclusiveLite( db_lock, TRUE );

        std::array drivers_to_remove { 0x57CD1415ull, 0x5284EAC3ull };

        auto entry_ptr = reinterpret_cast<std::uintptr_t>( cache_table->BalancedRoot.RightChild ) + sizeof( RTL_BALANCED_LINKS );
        auto first_entry = reinterpret_cast<piddb_cache_entry_t *>( entry_ptr );
        for ( auto iter = first_entry; iter && ( reinterpret_cast<void *>( iter ) != first_entry->list.Blink ); iter = reinterpret_cast<piddb_cache_entry_t *>( iter->list.Flink ) ) {
            for ( auto d : drivers_to_remove ) {
                if ( d == iter->time_date_stamp ) {
                    RemoveEntryList( &iter->list );
                    RtlDeleteElementGenericTableAvl( cache_table, iter );
                    logd( "removing driver: %wZ timestamp %x\n", iter->driver_name, iter->time_date_stamp );
                    break;
                }
            }
        }

        ExReleaseResourceLite( db_lock );
    };

    purge_db_cache();

    auto mi_remember_unloaded_driver = utils::mem::find_pattern( ntoskrnl, "8B 05 ? ? ? ? 83 F8 ? 0F 83 ? ? ? ? 48 8D 0C 80", 'EGAP' );
    auto last_unloaded = utils::mem::offset_t { mi_remember_unloaded_driver.get() }
                             .skip_bytes( 2 )
                             .resolve_rip()
                             .get<unsigned long *>();

    // MmUnloadedDrivers = MiAllocatePool(64, 2000, 'TDmM');
    auto unloaded_driver_list = *utils::mem::offset_t { mi_remember_unloaded_driver.get() }
                                     .find_back( std::array { std::uint8_t( 0x48 ), std::uint8_t( 0x8b ), std::uint8_t( 0x15 ) } )
                                     .skip_bytes( 3 )
                                     .resolve_rip()
                                     .get<unloaded_driver_t **>();

    auto ps_loaded_module_resource = utils::mem::offset_t { mi_remember_unloaded_driver.get() }
                                         .find_back( std::array { std::uint8_t( 0x48 ), std::uint8_t( 0x8d ), std::uint8_t( 0x0d ) } )
                                         .skip_bytes( 3 )
                                         .resolve_rip()
                                         .get<ERESOURCE *>();

    UNICODE_STRING capcom = RTL_CONSTANT_STRING( L"capcom.sys" );
    UNICODE_STRING intel = RTL_CONSTANT_STRING( L"iqvw64e.sys" );

    ExAcquireResourceExclusiveLite( ( PERESOURCE ) ps_loaded_module_resource, TRUE );
    clear_mm_drivers( ntoskrnl, capcom, last_unloaded, unloaded_driver_list );
    clear_mm_drivers( ntoskrnl, intel, last_unloaded, unloaded_driver_list );
    ExReleaseResourceLite( ( PERESOURCE ) ps_loaded_module_resource );
    return;
}

void handler::init()
{
    NTSTATUS status = STATUS_SUCCESS;
    SECURITY_DESCRIPTOR sd;

    if ( !NT_SUCCESS( status = RtlCreateSecurityDescriptor( &sd, SECURITY_DESCRIPTOR_REVISION ) ) ) {
        logd( "RtlCreateSecurityDescriptor failed with status code 0x%lx\n", status );
        return;
    }

    if ( !NT_SUCCESS( status = RtlSetDaclSecurityDescriptor( &sd, TRUE, nullptr, FALSE ) ) ) {
        logd( "RtlSetDaclSecurityDescriptor failed with status code 0x%lx\n", status );
        return;
    }

    UNICODE_STRING name = RTL_CONSTANT_STRING( L"\\BaseNamedObjects\\Global\\lotus_memory" );

    OBJECT_ATTRIBUTES ob;
    ob.RootDirectory = nullptr;
    ob.SecurityQualityOfService = nullptr;
    ob.SecurityDescriptor = &sd;
    ob.ObjectName = &name;
    ob.Attributes = OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE;
    ob.Length = sizeof( OBJECT_ATTRIBUTES );

    LARGE_INTEGER section_size;
    section_size.QuadPart = 1000;

    if ( !NT_SUCCESS( status = ZwCreateSection( &g_section_handle, SECTION_MAP_READ | SECTION_MAP_WRITE | SECTION_QUERY, &ob, &section_size, PAGE_READWRITE, SEC_COMMIT, nullptr ) ) ) {
        logd( "ZwCreateSection failed with status code 0x%lx\n", status );
        return;
    }

    HANDLE thread_handle;
    if ( !NT_SUCCESS( status = PsCreateSystemThread( &thread_handle, GENERIC_READ | GENERIC_WRITE, nullptr, nullptr, nullptr, ( PKSTART_ROUTINE ) main_loop, nullptr ) ) ) {
        logd( "error creating system thread; status 0x%lx\n", status );
        return;
    }

    //

    ZwClose( thread_handle );
}

void handler::main_loop( void * )
{
    utils::unlink_thread( PsGetCurrentProcess(), PsGetCurrentThreadId() );

    void *shared_memory_base = nullptr;
    size_t view_size = 1000;
    if ( const auto status = ZwMapViewOfSection( g_section_handle, ZwCurrentProcess(), &shared_memory_base, 0, view_size, nullptr, &view_size, SECTION_INHERIT::ViewUnmap, 0, PAGE_READWRITE );
         !NT_SUCCESS( status ) ||
         !shared_memory_base ) {
        logd( "ZwMapViewOfSection failed with status code 0x%lx\n", status );
        return;
    }

    const auto ticks_to_ms = []( std::int64_t tick ) {
        return ( tick * KeQueryTimeIncrement() ) / 10'000;
    };
    const auto current_tick = []() {
        LARGE_INTEGER r;
        KeQueryTickCount( &r );

        return r.QuadPart;
    };

    // last time before any command was executed
    ULONG64 old_tick = current_tick();

    int loop_count = 0;

    // is using a mutex/event here better/faster/slower?
    while ( true ) {
        auto request = static_cast<kernel_base_request *>( shared_memory_base );
        if ( request->code == request_code::none ) {

            // execute only every 5000 iterations
            if ( loop_count++ >= 5'000 ) {
                auto new_tick = current_tick();

                // if there wasn't a new command in 60 seconds, sleep for 500ms until a new
                // command is sent
                if ( ( ticks_to_ms( new_tick ) - ticks_to_ms( old_tick ) ) > ( 5 * 1'000 ) ) {
                    LARGE_INTEGER to;
                    to.QuadPart = -5'000;

                    KeDelayExecutionThread( KernelMode, FALSE, &to );
                }

                loop_count = 0;
            }

            continue;
        }

        // new request received, set "working" state
        request->status = request_status::working;

        loop_count = 0;
        old_tick = current_tick();

        switch ( request->code ) {
            case request_code::copy_mem: {
                copy_memory( reinterpret_cast<copy_memory_request *>( request ) );
                break;
            }

            case request_code::get_process_base: {
                get_process_base( reinterpret_cast<process_base_request *>( request ) );
                break;
            }

            case request_code::allocate_mem: {
                allocate_memory( reinterpret_cast<handler::allocate_memory_request *>( request ) );
                break;
            }

            case request_code::free_mem: {
                free_memory( reinterpret_cast<handler::free_memory_request *>( request ) );
                break;
            }

            case request_code::protect_mem: {
                protect_memory( reinterpret_cast<handler::protect_memory_request *>( request ) );
                break;
            }

            case request_code::query_mem: {
                query_memory( reinterpret_cast<handler::query_memory_request *>( request ) );
                break;
            }

            case request_code::remove_nx_prot: {
                remove_nx_protect( reinterpret_cast<handler::remove_nx_protect_request *>( request ) );
                break;
            }

            case request_code::allocate_mem_kernel: {
                allocate_memory_kernel( reinterpret_cast<handler::allocate_memory_kernel_request *>( request ) );
                break;
            }            
        }

        // request finished, cleanup status
        request->status = request_status::done;
        request->code = request_code::none;
    }
}

void handler::copy_memory( copy_memory_request *request )
{
    /*if ( ( request->source_address > 0x7FFFFFFFFFFFull ) ||
        ( request->target_address > 0x7FFFFFFFFFFFull ) ) {
        request->result = STATUS_INVALID_ADDRESS;
        return;
    }*/

    native::system_process source { request->source_pid };
    if ( !source.is_valid() ) {
        request->result = source.status();
        return;
    }

    native::system_process target { request->target_pid };
    if ( !target.is_valid() ) {
        request->result = target.status();
        return;
    }

    SIZE_T ret_size = 0;
    request->result = MmCopyVirtualMemory(
        source.get(), reinterpret_cast<void *>( request->source_address ), target.get(), reinterpret_cast<void *>( request->target_address ), request->size, KernelMode, &ret_size );
}

void handler::get_process_base( process_base_request *request )
{
    native::system_process p { request->pid };
    if ( !p.is_valid() ) {
        request->result = p.status();
        return;
    }

    request->base = reinterpret_cast<unsigned long long>( PsGetProcessSectionBaseAddress( p.get() ) );
    request->peb = reinterpret_cast<unsigned long long>( PsGetProcessPeb( p.get() ) );
    request->result = STATUS_SUCCESS;
}

void handler::allocate_memory( allocate_memory_request *request )
{
    PEPROCESS process;

    if ( auto ans = PsLookupProcessByProcessId( ( HANDLE ) request->pid, &process ); !NT_SUCCESS( ans ) ) {
        request->result = ans;
        return;
    }

    PVOID base_addr = &request->address;
    SIZE_T sz = request->size;
    auto protection = request->protection;

    KAPC_STATE apc;
    KeStackAttachProcess( process, &apc );

    NTSTATUS status = ZwAllocateVirtualMemory( ZwCurrentProcess(), &base_addr, 0ull, &sz, MEM_COMMIT | MEM_RESERVE, protection );

    KeUnstackDetachProcess( &apc );
    ObDereferenceObject( process );

    request->address = reinterpret_cast<unsigned long long>( base_addr );
    request->size = sz;
    request->result = status;
}

void handler::free_memory( free_memory_request *args_ptr )
{
    free_memory_request args = *args_ptr;
    native::system_process process { args.pid };
    if ( !process.is_valid() ) {
        args_ptr->result = process.status();
        return;
    }

    KAPC_STATE apc;
    KeStackAttachProcess( process.get(), &apc );

    args.result = ZwFreeVirtualMemory( ZwCurrentProcess(), ( void ** ) &args.address, &args.size, args.free_type );

    KeUnstackDetachProcess( &apc );

    args_ptr->result = args.result;
}

void handler::protect_memory( protect_memory_request *args_ptr )
{
    protect_memory_request args = *args_ptr;

    native::system_process process { args.pid };
    if ( !process.is_valid() ) {
        args_ptr->result = process.status();
        return;
    }

    ULONG old_protection = 0;

    KAPC_STATE apc;
    KeStackAttachProcess( process.get(), &apc );

    args.result = ZwProtectVirtualMemory( ZwCurrentProcess(), ( void ** ) &args.address, &args.size, args.protection, &old_protection );

    KeUnstackDetachProcess( &apc );

    args_ptr->address = args.address;
    args_ptr->protection = old_protection;
    args_ptr->result = args.result;
}

void handler::query_memory( query_memory_request *args_ptr )
{
    query_memory_request args = *args_ptr;
    native::system_process process { args.pid };
    if ( !process.is_valid() ) {
        args_ptr->result = process.status();
        return;
    }

    SIZE_T ans = 0ul;

    KAPC_STATE apc;
    KeStackAttachProcess( process.get(), &apc );

    args.result = ZwQueryVirtualMemory( ZwCurrentProcess(), reinterpret_cast<void *>( args.address ), static_cast<MEMORY_INFORMATION_CLASS>( MemoryBasicInformation ),
        &args.information, sizeof( MEMORY_BASIC_INFORMATION ), &ans );

    KeUnstackDetachProcess( &apc );

    args_ptr->result = args.result;
}

void handler::remove_nx_protect( remove_nx_protect_request *request )
{
    native::system_process process { request->pid };
    if ( !process.is_valid() ) {
        request->result = process.status();
        return;
    }

    for ( std::uintptr_t page = request->address; page < ( request->address + request->size ); page += PAGE_SIZE ) {
        if ( auto pte = imports::MiGetPteAddress( reinterpret_cast<void *>( page ) ); pte != nullptr ) {
            if ( pte->Hardware.Valid ) {
                pte->Hardware.NoExecute = 0;
            }
        } else {
            request->result = STATUS_ACCESS_VIOLATION;
        }
    }

    request->result = STATUS_SUCCESS;
}

void handler::allocate_memory_kernel( allocate_memory_kernel_request *request )
{
    auto ptr = utils::mem::alloc( request->size );

    request->address = reinterpret_cast<std::uintptr_t>( ptr );
    if ( !ptr ) {
        request->result = STATUS_INSUFFICIENT_RESOURCES;
    }
}