
#include <analyzer/plugins/plugin.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

namespace analyzer
{

namespace P10
{

/** Each chip is connected to two TOD topologies: active and backup. The values
 *  are important because some registers and documentation simply refer to them
 *  by number instead of name. Also, they can be used as array indexes if
 *  needed. */
enum class Topography
{
    ACTIVE = 0,
    BACKUP = 1,
};

class TodData
{
  public:
    TodData()               = default;
    ~TodData()              = default;
    TodData(const TodData&) = default;
    TodData(TodData&&)      = default;
    TodData& operator=(const TodData&) = default;
    TodData& operator=(TodData&&) = default;

  private:
    /** The MDMT chips at fault (only one per topography). */
    std::map<Topography, pdbg_target*> iv_mdmtFaultList;

    /** All chips with internal path faults. */
    std::map<Topography, std::vector<pdbg_target*>> iv_internalFaultList;

    /** The chips sourcing the clocks to non-MDMT chips with faults. */
    std::map<Topography, std::vector<pdbg_target*>> iv_networkFaultList;

  public:
    /**
     * @brief Sets this chip as the MDMT at fault for this topology.
     * @param i_topography  Target topography.
     * @param i_chipAtFault The chip reporting step check fault.
     */
    void setMdmtFault(Topography i_topography, pdbg_target* i_chipAtFault)
    {
        assert(nullptr != i_chipAtFault);
        iv_mdmtFaultList[i_topography] = i_chipAtFault;
    }

    /**
     * @param  i_topography Target topography.
     * @return The MDMT chip for this topology, if at fault. Otherwise, nullptr.
     */
    pdbg_target* getMdmtFault(Topography i_topography)
    {
        return iv_mdmtFaultList[i_topography];
    }

    /**
     * @brief Indicates the given chip has an internal fault.
     * @param i_topography  Target topography.
     * @param i_chipAtFault The chip reporting a step check fault.
     */
    void setInternalFault(Topography i_topography, pdbg_target* i_chipAtFault)
    {
        assert(nullptr != i_chipAtFault);
        iv_internalFaultList[i_topography].push_back(i_chipAtFault);
    }

    /**
     * @param  i_topography Target topography.
     * @return The list of all chips with internal faults.
     */
    const std::vector<pdbg_target*>& getInteralFaults(Topography i_topography)
    {
        return iv_internalFaultList[i_topography];
    }

    /**
     * @brief Indicates the given non-MDMT chip has seen a fault in the TOD
     *        network.
     * @param i_topography        Target topography.
     * @param i_chipSourcingClock The chip sourcing the clock for the chip at
     *                            fault. This is NOT the chip at fault.
     */
    void setNetworkFault(Topography i_topography,
                         pdbg_target* i_chipSourcingClock)
    {
        assert(nullptr != i_chipSourcingClock);
        iv_networkFaultList[i_topography].push_back(i_chipSourcingClock);
    }

    /**
     * @param  i_topography Target topography.
     * @return The list of all chips sourcing the clocks for the non-MDMT chips
     *         with step check faults.
     */
    const std::vector<pdbg_target*>& getNetworkFaults(Topography i_topography)
    {
        return iv_networkFaultList[i_topography];
    }
};

/**
 * @brief Collects TOD fault data for each processor chip.
 */
void collectTodFaultData(pdbg_target*, TodData&)
{
    // TODO: Need to query hardware for this chip and add to TodData if any
    // faults found.
}

/**
 * @brief Handles TOD step check fault attentions.
 */
void tod_step_check_fault(unsigned int, const libhei::Chip& i_chip,
                          ServiceData& io_servData)
{
    // Query hardware for TOD fault data from all active processors.
    TodData data{};
    std::vector<pdbg_target*> chipList;
    util::pdbg::getActiveProcessorChips(chipList);
    for (const auto& chip : chipList)
    {
        collectTodFaultData(chip, data);
    }

    // For each topography:
    //  - First, check if the MDMT chip is reporting a fault. If so, it is
    //    likely that any downstream step check faults are due to the fault in
    //    the MDMT.
    //  - If MDMT is not reporting a fault, look for any network path errors
    //    from the non-MDMT chips. In which case, we will want to call out all
    //    of the chips sourcing those step check errors (not the chips reporting
    //    them).
    //  - If no other errors found, callout any chips reporting internal step
    //    check faults.

    bool calloutsMade = false; // need to keep track for default case.

    for (const auto top : {Topography::ACTIVE, Topography::BACKUP})
    {
        auto mdmtFault      = data.getMdmtFault(top);
        auto internalFaults = data.getInteralFaults(top);
        auto networkFaults  = data.getNetworkFaults(top);

        if (nullptr != mdmtFault) // MDMT fault
        {
            calloutsMade = true;

            // Callout the TOD clock (guard).
            io_servData.calloutClock(callout::ClockType::TOD_CLOCK,
                                     callout::Priority::MED, true);

            // Callout the MDMT chip (no guard).
            io_servData.calloutTarget(mdmtFault, callout::Priority::MED, true);

            // Callout everything in between.
            // TODO: This isn't necessary for now because the clock callout is
            //       the backplane. However, we may need a procedure callout
            //       for future systems.
        }
        else if (!networkFaults.empty()) // network path faults
        {
            calloutsMade = true;

            // Callout all chips with network errors (guard).
            for (const auto& chip : networkFaults)
            {
                io_servData.calloutTarget(chip, callout::Priority::MED, true);
            }
        }
        else if (!internalFaults.empty()) // interal path faults
        {
            calloutsMade = true;

            // Callout all chips with internal errors (guard).
            for (const auto& chip : internalFaults)
            {
                io_servData.calloutTarget(chip, callout::Priority::MED, true);
            }
        }
    }

    // If no callouts are made, default to calling out the chip that reported
    // the original attention.
    if (!calloutsMade)
    {
        io_servData.calloutTarget(util::pdbg::getTrgt(i_chip),
                                  callout::Priority::MED, true);
    }
}

} // namespace P10

PLUGIN_DEFINE_NS(P10_10, P10, tod_step_check_fault);
PLUGIN_DEFINE_NS(P10_20, P10, tod_step_check_fault);

} // namespace analyzer
