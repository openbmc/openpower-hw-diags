#include <stdio.h>

#include <analyzer/analyzer_main.hpp>
#include <analyzer/resolution.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

#include <regex>

#include "gtest/gtest.h"

// Chip string
constexpr auto chip_str = "/proc0";

// Unit paths
constexpr auto proc_str = "";
constexpr auto iolink_str = "pib/perv26/pauc1/iohs0/smpgroup0";
constexpr auto omi_str = "pib/perv12/mc0/mi0/mcc0/omi0";
constexpr auto ocmb_str = "pib/perv12/mc0/mi0/mcc0/omi0/ocmb0";
constexpr auto core_str = "pib/perv39/eq7/fc1/core1";

using namespace analyzer;

TEST(Resolution, TestSet1)
{
    pdbg_targets_init(nullptr);

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
    libhei::Chip chip{util::pdbg::getTrgt(chip_str), 0xdeadbeef};
    libhei::Signature sig{chip, 0xabcd, 0, 0, libhei::ATTN_TYPE_CHIP_CS};
    ServiceData sd1{sig, AnalysisType::SYSTEM_CHECKSTOP,
                    libhei::IsolationData{}};
    ServiceData sd2{sig, AnalysisType::TERMINATE_IMMEDIATE,
                    libhei::IsolationData{}};

    // Resolve
    l1->resolve(sd1);
    l2->resolve(sd2);

    // Verify the subsystems
    std::pair<callout::SrcSubsystem, callout::Priority> subsys = {
        callout::SrcSubsystem::PROCESSOR_FRU, callout::Priority::HIGH};
    EXPECT_EQ(sd1.getSubsys(), subsys);

    subsys = {callout::SrcSubsystem::PROCESSOR_FRU, callout::Priority::HIGH};
    EXPECT_EQ(sd2.getSubsys(), subsys);

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
        "EntityPath": [],
        "GuardType": "GARD_Unrecoverable",
        "Guarded": true,
        "LocationCode": "/proc0/pib/perv12/mc0/mi0/mcc0/omi0",
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

    j = sd2.getCalloutList();
    s = R"([
    {
        "Priority": "L",
        "Procedure": "next_level_support"
    },
    {
        "Deconfigured": false,
        "EntityPath": [],
        "GuardType": "GARD_Predictive",
        "Guarded": true,
        "LocationCode": "/proc0/pib/perv39/eq7/fc1/core1",
        "Priority": "M"
    },
    {
        "Deconfigured": false,
        "Guarded": false,
        "LocationCode": "/proc0",
        "Priority": "H"
    },
    {
        "Deconfigured": false,
        "EntityPath": [],
        "GuardType": "GARD_Predictive",
        "Guarded": true,
        "LocationCode": "/proc0/pib/perv12/mc0/mi0/mcc0/omi0",
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
}

TEST(Resolution, HardwareCallout)
{
    pdbg_targets_init(nullptr);

    auto c1 = std::make_shared<HardwareCalloutResolution>(
        omi_str, callout::Priority::MED_A, true);

    libhei::Chip chip{util::pdbg::getTrgt(chip_str), 0xdeadbeef};
    libhei::Signature sig{chip, 0xabcd, 0, 0, libhei::ATTN_TYPE_CHIP_CS};
    ServiceData sd{sig, AnalysisType::SYSTEM_CHECKSTOP,
                   libhei::IsolationData{}};

    c1->resolve(sd);

    // Verify the subsystem
    std::pair<callout::SrcSubsystem, callout::Priority> subsys = {
        callout::SrcSubsystem::MEMORY_CTLR, callout::Priority::MED_A};
    EXPECT_EQ(sd.getSubsys(), subsys);

    nlohmann::json j{};
    std::string s{};

    // Callout list
    j = sd.getCalloutList();
    s = R"([
    {
        "Deconfigured": false,
        "EntityPath": [],
        "GuardType": "GARD_Unrecoverable",
        "Guarded": true,
        "LocationCode": "/proc0/pib/perv12/mc0/mi0/mcc0/omi0",
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
    pdbg_targets_init(nullptr);

    auto c1 = std::make_shared<ConnectedCalloutResolution>(
        callout::BusType::SMP_BUS, iolink_str, callout::Priority::MED_A, true);

    auto c2 = std::make_shared<ConnectedCalloutResolution>(
        callout::BusType::OMI_BUS, ocmb_str, callout::Priority::MED_B, true);

    auto c3 = std::make_shared<ConnectedCalloutResolution>(
        callout::BusType::OMI_BUS, omi_str, callout::Priority::MED_C, true);

    libhei::Chip chip{util::pdbg::getTrgt(chip_str), 0xdeadbeef};
    libhei::Signature sig{chip, 0xabcd, 0, 0, libhei::ATTN_TYPE_CHIP_CS};
    ServiceData sd{sig, AnalysisType::SYSTEM_CHECKSTOP,
                   libhei::IsolationData{}};

    nlohmann::json j{};
    std::string s{};

    c1->resolve(sd);
    c2->resolve(sd);
    c3->resolve(sd);

    // Verify the subsystem
    std::pair<callout::SrcSubsystem, callout::Priority> subsys = {
        callout::SrcSubsystem::PROCESSOR_BUS, callout::Priority::MED_A};
    EXPECT_EQ(sd.getSubsys(), subsys);

    // Callout list
    j = sd.getCalloutList();
    s = R"([
    {
        "Deconfigured": false,
        "EntityPath": [],
        "GuardType": "GARD_Unrecoverable",
        "Guarded": true,
        "LocationCode": "/proc1/pib/perv25/pauc0/iohs1/smpgroup0",
        "Priority": "A"
    },
    {
        "Deconfigured": false,
        "EntityPath": [],
        "GuardType": "GARD_Unrecoverable",
        "Guarded": true,
        "LocationCode": "/proc0/pib/perv12/mc0/mi0/mcc0/omi0",
        "Priority": "B"
    },
    {
        "Deconfigured": false,
        "EntityPath": [],
        "GuardType": "GARD_Unrecoverable",
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
        "RX Target": "/proc0/pib/perv26/pauc1/iohs0/smpgroup0",
        "TX Target": "/proc1/pib/perv25/pauc0/iohs1/smpgroup0"
    },
    {
        "Bus Type": "OMI_BUS",
        "Callout Type": "Connected Callout",
        "Guard": true,
        "Priority": "medium_group_B",
        "RX Target": "/proc0/pib/perv12/mc0/mi0/mcc0/omi0/ocmb0",
        "TX Target": "/proc0/pib/perv12/mc0/mi0/mcc0/omi0"
    },
    {
        "Bus Type": "OMI_BUS",
        "Callout Type": "Connected Callout",
        "Guard": true,
        "Priority": "medium_group_C",
        "RX Target": "/proc0/pib/perv12/mc0/mi0/mcc0/omi0",
        "TX Target": "/proc0/pib/perv12/mc0/mi0/mcc0/omi0/ocmb0"
    }
])";
    EXPECT_EQ(s, j.dump(4));
}

TEST(Resolution, BusCallout)
{
    pdbg_targets_init(nullptr);

    auto c1 = std::make_shared<HardwareCalloutResolution>(
        omi_str, callout::Priority::MED_A, true);

    auto c2 = std::make_shared<ConnectedCalloutResolution>(
        callout::BusType::OMI_BUS, omi_str, callout::Priority::MED_A, true);

    auto c3 = std::make_shared<BusCalloutResolution>(
        callout::BusType::OMI_BUS, omi_str, callout::Priority::LOW, false);

    libhei::Chip chip{util::pdbg::getTrgt(chip_str), 0xdeadbeef};
    libhei::Signature sig{chip, 0xabcd, 0, 0, libhei::ATTN_TYPE_CHIP_CS};
    ServiceData sd{sig, AnalysisType::SYSTEM_CHECKSTOP,
                   libhei::IsolationData{}};

    nlohmann::json j{};
    std::string s{};

    // Special case: The bus callout will add both ends of the bus to the
    // callout list at LOW priority with no guarding. Generally, this would be
    // the last thing to be added to the callout list because of the priority.
    // However, there was a field defect where a higher priority callout of one
    // of the endpoint was added to the list (with guard action) after the bus
    // callout. The priority of the callout was updated to match the higher
    // priority, but the guard action was not updated. So the following are
    // added to the callout list in a specific order:
    //  - First, one of the endpoints with MED_A and guard.
    //  - Then, the bus callout with LOW and no guard. This should result in
    //    both endpoints in the list: one at MED_A (guard), the other at LOW (no
    //    guard).
    //  - Finally, the other endpoint with MED_A and guard. This should result
    //    in both endpoints in the list with MED_A priority and guard actions.
    c1->resolve(sd);
    c3->resolve(sd);
    c2->resolve(sd);

    // Verify the subsystem
    std::pair<callout::SrcSubsystem, callout::Priority> subsys = {
        callout::SrcSubsystem::MEMORY_CTLR, callout::Priority::MED_A};
    EXPECT_EQ(sd.getSubsys(), subsys);

    // Callout list
    j = sd.getCalloutList();
    s = R"([
    {
        "Deconfigured": false,
        "EntityPath": [],
        "GuardType": "GARD_Unrecoverable",
        "Guarded": true,
        "LocationCode": "/proc0/pib/perv12/mc0/mi0/mcc0/omi0",
        "Priority": "A"
    },
    {
        "Deconfigured": false,
        "EntityPath": [],
        "GuardType": "GARD_Unrecoverable",
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
        "Callout Type": "Bus Callout",
        "Guard": false,
        "Priority": "low",
        "RX Target": "/proc0/pib/perv12/mc0/mi0/mcc0/omi0",
        "TX Target": "/proc0/pib/perv12/mc0/mi0/mcc0/omi0/ocmb0"
    },
    {
        "Bus Type": "OMI_BUS",
        "Callout Type": "Connected Callout",
        "Guard": true,
        "Priority": "medium_group_A",
        "RX Target": "/proc0/pib/perv12/mc0/mi0/mcc0/omi0",
        "TX Target": "/proc0/pib/perv12/mc0/mi0/mcc0/omi0/ocmb0"
    }
])";
    EXPECT_EQ(s, j.dump(4));
}

TEST(Resolution, ClockCallout)
{
    pdbg_targets_init(nullptr);

    auto c1 = std::make_shared<ClockCalloutResolution>(
        callout::ClockType::OSC_REF_CLOCK_1, callout::Priority::HIGH, false);

    libhei::Chip chip{util::pdbg::getTrgt(chip_str), 0xdeadbeef};
    libhei::Signature sig{chip, 0xabcd, 0, 0, libhei::ATTN_TYPE_CHIP_CS};
    ServiceData sd{sig, AnalysisType::SYSTEM_CHECKSTOP,
                   libhei::IsolationData{}};

    c1->resolve(sd);

    // Verify the subsystem
    std::pair<callout::SrcSubsystem, callout::Priority> subsys = {
        callout::SrcSubsystem::CEC_CLOCKS, callout::Priority::HIGH};
    EXPECT_EQ(sd.getSubsys(), subsys);

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
    pdbg_targets_init(nullptr);

    auto c1 = std::make_shared<ProcedureCalloutResolution>(
        callout::Procedure::NEXTLVL, callout::Priority::LOW);

    libhei::Chip chip{util::pdbg::getTrgt(chip_str), 0xdeadbeef};
    libhei::Signature sig{chip, 0xabcd, 0, 0, libhei::ATTN_TYPE_CHIP_CS};
    ServiceData sd{sig, AnalysisType::SYSTEM_CHECKSTOP,
                   libhei::IsolationData{}};

    c1->resolve(sd);

    // Verify the subsystem
    std::pair<callout::SrcSubsystem, callout::Priority> subsys = {
        callout::SrcSubsystem::OTHERS, callout::Priority::LOW};
    EXPECT_EQ(sd.getSubsys(), subsys);

    nlohmann::json j{};
    std::string s{};

    // Callout list
    j = sd.getCalloutList();
    s = R"([
    {
        "Priority": "L",
        "Procedure": "next_level_support"
    }
])";
    EXPECT_EQ(s, j.dump(4));

    // Callout FFDC
    j = sd.getCalloutFFDC();
    s = R"([
    {
        "Callout Type": "Procedure Callout",
        "Priority": "low",
        "Procedure": "next_level_support"
    }
])";
    EXPECT_EQ(s, j.dump(4));
}

TEST(Resolution, PartCallout)
{
    pdbg_targets_init(nullptr);

    auto c1 = std::make_shared<PartCalloutResolution>(callout::PartType::PNOR,
                                                      callout::Priority::MED);

    libhei::Chip chip{util::pdbg::getTrgt(chip_str), 0xdeadbeef};
    libhei::Signature sig{chip, 0xabcd, 0, 0, libhei::ATTN_TYPE_CHIP_CS};
    ServiceData sd{sig, AnalysisType::SYSTEM_CHECKSTOP,
                   libhei::IsolationData{}};

    c1->resolve(sd);

    // Verify the subsystem
    std::pair<callout::SrcSubsystem, callout::Priority> subsys = {
        callout::SrcSubsystem::CEC_HARDWARE, callout::Priority::MED};
    EXPECT_EQ(sd.getSubsys(), subsys);

    nlohmann::json j{};
    std::string s{};

    // Callout list
    j = sd.getCalloutList();
    s = R"([
    {
        "Deconfigured": false,
        "Guarded": false,
        "LocationCode": "/bmc0",
        "Priority": "M"
    }
])";
    EXPECT_EQ(s, j.dump(4));

    // Callout FFDC
    j = sd.getCalloutFFDC();
    s = R"([
    {
        "Callout Type": "Part Callout",
        "Part Type": "PNOR",
        "Priority": "medium"
    }
])";
    EXPECT_EQ(s, j.dump(4));
}
