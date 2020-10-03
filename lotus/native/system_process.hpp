#pragma once

#include <cstdint>
#include <ntifs.h>

namespace native
{
    class system_process
    {
    public:
        system_process( std::uint32_t process_id );
        ~system_process();

        system_process() = delete;
        system_process( const system_process & ) = delete;
        system_process( system_process && ) = delete;

        PEPROCESS get();

        bool is_valid();
        long status();

    private:
        PEPROCESS m_obj;
        long m_status;
    };
}