#include <attn/attn_config.hpp>

namespace attn
{
/** @brief Main constructor */
Config::Config(bool i_vital, bool i_checkstop, bool i_terminate,
               bool i_breakpoints)
{
    (true == i_vital) ? iv_flags.set(enVital) : iv_flags.reset(enVital);

    (true == i_checkstop) ? iv_flags.set(enCheckstop)
                          : iv_flags.reset(enCheckstop);

    (true == i_terminate) ? iv_flags.set(enTerminate)
                          : iv_flags.reset(enTerminate);

    (true == i_breakpoints) ? iv_flags.set(enBreakpoints)
                            : iv_flags.reset(enBreakpoints);
}

bool Config::getFlag(AttentionFlag i_flag) const
{
    return (iv_flags.test(i_flag));
}

void Config::setFlag(AttentionFlag i_flag)
{
    iv_flags.set(i_flag);
}

} // namespace attn
