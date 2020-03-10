#include <attention.hpp>
#include <attn_config.hpp>

namespace attn
{

/** @brief Main constructor. */
Attention::Attention(AttentionType i_type, int i_priority,
                     int (*i_handler)(Attention*), pdbg_target* i_target,
                     Config* i_config) :
    iv_type(i_type),
    iv_priority(i_priority), iv_handler(i_handler), iv_target(i_target),
    iv_config(i_config)

{}

/** @brief Get attention priority */
int Attention::getPriority() const
{
    return iv_priority;
}

/* @brief Get config object */
Config* Attention::getConfig() const
{
    return iv_config;
}

/* @brief Call attention handler function */
int Attention::handle()
{
    return iv_handler(this);
}

/** @brief less than operator, for heap creation */
bool Attention::operator<(const Attention& right) const
{
    return (getPriority() < right.getPriority());
}

} // namespace attn
