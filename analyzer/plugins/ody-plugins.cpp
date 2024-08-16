
#include <analyzer/plugins/plugin.hpp>
#include <hei_util.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

namespace analyzer
{

namespace Ody
{

/**
 * @brief Adds all chips in the OCMB PLL domain with active PLL unlock
 *        attentions to the callout list.
 *
 * An OCMB PLL domain is scoped to just the OCMBs under the same processor chip.
 * If more than one OCMB within the PLL domain is reporting a PLL unlock
 * attention, the clock source (the processor) is called out with high priority
 * and all connected OCMBs are called out with low priority. Otherwise, single
 * OCMB is called out high and the connected processor low.
 */
void pll_unlock(unsigned int, const libhei::Chip& i_ocmbChip,
                ServiceData& io_servData)
{
    using namespace util::pdbg;

    auto nodeId = libhei::hash<libhei::NodeId_t>("PLL_UNLOCK");

    auto sigList = io_servData.getIsolationData().getSignatureList();

    // The PLL list is initially the same size of the signature list.
    std::vector<libhei::Signature> pllList{sigList.size()};

    // Copy all signatures PLL signatures that match the node ID and parent
    // processor chip.
    auto procTrgt = getParentProcessor(getTrgt(i_ocmbChip));
    auto itr = std::copy_if(
        sigList.begin(), sigList.end(), pllList.begin(),
        [&nodeId, &procTrgt](const auto& s) {
            return (nodeId == s.getId() &&
                    procTrgt == getParentProcessor(getTrgt(s.getChip())));
        });

    // Shrink the size of the PLL list if necessary.
    pllList.resize(std::distance(pllList.begin(), itr));

    // There should be at list one signature in the list.
    if (0 == pllList.size())
    {
        throw std::logic_error("Expected at least one PLL unlock signature. "
                               "i_ocmbChip=" +
                               std::string{getPath(i_ocmbChip)});
    }

    // The hardware callouts will be all OCMBs with PLL unlock attentions and
    // the connected processor chip. The callout priorities are dependent on the
    // number of chips at attention.
    if (1 == pllList.size())
    {
        // There is only one OCMB chip with a PLL unlock. So, the error is
        // likely in the OCMB.
        io_servData.calloutTarget(getTrgt(pllList.front().getChip()),
                                  callout::Priority::HIGH, true);
        io_servData.calloutTarget(procTrgt, callout::Priority::LOW, false);
    }
    else
    {
        // There are more than one OCMB chip with a PLL unlock. So, the error is
        // likely the clock source, which is the processor.
        io_servData.calloutTarget(procTrgt, callout::Priority::HIGH, true);
        for (const auto& sig : pllList)
        {
            io_servData.calloutTarget(getTrgt(sig.getChip()),
                                      callout::Priority::LOW, false);
        }
    }
}

} // namespace Ody

PLUGIN_DEFINE_NS(ODYSSEY_10, Ody, pll_unlock);

} // namespace analyzer
