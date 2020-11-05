#pragma once

#include <hei_main.hpp>

namespace util
{

namespace pdbg
{

/**
 * @param  A chip.
 * @return A string representing the chip's devtree path.
 */
const char* getPath(const libhei::Chip& i_chip);

/**
 * @param  A chip.
 * @return The absolute position of the given chip.
 */
uint32_t getChipPos(const libhei::Chip& i_chip);

/**
 * @brief Returns the list of all active chips in the system.
 * @param o_chips The returned list of chips.
 */
void getActiveChips(std::vector<libhei::Chip>& o_chips);

} // namespace pdbg

} // namespace util
