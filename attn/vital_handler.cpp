#include <attn/attention.hpp>
#include <attn/attn_common.hpp>
#include <attn/attn_dump.hpp>
#include <attn/attn_handler.hpp>
#include <attn/attn_logging.hpp>
#include <sdbusplus/bus.hpp>
#include <util/dbus.hpp>
#include <util/pdbg.hpp>
#include <util/pldm.hpp>
#include <util/trace.hpp>

namespace attn
{
/*
 * @brief Request SBE hreset and try to clear sbe attentions
 *
 * @param[in] sbeInstance - sbe instance to hreset (0 based)
 *
 * @return true if hreset is successful and attentions cleared
 */
bool attemptSbeRecovery(int sbeInstance)
{
    trace::err("BAR0");
    // attempt sbe hreset and attention interrupt clear
    if (!util::pldm::hresetSbe(sbeInstance))
    {
        return false;
    }

    trace::inf("hreset completed");

    // try to clear attention interrupts
    clearAttnInterrupts();

    // loop through processors checking attention interrupts
    bool recovered = true;
    pdbg_target* procTarget;
    pdbg_for_each_class_target("proc", procTarget)
    {
        // active processors only
        if (PDBG_TARGET_ENABLED !=
            pdbg_target_probe(util::pdbg::getPibTrgt(procTarget)))
        {
            continue;
        }

        // get cfam is an fsi read
        pdbg_target* fsiTarget = util::pdbg::getFsiTrgt(procTarget);
        uint32_t int_val;

        // get attention interrupts on processor
        if (RC_SUCCESS == fsi_read(fsiTarget, 0x100b, &int_val))
        {
            if (int_val & SBE_ATTN)
            {
                trace::err("sbe attention did not clear");
                recovered = false;
                break;
            }
        }
        else
        {
            // log cfam read error
            trace::err("cfam read error");
            recovered = false;
            break;
        }
    }

    if (recovered)
    {
        trace::inf("sbe attention cleared");
    }

    return recovered;
}

/**
 * @brief Handle SBE vital attention
 *
 * @param i_attention Attention object
 * @return 0 indicates that the vital attention was successfully handled
 *         1 indicates that the vital attention was NOT successfully handled
 */
int handleVital(Attention* i_attention)
{
    trace::inf("vital handler started");

    // if vital handling disabled
    if (false == (i_attention->getConfig()->getFlag(enVital)))
    {
        trace::inf("vital handling disabled");
        return RC_NOT_HANDLED;
    }

    // wait for power fault handling before committing PEL
    sleepSeconds(POWER_FAULT_WAIT);

    // generate pel - todo add pgood check
    auto pelId = eventVital();

    // if host is running attempt sbei attention recovery
    if (util::dbus::HostRunningState::Started == util::dbus::hostRunningState())
    {
        if (attemptSbeRecovery(pdbg_target_index(i_attention->getTarget())))
        {
            return RC_SUCCESS;
        }
    }

    // host not running or recovery failed
    if (0 != pelId)
    {
        requestDump(pelId, DumpParameters{0, DumpType::SBE});
    }

    util::dbus::transitionHost(util::dbus::HostState::Quiesce);

    return RC_SUCCESS;
}

} // namespace attn
