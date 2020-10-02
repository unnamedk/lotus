#include "imports.hpp"
#include "../utils/utils.hpp"
#include "../utils/mem.hpp"

imports::MiGetPteAddress_t imports::MiGetPteAddress;

void imports::init() noexcept
{
    auto &kernel = utils::get_kernel_base();

    imports::MiGetPteAddress = utils::mem::find_pattern( kernel.base(), kernel.size(), "41 B8 ? ? ? ? 48 8B D6 49 8B CE E8 ? ? ? ? 48 8B D8" )
        .skip_bytes(12)
        .resolve_rip(5ll)
        .get<imports::MiGetPteAddress_t>();

}