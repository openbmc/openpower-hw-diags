#pragma once
#include <bitset>

namespace attn
{

/** @brief configuration flags */
enum AttentionFlag
{
    enVital       = 0,
    enCheckstop   = 1,
    enTerminate   = 2,
    enBreakpoints = 3,
    dfltTi        = 4,
    enClrAttnIntr = 5,
    lastFlag
};

/** @brief Objhects to hold configuration data */
class Config
{
  public: // methods
    /** @brief Default constructor */
    Config();

    /** @brief Default destructor */
    ~Config() = default;

    /** @brief Get state of flag */
    bool getFlag(AttentionFlag i_flag) const;

    /** @brief Set configuration flag */
    void setFlag(AttentionFlag i_flag);

    /** @brief Set all configuration flags */
    void setFlagAll();

    /** @brief Clear configuration flag */
    void clearFlag(AttentionFlag i_flag);

    /** @brief Clear all configuration flags */
    void clearFlagAll();

  private:
    std::bitset<lastFlag> iv_flags; // configuration flags
};

} // namespace attn
