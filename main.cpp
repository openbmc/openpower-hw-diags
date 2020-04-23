#include <analyzer/analyzer_main.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <cli.hpp>
#include <listener.hpp>

/**
 * @brief Attention handler application main()
 *
 * This is the main interface to the hardware diagnostics application. This
 * application will either be loaded as an application (--analyze) to analyze
 * hardware or as a daemon (--daemon) for monitoring and handling the attention
 * GPIO. The running daemon accepts commands to configure the GPIO monitor
 * (--start, --stop), to configure the attention handler (--vital, --checkstop,
 * --terminate, --breakpoints, --all) and to stop running (--close).
 *
 *     Usage:
 *        --analyze              Analyze the hardware
 *        --daemon               Start the listener daemon
 *        --start                Start the attention GPIO monitor
 *        --stop                 Stop the attention GPIO monitor
 *        --vital       <on|off> Vital attention handling
 *        --checkstop   <on|off> Checkstop attention handling
 *        --terminate   <on|off> TI attention handling
 *        --breakpoints <on|off> Breakpoint attention handling
 *        --all         <on|off> All attention handling
 *        --close                Stop the listener daemon
 *
 *     Example: openpower-hw-diags --start --vital off
 *
 * @return 0 = success
 */
int main(int argc, char* argv[])
{
    int rc = 0; // assume success

    using namespace boost::interprocess;

    if (argc == 1)
    {
        printf("openpower-hw-diags <options>\n");
        printf("options:\n");
        printf("  --analyze              Analyze the hardware\n");
        printf("  --daemon               Start the daemon\n");
        printf("When daemon is running:\n");
        printf("  --start                Start the attention gpio monitor\n");
        printf("  --stop                 Stop the attention gpio monitor\n");
        printf("  --vital       <on|off> Vital attention handling\n");
        printf("  --checkstop   <on|off> Checkstop attention handling\n");
        printf("  --terminate   <on|off> TI attention handling\n");
        printf("  --breakpoints <on|off> Breakpoint attention handling\n");
        printf("  --all         <on|off> All attention handling\n");
        printf("  --close                Stop the daemon\n");
    }
    else
    {
        // Either analyze (application mode) or daemon mode
        if (true == getCliOption(argv, argv + argc, "--analyze"))
        {
            analyzer::analyzeHardware();
        }
        // daemon mode
        else
        {
            // assume not starting new daemon
            bool newListener     = false;
            bool listenerStarted = true;

            pthread_t ptidListener; // handle to listener thread

            // start a new daemon
            if (true == getCliOption(argv, argv + argc, "--daemon"))
            {
                newListener     = true;
                listenerStarted = startListener(&ptidListener);
            }

            if (true == listenerStarted)
            {
                // send cmd line to listener thread
                if (argc != sendCmdLine(argc, argv))
                {
                    printf("daemon not running\n");
                    rc = 1;
                }
            }

            // if this is a new listener let it run until "stopped"
            if (true == newListener)
            {
                pthread_join(ptidListener, NULL);
            }
        }
    }
    return rc;
}
