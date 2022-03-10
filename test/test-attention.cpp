#include <attn/attention.hpp>
#include <attn/attn_common.hpp>
#include <attn/attn_config.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

#include "gtest/gtest.h"

using namespace attn;

/** @brief global function to be called back. */
int handleAttention(attn::Attention* attention)
{
    int rc = attn::RC_SUCCESS;
    if (attention != nullptr)
    {
        return rc;
    }
    else
    {
        return attn::RC_NOT_HANDLED;
    }
}

// Global variables
// Attention type
attn::Attention::AttentionType gType = attn::Attention::AttentionType::Special;
// pointer to handler callback function
int (*gHandler)(attn::Attention*)   = &handleAttention;
const char* gPosPath                = "/proc0/pib/perv12";
const uint32_t gPos                 = 12;
const attn::AttentionFlag gAttnFlag = attn::AttentionFlag::enBreakpoints;

/** @brief Fixture class for TEST_F(). */
class AttentionTest : public testing::Test
{
  public:
    AttentionTest() {}

    void SetUp()
    {
        pdbg_targets_init(nullptr);

        target = pdbg_target_from_path(nullptr, gPosPath);
        EXPECT_NE(nullptr, target);

        config = new Config;
        EXPECT_NE(nullptr, config);

        pAttn = std::make_unique<attn::Attention>(
            attn::Attention(gType, gHandler, target, config));
    }

    void TearDown()
    {
        delete config;
    }

    std::unique_ptr<attn::Attention> pAttn;
    Config* config      = nullptr;
    pdbg_target* target = nullptr;
};

TEST_F(AttentionTest, TestAttentionFixture)
{
    EXPECT_EQ(0, pAttn->getPriority());
    EXPECT_EQ(attn::RC_SUCCESS, pAttn->handle());

    // Verify the global target_tmp.
    EXPECT_NE(nullptr, target);
    uint32_t attr = std::numeric_limits<uint32_t>::max();
    pdbg_target_get_attribute(target, "ATTR_FAPI_POS", 4, 1, &attr);
    EXPECT_EQ(gPos, attr);

    // Verify the target in Attention object.
    attr                    = std::numeric_limits<uint32_t>::max();
    pdbg_target* target_tmp = pAttn->getTarget();
    EXPECT_NE(nullptr, target_tmp);
    pdbg_target_get_attribute(target_tmp, "ATTR_FAPI_POS", 4, 1, &attr);
    EXPECT_EQ(gPos, attr);

    // Verify the glbal config.
    EXPECT_EQ(true, config->getFlag(gAttnFlag));

    // Verify the config in Attention object.
    Config* config_tmp = pAttn->getConfig();
    EXPECT_NE(nullptr, config_tmp);
    EXPECT_EQ(true, config_tmp->getFlag(gAttnFlag));
    config_tmp->clearFlag(gAttnFlag);
    EXPECT_EQ(false, config_tmp->getFlag(gAttnFlag));
    config_tmp->setFlag(gAttnFlag);
    EXPECT_EQ(true, config_tmp->getFlag(gAttnFlag));
}
