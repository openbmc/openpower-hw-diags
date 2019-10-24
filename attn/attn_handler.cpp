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
              bool (*i_handler)(pdbg_target*, bool&), pdbg_target* i_target) :
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
        return iv_handler(iv_target, iv_remove);
    }

    /** @brief Copy constructor. */
    Attention(const Attention&) = default;

    /** @brief Assignment operator. */
    Attention & operator=(const Attention&) = default;

    /** @brief Less than operator */
    bool operator<(const Attention& attention) const
    {
        return (getPriority() < attention.getPriority());
    }

    /** @brief Comparison operator */
    bool operator==(const Attention& attention) const
    {
        return ((iv_type == attention.iv_type) &&
                (iv_target == attention.iv_target));
    }

    /** @brief Get remove flag */
    bool getRemove()
    {
        return iv_remove;
    }

    /** @brief Set remove flag */
    void setRemove(bool i_remove)
    {
        iv_remove = i_remove;
    }

  private:
    int iv_type;                             // attention type
    int iv_priority;                         // attention priority
    bool (*iv_handler)(pdbg_target*, bool&); // handler function
    pdbg_target* iv_target;                  // handler function target
    bool iv_remove = true;                   // remove from heap

}; // end class Attentiona

} // namespace attn

using namespace attn;

/**
 * @brief Handle pending vital attention
 *
 * @param i_target FSI target
 *
 * @return true = handler another attention
 */
bool handleVital(pdbg_target * i_target, bool& o_remove)
{
    int handleNext = false;

    printf("vital: ");
    (nullptr != i_target) ? printf("%s\n", pdbg_target_path(i_target)) :
                            printf("invalid FSI target\n");

    o_remove = true;

    return handleNext;
}

/**
 * @brief Handle pending checkstop attention
 *
 * @param i_target FSI target
 *
 * @return non-zero = error
 */

bool handleCheckstop(pdbg_target *i_target, bool& o_remove)
{
    bool handleNext = false;

    printf("chkstop: ");
    (nullptr != i_target) ? printf("%s\n", pdbg_target_path(i_target)) :
                            printf("invalid FSI target\n");

    o_remove = true;

    return handleNext;
}

/**
 * @brief Handle recoverable error
 *
 * @param i_target FSI target
 *
 * @return non-zero = error
 */
bool handleRecoverable(pdbg_target *i_target, bool& o_remove)
{
    int handleNext = false;

    printf("recoverable: ");
    (nullptr != i_target) ? printf("%s\n", pdbg_target_path(i_target)) :
                            printf("invalid FSI target\n");

    o_remove = true;

    return handleNext;
}

/**
 * @brief Gather active attentions
 *
 * Check each processor for active attentions and store an
 * attention objects in an array for all active attentions
 * found.
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
                Attention newAttention = Attention(ATTN_TYPE_VITAL,
                    ATTN_PRIORITY_VITAL, handleVital, target);

                if (std::find(active_attentions.begin(),
                              active_attentions.end(),
                              newAttention) ==
                    active_attentions.end())
                {
                    active_attentions.push_back(newAttention);
                }
            }

            // bit 1 = checkstop (bit 0 is on the "left")
            if (cfam_isr_true & 0x40000000)
            {
                Attention newAttention = Attention(ATTN_TYPE_CHECKSTOP,
                    ATTN_PRIORITY_CHECKSTOP, handleCheckstop, target);

                if (std::find(active_attentions.begin(),
                              active_attentions.end(),
                              newAttention) ==
                    active_attentions.end())
                {
                    active_attentions.push_back(newAttention);
                }
            }

            // bit 2 = special attention (bit 0 is on the "left")
            if (cfam_isr_true & 0x20000000)
            {
                Attention newAttention = Attention(ATTN_TYPE_SPECIAL,
                    ATTN_PRIORITY_SPECIAL, handleSpattn, target);

                if (std::find(active_attentions.begin(),
                              active_attentions.end(),
                              newAttention) ==
                    active_attentions.end())
                {
                    active_attentions.push_back(newAttention);
                }
            }

            // bit 3 = recoverable error (bit 0 is on the "left")
            if (cfam_isr_true & 0x10000000)
            {
                Attention newAttention = Attention(ATTN_TYPE_RECOVERABLE,
                    ATTN_PRIORITY_RECOVERABLE, handleRecoverable, target);

                if (std::find(active_attentions.begin(),
                              active_attentions.end(),
                              newAttention) ==
                    active_attentions.end())
                {
                    active_attentions.push_back(newAttention);
                }
            }
        }
    }
    return;
}

/**
 * @brief Handler for active attentions
 *
 * Gather active attentions, prioritize them in a heap and then
 * handle the highest priority attention.
 *
 * Each attention object on the heap is by default removed from the heap after
 * an attempt to handle it has occurred. An object can be marked to remain on
 * heap if it needs to be re-visited for some reason.
 *
 * After an attention was handled (or not) the attn handler can decide whether
 * to continue handling other attentions on the heap or exit.
 *
 * Before exiting, the attn handler can choose whether or not to have the gpio
 * monitor coninue to respond to attention gpio events. Also before exiting
 * the attn handler can choose to continue checking the processors for
 * attention events without returning control to the gpio monitor.
 *
 * Additionally each attention object on the heap can have its priority
 * increased or decreased as needed.
 *
 * @return true = continue monitoring attention gpio
 */
bool attnHandler()
{
    printf("*** Attention Handler started\n");

    bool pollForAttentions = false; // do not return control to gpio monitor
    bool continueMonitoring = true; // continue monitoring attn gpio
    bool handleAnother = true;      // handle another attentions on the heap

    // Vector of active attentions to be handled
    std::vector<Attention> active_attentions;

    do {
        // Gather active attentions
        attnParser(active_attentions);

        // Create heap from active attention vector. The "less than" values
        // will be moved to the back of the heap
        std::make_heap(active_attentions.begin(), active_attentions.end());

        // Handle highest attention
        while ((0 == active_attentions.empty()) && (true  == handleAnother))
        {
            // Handle highest priorty attention
            handleAnother = active_attentions.front().handle();

            // Move attention to back of the heap
            std::pop_heap(active_attentions.begin(), active_attentions.end());

            // Remove the highest priority value from the back of the heap
            if (true == active_attentions.front().getRemove())
            {
                active_attentions.pop_back();
            }
        }
    } while (true == pollForAttentions); // return control to gpio monitor?

    return continueMonitoring; // continue monitoring gpio pin?
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
