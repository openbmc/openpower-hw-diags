#include <attn/attention.hpp>
#include <attn/attn_common.hpp>
#include <attn/attn_config.hpp>

#include "gtest/gtest.h"

using namespace attn;

/** @brief global function to be called. */
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
// attention type
attn::Attention::AttentionType gType = attn::Attention::AttentionType::Special;
// handler function
int (*gHandler)(attn::Attention*) = &handleAttention;
// handler function target
pdbg_target* gTarget;
// configuration flags
Config* gConfig;

/** @brief Fixture class for TEST_F(). */
class AttentionTest : public testing::Test
{
  public:
    AttentionTest() {}

    void SetUp()
    {
        pAttn = std::make_unique<attn::Attention>(
            attn::Attention(gType, gHandler, gTarget, gConfig));
    }

    void TearDown() {}

    std::unique_ptr<attn::Attention> pAttn;
};

TEST_F(AttentionTest, TestAttentionClass)
{
    EXPECT_EQ(0, pAttn->getPriority());
    EXPECT_EQ(attn::RC_SUCCESS, pAttn->handle());

    // TODO: will test using these functions.
    // pdbg_target* target;
    // Config* config;
    // target = pAttn->getTarget();
    // config = pAttn->getConfig();
}