#pragma once

#include <analyzer/analyzer_main.hpp>
#include <analyzer/callout.hpp>
#include <hei_main.hpp>
#include <nlohmann/json.hpp>
#include <util/pdbg.hpp>

#include <format>

namespace analyzer
{

/**
 * @brief Data regarding required service actions based on the hardware error
 *        analysis.
 */
class ServiceData
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_rootCause    The signature of the root cause attention.
     * @param i_analysisType The type of analysis to perform.
     * @param i_isoData      The data found during isolation.
     */
    ServiceData(const libhei::Signature& i_rootCause,
                AnalysisType i_analysisType,
                const libhei::IsolationData& i_isoData) :
        iv_rootCause(i_rootCause), iv_analysisType(i_analysisType),
        iv_isoData(i_isoData)
    {}

    /** @brief Destructor. */
    ~ServiceData() = default;

    /** @brief Copy constructor. */
    ServiceData(const ServiceData&) = default;

    /** @brief Assignment operator. */
    ServiceData& operator=(const ServiceData&) = default;

  private:
    /** The signature of the root cause attention. */
    const libhei::Signature iv_rootCause;

    /** The type of analysis to perform. */
    const AnalysisType iv_analysisType;

    /** The data found during isolation. */
    const libhei::IsolationData iv_isoData;

    /** The list of callouts that will be added to a PEL. */
    nlohmann::json iv_calloutList = nlohmann::json::array();

    /** FFDC for callouts that would otherwise not be available in the
     *  callout list (unit paths, bus types, etc.). */
    nlohmann::json iv_calloutFFDC = nlohmann::json::array();

  public:
    /** @return The signature of the root cause attention. */
    const libhei::Signature& getRootCause() const
    {
        return iv_rootCause;
    }

    /** @return The type of analysis to perform. */
    AnalysisType getAnalysisType() const
    {
        return iv_analysisType;
    }

    /** @return The data found during isolation. */
    const libhei::IsolationData& getIsolationData() const
    {
        return iv_isoData;
    }

    /** @return Returns the guard type based on current analysis policies. */
    callout::GuardType queryGuardPolicy() const
    {
        if (AnalysisType::SYSTEM_CHECKSTOP == iv_analysisType)
        {
            return callout::GuardType::UNRECOVERABLE;
        }
        else if (AnalysisType::TERMINATE_IMMEDIATE == iv_analysisType)
        {
            return callout::GuardType::PREDICTIVE;
        }

        return callout::GuardType::NONE;
    }

    /**
     * @brief Add callout for a pdbg_target.
     * @param i_target   The chip or unit target to add to the callout list.
     * @param i_priority The callout priority.
     * @param i_guard    True if guard is required. False, otherwise.
     */
    void calloutTarget(pdbg_target* i_target, callout::Priority i_priority,
                       bool i_guard);

    /**
     * @brief Add callout for a connected target on the other side of a bus.
     * @param i_rxTarget The target on the receiving side (RX) of the bus.
     * @param i_busType  The bus type.
     * @param i_priority The callout priority.
     * @param i_guard    True if guard is required. False, otherwise.
     */
    void calloutConnected(pdbg_target* i_rxTarget,
                          const callout::BusType& i_busType,
                          callout::Priority i_priority, bool i_guard);

    /**
     * @brief Add callout for an entire bus.
     * @param i_rxTarget The target on the receiving side (RX) of the bus.
     * @param i_busType  The bus type.
     * @param i_priority The callout priority.
     * @param i_guard    True if guard is required. False, otherwise.
     */
    void calloutBus(pdbg_target* i_rxTarget, const callout::BusType& i_busType,
                    callout::Priority i_priority, bool i_guard);

    /**
     * @brief Add callout for a clock.
     * @param i_clockType The clock type.
     * @param i_priority  The callout priority.
     * @param i_guard     True if guard is required. False, otherwise.
     */
    void calloutClock(const callout::ClockType& i_clockType,
                      callout::Priority i_priority, bool i_guard);

    /**
     * @brief Add callout for a service procedure.
     * @param i_procedure The procedure type.
     * @param i_priority  The callout priority.
     */
    void calloutProcedure(const callout::Procedure& i_procedure,
                          callout::Priority i_priority);

    /**
     * @brief Add callout for part type.
     * @param i_part     The part type.
     * @param i_priority The callout priority.
     */
    void calloutPart(const callout::PartType& i_part,
                     callout::Priority i_priority);

    /** @brief Accessor to iv_calloutList. */
    const nlohmann::json& getCalloutList() const
    {
        return iv_calloutList;
    }

    /** @brief Accessor to iv_calloutFFDC. */
    const nlohmann::json& getCalloutFFDC() const
    {
        return iv_calloutFFDC;
    }

    /**
     * @brief Adds the SRC subsystem to the given additional PEL data.
     * @param io_additionalData The additional PEL data.
     */
    void addSrcSubsystem(
        std::map<std::string, std::string>& io_additionalData) const
    {
        io_additionalData["PEL_SUBSYSTEM"] = std::format(
            "0x{:02x}", static_cast<uint8_t>(iv_srcSubsystem.first));
    }

    /** @brief Accessor to iv_srcSubsystem. */
    const std::pair<callout::SrcSubsystem, callout::Priority> getSubsys() const
    {
        return iv_srcSubsystem;
    }

  private:
    /**
     * @brief Add callout information to the callout list.
     * @param The JSON object for this callout.
     */
    void addCallout(const nlohmann::json& i_callout);

    /**
     * @brief Add FFDC for a callout that would otherwise not be available in
     *        the callout list (unit paths, bus types, etc.).
     * @param The JSON object for this callout.
     */
    void addCalloutFFDC(const nlohmann::json& i_ffdc)
    {
        iv_calloutFFDC.push_back(i_ffdc);
    }

    /**
     * @brief A simple helper function for all the callout functions that need
     *        to callout a target (callout only, no FFDC).
     * @param i_target   The chip or unit target to add to the callout list.
     * @param i_priority The callout priority.
     * @param i_guard    True if guard is required. False, otherwise.
     */
    void addTargetCallout(pdbg_target* i_target, callout::Priority i_priority,
                          bool i_guard);

    /**
     * @brief A simple helper function for all the callout functions that need
     *        to callout a the backplane (callout only, no FFDC).
     * @param i_priority The callout priority.
     */
    void addBackplaneCallout(callout::Priority i_priority);

  private:
    /**
     * @brief Compares the current SRC subsystem type with the given SRC
     *        subsystem type and stores the highest priority callout subsystem.
     *        If the two subsystems are of equal priority. The stored subsystem
     *        is used.
     * @param i_subsystem The given subsystem type.
     * @param i_priority  The callout priority associated with the given
     *                    subsystem.
     */
    void setSrcSubsystem(callout::SrcSubsystem i_subsystem,
                         callout::Priority i_priority);

    /**
     * @brief Returns the appropriate SRC subsystem based on the input target.
     * @param i_trgt The given pdbg target.
     */
    callout::SrcSubsystem getTargetSubsystem(pdbg_target* i_target);

    /** The SRC subsystem field (2nd byte of the primary SRC) is based on the
     *  callouts the PEL. As callouts are to the service data, we'll need to
     *  keep track of the highest priority callout subsystem. */
    std::pair<callout::SrcSubsystem, callout::Priority> iv_srcSubsystem = {
        callout::SrcSubsystem::CEC_HARDWARE, callout::Priority::LOW};
};

} // namespace analyzer
