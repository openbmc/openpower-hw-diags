#include <attn/attention.hpp>
#include <attn/attn_dbus.hpp>
#include <cli.hpp>
#include <util/dbus.hpp>

#include <algorithm>
#include <string>

// forward reference (?)
namespace attn
{
int handleCheckstop(Attention* attention);
}

using namespace attn;

/** @brief Parse command line for configuration flags */
int attnDebug(int argc, char* argv[])
{
    if (true != getCliOption(argv, argv + argc, "--util"))
    {
        return 1;
    }

    if (true == getCliOption(argv, argv + argc, "checkstop"))
    {
        printf("checkstop\n");
        Attention::AttentionType type{Attention::Checkstop};
        Config config{};
        Attention attention{type, nullptr, nullptr, &config};
        handleCheckstop(&attention);
        return 0;
    }

    if (true == getCliOption(argv, argv + argc, "hbti"))
    {
        printf("hbti - not yet\n");
        return 0;
    }

    if (true == getCliOption(argv, argv + argc, "phypti"))
    {
        printf("phypti - not yet\n");
        return 0;
    }

    return 0;
}
