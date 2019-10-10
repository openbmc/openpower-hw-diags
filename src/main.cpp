#include <iostream>
#include <sdbusplus/server.hpp>
#include <org/open_power/hw/Diags/error.hpp>     // auto generated
#include <org/open_power/hw/Diags/server.hpp>    // auto generated

// using sdbusplus server application bindings support
using Diags_inherit =
    sdbusplus::server::object_t<sdbusplus::org::open_power::hw::server::Diags>;


/**
 * @brief Hardware Diagnostics dbus application support
 *
 */
struct Diags : Diags_inherit
{
    /** Constructor */
    Diags(sdbusplus::bus::bus& bus, const char* path) :
        Diags_inherit(bus, path)
    {
    }

    /** @brief sdbus method handler (checkstop method) */
    int checkstop()
    {
        printf("hardware diagnostics checkstop\n");
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
    constexpr auto path = "/org/open_power/hw/Diags";
    auto bus = sdbusplus::bus::new_default();

    // create an hwdiags application manager on dbus
    sdbusplus::server::manager_t manager{bus, path};

    // register hwdiags application on dbus
    bus.request_name("org.open_power.hw.Diags");

    // attach hwdiags application to dbus manager
    Diags diags{bus, path};

    // wait for hwdiags methods to be called
    while (1)
    {
        bus.process_discard(); // discard unhandled methods
        bus.wait(); // wait for method calls
    }

    return 0;
}
