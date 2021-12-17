
#include <analyzer/plugins/plugin.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

namespace analyzer
{

namespace P10
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
void pll_unlock(unsigned int i_instance, const libhei::Chip&, ServiceData&)
{
    // TODO
    trace::inf("pll_unlock plugin: i_instance=%u", i_instance);
}

/**
 * @brief Queries for an LPC timeout. If present, will callout all appropriate
 *        hardware.
 */
void lpc_timeout(unsigned int, const libhei::Chip& i_chip,
                 ServiceData& io_servData)
{
    auto target = util::pdbg::getTrgt(i_chip);
    auto path   = util::pdbg::getPath(target);

    if (util::pdbg::queryLpcTimeout(target))
    {
        trace::inf("LPC timeout detected on %s", path);

        // Callout the PNOR.
        io_servData.calloutPart(callout::PartType::PNOR,
                                callout::Priority::MED);

        // Callout the associated clock, no guard.
        auto chipPos = util::pdbg::getChipPos(target);
        if (0 == chipPos)
        {
            // Clock 0 is hardwired to proc 0.
            io_servData.calloutClock(callout::ClockType::OSC_REF_CLOCK_0,
                                     callout::Priority::MED, false);
        }
        else if (1 == chipPos)
        {
            // Clock 1 is hardwired to proc 1.
            io_servData.calloutClock(callout::ClockType::OSC_REF_CLOCK_1,
                                     callout::Priority::MED, false);
        }
        else
        {
            trace::err("LPC timeout on unexpected processor: %s", path);
        }

        // Callout the processor, no guard.
        io_servData.calloutTarget(target, callout::Priority::MED, false);
    }
    else
    {
        trace::inf("No LPC timeout detected on %s", path);

        io_servData.calloutProcedure(callout::Procedure::NEXTLVL,
                                     callout::Priority::HIGH);
    }
}

} // namespace P10

PLUGIN_DEFINE_NS(P10_10, P10, pll_unlock);
PLUGIN_DEFINE_NS(P10_20, P10, pll_unlock);

PLUGIN_DEFINE_NS(P10_10, P10, lpc_timeout);
PLUGIN_DEFINE_NS(P10_20, P10, lpc_timeout);

} // namespace analyzer
