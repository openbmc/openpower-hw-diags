#include <attn_handler.hpp>
#include <spattn.hpp>

#include <libpdbg.h>

#include <sdbusplus/bus.hpp>

/* Attention handler */
void attnHandler()
{
    printf("*** Attention Handler started\n");

    uint32_t cfam_isr, cfam_isr_mask;

    // expand dtb to fdt and setup lipdbg targets
    pdbg_targets_init(nullptr);

    // loop through processors
    // TODO: Add logic to check all processor and prioritize attentions
    pdbg_target *target;
    pdbg_for_each_class_target("fsi", target)
    {
        uint32_t proc = pdbg_target_index(target); // get processor number

        printf ("checking processor %u\n", proc);

        // get active attentions on processor
        cfamReadProc(target, 0x1007, cfam_isr);
        cfamReadProc(target, 0x100d, cfam_isr_mask);

        // only consider "true" attentions
        uint32_t cfam_isr_true = (cfam_isr & cfam_isr_mask);

        // bit 30 = SBE vital attention (bit 0 is on the "left")
        if ( cfam_isr_true & 0x00000002 )
        {
            if ( 0 == handleVital(target) ) return;
        }

        // bit 1 = checkstop (bit 0 is on the "left")
        if ( cfam_isr_true & 0x40000000 )
        {
            if ( 0 == handleCheckstop(target) ) return;
        }

        // bit 2 = special attention (bit 0 is on the "left")
        if ( cfam_isr_true & 0x20000000 )
        {
            if ( 0 == handleSpattn(target) ) return;
        }

        // bit 3 = recoverable error (bit 0 is on the "left")
        if ( cfam_isr_true & 0x10000000 )
        {
            if ( 0 == handleRecoverable(target) ) return;
        }
    }

    return; // checked all processors
}


/* Handle pending vital attention */
int handleVital(pdbg_target * i_target)
{
    printf("vital: ");
    (nullptr != i_target) ? printf("%s\n", pdbg_target_path(i_target)) :
                            printf("invalid FSI target\n");

    return 1; // not handled
}


/* Handle pending checkstop attention */
int handleCheckstop(pdbg_target *i_target)
{
    printf("chkstop: ");
    (nullptr != i_target) ? printf("%s\n", pdbg_target_path(i_target)) :
                            printf("invalid FSI target\n");

    return 1; // not handled
}


/* Handle recoverable error */
int handleRecoverable(pdbg_target *i_target)
{
    printf("recoverable: ");
    (nullptr != i_target) ? printf("%s\n", pdbg_target_path(i_target)) :
                            printf("invalid FSI target\n");

    return 1; // not handled
}


/* @brief Send processor, core and thread to Cronus */
void notifyCronus(uint32_t proc, uint32_t core, uint32_t thread)
{
    printf("notifying cronus  p:%u c:%u t:%u ...\n", proc, core, thread);

    auto bus = sdbusplus::bus::new_default_system();
    auto msg = bus.new_signal("/", "org.openbmc.cronus", "Brkpt");

    std::array<uint32_t, 3> params{proc, core, thread};
    msg.append(params);

    msg.signal_send();
}


/* Scom read to a specific core */
int scomReadCore(pdbg_target *i_target, uint32_t i_core,
                 uint64_t i_address, uint64_t &o_data)
{
    // assume scom read not successful
    int rc = 1;
    o_data = 0xffffffffffffffff;

    i_address |= (i_core << 24); // arrange core number into address

    // scom read first pib
    pdbg_target *pib_target;
    pdbg_for_each_target("pib", i_target, pib_target)
    {
        if ( PDBG_TARGET_ENABLED == pdbg_target_probe(pib_target) )
        {
            rc = pib_read(pib_target, i_address, &o_data);
            printf("scom read  0x%016" PRIx64 " = 0x%016" PRIx64 "\n",
                    i_address, o_data);
            if (rc)
            {
                printf("! scom read FAILED\n");
            }
        }
        else
        {
            printf("! scom target %s DISABLED\n", pdbg_target_path(pib_target));
        }

        break; // target 1 pib only
    }

    return rc;
}


/* Scom write to a specific core */
int scomWriteCore(pdbg_target *i_target, uint32_t i_core,
                  uint64_t i_address, uint64_t i_data)
{
    int rc = 1; // assume scom write unsuccessful

    i_address |= (i_core << 24);

    // scom write first pib
    pdbg_target *pib_target;
    pdbg_for_each_target("pib", i_target, pib_target)
    {
        if ( PDBG_TARGET_ENABLED == pdbg_target_probe(pib_target) )
        {
            printf("scom write 0x%016" PRIx64 " = 0x%016" PRIx64 "\n",
                    i_address, i_data);
            rc = pib_write(pib_target, i_address, i_data);
            if (rc)
            {
                printf("! scom write FAILED\n");
            }
        }
        else
        {
            printf("! scom target %s DISABLED\n", pdbg_target_path(pib_target));
        }

        break; // target 1 pib only
    }

    return rc;
}


/* Read CFAM of a specific processor */
int cfamReadProc(pdbg_target *i_target, uint32_t i_address, uint32_t &o_data)
{
    // assume cfam read not successful
    int rc = 1;
    o_data = 0xffffffff;

    if ( PDBG_TARGET_ENABLED == pdbg_target_probe(i_target) )
    {
        rc = fsi_read(i_target, i_address, &o_data);
        printf("cfam read  0x%08x = 0x%08x\n", i_address, o_data);
        if (rc)
        {
            printf("! cfam read FAILED\n");
        }
    }
    else
    {
        printf("! cfam target %s DISABLED\n", pdbg_target_path(i_target));
    }

    return rc;
}


/* Write CFAM of a specific processor */
int cfamWriteProc(pdbg_target *i_target, uint32_t i_address, uint32_t i_data)
{
    int rc = 1; // assume cfam write unsuccessful

    if ( PDBG_TARGET_ENABLED == pdbg_target_probe(i_target) )
    {
        printf("cfam write 0x%08x = 0x%08x\n", i_address, i_data);
        rc = fsi_write(i_target, i_address, i_data);
        if (rc)
        {
            printf("! cfam write FAILED\n");
        }
    }
    else
    {
        printf("! cfam target %s DISABLED\n", pdbg_target_path(i_target));
    }

    return rc;
}
