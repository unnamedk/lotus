#pragma once

#include <cstdint>
#include <cstring>

// https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.2.0.pdf
#pragma pack( push, 1 )
struct smbios_base_header
{
    std::uint8_t type;
    std::uint8_t length;
    std::uint16_t handle;

    char *parse_string( std::uint8_t index )
    {
        int i = 0;
        char *c = nullptr;
        for ( i = 1, c = ( char * ) this + this->length; '\0' != *c; c += strlen( c ) + 1, ++i ) {
            if ( i == index ) {
                return c;
            }
        }
        return c;
    }
};
struct smbios_entry_point_area
{
    std::uint8_t anchor_string[ 4 ]; // string that says "_SM_" (5F 53 4D 5F)
    std::uint8_t entry_point_structure_checksum; // checksum of the EPS
    std::uint8_t entry_point_length; // length of the EPS
    std::uint8_t smbios_major_ver; // major version of the implemented specification
    std::uint8_t smbios_minor_ver; // minor version of the implemented specification
    std::uint16_t max_struct_size; // size of the largest smbios structure
    std::uint8_t entry_point_revision; // EPS revision
    std::uint8_t formatted_area[ 5 ]; // implementation defined
    std::uint8_t intermediate_anchor_string[ 5 ]; // string that says "_DMI_" (5F 44 4D 49 5F)
    std::uint8_t intermediate_checksum; // checksum of IEPS
    std::uint16_t structure_table_length; // total length of the smbios structure
    std::uint32_t structure_table_address; // 32-bit physical address of the SMBIOS structure table
    std::uint16_t num_of_smbios_structures; // smbios struct count
    std::uint8_t bcd_revision; // level of compliance with the standard
};
struct smbios_uuid
{
    std::uint32_t time_low;
    std::uint16_t time_mid;
    std::uint16_t time_hi_and_version;
    std::uint8_t clock_seq_hi_and_reserved;
    std::uint8_t clock_seq_low;
    std::uint8_t node[ 6 ];
};
struct bios_info : public smbios_base_header
{
    std::uint8_t vendor; // type: string
    std::uint8_t version; // type: string
    std::uint16_t start_address; // type: word
    std::uint8_t release_date; // type: string
    std::uint8_t rom_size; // type: varies
    std::uint64_t characteristics; // type: bitfield
    // rest is optional
};
struct system_info : public smbios_base_header
{
    std::uint8_t manufacturer; // type: string
    std::uint8_t product_name; // type: string
    std::uint8_t version; // type: string
    std::uint8_t serial_number; // type: string
    smbios_uuid uuid; // type: uuid
    std::uint8_t wakeup_type; // type: enum
    std::uint8_t sku_number; // type: string
    std::uint8_t family; // type: string
};
struct baseboard_info : public smbios_base_header
{
    std::uint8_t manufacturer; // type: string
    std::uint8_t product_name; // type: string
    std::uint8_t version; // type: string
    std::uint8_t serial_number; // type: string
    std::uint8_t asset_tag; // type: string
    std::uint8_t feature_flags; // type: bitfield
};
struct system_enclosure : public smbios_base_header
{
    std::uint8_t manufacturer; // type: string
    std::uint8_t type; // type: string
    std::uint8_t version; // type: string
    std::uint8_t serial_number; // type: string
    std::uint8_t asset_tag_number; // type: string
    std::uint8_t bootup_state; // type: enum
    std::uint8_t power_supply_state; // type: enum
    std::uint8_t thermal_state; // type: enum
    std::uint8_t security_status; // type: enum
    std::uint32_t oem_reserved; // type: varies
    std::uint8_t height; // type: byte
    std::uint8_t num_of_power_cords; // type: byte
};
struct processor_info : public smbios_base_header
{
    std::uint8_t socket; // type: string
    std::uint8_t processor_type; // type: enum
    std::uint8_t family; // type: enum
    std::uint8_t manufacturer; // type: string
    std::uint64_t id; // type: varies
    std::uint8_t version; // type: string
    std::uint8_t voltage; // type: varies
    std::uint16_t external_clock; // type: varies
    std::uint16_t max_speed; // type: varies
    std::uint16_t current_speed; // type: varies
    std::uint8_t status; // type: varies
    std::uint8_t upgrade; // type: enum
    std::uint16_t l1_cache_handle; // type: varies
    std::uint16_t l2_cache_handle; // type: varies
    std::uint16_t l3_cache_handle; // type: varies
    std::uint8_t serial_number; // type: string
    std::uint8_t asset_tag; // type: string
    std::uint16_t part_number; // type: string
};
struct mem_device_info : public smbios_base_header
{
    std::uint16_t physical_mem_array_handle;
    std::uint16_t mem_error_info_handle;
    std::uint16_t total_width;
    std::uint16_t data_width;
    std::uint16_t size;
    std::uint8_t form_factor;
    std::uint8_t device_set;
    std::uint8_t device_locator;
    std::uint8_t bank_locator;
    std::uint8_t memory_type;
    std::uint16_t type_detail;
    std::uint16_t speed;
    std::uint8_t manufacturer;
    std::uint8_t serial_number;
    std::uint8_t asset_tag;
    std::uint8_t part_number;
};
#pragma pack( pop )