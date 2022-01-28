#pragma once

#include <analyzer/analyzer_main.hpp>
#include <analyzer/callout.hpp>
#include <hei_main.hpp>
#include <nlohmann/json.hpp>
#include <util/pdbg.hpp>

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
        iv_rootCause(i_rootCause),
        iv_analysisType(i_analysisType), iv_isoData(i_isoData)
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
    void getSrcSubsystem(
        std::map<std::string, std::string>& io_additionalData) const
    {
        io_additionalData["PEL_SUBSYSTEM"] =
            std::to_string(static_cast<unsigned int>(iv_srcSubsystem.first));
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
    /** These SRC subsystem values are defined by the PEL spec. */
    enum class SrcSubsystem
    {
        // Can also reference `extensions/openpower-pels/pel_values.cpp` from
        // `openbmc/phosphor-logging` for these values.

        PROCESSOR      = 0x10,
        PROCESSOR_FRU  = 0x11,
        PROCESSOR_CHIP = 0x12,
        PROCESSOR_UNIT = 0x13,
        PROCESSOR_BUS  = 0x14,

        MEMORY         = 0x20,
        MEMORY_CTLR    = 0x21,
        MEMORY_BUS     = 0x22,
        MEMORY_DIMM    = 0x23,
        MEMORY_FRU     = 0x24,
        EXTERNAL_CACHE = 0x25,

        IO           = 0x30,
        IO_HUB       = 0x31,
        IO_BRIDGE    = 0x32,
        IO_BUS       = 0x33,
        IO_PROCESSOR = 0x34,
        IO_HUB_OTHER = 0x35,
        PHB          = 0x38,

        IO_ADAPTER              = 0x40,
        IO_ADAPTER_COMM         = 0x41,
        IO_DEVICE               = 0x46,
        IO_DEVICE_DASD          = 0x47,
        IO_EXTERNAL_GENERAL     = 0x4C,
        IO_EXTERNAL_WORKSTATION = 0x4D,
        IO_STORAGE_MEZZ         = 0x4E,

        CEC_HARDWARE          = 0x50,
        CEC_SP_A              = 0x51,
        CEC_SP_B              = 0x52,
        CEC_NODE_CONTROLLER   = 0x53,
        CEC_VPD               = 0x55,
        CEC_I2C               = 0x56,
        CEC_CHIP_IFACE        = 0x57,
        CEC_CLOCKS            = 0x58,
        CEC_OP_PANEL          = 0x59,
        CEC_TOD               = 0x5A,
        CEC_STORAGE_DEVICE    = 0x5B,
        CEC_SP_HYP_IFACE      = 0x5C,
        CEC_SERVICE_NETWORK   = 0x5D,
        CEC_SP_HOSTBOOT_IFACE = 0x5E,

        POWER            = 0x60,
        POWER_SUPPLY     = 0x61,
        POWER_CONTROL_HW = 0x62,
        POWER_FANS       = 0x63,
        POWER_SEQUENCER  = 0x64,

        OTHERS                    = 0x70,
        OTHER_HMC                 = 0x71,
        OTHER_TEST_TOOL           = 0x72,
        OTHER_MEDIA               = 0x73,
        OTHER_MULTIPLE_SUBSYSTEMS = 0x74,
        OTHER_NA                  = 0x75,
        OTHER_INFO_SRC            = 0x76,

        SURV_HYP_LOST_SP   = 0x7A,
        SURV_SP_LOST_HYP   = 0x7B,
        SURV_SP_LOST_HMC   = 0x7C,
        SURV_HMC_LOST_LPAR = 0x7D,
        SURV_HMC_LOST_BPA  = 0x7E,
        SURV_HMC_LOST_HMC  = 0x7F,

        PLATFORM_FIRMWARE          = 0x80,
        SP_FIRMWARE                = 0x81,
        HYP_FIRMWARE               = 0x82,
        PARTITION_FIRMWARE         = 0x83,
        SLIC_FIRMWARE              = 0x84,
        SPCN_FIRMWARE              = 0x85,
        BULK_POWER_FIRMWARE_SIDE_A = 0x86,
        HMC_CODE_FIRMWARE          = 0x87,
        BULK_POWER_FIRMWARE_SIDE_B = 0x88,
        VIRTUAL_SP                 = 0x89,
        HOSTBOOT                   = 0x8A,
        OCC                        = 0x8B,
        BMC_FIRMWARE               = 0x8D,

        SOFTWARE     = 0x90,
        OS_SOFTWARE  = 0x91,
        XPF_SOFTWARE = 0x92,
        APP_SOFTWARE = 0x93,

        EXT_ENV            = 0xA0,
        INPUT_POWER_SOURCE = 0xA1,
        AMBIENT_TEMP       = 0xA2,
        USER_ERROR         = 0xA3,
        CORROSION          = 0xA4,
    };

    /**
     * @brief Compares the current SRC subsystem type with the given SRC
     *        subsystem type and stores the highest priority callout subsystem.
     *        If the two subsystems are of equal priority. The stored subsystem
     *        is used.
     * @param i_subsystem The given subsystem type.
     * @param i_priority  The callout priority associated with the given
     *                    subsystem.
     */
    void setSrcSubsystem(SrcSubsystem i_subsystem,
                         callout::Priority i_priority);

    /** The SRC subsystem field (2nd byte of the primary SRC) is based on the
     *  callouts the PEL. As callouts are to the service data, we'll need to
     *  keep track of the highest priority callout subsystem. */
    std::pair<SrcSubsystem, callout::Priority> iv_srcSubsystem = {
        SrcSubsystem::CEC_HARDWARE, callout::Priority::LOW};
};

} // namespace analyzer
