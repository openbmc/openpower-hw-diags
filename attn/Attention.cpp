#include <Attention.hpp>

namespace attn
{

// absolute value macro
#define ABS(N) ((N < 0) ? (-N) : (N))

/** @brief Main constructor. */
Attention::Attention(AttentionType i_type, int i_priority,
                     int (*i_handler)(Attention*), pdbg_target* i_target,
                     bool i_breakpoints) :
    iv_type(i_type),
    iv_priority(i_priority), iv_handler(i_handler), iv_target(i_target)

{
    // set attention handler configuration flags
    if (true == i_breakpoints)
    {
        iv_flags |= enableBreakpoints;
    }
}

/** @brief Get attention priority */
int Attention::getPriority() const
{
    return iv_priority;
}

/** @brief Set attention priority */
void Attention::setPriority(int i_priority)
{
    iv_priority = i_priority;
}

/** @brief Enable handling of this attention event */
void Attention::enable()
{
    iv_priority = ABS(iv_priority);
}

/** @brief Disble handling of this attention event */
void Attention::disable()
{
    iv_priority = -(ABS(iv_priority));
}

/** @brief Get enabled status of this attention event */
bool Attention::isEnabled()
{
    return iv_priority >= 0 ? true : false;
}

/** @brief Get configuration flags */
uint32_t Attention::getFlags()
{
    return iv_flags;
}

/** @brief Set configuration flags */
void Attention::setFlags(uint32_t i_flags)
{
    iv_flags = i_flags;
}

/* @brief Call attention handler function */
int Attention::handle()
{
    return iv_handler(this);
}

} // namespace attn
