#include <libpdbg.h>

#include <attn_logging.hpp>
#include <phosphor-logging/log.hpp>
#include <sdbusplus/bus.hpp>

#include <iomanip>

using namespace phosphor::logging;

namespace attn
{

/**
 * @brief Handle SBE vital attention
 *
 * @return 0 = success
 */
int handleVital();

/**
 * @brief Handle checkstop attention
 *
 * @return 0 = success
 */
int handleCheckstop();

/**
 * @brief Handle special attention
 *
 * @return 0 = success
 */
int handleSpecial();

/**
 * @brief Notify Cronus over dbus interface
 *
 * @param i_proc   Processor number with Special attention
 * @param i_core   Core number with special attention
 * @param i_thread Thread number with special attention
 */
void notifyCronus(uint32_t i_proc, uint32_t i_core, uint32_t i_thread);

/**
 * @brief Attention handler
 *
 * Check each processor for active attentions of type SBE Vital (vital),
 * System Checkstop (checkstop) and Special Attention (special) and handle
 * each as follows:
 *
 * checkstop: TBD
 * vital:     TBD
 * special:   Determine if the special attention is a Breakpoint (BP),
 *            Terminate Immediately (TI) or CoreCodeToSp (corecode). For each
 *            special attention type, do the following:
 *
 *            BP:          Notify Cronus
 *            TI:          TBD
 *            Corecode:    TBD
 *            Recoverable: TBD
 */
void attnHandler()
{
    uint32_t isr_val, isr_mask;
    uint32_t proc;

    // loop through processors looking for active attentions
    pdbg_target* target;
    pdbg_for_each_class_target("fsi", target)
    {
        if (PDBG_TARGET_ENABLED == pdbg_target_probe(target))
        {
            proc = pdbg_target_index(target); // get processor number

            std::stringstream ss; // log message stream
            ss << "[ATTN] checking processor " << proc << std::endl;
            log<level::INFO>(ss.str().c_str());

            // get active attentions on processor
            if (0 != fsi_read(target, 0x1007, &isr_val))
            {
                std::stringstream ss; // log message stream
                ss << "[ATTN] Error! cfam read 0x1007 FAILED" << std::endl;
                log<level::INFO>(ss.str().c_str());
            }
            else
            {
                std::stringstream ss; // log message stream
                ss << "[ATTN] cfam 0x1007 = 0x";
                ss << std::hex << std::setw(8) << std::setfill('0');
                ss << isr_val << std::endl;
                log<level::INFO>(ss.str().c_str());

                // get interrupt enabled special attentions mask
                if (0 != fsi_read(target, 0x100d, &isr_mask))
                {
                    std::stringstream ss; // log message stream
                    ss << "[ATTN] Error! cfam read 0x100d FAILED" << std::endl;
                    log<level::INFO>(ss.str().c_str());
                }
                else
                {
                    std::stringstream ss; // log message stream
                    ss << "[ATTN] cfam 0x100d = 0x";
                    ss << std::hex << std::setw(8) << std::setfill('0');
                    ss << isr_mask << std::endl;
                    log<level::INFO>(ss.str().c_str());

                    // bit 0 on "left": bit 30 = SBE vital attention
                    if (isr_val & isr_mask & 0x00000002)
                    {
                        handleVital();
                    }

                    // bit 0 on "left": bit 1 = checkstop
                    if (isr_val & isr_mask & 0x40000000)
                    {
                        handleCheckstop();
                    }

                    // bit 0 on "left": bit 2 = special attention
                    if (isr_val & isr_mask & 0x20000000)
                    {
                        handleSpecial();
                    }

                    // bit 0 on "left": bit 3 = recoverable errors
                    if (isr_val & isr_mask & 0x10000000)
                    {
                        std::stringstream ss; // log message stream
                        ss << "[ATTN] recoverable errors present" << std::endl;
                        log<level::INFO>(ss.str().c_str());
                    }
                } // cfam 0x100d valid
            }     // cfam 0x1007 valid
        }         // fsi target enabled
    }             // next processor

    return; // checked all processors
}

/**
 * @brief Handle SBE vital attention
 */
int handleVital()
{
    int rc = 0; // vital attention supported

    // Trace message
    std::stringstream ss; // log message stream
    ss << "[ATTN] vital" << std::endl;
    log<level::INFO>(ss.str().c_str());

    // Additional log entry data can be stored as key/value pairs
    std::map<std::string, std::string> additional;
    additional["KEY1"] = "VALUE1";

    // Add event to the log
    logEntry("SBE Vital Attention", additional);

    return rc;
}

/**
 * @brief Handle checkstop attention
 */
int handleCheckstop()
{
    int rc = 0; // checkstop attention supported

    // Trace message
    std::stringstream ss; // log message stream
    ss << "[ATTN] checkstop" << std::endl;
    log<level::INFO>(ss.str().c_str());

    // Additional log entry data can be stored as key/value pairs
    std::map<std::string, std::string> additional;
    additional["KEY1"] = "VALUE1";

    // Add event to the log
    logEntry("Checkstop Attention", additional);

    return rc;
}

/**
 * @brief Handle special attention
 *
 * Currently we are only handling PHYP breakpoint in which case we will
 * add a trace message to the journal and notify the breakpoint handler
 * of the event.
 */
int handleSpecial()
{
    int rc = 0; // special attention handling supported

    // Trace message
    std::stringstream ss; // log message stream
    ss << "[ATTN] special" << std::endl;

    // Trace message
    ss << "[ATTN] breakpoint" << std::endl;
    log<level::INFO>(ss.str().c_str());

    // Cronus will determine proc, core and thread so just notify
    notifyCronus(0, 0, 0); // proc-0, core-0, thread-0

    return rc;
}

/**
 * @brief Notify Cronus over dbus interface
 */
void notifyCronus(uint32_t i_proc, uint32_t i_core, uint32_t i_thread)
{
    std::stringstream ss; // log message stream

    // log status info
    ss << "[ATTN] notify ";
    ss << i_proc << ", " << i_core << ", " << i_thread << std::endl;
    log<level::INFO>(ss.str().c_str());

    // notify Cronus over dbus
    auto bus = sdbusplus::bus::new_system();
    auto msg = bus.new_signal("/", "org.openbmc.cronus", "Brkpt");

    std::array<uint32_t, 3> params{i_proc, i_core, i_thread};
    msg.append(params);

    msg.signal_send();
}

} // namespace attn
