#include <libpdbg.h>

#include <analyzer/analyzer_main.hpp>
#include <attn/attention.hpp>
#include <attn/attn_config.hpp>
#include <attn/attn_handler.hpp>
#include <attn/attn_main.hpp>
#include <buildinfo.hpp>
#include <cli.hpp>
#include <util/dbus.hpp>

/**
 * @brief Attention handler application main()
 *
 * This is the main interface to the hardware diagnostics application. This
 * application can be loaded as a daemon for monitoring the attention
 * gpio or it can be loaded as an application to analyze hardware and
 * diagnose hardware error conditions.
 *
 *     Usage:
 *        --analyze:              Analyze the hardware
 *        --daemon:               Start the attention handler daemon
 *
 * @return 0 = success
 */
int main(int argc, char* argv[])
{
    int rc = 0; // assume success

    if (argc == 1)
    {
        printf("openpower-hw-diags <options>\n");
        printf("options:\n");
        printf("  --analyze:              Analyze the hardware\n");
        printf("  --daemon:               Start the attn handler daemon\n");
        printf("hwdiag: %s, hei: %s\n", BUILDINFO, analyzer::getBuildInfo());
    }
    else
    {
        // Pdbg targets should only be initialized once according to
        // libpdbg documentation. Initializing them here will make sure
        // they are initialized for the attention handler, invocation of
        // the analyzer via attention handler and direct invocation of
        // the analyzer via command line (--analyze).

        pdbg_targets_init(nullptr); // nullptr == use default fdt

        // Either analyze (application mode) or daemon mode
        if (true == getCliOption(argv, argv + argc, "--analyze"))
        {
            rc = analyzer::analyzeHardware(); // analyze hardware
        }
        // temp
        else if (true == getCliOption(argv, argv + argc, "--util"))
        {
            if (true == getCliOption(argv, argv + argc, "hostrunningstate"))
            {
                util::dbus::HostRunningState runningState =
                    util::dbus::hostRunningState();

                if (util::dbus::HostRunningState::Started == runningState)
                {
                    printf("hostRunningState: STARTED\n");
                }
                if (util::dbus::HostRunningState::NotStarted == runningState)
                {
                    printf("hostRunningState: NOTSTARTED\n");
                }
                if (util::dbus::HostRunningState::Unknown == runningState)
                {
                    printf("hostRunningState: UNKNOWN\n");
                }
            }
            if (true == getCliOption(argv, argv + argc, "transitionhost"))
            {
                util::dbus::transitionHost(util::dbus::HostState::Quiesce);
            }
            if (true == getCliOption(argv, argv + argc, "autoreboot"))
            {
                auto autoreboot = util::dbus::autoRebootEnabled();
                if (true == autoreboot)
                {
                    printf("AutoRebootEnabled = TRUE\n");
                }
                else
                {
                    printf("AutoRebootEnabled = FALSE\n");
                }
            }
        }
        // daemon mode
        else
        {
            if (true == getCliOption(argv, argv + argc, "--daemon"))
            {
                attn::Config attnConfig; // default config

                // convert remaining cmd line args to config values
                parseConfig(argv, argv + argc, &attnConfig);

                attn::attnHandler(&attnConfig); // handle pending attentions

                attn::attnDaemon(&attnConfig); // start daemon
            }
        }
    }

    return rc;
}
