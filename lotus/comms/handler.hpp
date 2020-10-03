#pragma once

#include <ntifs.h>

namespace handler
{
    extern HANDLE g_section_handle;

    enum class request_code
    {
        none = 0x0,
        copy_mem = 0x701,
        spoof = 0x702,
        get_process_base = 0x703,
        allocate_mem = 0x704,
        protect_mem = 0x705,
        free_mem = 0x706,
        query_mem = 0x707,
        remove_nx_prot = 0x708,
    };

    enum class request_status
    {
        done,
        working
    };

    struct kernel_base_request
    {
        request_code code;
        request_status status;
        long result;
    };

    struct copy_memory_request : public kernel_base_request
    {
        // source
        unsigned long source_pid;
        unsigned long long source_address;

        // target
        unsigned long target_pid;
        unsigned long long target_address;

        // size of buffer
        unsigned long long size;
    };

    struct process_base_request : public kernel_base_request
    {
        unsigned long pid;
        unsigned long long base;
        unsigned long long peb;
    };

    struct allocate_memory_request : public kernel_base_request
    {
        unsigned long pid;
        unsigned long allocation_type;
        unsigned long protection;
        unsigned long long address;
        unsigned long long size;
    };

    struct free_memory_request : public kernel_base_request
    {
        unsigned long pid;
        unsigned long free_type;
        unsigned long long address;
        unsigned long long size;
    };

    struct protect_memory_request : public kernel_base_request
    {
        unsigned long pid;
        unsigned long protection;
        unsigned long long address;
        unsigned long long size;
    };

    struct query_memory_request : public kernel_base_request
    {
        unsigned long pid;

        unsigned long long address;
        MEMORY_BASIC_INFORMATION information;
    };

    struct spoof_request : public kernel_base_request
    {
        unsigned int seed;
    };

    struct remove_nx_protect_request : kernel_base_request
    {
        unsigned long pid;
        uintptr_t address;
        size_t size;
    };

    void clear_loaded_drivers();
    void init();

    void NTAPI main_loop( void *ctx );

    // request reader
    void copy_memory( copy_memory_request *request );
    void get_process_base( process_base_request *request );
    void allocate_memory( allocate_memory_request *request );
    void free_memory( free_memory_request *request );
    void protect_memory( protect_memory_request *request );
    void query_memory( query_memory_request *request );
    void remove_nx_protect( remove_nx_protect_request *request );
}