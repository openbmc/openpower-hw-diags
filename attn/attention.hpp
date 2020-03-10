#pragma once

#include <libpdbg.h>

#include <attn/attn_config.hpp>

#include <bitset>

namespace attn
{

/** @brief types of attentions to be handled (by priority low to high) */
enum class AttentionType
{
    Special,
    Checkstop,
    Vital
};

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
    /** @brief Default constructor */
    Attention() = delete;

    /** @brief Create attention object
     *
     * Create attention object for each active attention
     *
     * @param i_type     Attention type
     * @param i_priority Attention handling priority
     * @param i_target   Target with the active attention
     * @param i_config   Attention handler configuration data
     */
    Attention(AttentionType i_type, int i_priority,
              int (*i_handler)(Attention*), pdbg_target* i_target,
              Config* i_config);

    /** @brief Destructor */
    ~Attention() = default;

    /** @brief Get attention priority */
    int getPriority() const;

    /* @brief Get config object */
    Config* getConfig() const;

    /* @brief Call attention handler function */
    int handle();

    /** @brief Copy constructor. */
    Attention(const Attention&) = default;

    /** @brief Assignment operator. */
    Attention& operator=(const Attention&) = default;

    /** @brief less than operator */
    bool operator<(const Attention& right) const;

  private:
    AttentionType iv_type;         // attention type
    int iv_priority;               // attention priority
    int (*iv_handler)(Attention*); // handler function
    pdbg_target* iv_target;        // handler function target
    Config* iv_config;             // configuration flags
};

} // namespace attn
