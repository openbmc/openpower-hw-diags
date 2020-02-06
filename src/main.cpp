#include <../org/open_power/HwDiags/error.hpp>  // auto generated file
#include <../org/open_power/HwDiags/server.hpp> // auto generated file
#include <hei_main.hpp>
#include <sdbusplus/server.hpp>

#include <iostream>

// using sdbusplus server application bindings support
using HwDiags_inherit =
    sdbusplus::server::object_t<sdbusplus::org::open_power::server::HwDiags>;

/**
 * @brief Hardware Diagnostics dbus application support
 *
 */
struct HwDiags : HwDiags_inherit
{
    /** Constructor */
    HwDiags(sdbusplus::bus::bus& bus, const char* path) :
        HwDiags_inherit(bus, path)
    {}

    /** @brief sdbus method handler (checkstop method) */
    int analyzeHardware()
    {
        printf("HwDiags analyzeHardware()\n");
        return 0;
    }

    /** @brief sdbus method handler (checkstop method) */
    int handleAttention()
    {
        printf("HwDiags handleAttention()\n");
        return 0;
    }
};

/**
 * @brief Create, register and start the hwdiags dbus service
 *
 */
int main()
{

    // create sdbus service for hwdiags application
    constexpr auto path = "/org/open_power/HwDiags";
    auto bus            = sdbusplus::bus::new_default();

    // create an hwdiags application manager on dbus
    sdbusplus::server::manager_t manager{bus, path};

    // register hwdiags application on dbus
    bus.request_name("org.open_power.HwDiags");

    // attach hwdiags application to dbus manager
    HwDiags diags{bus, path};

    // wait for hwdiags methods to be called
    while (1)
    {
        bus.process_discard(); // discard unhandled methods
        bus.wait();            // wait for method calls
    }

    return 0;
}
