#include <attn_handler.hpp>

#include <libpdbg.h>

#include <sdbusplus/bus.hpp>

/**
 * @brief Scom read to a specific core
 *
 * @param i_core    Core we are targeting
 * @param i_address Scom address
 * @param o_data    Data read by Scom
 */
int scomReadCore(uint32_t i_proc, uint32_t i_core,
                 uint64_t i_address, uint64_t &o_data)
{
    // assume scom read not successful
    int rc = 1;
    o_data = 0xffffffffffffffff;

    i_address |= (i_core << 24); // arrange core number into address

    // find fsi target for processor
    pdbg_target * fsi_target;
    pdbg_for_each_class_target("fsi", fsi_target)
    {
        if (i_proc == pdbg_target_index(fsi_target))
            break;
    }

    // scom read first pib
    pdbg_target *pib_target;
    pdbg_for_each_target("pib", fsi_target, pib_target)
    {
        if (PDBG_TARGET_ENABLED == pdbg_target_probe(pib_target))
        {
            rc = pib_read(pib_target, i_address, &o_data);
            printf("scom read  0x%016" PRIx64 " = 0x%016" PRIx64 "\n",
                    i_address, o_data);
            if (rc)
                printf("! scom read FAILED\n");
        }
        else
            printf("! scom target %s DISABLED\n", pdbg_target_path(pib_target));

        break; // target 1 pib only
    }

    return rc;
}


/**
 * @brief Scom write to a specific core
 *
 * @param i_core    Core we are targeting
 * @param i_address Scom address
 * @param i_data    Data to write by Scom
 */
int scomWriteCore(uint32_t i_proc, uint32_t i_core,
                  uint64_t i_address, uint64_t i_data)
{
    int rc = 1; // assume scom write unsuccessful

    i_address |= (i_core << 24);

    // find fsi target for processor
    pdbg_target * fsi_target;
    pdbg_for_each_class_target("fsi", fsi_target)
    {
        if (i_proc == pdbg_target_index(fsi_target))
            break;
    }

    // scom write first pib
    pdbg_target *pib_target;
    pdbg_for_each_class_target("pib", pib_target)
    {
        if (PDBG_TARGET_ENABLED == pdbg_target_probe(pib_target))
        {
            printf("scom write 0x%016" PRIx64 " = 0x%016" PRIx64 "\n",
                    i_address, i_data);
            rc = pib_write(pib_target, i_address, i_data);
            if (rc)
                printf("! scom write FAILED\n");
        }
        else
            printf("! scom target %s DISABLED\n", pdbg_target_path(pib_target));

        break; // target 1 pib only
    }

    return rc;
}


/**
 * @brief Read CFAM of a specific processor
 *
 * @param i_proc    Processor we are targeting
 * @param i_address CFAM address
 * @param o_data    CFAM data read
 */
int cfamReadProc(uint32_t i_proc, uint32_t i_address, uint32_t &o_data)
{
    // assume cfam read not successful
    int rc = 1;
    o_data = 0xffffffff;

    pdbg_target *fsi_target;
    pdbg_for_each_class_target("fsi", fsi_target)
    {
        if (i_proc == pdbg_target_index(fsi_target))
            break;
    }

    if (PDBG_TARGET_ENABLED == pdbg_target_probe(fsi_target))
    {
        rc = fsi_read(fsi_target, i_address, &o_data);
        printf("cfam read  0x%08x = 0x%08x\n", i_address, o_data);
        if (rc)
            printf("! cfam read FAILED\n");
    }
    else
        printf("! cfam target %s DISABLED\n", pdbg_target_path(fsi_target));

    return rc;
}


/**
 * @brief Write CFAM of a specific processor
 *
 * @param i_proc    Processor we are targeting
 * @param i_address CFAM address
 * @param i_data    CFAM data to write
 */
int cfamWriteProc(uint32_t i_proc, uint32_t i_address, uint32_t i_data)
{
    int rc = 1; // assume cfam write unsuccessful

    pdbg_target *fsi_target;
    pdbg_for_each_class_target("fsi", fsi_target)
    {
        if (i_proc == pdbg_target_index(fsi_target))
            break;
    }

    if (PDBG_TARGET_ENABLED == pdbg_target_probe(fsi_target))
    {
        printf("cfam write 0x%08x = 0x%08x\n", i_address, i_data);
        rc = fsi_write(fsi_target, i_address, i_data);
        if (rc)
            printf("! cfam write FAILED\n");
    }
    else
        printf("! cfam target %s DISABLED\n", pdbg_target_path(fsi_target));

    return rc;
}


/**
 * @brief Update core and thread based on core being fused/unfused
 *
 * @param o_core   Core number if core fused/unfused
 * @param o_thread Thread number if core fused/unfused
 */
int xlateCoreThread(uint32_t i_proc, uint32_t &o_core, uint32_t &o_thread)
{
    uint64_t core_state;

    printf("translating core and thread (c:%u t:%u) ...\n", o_core, o_thread);

    if (scomReadCore(i_proc, o_core, 0x200a0ab3, core_state))
        return 1;

    // bit 63 set = fused core, adjust thread and core
    if (core_state & 0x0000000000000001)
    {
        (o_thread % 2) ? (o_core |= 1ul) : (o_core &= ~1ul);

        o_thread >>= 1ul;

        printf("fused core found, core and thread may have changed\n");
    }

    return 0;
}


/**
 * @brief Send processor, core and thread to Cronus
 *
 * @param i_proc   Processor number with Special attention
 * @param i_core   Core number with special attention
 * @param i_thread Thread number with special attention
 */
void notifyCronus(uint32_t proc, uint32_t core, uint32_t thread)
{
    printf("notifying cronus  p:%u c:%u t:%u ...\n", proc, core, thread);

    auto bus = sdbusplus::bus::new_default_system();
    auto msg = bus.new_signal("/", "org.openbmc.cronus", "Brkpt");

    std::array<uint32_t, 3> params{proc, core, thread};
    msg.append(params);

    msg.signal_send();
}


/**
 * @brief Get core, thread and type of special attention
 *
 * @param o_proc   Processor with special attention
 * @param o_core   Core with special attention
 * @param o_thread Thread with special attention
 * @param o_type   Special attention type
 */
int parseSattn(uint32_t proc, uint32_t &o_core,
               uint32_t &o_thread, uint32_t &o_type)
{
    uint64_t global_sattn_reg, sattn_data, sattn_mask, sattn_active;

    // currently we are only handling Cronus breakpoint special attention
    o_type = SATTN_TYPE_CRONUSBP;

    // Scom read the global special attention register
    if (scomReadCore(proc, 0, 0x500f001a, global_sattn_reg))
    {
        printf("! ABORT failed to read global special attention register\n");
        return 1;
    }

    // Scan up to 24 bits for core with special attention
    uint64_t core_bit = (0x8000000000000000 >> 0x20); // start at offset 0x20
    for (o_core = 0; o_core < 24; o_core++)
    {
        if (global_sattn_reg & core_bit)
            break;
        core_bit >>= 1;
    }

    if (o_core == 24)
    {
        printf("! no special attention core found, setting core = 4\n");
        o_core = 4;
    }
    else
        printf("special attention on core %u\n", o_core);

    // Scom read the data and mask for the core
    if (scomReadCore(proc, o_core, 0x20010a99, sattn_data))
        return 1;
    if (scomReadCore(proc, o_core, 0x20010a9a, sattn_mask))
        return 1;

    // get attention complete on thread bits
    sattn_active = (sattn_data & ~sattn_mask);

    // scan up to 8 threads to for first attn complete
    uint64_t attn_complete = (0x4000000000000000);
    for (o_thread = 0; o_thread < 8; o_thread++)
    {
        if (sattn_active & (attn_complete >> (o_thread * 4)))
            break;
    }

    // thread not found
    if (o_thread == 8)
    {
        printf("! no attention complete found, setting thread = 0\n");
        o_thread = 0;
    }
    else
        printf("attention complete on thread %u\n", o_thread);

    return 0;
}


/**
 * @brief Clear special attention status
 *
 * @param i_proc   Processor number to target
 * @param i_core   Core number to target
 * @param i_thread Thread number to target
 */
int clearSattn(uint32_t i_proc, uint32_t i_core, uint32_t i_thread)
{
    uint64_t scom_data;
    uint32_t cfam_data;

    printf("clearing special attention ...\n");

    // read the thread status data
    if (scomReadCore(i_proc, i_core, 0x20010a99, scom_data))
        return 1;

    // clear attention complete bit
    uint64_t attn_complete = (0x4000000000000000);
    scom_data &= ~(attn_complete >> (i_thread * 4));

    if (scomWriteCore(i_proc, i_core, 0x20010a99, scom_data))
        return 1;

    // Clear interrupt on processor
    if (scomReadCore(i_proc, 0, 0x500f001a, scom_data))
        return 1;

    // Clear core special attention bit
    uint64_t core_bit = (0x8000000000000000 >> 0x20); // start at offset 0x20
    scom_data &= ~(core_bit >> i_core);
    if (scomWriteCore(i_proc, 0, 0x000f001a, scom_data))
        return 1;

    // clear special attention on processor)
    if (cfamReadProc(i_proc, 0x1007, cfam_data))
        return 1;
    cfam_data &= ~0x20000000;
    if (cfamWriteProc(i_proc, 0x100b, cfam_data))
        return 1;

    return 0;
}


/**
 * @brief Get processor, core, thread and type of special attention via chipop
 *
 * @param o_proc   Processor with special attention
 * @param o_core   Core with special attention
 * @param o_thread Thread with special attention
 * @paran o_type   Special attention type
 */
int sattnChipop(uint32_t &o_proc, uint32_t &o_core, uint32_t &o_thread,
                uint32_t &o_type)
{
    o_proc = o_core = o_thread = 0;
    o_type = SATTN_TYPE_UNKNOWN;

    return 1; // not implmeneted
}


/**
 * A special attention is acive so Determine the processor, core, thread and
 * type of the special attention. If the special attention is Cronus breakpoint
 * then notify Cronus with the processor, core and thread. Otherwise ... TBD
 *
 * @param i_target Processor target (libpdbg "fsi" target)
 */
int handleSpecial(uint32_t i_proc, bool i_vital)
{
    uint32_t core, thread, sattn_type;

    printf("special attention processor %u\n", i_proc);

    // If the SBE is reporting a vital attention or the SBE chip op is not
    // successful then determine the processor, core and thread directly.
    if ((false == i_vital) ||
        (0 != sattnChipop(i_proc, core, thread, sattn_type)))
    {
        // determin core, thread and type of special attention
        if (parseSattn(i_proc, core, thread, sattn_type))
            return 1;

        // clear special attention status bits
        clearSattn(i_proc, core, thread); // debug
        if (clearSattn(i_proc, core, thread))
            return 1;

        // translate core and thread based on fused/unfused core
        xlateCoreThread(i_proc, core, thread); // debug
        if (xlateCoreThread(i_proc, core, thread))
            return 1;
    }

    // notify cronus of breakpoint
    if (SATTN_TYPE_CRONUSBP == sattn_type)
        notifyCronus(i_proc, core, thread);

    return 0; // handled
}


/**
 * @brief Handle pending checkstop attention
 *
 * @param i_target Processor target (libpdbg "fsi" target)
 */
int handleCheckstop(pdbg_target * i_target)
{
    printf("chkstop: %s\n", pdbg_target_path(i_target));
    return 1; // not handled
}


/**
 * @brief Handle pending vital attention
 *
 * @param i_target Processor target (libpdbg "fsi" target)
 */
int handleVital(pdbg_target * i_target)
{
    printf("vital: %s\n", pdbg_target_path(i_target));
    return 1; // not handled
}


/**
 * @brief Attention handler
 *
 * Check each processor for active attentions of type SBE Vital (vital),
 * System Checkstop (checkstop) and Special Attention (special) and handle
 * each as follows:
 *
 * checkstop: Notify the hardware diagnostics application which will ... TBD
 * vital:     Make note for use in special attention handling and ... TBD
 * special:   Determine if the special attention is a Breakpoint (BP),
 *            Terminate Immediately (TI) or CoreCodeToSp (corecode). For each
 *            special attention type, do the following:
 *
 *            BP: clear the attention status and notify Cronus
 *            TI: write associated memory dump to a file
 *            Corecode: Clear attention status.
 */
void attnHandler()
{
    printf("*** Attention Handler started\n");

    uint32_t cfam_isr, cfam_isr_mask;

    bool have_vital = false; // no SBE vital attention

    // expand dtb to fdt and setup lipdbg targets
    pdbg_targets_init(nullptr);

    // loop through processors looking for active attentions
    pdbg_target *target;
    pdbg_for_each_class_target("fsi", target)
    {
        uint32_t proc = pdbg_target_index(target); // get processor number

        printf ("checking processor %u\n", proc);

        // get active attentions on processor
        cfamReadProc(proc, 0x1007, cfam_isr);
        cfamReadProc(proc, 0x100d, cfam_isr_mask);

        // only consider "true" attentions
        uint32_t cfam_isr_true = (cfam_isr & cfam_isr_mask);

        // bit 0 on "left": bit 30 = SBE vital attention
        if (cfam_isr_true & 0x00000002)
        {
            have_vital = true;

            if (0 == handleVital(target))
                return;
        }

        // bit 0 on "left": bit 1 = checkstop
        if (cfam_isr_true & 0x40000000)
        {
            if (0 == handleCheckstop(target))
                return;
        }

        // bit 0 on "left": bit 2 = special attention
        if (cfam_isr_true & 0x20000000)
        {
            if (0 == handleSpecial(proc, have_vital))
                return;
        }

        // bit 0 on "left": bit 3 = recoverable error - should be masked ?
        if (cfam_isr_true & 0x10000000)
            printf("recoverable error on processor %u\n", proc);
    }

    return; // checked all processors
}
