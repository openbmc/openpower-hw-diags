#include <fmt/format.h>

#include <util/dbus.hpp>
#include <util/trace.hpp>

namespace util
{

namespace dbus
{

MachineType getMachineType()
{
    // default to Rainier 2S4U
    MachineType machineType = MachineType::Rainier_2S4U;

    return machineType;
}

} // namespace dbus

} // namespace util
