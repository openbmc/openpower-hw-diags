#pragma once
#include <bitset>

namespace attn
{

/** @brief configuration flags */
enum AttentionFlag
{
    enVital,
    enCheckstop,
    enTerminate,
    enBreakpoints,
    lastFlag
};

/** @brief Objhects to hold configuration data */
class Config
{
  public: // methods
    /** @brief Default constructor */
    Config() = delete;

    /** @brief Crate configuration object
     *
     * Create a configuration object to hold the attention handler
     * configuration data
     *
     * @param i_vital       Enable vital attention handling
     * @param i_checkstop   Enable checkstop attention handling
     * @param i_terminate   Enable TI attention handling
     * @param i_+breakpoint Enable breakpoint attention handling
     */
    Config(bool i_vital, bool i_checkstop, bool i_terminate,
           bool i_breakpoints);

    /** @brief Default destructor */
    ~Config() = default;

    bool getFlag(AttentionFlag i_flag) const;

    void setFlag(AttentionFlag i_flag);

  private:
    std::bitset<lastFlag> iv_flags; // configuration flags
};

} // namespace attn
