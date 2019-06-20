//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wunused-variable"
//#pragma GCC diagnostic ignored "-Wunused-parameter"
//#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

#include <attn_handler.hpp>

#include <libpdbg.h>

#include <vector>

// libpdbg backend engine selection
static constexpr pdbg_backend backend = PDBG_BACKEND_FAKE;
static constexpr char *backend_option = nullptr;

/**
 * @brief Data and methods for processor-level attention activity
 *
 * @param i_target pointer to processor target
 *
 */
class ProcData
{
  public: // methods

    // constructor
    ProcData(pdbg_target *i_target)
    {
        target = i_target;

        fsi_read(target, CFAM_ISR_VAL, &cfam_isr);
        fsi_read(target, CFAM_ISR_MASK, &cfam_true);

        uint32_t attn_true = (cfam_isr & cfam_true);

        if (CFAM_ISR_SBE & attn_true)
        {
            attn_vital = true;
        }
        if (CFAM_ISR_CHKSTOP & attn_true)
        {
            attn_chkstop = true;
        }
        if (CFAM_ISR_SATTN & attn_true)
        {
            attn_special = true;
        }
        if (CFAM_ISR_RECOV & attn_true)
        {
            attn_recoverable = true;
        }
    }

    // return pdbg target
    pdbg_target *getTarget()
    {
        return target;
    }

  public: // data
    bool attn_vital = false;
    bool attn_chkstop = false;
    bool attn_special = false;
    bool attn_recoverable = false;

  private: // data
    pdbg_target *target;
    uint32_t cfam_isr;
    uint32_t cfam_true;
};


/**
 * @brief Data and methods for core-level attention activity
 *
 * @param i_target pointer to core target
 *
 */
class CoreData
{
  public: // methods
    CoreData(pdbg_target *i_target)
    {
        target = i_target;
        uint32_t core_pos = (pdbg_target_index(target) << 28);

        attn_reg  |= core_pos;
        mask_reg  |= core_pos;
        state_reg |= core_pos;
    }

    // return pdbg target
    pdbg_target *getTarget()
    {
        return target;
    }

  private: // data
    pdbg_target *target;
    uint32_t attn_reg =  SCOM_ATTN_STATUS_REG;
    uint32_t mask_reg =  SCOM_ATTN_MASK_REG;
    uint32_t state_reg = SCOM_CORE_STATE_REG;
};


/**
 * @brief Data and methods for thread-level attention activity
 *
 * @param i_target pointer to thread target
 *
 */
class ThrdData
{
  public: // methods
    ThrdData(pdbg_target *i_target)
    {
        target = i_target;
        uint32_t index = pdbg_target_index(target);

        instr_stop      = ((1 << INSTR_STOP_BIT)        << (index * 4));
        attn_complete   = ((1 << ATTN_COMPLETE_BIT)     << (index * 4));
        recov_handshake = ((1 << RECOV_HANDSHAKE_BIT)   << (index * 4));
        core_code       = ((1 << CORE_CODE_BIT)         << (index * 4));
    }

    // return pdbg target
    pdbg_target *getTarget()
    {
        return target;
    }

  private: // data
    pdbg_target *target;

    // status isolation bits for this thread
    uint32_t instr_stop;
    uint32_t attn_complete;
    uint32_t recov_handshake;
    uint32_t core_code;
};


/**
 * @brief Parse fdt and intitialize each targets associated attention data
 *
 * @param o_proc_data reference to vector of processor attention objects
 * @param o_core_data reference to vector of core attention objects
 * @param o_thrd_data reference to vector of thread attention objects
 *
 */
void initTargets(std::vector<ProcData> &o_proc_data, \
                 std::vector<CoreData> &o_core_data, \
                 std::vector<ThrdData> &o_thrd_data)
{
    pdbg_target *proc, *core, *thrd;

    pdbg_for_each_target("fsi", pdbg_target_root(), proc)
    {
        o_proc_data.push_back(ProcData(proc));

        pdbg_for_each_target("core", proc, core)
        {
            o_core_data.push_back(CoreData(core));

            pdbg_for_each_target("thread", core, thrd)
            {
                o_thrd_data.push_back(ThrdData(thrd));
            }

        }
    }
    return;
}


/**
 * @brief Handle pending vital attention
 *
 * @param pointer to processor target
 *
 */
void handleVital(pdbg_target * i_target)
{
    printf("vital: %s\n", pdbg_target_path(i_target));
    // todo - what now :)
}


/**
 * @brief Handle pending checkstop attention
 *
 * @param pointer to processor target
 *
 */
void handleChkstop(pdbg_target * i_target)
{
    printf("chkstop: %s\n", pdbg_target_path(i_target));
    // todo - what now :)
}


/**
 * @brief Handle special attetion
 *
 * @param pointer to processor target
 *
 */
void handleSpecial(pdbg_target * i_target)
{
    printf("special: %s\n", pdbg_target_path(i_target));
    // todo - what now :)
}


/**
 * @brief Handle recoverable error attention
 *
 * @param pointer to processor target
 *
 */
void handleRecov(pdbg_target * i_target)
{
    printf("recoverable: %s\n", pdbg_target_path(i_target));
    // todo - what now :)
}


/**
 * @brief Handle other/unknown attention
 *
 */
void handleOther()
{
    printf("unknown reason\n");
    // todo - what now :)
}


/**
 * @brief Attention handler
 *
 * This is the base logic for the attention handler application.
 *
 */
void attnHandler()
{
    bool attn_handled = false; // not yet handled

    // sets libpdbg::pdbg_backend and libpdbg::pdbg_backend_option
    pdbg_set_backend(backend, backend_option);

    // expand dtb to fdt and setup lipdbg targets
    pdbg_targets_init(NULL);

    // processor, core and thread vectors
    std::vector<ProcData> proc_data;
    std::vector<CoreData> core_data;
    std::vector<ThrdData> thrd_data;

    // processor, core and thread iterators
    std::vector<ProcData>::iterator it_proc;
    std::vector<CoreData>::iterator it_core;
    std::vector<ThrdData>::iterator it_thrd;

    // setup attention targets
    initTargets(proc_data, core_data, thrd_data);

    // handle sbe vital attentions
    for (it_proc = proc_data.begin(); it_proc != proc_data.end(); it_proc++)
    {
        if ((*it_proc).attn_vital)
        {
            handleVital((*it_proc).getTarget());
            attn_handled = true;
        }
    }

    // handle checkstops
    for (it_proc = proc_data.begin(); it_proc != proc_data.end(); it_proc++)
    {
        if ((*it_proc).attn_chkstop)
        {
            handleChkstop((*it_proc).getTarget());
            attn_handled = true;
        }
    }

    // handle special attention
    for (it_proc = proc_data.begin(); it_proc != proc_data.end(); it_proc++)
    {
        if ((*it_proc).attn_special)
        {
            handleSpecial((*it_proc).getTarget());
            attn_handled = true;
        }
    }

    // handle recoverable error
    for (it_proc = proc_data.begin(); it_proc != proc_data.end(); it_proc++)
    {
        if ((*it_proc).attn_recoverable)
        {
            handleRecov((*it_proc).getTarget());
            attn_handled = true;
        }
    }

    // handle unknown reason
    if (!attn_handled)
        handleOther(); // unknown reason

    return;
}

//#pragma GCC diagnostic pop
