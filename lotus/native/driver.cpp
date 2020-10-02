#include "driver.hpp"

#include "../utils/mem.hpp"
#include "../utils/utils.hpp"
#include "../utils/log.hpp"
#include "imports.hpp"

native::named_driver::named_driver( const wchar_t *name )
    : m_object_ptr( nullptr )
    , m_result( STATUS_SUCCESS )
    , m_original_dispatch( nullptr )
{
    UNICODE_STRING str;
    RtlInitUnicodeString( &str, name );

    this->m_result = ObReferenceObjectByName( &str, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, nullptr, 0, *IoDriverObjectType, KernelMode, nullptr, reinterpret_cast<void **>( &m_object_ptr ) );
}

native::named_driver::named_driver( void *object_ptr )
    : m_object_ptr( object_ptr )
    , m_original_dispatch( nullptr )
    , m_result( STATUS_SUCCESS )
{ }

native::named_driver::~named_driver()
{
    if ( this->is_valid() ) {
        ObDereferenceObject( this->m_object_ptr );
    }
}

bool native::named_driver::is_valid() const
{
    return NT_SUCCESS( this->m_result );
}

void *native::named_driver::get()
{
    return this->m_object_ptr;
}

void *native::named_driver::original_dispatch()
{
    return this->m_original_dispatch;
}

std::uintptr_t native::named_driver::to_virtual_address( std::uintptr_t rva )
{
    return reinterpret_cast<std::uintptr_t>( this->get() ) + rva;
}

// thanks GINA for this method
bool native::named_driver::hook_device_control( std::uintptr_t new_function_address )
{
    if ( !this->is_valid() ) {
        return false;
    }

    // get PE's start address
    auto driver = static_cast<DRIVER_OBJECT *>( this->m_object_ptr );

    // parse sections
    const auto nt = RtlImageNtHeader( driver->DriverStart );

    PIMAGE_SECTION_HEADER discardable_section_header = nullptr;
    auto first_section = IMAGE_FIRST_SECTION( nt );
    for ( auto section = first_section;
          section < ( first_section + nt->FileHeader.NumberOfSections );
          ++section ) {
        if ( ( section->Characteristics & 0x02000000 ) &&
            ( section->Misc.VirtualSize > 0 ) &&
            ( section->Misc.VirtualSize <= PAGE_SIZE ) ) {
            discardable_section_header = section;
            break;
        }
    }

    if ( discardable_section_header == nullptr ) {
        return false;
    }

    auto discardable_section = reinterpret_cast<void *>( reinterpret_cast<std::uintptr_t>( driver->DriverStart ) + ( discardable_section_header->VirtualAddress ) );
    auto discardable_section_pte = imports::MiGetPteAddress( discardable_section );

    const auto discardable_allocated_buffer = utils::mem::alloc( ROUND_TO_PAGES( discardable_section_header->Misc.VirtualSize ) );
    auto discardable_allocated_buffer_pte = imports::MiGetPteAddress( discardable_allocated_buffer );

    if ( ( discardable_allocated_buffer_pte == nullptr ) || ( discardable_section_pte == nullptr ) ) {
        return false;
    }

    *discardable_section_pte = *discardable_allocated_buffer_pte;

    // assembly bytecode that's going to be executed
    const unsigned char jmp_buffer[] = {
        0x48, 0xB8, // mov rax,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 64-bit address
        0xFF, 0xE0 //jmp rax
    };
    memcpy( discardable_section, jmp_buffer, sizeof( jmp_buffer ) );

    // write to the 2nd byte onward with the function address
    *reinterpret_cast<uintptr_t *>( &reinterpret_cast<std::uint8_t *>( discardable_section )[ 2 ] ) = new_function_address;

    // TODO: Maybe return the dispatch (if hook was successful?)
    // backup the original dispatch
    m_original_dispatch = driver->MajorFunction[ IRP_MJ_DEVICE_CONTROL ];
    driver->MajorFunction[ IRP_MJ_DEVICE_CONTROL ] = reinterpret_cast<DRIVER_DISPATCH *>( discardable_section );

    return true;
}

native::system_driver::system_driver( const char *name )
    : m_base( 0ull )
    , m_size( 0 )
    , m_name{ 0 } 
{
    const auto list = static_cast<RTL_PROCESS_MODULES *>( utils::query_system_information( static_cast<int>( SYSTEM_INFORMATION_CLASS::SystemModuleInformation ) ) );
    if ( list ) {
        for ( auto i = 0ul; i < list->NumberOfModules; ++i ) {
            auto mod = list->Modules[ i ];
            if ( !mod.ImageBase ) {
                continue;
            }

            if ( strstr( reinterpret_cast<char *>( mod.FullPathName ), name ) ) {
                RtlCopyMemory( this->m_name, mod.FullPathName, 256 );
                this->m_base = reinterpret_cast<std::uintptr_t>( mod.ImageBase );
                this->m_size = mod.ImageSize;
                break;
            }
        }

        utils::mem::free( list );
    }
}

std::uintptr_t native::system_driver::base() const
{
    return this->m_base;
}

std::uintptr_t native::system_driver::size() const
{
    return this->m_size;
}

native::named_device::named_device( const wchar_t *name )
    : m_file_object( nullptr )
    , m_device_object( nullptr )
    , m_result( STATUS_SUCCESS )
{
    UNICODE_STRING str;
    RtlInitUnicodeString( &str, name );

    this->m_result = IoGetDeviceObjectPointer( &str, FILE_READ_DATA | FILE_WRITE_DATA, reinterpret_cast<PFILE_OBJECT *>( &m_file_object ), reinterpret_cast<PDEVICE_OBJECT *>( &m_device_object ) );
}

native::named_device::~named_device()
{
    if ( is_valid() ) {
        ObDereferenceObject( this->m_file_object );
        ObDereferenceObject( this->m_device_object );
    }
}

bool native::named_device::is_valid()
{
    return NT_SUCCESS( this->m_result );
}

void *native::named_device::file()
{
    return this->m_file_object;
}

void *native::named_device::device()
{
    return this->m_device_object;
}
