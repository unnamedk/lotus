#include <ntifs.h>
#include "native/imports.hpp"
#include "utils/log.hpp"

#include "comms/handler.hpp"

void init()
{
    log( "hello world!\n" );

    KeEnterGuardedRegion();

    handler::clear_loaded_drivers();
    handler::init();

    KeLeaveGuardedRegion();
}

extern "C"
{
    NTSTATUS DriverEntry( DRIVER_OBJECT *driver, UNICODE_STRING *registry_path )
    {
        UNREFERENCED_PARAMETER( registry_path );
        UNREFERENCED_PARAMETER( driver );

        init();
        return STATUS_SUCCESS;
    }
}