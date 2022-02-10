
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

    /** Secondary chips (non-MDMT) with step check faults. */
    std::map<Topography, std::vector<pdbg_target*>> iv_stepCheckFaultList;

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
     * @param i_chipAtFault The chip reporting an internal fault.
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
     * @brief Indicates the given secondary (non-MDMT) chip has seen a step
     *        check fault.
     * @param i_topography  Target topography.
     * @param i_chipAtFault The chip reporting a step check fault.
     */
    void setStepCheckFault(Topography i_topography, pdbg_target* i_chipAtFault)
    {
        assert(nullptr != i_chipAtFault);
        iv_stepCheckFaultList[i_topography].push_back(i_chipAtFault);
    }

    /**
     * @param  i_topography Target topography.
     * @return The list of all chips with step check faults.
     */
    const std::vector<pdbg_target*>& getStepCheckFaults(Topography i_topography)
    {
        return iv_stepCheckFaultList[i_topography];
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

    // Regarding callouts, we'll categorize the failures as:
    //  - MDMT clock problem
    //  - Internal path error
    //  - Network path error (i.e step check errors)
    //
    // For each topology, we'll only callout parts for the first category in
    // the above order that reported faults.

    bool calloutsMade = false; // need to keep track for default case.

    for (const auto top : {Topography::ACTIVE, Topography::BACKUP})
    {
        auto mdmtFault       = data.getMdmtFault(top);
        auto internalFaults  = data.getInteralFaults(top);
        auto stepCheckFaults = data.getStepCheckFaults(top);

        // Check MDMT fault.
        if (nullptr != mdmtFault)
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

            continue; // go to next topography
        }

        // Check interal path fault.
        // Note that we only want to make callouts for internal errors if are
        // only internal errors present (i.e. no step check errors).
        if (!internalFaults.empty() && stepCheckFaults.empty())
        {
            calloutsMade = true;

            // Callout all chips with internal errors (guard).
            for (const auto& chip : internalFaults)
            {
                io_servData.calloutTarget(chip, callout::Priority::MED, true);
            }

            continue; // go to next topography
        }

        // Check network path fault.
        if (!stepCheckFaults.empty())
        {
            calloutsMade = true;

            // Callout all chips with network errors (guard).
            for (const auto& chip : stepCheckFaults)
            {
                io_servData.calloutTarget(chip, callout::Priority::MED, true);
            }

            continue; // go to next topography
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
