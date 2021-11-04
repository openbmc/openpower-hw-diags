#include <stdio.h>

#include <analyzer/resolution.hpp>
#include <util/trace.hpp>

#include <regex>

#include "gtest/gtest.h"

// Chip string
constexpr auto chip_str = "/proc0";

// Unit paths
constexpr auto proc_str   = "";
constexpr auto iolink_str = "pib/perv24/pauc0/iohs0/smpgroup0";
constexpr auto omi_str    = "pib/perv12/mc0/mi0/mcc0/omi0";
constexpr auto ocmb_str   = "pib/perv12/mc0/mi0/mcc0/omi0/ocmb0";
constexpr auto core_str   = "pib/perv39/eq7/fc1/core1";

// Local implementation of this function.
namespace analyzer
{

//------------------------------------------------------------------------------

// Helper function to get the root cause chip target path from the service data.
std::string __getRootCauseChipPath(const ServiceData& i_sd)
{
    return std::string{(const char*)i_sd.getRootCause().getChip().getChip()};
}

//------------------------------------------------------------------------------

// Helper function to get a unit target path from the given unit path, which is
// a devtree path relative the the containing chip. An empty string indicates
// the chip target path should be returned.
std::string __getUnitPath(const std::string& i_chipPath,
                          const std::string& i_unitPath)
{
    auto path = i_chipPath; // default, if i_unitPath is empty

    if (!i_unitPath.empty())
    {
        path += "/" + i_unitPath;
    }

    return path;
}

//------------------------------------------------------------------------------

// Helper function to get the connected target Path on the other side of the
// given bus.
std::tuple<std::string, std::string>
    __getConnectedPath(const std::string& i_rxPath,
                       const callout::BusType& i_busType)
{
    std::string txUnitPath{};
    std::string txChipPath{};

    // Need to get the target type from the RX path.
    const std::regex re{"(/proc0)(.*)/([a-z]+)([0-9]+)"};
    std::smatch match;
    std::regex_match(i_rxPath, match, re);
    assert(5 == match.size());
    std::string rxType = match[3].str();

    if (callout::BusType::SMP_BUS == i_busType && "smpgroup" == rxType)
    {
        // Use the RX unit path on a different processor.
        txUnitPath = "/proc1" + match[2].str() + "/" + rxType + match[4].str();
        txChipPath = "/proc1";
    }
    else if (callout::BusType::OMI_BUS == i_busType && "omi" == rxType)
    {
        // Append the OCMB to the RX path.
        txUnitPath = i_rxPath + "/ocmb0";
        txChipPath = txUnitPath;
    }
    else if (callout::BusType::OMI_BUS == i_busType && "ocmb" == rxType)
    {
        // Strip the OCMB off of the RX path.
        txUnitPath = match[1].str() + match[2].str();
        txChipPath = "/proc0";
    }
    else
    {
        // This would be a code bug.
        throw std::logic_error("Unsupported config: i_rxTarget=" + i_rxPath +
                               " i_busType=" + i_busType.getString());
    }

    assert(!txUnitPath.empty()); // just in case we missed something above

    return std::make_tuple(txUnitPath, txChipPath);
}

//------------------------------------------------------------------------------

void __calloutTarget(ServiceData& io_sd, const std::string& i_locCode,
                     const callout::Priority& i_priority, bool i_guard)
{
    nlohmann::json callout;
    callout["LocationCode"] = i_locCode;
    callout["Priority"]     = i_priority.getUserDataString();
    callout["Deconfigured"] = false;
    callout["Guarded"]      = i_guard;
    io_sd.addCallout(callout);
}

//------------------------------------------------------------------------------

void __calloutBackplane(ServiceData& io_sd, const callout::Priority& i_priority)
{
    // TODO: There isn't a device tree object for this. So will need to hardcode
    //       the location code for now. In the future, we will need a mechanism
    //       to make this data driven.

    nlohmann::json callout;
    callout["LocationCode"] = "P0";
    callout["Priority"]     = i_priority.getUserDataString();
    callout["Deconfigured"] = false;
    callout["Guarded"]      = false;
    io_sd.addCallout(callout);
}

//------------------------------------------------------------------------------

void HardwareCalloutResolution::resolve(ServiceData& io_sd) const
{
    // Get the location code and entity path for this target.
    auto locCode    = __getRootCauseChipPath(io_sd);
    auto entityPath = __getUnitPath(locCode, iv_unitPath);

    // Add the actual callout to the service data.
    __calloutTarget(io_sd, locCode, iv_priority, iv_guard);

    // Add the callout FFDC to the service data.
    nlohmann::json ffdc;
    ffdc["Callout Type"] = "Hardware Callout";
    ffdc["Target"]       = entityPath;
    ffdc["Priority"]     = iv_priority.getRegistryString();
    ffdc["Guard"]        = iv_guard;
    io_sd.addCalloutFFDC(ffdc);
}

//------------------------------------------------------------------------------

void ConnectedCalloutResolution::resolve(ServiceData& io_sd) const
{
    // Get the chip target path from the root cause signature.
    auto chipPath = __getRootCauseChipPath(io_sd);

    // Get the endpoint target path for the receiving side of the bus.
    auto rxPath = __getUnitPath(chipPath, iv_unitPath);

    // Get the endpoint target path for the transfer side of the bus.
    auto txPath = __getConnectedPath(rxPath, iv_busType);

    // Callout the TX endpoint.
    __calloutTarget(io_sd, std::get<1>(txPath), iv_priority, iv_guard);

    // Add the callout FFDC to the service data.
    nlohmann::json ffdc;
    ffdc["Callout Type"] = "Connected Callout";
    ffdc["Bus Type"]     = iv_busType.getString();
    ffdc["Target"]       = std::get<0>(txPath);
    ffdc["Priority"]     = iv_priority.getRegistryString();
    ffdc["Guard"]        = iv_guard;
    io_sd.addCalloutFFDC(ffdc);
}

//------------------------------------------------------------------------------

void BusCalloutResolution::resolve(ServiceData& io_sd) const
{
    // Get the chip target path from the root cause signature.
    auto chipPath = __getRootCauseChipPath(io_sd);

    // Get the endpoint target path for the receiving side of the bus.
    auto rxPath = __getUnitPath(chipPath, iv_unitPath);

    // Get the endpoint target path for the transfer side of the bus.
    auto txPath = __getConnectedPath(rxPath, iv_busType);

    // Callout the RX endpoint.
    __calloutTarget(io_sd, chipPath, iv_priority, iv_guard);

    // Callout the TX endpoint.
    __calloutTarget(io_sd, std::get<1>(txPath), iv_priority, iv_guard);

    // Callout everything else in between.
    // TODO: For P10 (OMI bus and XBUS), the callout is simply the backplane.
    __calloutBackplane(io_sd, iv_priority);

    // Add the callout FFDC to the service data.
    nlohmann::json ffdc;
    ffdc["Callout Type"] = "Bus Callout";
    ffdc["Bus Type"]     = iv_busType.getString();
    ffdc["RX Target"]    = rxPath;
    ffdc["TX Target"]    = std::get<0>(txPath);
    ffdc["Priority"]     = iv_priority.getRegistryString();
    ffdc["Guard"]        = iv_guard;
    io_sd.addCalloutFFDC(ffdc);
}

//------------------------------------------------------------------------------

void ProcedureCalloutResolution::resolve(ServiceData& io_sd) const
{
    // Add the actual callout to the service data.
    nlohmann::json callout;
    callout["Procedure"] = iv_procedure.getString();
    callout["Priority"]  = iv_priority.getUserDataString();
    io_sd.addCallout(callout);

    // Add the callout FFDC to the service data.
    nlohmann::json ffdc;
    ffdc["Callout Type"] = "Procedure Callout";
    ffdc["Procedure"]    = iv_procedure.getString();
    ffdc["Priority"]     = iv_priority.getRegistryString();
    io_sd.addCalloutFFDC(ffdc);
}

//------------------------------------------------------------------------------

void ClockCalloutResolution::resolve(ServiceData& io_sd) const
{
    // Callout the clock target.
    // TODO: For P10, the callout is simply the backplane. Also, there are no
    //       clock targets in the device tree. So at the moment there is no
    //       guard support for clock targets.
    __calloutBackplane(io_sd, iv_priority);

    // Add the callout FFDC to the service data.
    // TODO: Add the target and guard type if guard is ever supported.
    nlohmann::json ffdc;
    ffdc["Callout Type"] = "Clock Callout";
    ffdc["Clock Type"]   = iv_clockType.getString();
    ffdc["Priority"]     = iv_priority.getRegistryString();
    io_sd.addCalloutFFDC(ffdc);
}

//------------------------------------------------------------------------------

} // namespace analyzer

using namespace analyzer;

TEST(Resolution, TestSet1)
{
    // Create a few resolutions
    auto c1 = std::make_shared<HardwareCalloutResolution>(
        proc_str, callout::Priority::HIGH, false);

    auto c2 = std::make_shared<HardwareCalloutResolution>(
        omi_str, callout::Priority::MED_A, true);

    auto c3 = std::make_shared<HardwareCalloutResolution>(
        core_str, callout::Priority::MED, true);

    auto c4 = std::make_shared<ProcedureCalloutResolution>(
        callout::Procedure::NEXTLVL, callout::Priority::LOW);

    auto c5 = std::make_shared<ClockCalloutResolution>(
        callout::ClockType::OSC_REF_CLOCK_1, callout::Priority::LOW, false);

    // l1 = (c1, c2, c5)
    auto l1 = std::make_shared<ResolutionList>();
    l1->push(c1);
    l1->push(c2);
    l1->push(c5);

    // l2 = (c4, c3, c1, c2, c5)
    auto l2 = std::make_shared<ResolutionList>();
    l2->push(c4);
    l2->push(c3);
    l2->push(l1);

    // Get some ServiceData objects
    libhei::Chip chip{chip_str, 0xdeadbeef};
    libhei::Signature sig{chip, 0xabcd, 0, 0, libhei::ATTN_TYPE_CHECKSTOP};
    ServiceData sd1{sig, true};
    ServiceData sd2{sig, false};

    // Resolve
    l1->resolve(sd1);
    l2->resolve(sd2);

    // Start verifying
    nlohmann::json j{};
    std::string s{};

    j = sd1.getCalloutList();
    s = R"([
    {
        "Deconfigured": false,
        "Guarded": false,
        "LocationCode": "/proc0",
        "Priority": "H"
    },
    {
        "Deconfigured": false,
        "Guarded": false,
        "LocationCode": "P0",
        "Priority": "L"
    }
])";
    EXPECT_EQ(s, j.dump(4));

    j = sd2.getCalloutList();
    s = R"([
    {
        "Priority": "L",
        "Procedure": "NEXTLVL"
    },
    {
        "Deconfigured": false,
        "Guarded": true,
        "LocationCode": "/proc0",
        "Priority": "H"
    },
    {
        "Deconfigured": false,
        "Guarded": false,
        "LocationCode": "P0",
        "Priority": "L"
    }
])";
    EXPECT_EQ(s, j.dump(4));
}

TEST(Resolution, HardwareCallout)
{
    auto c1 = std::make_shared<HardwareCalloutResolution>(
        omi_str, callout::Priority::MED_A, true);

    libhei::Chip chip{chip_str, 0xdeadbeef};
    libhei::Signature sig{chip, 0xabcd, 0, 0, libhei::ATTN_TYPE_CHECKSTOP};
    ServiceData sd{sig, true};

    c1->resolve(sd);

    nlohmann::json j{};
    std::string s{};

    // Callout list
    j = sd.getCalloutList();
    s = R"([
    {
        "Deconfigured": false,
        "Guarded": true,
        "LocationCode": "/proc0",
        "Priority": "A"
    }
])";
    EXPECT_EQ(s, j.dump(4));

    // Callout FFDC
    j = sd.getCalloutFFDC();
    s = R"([
    {
        "Callout Type": "Hardware Callout",
        "Guard": true,
        "Priority": "medium_group_A",
        "Target": "/proc0/pib/perv12/mc0/mi0/mcc0/omi0"
    }
])";
    EXPECT_EQ(s, j.dump(4));
}

TEST(Resolution, ConnectedCallout)
{
    auto c1 = std::make_shared<ConnectedCalloutResolution>(
        callout::BusType::SMP_BUS, iolink_str, callout::Priority::MED_A, true);

    auto c2 = std::make_shared<ConnectedCalloutResolution>(
        callout::BusType::OMI_BUS, ocmb_str, callout::Priority::MED_B, true);

    auto c3 = std::make_shared<ConnectedCalloutResolution>(
        callout::BusType::OMI_BUS, omi_str, callout::Priority::MED_C, true);

    libhei::Chip chip{chip_str, 0xdeadbeef};
    libhei::Signature sig{chip, 0xabcd, 0, 0, libhei::ATTN_TYPE_CHECKSTOP};
    ServiceData sd{sig, true};

    nlohmann::json j{};
    std::string s{};

    c1->resolve(sd);
    c2->resolve(sd);
    c3->resolve(sd);

    // Callout list
    j = sd.getCalloutList();
    s = R"([
    {
        "Deconfigured": false,
        "Guarded": true,
        "LocationCode": "/proc1",
        "Priority": "A"
    },
    {
        "Deconfigured": false,
        "Guarded": true,
        "LocationCode": "/proc0",
        "Priority": "B"
    },
    {
        "Deconfigured": false,
        "Guarded": true,
        "LocationCode": "/proc0/pib/perv12/mc0/mi0/mcc0/omi0/ocmb0",
        "Priority": "C"
    }
])";
    EXPECT_EQ(s, j.dump(4));

    // Callout FFDC
    j = sd.getCalloutFFDC();
    s = R"([
    {
        "Bus Type": "SMP_BUS",
        "Callout Type": "Connected Callout",
        "Guard": true,
        "Priority": "medium_group_A",
        "Target": "/proc1/pib/perv24/pauc0/iohs0/smpgroup0"
    },
    {
        "Bus Type": "OMI_BUS",
        "Callout Type": "Connected Callout",
        "Guard": true,
        "Priority": "medium_group_B",
        "Target": "/proc0/pib/perv12/mc0/mi0/mcc0/omi0"
    },
    {
        "Bus Type": "OMI_BUS",
        "Callout Type": "Connected Callout",
        "Guard": true,
        "Priority": "medium_group_C",
        "Target": "/proc0/pib/perv12/mc0/mi0/mcc0/omi0/ocmb0"
    }
])";
    EXPECT_EQ(s, j.dump(4));
}

TEST(Resolution, BusCallout)
{
    auto c1 = std::make_shared<HardwareCalloutResolution>(
        omi_str, callout::Priority::MED_A, true);

    auto c2 = std::make_shared<ConnectedCalloutResolution>(
        callout::BusType::OMI_BUS, omi_str, callout::Priority::MED_A, true);

    auto c3 = std::make_shared<BusCalloutResolution>(
        callout::BusType::OMI_BUS, omi_str, callout::Priority::LOW, false);

    libhei::Chip chip{chip_str, 0xdeadbeef};
    libhei::Signature sig{chip, 0xabcd, 0, 0, libhei::ATTN_TYPE_CHECKSTOP};
    ServiceData sd{sig, true};

    nlohmann::json j{};
    std::string s{};

    c1->resolve(sd);
    c2->resolve(sd);
    c3->resolve(sd);

    // Callout list
    j = sd.getCalloutList();
    s = R"([
    {
        "Deconfigured": false,
        "Guarded": true,
        "LocationCode": "/proc0",
        "Priority": "A"
    },
    {
        "Deconfigured": false,
        "Guarded": true,
        "LocationCode": "/proc0/pib/perv12/mc0/mi0/mcc0/omi0/ocmb0",
        "Priority": "A"
    },
    {
        "Deconfigured": false,
        "Guarded": false,
        "LocationCode": "P0",
        "Priority": "L"
    }
])";
    EXPECT_EQ(s, j.dump(4));

    // Callout FFDC
    j = sd.getCalloutFFDC();
    s = R"([
    {
        "Callout Type": "Hardware Callout",
        "Guard": true,
        "Priority": "medium_group_A",
        "Target": "/proc0/pib/perv12/mc0/mi0/mcc0/omi0"
    },
    {
        "Bus Type": "OMI_BUS",
        "Callout Type": "Connected Callout",
        "Guard": true,
        "Priority": "medium_group_A",
        "Target": "/proc0/pib/perv12/mc0/mi0/mcc0/omi0/ocmb0"
    },
    {
        "Bus Type": "OMI_BUS",
        "Callout Type": "Bus Callout",
        "Guard": false,
        "Priority": "low",
        "RX Target": "/proc0/pib/perv12/mc0/mi0/mcc0/omi0",
        "TX Target": "/proc0/pib/perv12/mc0/mi0/mcc0/omi0/ocmb0"
    }
])";
    EXPECT_EQ(s, j.dump(4));
}

TEST(Resolution, ClockCallout)
{
    auto c1 = std::make_shared<ClockCalloutResolution>(
        callout::ClockType::OSC_REF_CLOCK_1, callout::Priority::HIGH, false);

    libhei::Chip chip{chip_str, 0xdeadbeef};
    libhei::Signature sig{chip, 0xabcd, 0, 0, libhei::ATTN_TYPE_CHECKSTOP};
    ServiceData sd{sig, true};

    c1->resolve(sd);

    nlohmann::json j{};
    std::string s{};

    // Callout list
    j = sd.getCalloutList();
    s = R"([
    {
        "Deconfigured": false,
        "Guarded": false,
        "LocationCode": "P0",
        "Priority": "H"
    }
])";
    EXPECT_EQ(s, j.dump(4));

    // Callout FFDC
    j = sd.getCalloutFFDC();
    s = R"([
    {
        "Callout Type": "Clock Callout",
        "Clock Type": "OSC_REF_CLOCK_1",
        "Priority": "high"
    }
])";
    EXPECT_EQ(s, j.dump(4));
}

TEST(Resolution, ProcedureCallout)
{
    auto c1 = std::make_shared<ProcedureCalloutResolution>(
        callout::Procedure::NEXTLVL, callout::Priority::LOW);

    libhei::Chip chip{chip_str, 0xdeadbeef};
    libhei::Signature sig{chip, 0xabcd, 0, 0, libhei::ATTN_TYPE_CHECKSTOP};
    ServiceData sd{sig, true};

    c1->resolve(sd);

    nlohmann::json j{};
    std::string s{};

    // Callout list
    j = sd.getCalloutList();
    s = R"([
    {
        "Priority": "L",
        "Procedure": "NEXTLVL"
    }
])";
    EXPECT_EQ(s, j.dump(4));

    // Callout FFDC
    j = sd.getCalloutFFDC();
    s = R"([
    {
        "Callout Type": "Procedure Callout",
        "Priority": "low",
        "Procedure": "NEXTLVL"
    }
])";
    EXPECT_EQ(s, j.dump(4));
}
