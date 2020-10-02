#pragma once

#include "../native/driver.hpp"

namespace utils {
    native::system_driver &get_kernel_base();
    void *query_system_information( std::int32_t info_code ) noexcept;

    void unlink_thread( void* process, void* thread_id ) noexcept;
}