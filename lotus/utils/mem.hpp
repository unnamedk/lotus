#pragma once

#include <cstdint>
#include <array>
#include <algorithm>
#include <type_traits>

namespace native
{
    class system_driver;
}

namespace utils::mem
{
    class offset_t
    {
    public:
        constexpr offset_t( std::uintptr_t address )
            : m_address( address )
        { }

        constexpr operator std::uintptr_t() { return this->m_address; }

        constexpr offset_t &skip_bytes( std::int32_t n )
        {
            this->m_address += n;
            return *this;
        }

        inline std::int32_t rip() const
        {
            return *reinterpret_cast<std::int32_t *>( m_address );
        }

        offset_t &resolve_rip( int64_t rip_offs = 4ll )
        {
            this->m_address += ( static_cast<int64_t>( this->rip() ) + rip_offs ); 
            return *this;
        }

        template <std::uint64_t s>
        offset_t &find_back( std::array<std::uint8_t, s> bytes )
        {
            for ( auto address = m_address;; --address ) {
                if ( std::equal( reinterpret_cast<std::uint8_t *>( address ), reinterpret_cast<std::uint8_t *>( address + bytes.size() ), bytes.data() ) ) {
                    m_address = address;
                    break;
                }
            }

            return *this;
        }

        template <typename T = std::uintptr_t>
        constexpr T get()
        {
            if constexpr (std::is_same_v<T, std::uintptr_t>) {
                return this->m_address;
            } else {
                return reinterpret_cast<T>( this->m_address );
            }
        }

    private:
        std::uintptr_t m_address;
    };

    utils::mem::offset_t find_pattern( std::uintptr_t base, std::uint64_t size, const char *pattern ) noexcept;
    utils::mem::offset_t find_pattern( native::system_driver& driver, const char *pattern ) noexcept;
    void *alloc( std::uint64_t sz ) noexcept;
    void free( void *ptr, unsigned long tag = 'enoN' ) noexcept;
}