#pragma once

namespace util
{
namespace pldm
{
/*
 *  @brief HRESET the SBE
 *
 *  @pre Host must be running
 *
 *  @param[in] sbeInstance - SBE to target (0 based)
 *
 *  @return true if HRESET successful
 *
 */
bool hresetSbe(unsigned int sbeInstance);

} // namespace pldm
} // namespace util
