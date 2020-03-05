#pragma once

#include <libpdbg.h>

namespace attn
{

/** @brief types of attentions to be handled */
enum class AttentionType
{
    Vital,
    Checkstop,
    Special
};

/** @brief attention handler configuration flags */
inline constexpr uint32_t enableBreakpoints = (1 << 0);

/** @brief attention priority default values
 *
 * The choice of actual numerical value is arbitrary and just sets a
 * priority order of high to low. There is a gap (25 points) between these
 * default groupings so that a priority can be adjusted up to increase
 * priority while remaining within the group. There is no real enforcement
 * of these values or groupings and any attention can be assigned any prioritoy
 * value.
 * */
inline constexpr int VitalPriorityDefault     = 75;
inline constexpr int CheckstopPriorityDefault = 50;
inline constexpr int SpecialPriorityDefault   = 25;

/**
 * @brief These objects contain information about an active attention.
 *
 * An Attention object is created for each active attention. These objects
 * carry with them various configuration and status information as well
 * the attention handler function to call for handling the attention. Each
 * Attention object also carries a priority value. This priority is used
 * to determine which attention event(s) to handle when there are more than
 * one active event.
 */
class Attention
{
  public: // Constructors, destructor, assignment, etc.
    /** @brief Default constructor. */
    Attention() = delete;

    /** @brief Main constructors */
    Attention(AttentionType i_type, int i_priority,
              int (*i_handler)(Attention*), pdbg_target* i_target,
              bool i_breakpoints);

    /** @brief Destructor */
    ~Attention() = default;

    /** @brief Get attention priority */
    int getPriority() const;

    /** @brief Set attention priority */
    void setPriority(int i_priority);

    /** @brief Enable handling of this attention event */
    void enable();

    /** @brief Disble handling of this attention event */
    void disable();

    /** @brief Get enabled status of this attention event */
    bool isEnabled();

    /** @brief Get configuration flags */
    uint32_t getFlags();

    /** @brief Set configuration flags */
    void setFlags(uint32_t i_flags);

    /* @brief Call attention handler function */
    int handle();

    /** @brief Copy constructor. */
    Attention(const Attention&) = default;

    /** @brief Assignment operator. */
    Attention& operator=(const Attention&) = default;

  private:
    AttentionType iv_type;         // attention type
    int iv_priority;               // attention priority
    int (*iv_handler)(Attention*); // handler function
    pdbg_target* iv_target;        // handler function target
    uint32_t iv_flags = 0;         // configuration flags

}; // end class Attentiona

} // namespace attn
