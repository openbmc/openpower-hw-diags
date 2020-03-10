#include <analyzer/analyzer_main.hpp>
#include <attention.hpp>
#include <bp_handler.hpp>
#include <logging.hpp>
#include <ti_handler.hpp>

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <vector>

namespace attn
{

/**
 * @brief Handle SBE vital attention
 *
 * @param i_attention Attention object
 * @return 0 = success
 */
int handleVital(Attention* i_attention);

/**
 * @brief Handle checkstop attention
 *
 * @param i_attention Attention object
 * @return 0 = success
 */
int handleCheckstop(Attention* i_attention);

/**
 * @brief Handle special attention
 *
 * @param i_attention Attention object
 * @return 0 = success
 */
int handleSpecial(Attention* i_attention);

/**
 * @brief The main attention handler logic
 *
 * @param i_breakpoints true = breakpoint special attn handling enabled
 */
void attnHandler(const bool i_vital, const bool i_checkstop,
                 const bool i_terminate, const bool i_breakpoints)
{
    // Vector of active attentions to be handled
    std::vector<Attention> active_attentions;

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
            ss << "checking processor " << proc << std::endl;
            log<level::INFO>(ss.str().c_str());

            // get active attentions on processor
            if (0 != fsi_read(target, 0x1007, &isr_val))
            {
                std::stringstream ss; // log message stream
                ss << "Error! cfam read 0x1007 FAILED" << std::endl;
                log<level::INFO>(ss.str().c_str());
            }
            else
            {
                std::stringstream ss; // log message stream
                ss << "cfam 0x1007 = 0x";
                ss << std::hex << std::setw(8) << std::setfill('0');
                ss << isr_val << std::endl;
                log<level::INFO>(ss.str().c_str());

                // get interrupt enabled special attentions mask
                if (0 != fsi_read(target, 0x100d, &isr_mask))
                {
                    std::stringstream ss; // log message stream
                    ss << "Error! cfam read 0x100d FAILED" << std::endl;
                    log<level::INFO>(ss.str().c_str());
                }
                else
                {
                    std::stringstream ss; // log message stream
                    ss << "cfam 0x100d = 0x";
                    ss << std::hex << std::setw(8) << std::setfill('0');
                    ss << isr_mask << std::endl;
                    log<level::INFO>(ss.str().c_str());

                    // bit 0 on "left": bit 30 = SBE vital attention
                    if (isr_val & isr_mask & 0x00000002)
                    {
                        active_attentions.emplace_back(
                            AttentionType::Vital,
                            static_cast<int>(AttentionType::Vital), handleVital,
                            target, i_vital, i_checkstop, i_terminate,
                            i_breakpoints);
                    }

                    // bit 0 on "left": bit 1 = checkstop
                    if (isr_val & isr_mask & 0x40000000)
                    {
                        active_attentions.emplace_back(
                            AttentionType::Checkstop,
                            static_cast<int>(AttentionType::Checkstop),
                            handleCheckstop, target, i_vital, i_checkstop,
                            i_terminate, i_breakpoints);
                    }

                    // bit 0 on "left": bit 2 = special attention
                    if (isr_val & isr_mask & 0x20000000)
                    {
                        active_attentions.emplace_back(
                            AttentionType::Special,
                            static_cast<int>(AttentionType::Special),
                            handleSpecial, target, i_vital, i_checkstop,
                            i_terminate, i_breakpoints);
                    }
                } // cfam 0x100d valid
            }     // cfam 0x1007 valid
        }         // fsi target enabled
    }             // next processor

    // convert to heap, highest priority is at front
    if (!std::is_heap(active_attentions.begin(), active_attentions.end()))
    {
        std::make_heap(active_attentions.begin(), active_attentions.end());
    }

    // call the attention handler until one is handled or all were attempted
    while (false == active_attentions.empty())
    {
        // handle highest priority attention, done if successful
        if (0 == active_attentions.front().handle())
        {
            break;
        }

        // move attention to back of vector
        std::pop_heap(active_attentions.begin(), active_attentions.end());

        // remove attention from vector
        active_attentions.pop_back();
    }
}

/**
 * @brief Handle SBE vital attention
 */
int handleVital(Attention* i_attention)
{
    int rc;

    std::stringstream ss; // log message stream
    ss << "vital" << std::endl;
    log<level::INFO>(ss.str().c_str());

    // if vital handling enabled, handle vital attention
    if (0 == (i_attention->getFlags() & enableVital))
    {
        std::stringstream ss; // log message stream
        ss << "vital handling disabled" << std::endl;
        log<level::INFO>(ss.str().c_str());
        rc = 1;
    }
    else
    {
        std::stringstream ss; // log message stream
        ss << "vital NOT handled" << std::endl;
        log<level::INFO>(ss.str().c_str());
        rc = 1;
    }

    return rc;
}

/**
 * @brief Handle checkstop attention
 */
int handleCheckstop(Attention* i_attention)
{
    int rc;

    std::stringstream ss; // log message stream
    ss << "checkstop" << std::endl;
    log<level::INFO>(ss.str().c_str());

    // if checkstop handling enabled, handle checkstop attention
    if (0 == (i_attention->getFlags() & enableCheckstop))
    {
        std::stringstream ss; // log message stream
        ss << "Checkstop handling disabled" << std::endl;
        log<level::INFO>(ss.str().c_str());
        rc = 1;
    }
    else
    {
        analyzer::analyzeHardware();
        rc = 0;
    }

    return rc;
}

/**
 * @brief Handle special attention
 */
int handleSpecial(Attention* i_attention)
{
    int rc = 1; // assume both TI and breakpoint handling disabled

    std::stringstream ss; // log message stream
    ss << "special" << std::endl;

    // get enablei/disable status for TI and breakpoints handling
    bool terminate_enabled =
        (0 != (i_attention->getFlags() & enableTerminate) ? true : false);
    bool breakpoints_enabled =
        (0 != (i_attention->getFlags() & enableBreakpoints) ? true : false);

    // Until the special attention chipop is availabe we will treat the special
    // attention as a TI. If TI handling is disabled we will treat the special
    // attention as a breakpopint.

    // if either TI or breakponts handling is enabled
    if (!((false == breakpoints_enabled) && (false == terminate_enabled)))
    {
        // TI attention gets priority over breakpoints, if enabled then handle
        if (true == terminate_enabled)
        {
            ss << "TI (terminate immediately)" << std::endl;
            log<level::INFO>(ss.str().c_str());

            // Call TI special attention handler
            tiHandler();
            rc = 0;
        }
        else
        {
            // TI handling was disabled so handle breakpoint
            if (true == breakpoints_enabled)
            {
                ss << "breakpoint" << std::endl;
                log<level::INFO>(ss.str().c_str());

                // Call the breakpoint special attention handler
                bpHandler();
                rc = 0;
            }
        }
    }
    if (0 != rc)
    {
        std::stringstream ss; // log message stream
        ss << "Special attn handling disabled" << std::endl;
        log<level::INFO>(ss.str().c_str());
    }

    return rc;
}

} // namespace attn
