#pragma once

#include <cstdint>

namespace native
{
    // a named driver is a driver object received with the
    // ObReferenceObjectByName system call
    class named_driver
    {
    public:
        named_driver( const wchar_t *name );
        named_driver( void *object_ptr );
        ~named_driver();

        // no copy constructors allowed
        named_driver( const named_driver & ) = delete;
        named_driver &operator=( const named_driver & ) = delete;

        // whether this function holds an object
        bool is_valid() const;

        // get the internal object
        void *get();
        void *original_dispatch();

        std::uintptr_t to_virtual_address( std::uintptr_t rva );
        bool hook_device_control( std::uintptr_t new_function_address );

    private:
        void *m_original_dispatch;

        long m_result;
        void *m_object_ptr;
    };

    // just like a named driver, but it looks for devices
    class named_device
    {
    public:
        named_device( const wchar_t *name );
        ~named_device();

        bool is_valid();

        void *file();
        void *device();

    private:
        long m_result;
        void *m_file_object;
        void *m_device_object;
    };

    // a system driver is a driver that's in the loaded module
    // list (query is with ZwQuerySystemInformation)
    class system_driver
    {
    public:
        system_driver( const char *name );

        std::uintptr_t base() const;
        std::uintptr_t size() const;

    private:
        std::uintptr_t m_base;
        std::uint32_t m_size;
        std::uint8_t m_name[ 256 ];
    };
}