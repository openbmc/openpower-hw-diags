#pragma once

#include <gpiod.h>

#include <boost/asio/io_service.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>

namespace attn
{

/**
 *  @brief Responsible for monitoring attention GPIO state change
 */
class AttnMonitor
{
  public:
    AttnMonitor()  = delete;
    ~AttnMonitor() = default;

    /** @brief Constructs AttnMonitor object.
     *
     * The AttnMonitor constructor will create a new object and start
     * the objects associated GPIO listener.
     *
     * @param line     GPIO line handle
     * @param config   configuration of line
     * @param io       io service
     * @param i_vital       true = vital attn handling enabled
     * @param i_checkstop   true = checkstop attn handling enabled
     * @param i_terminate   true = TI attn handling enabled
     * @param i_breakpoints true = breakpoint attn handling enabled
     */
    AttnMonitor(gpiod_line* line, gpiod_line_request_config& config,
                boost::asio::io_service& io, bool i_vital, bool i_checkstop,
                bool i_terminate, bool i_breakpoints) :
        iv_gpioLine(line),
        iv_gpioConfig(config), iv_gpioEventDescriptor(io), iv_vital(i_vital),
        iv_checkstop(i_checkstop), iv_terminate(i_terminate),
        iv_breakpoints(i_breakpoints)
    {

        requestGPIOEvent(); // registers the event handler
    }

    // delete copy constructor
    AttnMonitor(const AttnMonitor&) = delete;

    // delete assignment operator
    AttnMonitor& operator=(const AttnMonitor&) = delete;

    // delere move copy consructor
    AttnMonitor(AttnMonitor&&) = delete;

    // delete move assignment operator
    AttnMonitor& operator=(AttnMonitor&&) = delete;

  private: // instance variables
    /** @brief gpiod handle to gpio line */
    gpiod_line* iv_gpioLine;

    /** @brief gpiod line config data */
    gpiod_line_request_config iv_gpioConfig;

    /** @brief GPIO event descriptor */
    boost::asio::posix::stream_descriptor iv_gpioEventDescriptor;

  private: // class methods
    /** @brief schedule a gpio event handler */
    void scheduleGPIOEvent();

    /** @brief handle the GPIO event */
    void handleGPIOEvent();

    /** @brief register for a gpio event */
    void requestGPIOEvent();

    /** @brief attention handler configuration flags */
    bool iv_vital;       // enable vital attention handling
    bool iv_checkstop;   // enable checkstop handling
    bool iv_terminate;   // enable TI jhandling
    bool iv_breakpoints; // enable breakpoint handling
};

} // namespace attn
