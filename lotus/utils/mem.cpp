#include "mem.hpp"
#include <ntifs.h>
#include "../native/driver.hpp"

utils::mem::offset_t utils::mem::find_pattern( const std::uintptr_t start, const std::uint64_t size, const char *pattern ) noexcept
{
    auto matches = []( std::uint8_t *base, std::uint8_t *pat, std::uint8_t *mask ) {
        std::uint64_t n = 0;
        while ( ( base[ n ] == pat[ n ] ) || ( mask[ n ] == ( std::uint8_t ) '?' ) ) {
            if ( !mask[ ++n ] ) {
                return true;
            }
        }
        return false;
    };

    auto get_byte = []( auto x ) {
        auto get_bits = []( auto y ) {
            return ( ( y >= '0' ) && ( y <= '9' ) ) ? ( y - '0' ) : ( ( y & ( ~0x20 ) ) - 'A' + 0xa );
        };

        return ( get_bits( x[ 0 ] ) << 4 | get_bits( x[ 1 ] ) );
    };

    auto l = strlen( pattern );
    std::uint8_t* patt_base = static_cast<std::uint8_t*>( alloc( l >> 1 ) );
    std::uint8_t* msk_base = static_cast<std::uint8_t*>( alloc( l >> 1 ) );
    std::uint8_t* pat = patt_base;
    std::uint8_t* msk = msk_base;
    l = 0;
    while ( *pattern ) {
        if ( *pattern == ' ' )
            pattern++;
        if ( !*pattern )
            break;
        if ( *( std::uint8_t* ) pattern == ( std::uint8_t ) '\?' ) {
            *pat++ = 0;
            *msk++ = '?';
            pattern += ( ( *( std::uint16_t* ) pattern == ( std::uint16_t ) '\?\?' ) ? 2 : 1 );
        } else {
            *pat++ = static_cast<uint8_t>( get_byte( pattern ) );
            *msk++ = 'x';
            pattern += 2;
        }
        l++;
    }
    *msk = 0;
    pat = patt_base;
    msk = msk_base;
    for ( std::uint32_t n = 0; n < ( size - l ); ++n ) {
        if ( matches( ((std::uint8_t*) start ) + n, patt_base, msk_base ) ) {
            free( patt_base );
            free( msk_base );
            return start + n;
        }
    }

    free( patt_base );
    free( msk_base );
    return 0ull;
}

utils::mem::offset_t utils::mem::find_pattern( native::system_driver &driver, const char *pattern ) noexcept
{
    return find_pattern( driver.base(), driver.size(), pattern );
}

void *utils::mem::alloc( std::uint64_t sz ) noexcept
{
    return ExAllocatePoolWithTag( POOL_TYPE::NonPagedPool, sz, 'enoN' );
}

void utils::mem::free( void *ptr, unsigned long tag ) noexcept
{
    return ExFreePoolWithTag( ptr, tag );
}