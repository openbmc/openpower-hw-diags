#include <attn_handler.hpp>
#include <spattn.hpp>

#include <libpdbg.h>

#include <sdbusplus/bus.hpp>

namespace attn
{

/**
 * @brief These objects contain information about an active attention.
 */
class Attention
{
  public: // Constructors, destructor, assignment, etc.

    /** @brief Default constructor. */
    Attention() = delete;

    /** @brief Main constructor. */
    Attention(int i_type, uint8_t i_priority,
              int (*i_handler)(pdbg_target*), pdbg_target* i_target) :
              iv_type(i_type), iv_priority(i_priority),
              iv_handler(i_handler), iv_target(i_target)
    {
    }

    /** @brief Destructor. */
    ~Attention() = default;

    /** @brief Get attention priority */
    int getPriority() const
    {
        return iv_priority;
    }

    /** @brief Increment attention priority */
    int incPriority(uint i_inc = 1)
    {
        if (0 != i_inc)
        {
            iv_priority += i_inc;
        }
        return iv_priority;
    }

    /** @brief Decrement attention priority */
    int decPriority(uint i_dec = 1)
    {
        if (0 != i_dec)
        {
            iv_priority -= i_dec;
        }
        return iv_priority;
    }

    /* @brief Call attention handler function */
    int handle()
    {
        return iv_handler(iv_target);
    }

    /** @brief Copy constructor. */
    Attention(const Attention&) = default;

    /** @brief Assignment operator. */
    Attention & operator=(const Attention&) = default;

  private:
    int iv_type;                     // attention type
    int iv_priority;                 // attention priority
    int (*iv_handler)(pdbg_target*); // handler function
    pdbg_target* iv_target;          // handler function target
}; // end class Attentiona

} // namespace attn

using namespace attn;

/**
 * @brief Handle pending vital attention
 *
 * @param i_target FSI target
 *
 * @return non-zero = error
 */
int handleVital(pdbg_target * i_target)
{
    printf("vital: ");
    (nullptr != i_target) ? printf("%s\n", pdbg_target_path(i_target)) :
                            printf("invalid FSI target\n");

    return 1; // not handled
}

/**
 * @brief Handle pending checkstop attention
 *
 * @param i_target FSI target
 *
 * @return non-zero = error
 */
int handleCheckstop(pdbg_target *i_target)
{
    printf("chkstop: ");
    (nullptr != i_target) ? printf("%s\n", pdbg_target_path(i_target)) :
                            printf("invalid FSI target\n");

    return 1; // not handled
}

/**
 * @brief Handle recoverable error
 *
 * @param i_target FSI target
 *
 * @return non-zero = error
 */
int handleRecoverable(pdbg_target *i_target)
{
    printf("recoverable: ");
    (nullptr != i_target) ? printf("%s\n", pdbg_target_path(i_target)) :
                            printf("invalid FSI target\n");

    return 1; // not handled
}

/** @brief Priority "less than" function (used for creating heap) */
struct priorityLessThan
{
    bool operator() (const Attention& left, const Attention& right) const
    {
        return (left.getPriority() < right.getPriority());
    }
};

/**
 * @brief Gather active attentions
 *
 * @param active_attentions Vectory to hold active attentions
 */
void attnParser(std::vector<Attention>& active_attentions)
{
    uint32_t cfam_isr, cfam_isr_mask;

    // debug - use this for qemu testing
    //pdbg_set_backend(PDBG_BACKEND_FAKE, nullptr);

    // expand dtb to fdt and setup lipdbg targets
    pdbg_targets_init(nullptr);

    // loop through processors
    pdbg_target *target;
    pdbg_for_each_class_target("fsi", target)
    {
        uint32_t proc = pdbg_target_index(target); // get processor number

        printf ("checking processor %u\n", proc);

        // Get active attentions on processor. If unable to read the
        // necessary cfam registers then continue to try next processor.
        if ((0 != cfamReadProc(target, 0x1007, cfam_isr)) ||
           (0 != cfamReadProc(target, 0x100d, cfam_isr_mask)))
        {
            continue;
        }

        // Only consider "true" active attentions
        uint32_t cfam_isr_true = (cfam_isr & cfam_isr_mask);

        // Create vector of active attentions
        if (0 != cfam_isr_true)
        {
            // bit 30 = SBE vital attention (bit 0 is on the "left")
            if (cfam_isr_true & 0x00000002)
            {
                active_attentions.emplace_back(ATTN_TYPE_VITAL,
                ATTN_PRIORITY_VITAL, handleVital, target);
            }

            // bit 1 = checkstop (bit 0 is on the "left")
            if (cfam_isr_true & 0x40000000)
            {
                active_attentions.emplace_back(ATTN_TYPE_CHECKSTOP,
                ATTN_PRIORITY_CHECKSTOP, handleCheckstop, target);
            }

            // bit 2 = special attention (bit 0 is on the "left")
            if (cfam_isr_true & 0x20000000)
            {
                active_attentions.emplace_back(ATTN_TYPE_SPECIAL,
                ATTN_PRIORITY_SPECIAL, handleSpattn, target);
            }

            // bit 3 = recoverable error (bit 0 is on the "left")
            if (cfam_isr_true & 0x10000000)
            {
                active_attentions.emplace_back(ATTN_TYPE_RECOVERABLE,
                ATTN_PRIORITY_RECOVERABLE, handleRecoverable, target);
            }
        }
    }
    return;
}

/* Attention handler */
int attnHandler()
{
    printf("*** Attention Handler started\n");

    bool handled = false; // attention not handled

    // Vector of active attentions to be handled
    std::vector<Attention> active_attentions;

    // Gather active attentions
    attnParser(active_attentions);

    // Create heap from active attention vector. The "less than" values
    // will be moved to the back of the heap
    std::make_heap(active_attentions.begin(), active_attentions.end(),
              priorityLessThan());

    // Handle highest priority attention
    while ((0 == active_attentions.empty()) && (false == handled))
    {
        // Highest priorty attentions are at the front of the heap
        if (0 == active_attentions.front().handle())
        {
           handled = true; // handled
        }

        // Move highest priority value to back of the heap
        std::pop_heap(active_attentions.begin(), active_attentions.end(),
            priorityLessThan());

        // Remove the highest priority value from the back of the heap
        active_attentions.pop_back();
    }

    return (true == handled) ? 0 : 1;
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
