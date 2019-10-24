#pragma once

#include <libpdbg.h>

/**
 * @brief The special attention handler logic
 *
 * @param i_target FSI target
 *
 * @return non-zero = error
 */
bool handleSpattn(pdbg_target *i_target, bool& i_remove);
