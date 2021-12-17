
#include <plugin.hpp>

namespace analyzer
{

/**
 * @brief Adds all clocks/chips reporting PLL unlock attentions to the callout
 *        list.
 *
 *  Processors are always called out at medium priority and never guarded. If
 *  more than one processor is reporting a PLL unlock attention on the same
 *  clock, the clock is called out with high priority. Otherwise, the clock
 *  callout priority is medium.
 */
void pll_unlock(unsigned int i_instance, const libhei::Chip& i_chip,
                ServiceData& io_sd)
{
    // TODO
}

PLUGIN_DEFINE_NS(P10_10, pll_unlock);
PLUGIN_DEFINE_NS(P10_20, pll_unlock);

} // namespace analyzer
