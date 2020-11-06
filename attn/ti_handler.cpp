#include <attn/attn_handler.hpp>
#include <attn/attn_logging.hpp>
#include <attn/ti_handler.hpp>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/exception.hpp>

#include <iomanip>
#include <iostream>

namespace attn
{

/**
 * @brief Determine if this is a HB or PHYP TI event
 *
 * Use the TI info data area to determine if this is either a HB or a PHYP
 * TI event then handle the event.
 *
 * @param i_tiDataArea pointer to the TI infor data
 */
int tiHandler(TiDataArea* i_tiDataArea)
{
    int rc = RC_SUCCESS;

    // check TI data area if it is available
    if (nullptr != i_tiDataArea)
    {
        // HB v. PHYP TI logic: Only hosboot will fill in hbTerminateType
        // and it will be non-zero. Only hostboot will fill out source and it
        // it will be non-zero. Only PHYP will fill in srcFormat and it will
        // be non-zero.
        // source and only PHYP will fill in srcFormat.
        if ((0 == i_tiDataArea->hbTerminateType) &&
            (0 == i_tiDataArea->source) && (0 != i_tiDataArea->srcFormat))
        {
            handlePhypTi(i_tiDataArea);
        }
        else
        {
            handleHbTi(i_tiDataArea);
        }
    }
    else
    {
        // TI data was not available, assume PHYP TI for now. When a host state
        // management interface becomes availabe we may be able to make a more
        // informed decision here.
        handlePhypTi(i_tiDataArea);
    }

    return rc;
}

/**
 * @brief Transition the host state
 *
 * We will transition the host state by starting the appropriate dbus target.
 *
 * @param i_target the dbus target to start
 */
void transitionHost(const char* i_target)
{
    // We will be transitioning host by starting appropriate dbus target
    auto bus    = sdbusplus::bus::new_system();
    auto method = bus.new_method_call(
        "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
        "org.freedesktop.systemd1.Manager", "StartUnit");

    method.append(i_target);  // target unit to start
    method.append("replace"); // mode = replace conflicting queued jobs

    trace<level::INFO>(i_target);

    bus.call_noreply(method); // start the service
}

/**
 * @brief Handle a PHYP terminate immediate special attention
 *
 * The TI info data area will contain information pertaining to the TI
 * condition. We will wither quiesce the host or initiate a MPIPL depending
 * depending on the auto reboot configuration. We will also create a PEL which
 * will contain the TI info data and FFDC data captured in the system journal.
 *
 * @param i_tiDataArea pointer to TI information filled in by hostboot
 */
void handlePhypTi(TiDataArea* i_tiDataArea)
{
    trace<level::INFO>("PHYP TI");

    // gather additional data for PEL
    std::map<std::string, std::string> tiAdditionalData;

    parsePhypOpalTiInfo(tiAdditionalData, i_tiDataArea);
    parseRawTiInfo(tiAdditionalData, i_tiDataArea);

    if (autoRebootEnabled())
    {
        // If autoreboot is enabled we will start diagnostic mode target
        // which will ultimately mpipl the host.
        transitionHost("obmc-host-diagnostic-mode@0.target");
    }
    else
    {
        // If autoreboot is disabled we will quiesce the host
        transitionHost("obmc-host-quiesce@0.target");
    }

    eventTerminate(tiAdditionalData); // generate PEL
}

/**
 * @brief Handle a hostboot terminate immediate special attention
 *
 * The TI info data area will contain information pertaining to the TI
 * condition. The course of action to take regarding the host state will
 * depend on the contents of the TI info data area. We will also create a
 * PEL containing the TI info data and FFDC data captured in the system
 * journal.
 *
 * @param i_tiDataArea pointer to TI information filled in by hostboot
 */
void handleHbTi(TiDataArea* i_tiDataArea)
{
    trace<level::INFO>("HB TI");

    // gather additional data for PEL
    std::map<std::string, std::string> tiAdditionalData;
    parseHbTiInfo(tiAdditionalData, i_tiDataArea);
    parseRawTiInfo(tiAdditionalData, i_tiDataArea);

    bool hbDumpRequested = true; // HB dump is common case
    bool generatePel     = true; // assume PEL will be created
    bool terminateHost   = true; // transition host state

    if (nullptr != i_tiDataArea)
    {
        std::stringstream ss;
        ss << std::hex << std::showbase;

        switch (i_tiDataArea->hbTerminateType)
        {
            case TI_WITH_PLID:
            case TI_WITH_EID:
                ss << "TI with PLID/EID: " << be32toh(i_tiDataArea->asciiData1);
                trace<level::INFO>(ss.str().c_str());
                if (0 == i_tiDataArea->hbDumpFlag)
                {
                    hbDumpRequested = false; // no HB dump requested
                }
                break;
            case TI_WITH_SRC:
                // SRC is byte 2 and 3 of 4 byte srcWord12HbWord0
                uint16_t hbSrc =
                    (be32toh(i_tiDataArea->srcWord12HbWord0) >> 16);

                // trace some info
                ss << "TI with SRC: " << (int)hbSrc;
                trace<level::INFO>(ss.str().c_str());
                ss.str(std::string()); // clear stream

                switch (hbSrc)
                {
                    case HB_SRC_SHUTDOWN_REQUEST:
                        trace<level::INFO>("shutdown request");
                        generatePel = false;
                        break;
                    case HB_SRC_KEY_TRANSITION:
                        trace<level::INFO>("key transition");
                        terminateHost = false;
                        break;
                    case HB_SRC_INSUFFICIENT_HW:
                        trace<level::INFO>("insufficient hardware");
                        break;
                    case HB_SRC_TPM_FAIL:
                        trace<level::INFO>("TPM fail");
                        break;
                    case HB_SRC_ROM_VERIFY:
                        trace<level::INFO>("ROM verify");
                        break;
                    case HB_SRC_EXT_MISMATCH:
                        trace<level::INFO>("EXT mismatch");
                        break;
                    case HB_SRC_ECC_UE:
                        trace<level::INFO>("ECC UE");
                        break;
                    case HB_SRC_UNSUPPORTED_MODE:
                        trace<level::INFO>("unsupported mode");
                        break;
                    case HB_SRC_UNSUPPORTED_SFCRANGE:
                        trace<level::INFO>("unsupported SFC range");
                        break;
                    case HB_SRC_PARTITION_TABLE:
                        trace<level::INFO>("partition table invalid");
                        break;
                    case HB_SRC_UNSUPPORTED_HARDWARE:
                        trace<level::INFO>("unsupported hardware");
                        break;
                    case HB_SRC_PNOR_CORRUPTION:
                        trace<level::INFO>("PNOR corruption");
                        break;
                    default:
                        trace<level::INFO>("reason: other");
                }

                break;
        }
    }

    if (true == terminateHost)
    {
        // if hostboot dump is requested initiate dump
        if (hbDumpRequested)
        {
            // Until HB dump support available just quiesce the host - once
            // dump support is available the dump component will transition
            // (ipl/halt) the host.
            transitionHost("obmc-host-quiesce@0.target");
        }
        else
        {
            // Quiese the host - when the host is quiesced it will either
            // "halt" or IPL depending on autoreboot setting.
            transitionHost("obmc-host-quiesce@0.target");
        }
    }

    if (true == generatePel)
    {
        eventTerminate(tiAdditionalData); // generate PEL
    }
}

/** @brief Parse the TI info data area into map as raw 32-bit fields */
void parseRawTiInfo(std::map<std::string, std::string>& i_map,
                    TiDataArea* i_buffer)
{

    uint32_t* tiDataArea = (uint32_t*)i_buffer;
    std::stringstream ss;

    ss << std::hex << std::setfill('0');
    ss << "raw:";
    while (tiDataArea <= (uint32_t*)((char*)i_buffer + sizeof(TiDataArea)))
    {
        ss << std::setw(8) << std::endl << be32toh(*tiDataArea);
        tiDataArea++;
    }

    std::string key, value;
    char delim = ':';

    while (std::getline(ss, key, delim))
    {
        std::getline(ss, value, delim);
        i_map[key] = value;
    }
}

/** @brief Parse the TI info data area into map as PHYP/OPAL data */
void parsePhypOpalTiInfo(std::map<std::string, std::string>& i_map,
                         TiDataArea* i_tiDataArea)
{
    std::stringstream ss;

    ss << std::hex << std::showbase;
    ss << "0x00 TI Area Valid:" << (int)i_tiDataArea->tiAreaValid << ":";
    ss << "0x01 Command:" << (int)i_tiDataArea->command << ":";
    ss << "0x02 Num. Data Bytes:" << be16toh(i_tiDataArea->numDataBytes) << ":";
    ss << "0x04 Reserved:" << (int)i_tiDataArea->reserved1 << ":";
    ss << "0x06 HWDump Type:" << be16toh(i_tiDataArea->hardwareDumpType) << ":";
    ss << "0x08 SRC Format:" << (int)i_tiDataArea->srcFormat << ":";
    ss << "0x09 SRC Flags:" << (int)i_tiDataArea->srcFlags << ":";
    ss << "0x0a Num. ASCII Words:" << (int)i_tiDataArea->numAsciiWords << ":";
    ss << "0x0b Num. Hex Words:" << (int)i_tiDataArea->numHexWords << ":";
    ss << "0x0e Length of SRC:" << be16toh(i_tiDataArea->lenSrc) << ":";
    ss << "0x10 SRC Word 12:" << be32toh(i_tiDataArea->srcWord12HbWord0) << ":";
    ss << "0x14 SRC Word 13:" << be32toh(i_tiDataArea->srcWord13HbWord2) << ":";
    ss << "0x18 SRC Word 14:" << be32toh(i_tiDataArea->srcWord14HbWord3) << ":";
    ss << "0x1c SRC Word 15:" << be32toh(i_tiDataArea->srcWord15HbWord4) << ":";
    ss << "0x20 SRC Word 16:" << be32toh(i_tiDataArea->srcWord16HbWord5) << ":";
    ss << "0x24 SRC Word 17:" << be32toh(i_tiDataArea->srcWord17HbWord6) << ":";
    ss << "0x28 SRC Word 18:" << be32toh(i_tiDataArea->srcWord18HbWord7) << ":";
    ss << "0x2c SRC Word 19:" << be32toh(i_tiDataArea->srcWord19HbWord8) << ":";
    ss << "0x30 ASCII Data:" << be32toh(i_tiDataArea->asciiData0) << ":";
    ss << "0x34 ASCII Data:" << be32toh(i_tiDataArea->asciiData1) << ":";
    ss << "0x38 ASCII Data:" << be32toh(i_tiDataArea->asciiData2) << ":";
    ss << "0x3c ASCII Data:" << be32toh(i_tiDataArea->asciiData3) << ":";
    ss << "0x40 ASCII Data:" << be32toh(i_tiDataArea->asciiData4) << ":";
    ss << "0x44 ASCII Data:" << be32toh(i_tiDataArea->asciiData5) << ":";
    ss << "0x48 ASCII Data:" << be32toh(i_tiDataArea->asciiData6) << ":";
    ss << "0x4c ASCII Data:" << be32toh(i_tiDataArea->asciiData7) << ":";
    ss << "0x50 Location:" << (int)i_tiDataArea->location << ":";
    ss << "0x51 Code Sections:" << (int)i_tiDataArea->codeSection << ":";
    ss << "0x52 Additional Size:" << (int)i_tiDataArea->additionalSize << ":";
    ss << "0x53 Additional Data:" << (int)i_tiDataArea->andData;

    std::string key, value;
    char delim = ':';

    while (std::getline(ss, key, delim))
    {
        std::getline(ss, value, delim);
        i_map[key] = value;
    }
}

/** @brief Parse the TI info data area into map as hostboot data */
void parseHbTiInfo(std::map<std::string, std::string>& i_map,
                   TiDataArea* i_tiDataArea)
{
    std::stringstream ss;

    ss << std::hex << std::showbase;
    ss << "0x00 TI Area Valid:" << (int)i_tiDataArea->tiAreaValid << ":";
    ss << "0x04 Reserved:" << (int)i_tiDataArea->reserved1 << ":";
    ss << "0x05 HB_Term. Type:" << (int)i_tiDataArea->hbTerminateType << ":";
    ss << "0x0c HB Dump Flag:" << (int)i_tiDataArea->hbDumpFlag << ":";
    ss << "0x0d Source:" << (int)i_tiDataArea->source << ":";
    ss << "0x10 HB Word 0:" << be32toh(i_tiDataArea->srcWord12HbWord0) << ":";
    ss << "0x14 HB Word 2:" << be32toh(i_tiDataArea->srcWord13HbWord2) << ":";
    ss << "0x18 HB Word 3:" << be32toh(i_tiDataArea->srcWord14HbWord3) << ":";
    ss << "0x1c HB Word 4:" << be32toh(i_tiDataArea->srcWord15HbWord4) << ":";
    ss << "0x20 HB Word 5:" << be32toh(i_tiDataArea->srcWord16HbWord5) << ":";
    ss << "0x24 HB Word 6:" << be32toh(i_tiDataArea->srcWord17HbWord6) << ":";
    ss << "0x28 HB Word 7:" << be32toh(i_tiDataArea->srcWord18HbWord7) << ":";
    ss << "0x2c HB Word 8:" << be32toh(i_tiDataArea->srcWord19HbWord8) << ":";
    ss << "0x30 error_data:" << be32toh(i_tiDataArea->asciiData0) << ":";
    ss << "0x34 EID:" << be32toh(i_tiDataArea->asciiData1);

    std::string key, value;
    char delim = ':';

    while (std::getline(ss, key, delim))
    {
        std::getline(ss, value, delim);
        i_map[key] = value;
    }
}

/** @brief Read state of autoreboot propertyi via dbus */
bool autoRebootEnabled()
{
    // Use dbus get-property interface to read the autoreboot property
    auto bus = sdbusplus::bus::new_system();
    auto method =
        bus.new_method_call("xyz.openbmc_project.Settings",
                            "/xyz/openbmc_project/control/host0/auto_reboot",
                            "org.freedesktop.DBus.Properties", "Get");

    method.append("xyz.openbmc_project.Control.Boot.RebootPolicy",
                  "AutoReboot");

    bool autoReboot = false; // assume autoreboot attribute not available

    try
    {
        auto reply = bus.call(method);

        std::variant<bool> result;
        reply.read(result);
        autoReboot = std::get<bool>(result);
    }
    catch (const sdbusplus::exception::SdBusError& ec)
    {
        std::string traceMessage =
            "Error in AutoReboot Get: " + std::string(ec.what());
        trace<level::INFO>(traceMessage.c_str());
    }

    return autoReboot;
}

} // namespace attn
