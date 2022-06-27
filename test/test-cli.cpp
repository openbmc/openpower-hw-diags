#include <cli.hpp>

#include "gtest/gtest.h"

using namespace attn;

TEST(TestCli, TestCliOptAll)
{
    // Test --all on options
    Config* config = new Config();
    char* argv[2];
    int i     = 0;
    argv[i++] = (char*)"--all";
    argv[i++] = (char*)"on";
    parseConfig(argv, argv + i, config);
    EXPECT_EQ(true, config->getFlag(AttentionFlag::enVital));
    EXPECT_EQ(true, config->getFlag(AttentionFlag::enCheckstop));
    EXPECT_EQ(true, config->getFlag(AttentionFlag::enTerminate));
    EXPECT_EQ(true, config->getFlag(AttentionFlag::enBreakpoints));
    // The dfltTi flag is cleared by default and is not updated in the function.
    EXPECT_EQ(false, config->getFlag(AttentionFlag::dfltTi));
    EXPECT_EQ(true, config->getFlag(AttentionFlag::enClrAttnIntr));

    // Test --all off options
    // Set the dfltTi flag to default value.
    config->clearFlag(dfltTi);
    i         = 0;
    argv[i++] = (char*)"--all";
    argv[i++] = (char*)"off";
    parseConfig(argv, argv + i, config);
    EXPECT_EQ(false, config->getFlag(AttentionFlag::enVital));
    EXPECT_EQ(false, config->getFlag(AttentionFlag::enCheckstop));
    EXPECT_EQ(false, config->getFlag(AttentionFlag::enTerminate));
    EXPECT_EQ(false, config->getFlag(AttentionFlag::enBreakpoints));
    // The same, the dfltTi flag is not updated in the function.
    EXPECT_EQ(false, config->getFlag(AttentionFlag::dfltTi));
    EXPECT_EQ(false, config->getFlag(AttentionFlag::enClrAttnIntr));
    delete config;
}

TEST(TestCli, TestCliNonAll)
{
    // Test options with on switch.
    Config* config = new Config();
    char* argv[11];
    int i     = 0;
    argv[i++] = (char*)"--vital";
    argv[i++] = (char*)"on";
    argv[i++] = (char*)"--checkstop";
    argv[i++] = (char*)"on";
    argv[i++] = (char*)"--terminate";
    argv[i++] = (char*)"on";
    argv[i++] = (char*)"--breakpoints";
    argv[i++] = (char*)"on";
    argv[i++] = (char*)"--clrattnintr";
    argv[i++] = (char*)"on";
    // The --defaultti option does not have on/off switch.
    // If this option is specified, it is enabled.
    argv[i++] = (char*)"--defaultti";

    parseConfig(argv, argv + i, config);
    EXPECT_EQ(true, config->getFlag(AttentionFlag::enVital));
    EXPECT_EQ(true, config->getFlag(AttentionFlag::enCheckstop));
    EXPECT_EQ(true, config->getFlag(AttentionFlag::enTerminate));
    EXPECT_EQ(true, config->getFlag(AttentionFlag::enBreakpoints));
    EXPECT_EQ(true, config->getFlag(AttentionFlag::enClrAttnIntr));
    EXPECT_EQ(true, config->getFlag(AttentionFlag::dfltTi));

    // Test options with off switch.
    // Set the dfltTi flag to default value.
    config->clearFlag(dfltTi);
    i         = 0;
    argv[i++] = (char*)"--vital";
    argv[i++] = (char*)"off";
    argv[i++] = (char*)"--checkstop";
    argv[i++] = (char*)"off";
    argv[i++] = (char*)"--terminate";
    argv[i++] = (char*)"off";
    argv[i++] = (char*)"--breakpoints";
    argv[i++] = (char*)"off";
    argv[i++] = (char*)"--clrattnintr";
    argv[i++] = (char*)"off";

    parseConfig(argv, argv + i, config);
    EXPECT_EQ(false, config->getFlag(AttentionFlag::enVital));
    EXPECT_EQ(false, config->getFlag(AttentionFlag::enCheckstop));
    EXPECT_EQ(false, config->getFlag(AttentionFlag::enTerminate));
    EXPECT_EQ(false, config->getFlag(AttentionFlag::enBreakpoints));
    EXPECT_EQ(false, config->getFlag(AttentionFlag::enClrAttnIntr));
    // If the --defaultti option is not specified, it is disabled by default.
    EXPECT_EQ(false, config->getFlag(AttentionFlag::dfltTi));
    delete config;
}
