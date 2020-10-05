#pragma once

#define DRIVERNAME "lotus-drv"
#define logd( ... ) DbgPrintEx( 0, 0, "[" DRIVERNAME "] " __FUNCTION__ ": " __VA_ARGS__ )