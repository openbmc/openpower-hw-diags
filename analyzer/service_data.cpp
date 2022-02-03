#include <analyzer/service_data.hpp>

namespace analyzer
{

//------------------------------------------------------------------------------

void ServiceData::calloutTarget(pdbg_target* i_target,
                                callout::Priority i_priority, bool i_guard)
{
    // Add the target to the callout list.
    addTargetCallout(i_target, i_priority, i_guard);

    // Add the callout FFDC.
    nlohmann::json ffdc;
    ffdc["Callout Type"] = "Hardware Callout";
    ffdc["Target"]       = util::pdbg::getPhysDevPath(i_target);
    ffdc["Priority"]     = callout::getStringFFDC(i_priority);
    ffdc["Guard"]        = i_guard;
    addCalloutFFDC(ffdc);
}

//------------------------------------------------------------------------------

void ServiceData::calloutConnected(pdbg_target* i_rxTarget,
                                   const callout::BusType& i_busType,
                                   callout::Priority i_priority, bool i_guard)
{
    // Get the endpoint target for the transfer side of the bus.
    auto txTarget = util::pdbg::getConnectedTarget(i_rxTarget, i_busType);

    // Callout the TX endpoint.
    addTargetCallout(txTarget, i_priority, i_guard);

    // Add the callout FFDC.
    nlohmann::json ffdc;
    ffdc["Callout Type"] = "Connected Callout";
    ffdc["Bus Type"]     = i_busType.getString();
    ffdc["RX Target"]    = util::pdbg::getPhysDevPath(i_rxTarget);
    ffdc["TX Target"]    = util::pdbg::getPhysDevPath(txTarget);
    ffdc["Priority"]     = callout::getStringFFDC(i_priority);
    ffdc["Guard"]        = i_guard;
    addCalloutFFDC(ffdc);
}

//------------------------------------------------------------------------------

void ServiceData::calloutBus(pdbg_target* i_rxTarget,
                             const callout::BusType& i_busType,
                             callout::Priority i_priority, bool i_guard)
{
    // Get the endpoint target for the transfer side of the bus.
    auto txTarget = util::pdbg::getConnectedTarget(i_rxTarget, i_busType);

    // Callout the RX endpoint.
    addTargetCallout(i_rxTarget, i_priority, i_guard);

    // Callout the TX endpoint.
    addTargetCallout(txTarget, i_priority, i_guard);

    // Callout everything else in between.
    // TODO: For P10 (OMI bus and XBUS), the callout is simply the backplane.
    addBackplaneCallout(i_priority);

    // Add the callout FFDC.
    nlohmann::json ffdc;
    ffdc["Callout Type"] = "Bus Callout";
    ffdc["Bus Type"]     = i_busType.getString();
    ffdc["RX Target"]    = util::pdbg::getPhysDevPath(i_rxTarget);
    ffdc["TX Target"]    = util::pdbg::getPhysDevPath(txTarget);
    ffdc["Priority"]     = callout::getStringFFDC(i_priority);
    ffdc["Guard"]        = i_guard;
    addCalloutFFDC(ffdc);
}

//------------------------------------------------------------------------------

void ServiceData::calloutClock(const callout::ClockType& i_clockType,
                               callout::Priority i_priority, bool)
{
    // Callout the clock target.
    // TODO: For P10, the callout is simply the backplane. Also, there are no
    //       clock targets in the device tree. So at the moment there is no
    //       guard support for clock targets.
    addBackplaneCallout(i_priority);

    // Add the callout FFDC.
    // TODO: Add the target and guard type if guard is ever supported.
    nlohmann::json ffdc;
    ffdc["Callout Type"] = "Clock Callout";
    ffdc["Clock Type"]   = i_clockType.getString();
    ffdc["Priority"]     = callout::getStringFFDC(i_priority);
    addCalloutFFDC(ffdc);
}

//------------------------------------------------------------------------------

void ServiceData::calloutProcedure(const callout::Procedure& i_procedure,
                                   callout::Priority i_priority)
{
    // Add the actual callout to the service data.
    nlohmann::json callout;
    callout["Procedure"] = i_procedure.getString();
    callout["Priority"]  = callout::getString(i_priority);
    addCallout(callout);

    // Add the callout FFDC.
    nlohmann::json ffdc;
    ffdc["Callout Type"] = "Procedure Callout";
    ffdc["Procedure"]    = i_procedure.getString();
    ffdc["Priority"]     = callout::getStringFFDC(i_priority);
    addCalloutFFDC(ffdc);
}

//------------------------------------------------------------------------------

void ServiceData::calloutPart(const callout::PartType& i_part,
                              callout::Priority i_priority)
{
    if (callout::PartType::PNOR == i_part)
    {
        // The PNOR is on the BMC card.
        // TODO: Will need to be modified if we ever support systems with more
        //       than one BMC.
        addTargetCallout(util::pdbg::getTrgt("/bmc0"), i_priority, false);
    }
    else
    {
        throw std::logic_error("Unsupported part type: " + i_part.getString());
    }

    // Add the callout FFDC.
    nlohmann::json ffdc;
    ffdc["Callout Type"] = "Part Callout";
    ffdc["Part Type"]    = i_part.getString();
    ffdc["Priority"]     = callout::getStringFFDC(i_priority);
    addCalloutFFDC(ffdc);
}

//------------------------------------------------------------------------------

void ServiceData::addCallout(const nlohmann::json& i_callout)
{
    // The new callout is either a hardware callout with a location code or a
    // procedure callout.

    std::string type{};
    if (i_callout.contains("LocationCode"))
    {
        type = "LocationCode";
    }
    else if (i_callout.contains("Procedure"))
    {
        type = "Procedure";
    }
    else
    {
        throw std::logic_error("Unsupported callout: " + i_callout.dump());
    }

    // A map to determine the priority order. All of the medium priorities,
    // including the medium group priorities, are all the same level.
    // clang-format off
    static const std::map<std::string, unsigned int> m = {
        {callout::getString(callout::Priority::HIGH),  3},
        {callout::getString(callout::Priority::MED),   2},
        {callout::getString(callout::Priority::MED_A), 2},
        {callout::getString(callout::Priority::MED_B), 2},
        {callout::getString(callout::Priority::MED_C), 2},
        {callout::getString(callout::Priority::LOW),   1},
    };
    // clang-format on

    // The new callout must contain a valid priority.
    assert(i_callout.contains("Priority") &&
           m.contains(i_callout.at("Priority")));

    bool addCallout = true;

    for (auto& c : iv_calloutList)
    {
        if (c.contains(type) && (c.at(type) == i_callout.at(type)))
        {
            // The new callout already exists. Don't add a new callout.
            addCallout = false;

            if (m.at(c.at("Priority")) < m.at(i_callout.at("Priority")))
            {
                // The new callout has a higher priority, update it.
                c["Priority"] = i_callout.at("Priority");
            }
        }
    }

    if (addCallout)
    {
        iv_calloutList.push_back(i_callout);
    }
}

//------------------------------------------------------------------------------

void ServiceData::addTargetCallout(pdbg_target* i_target,
                                   callout::Priority i_priority, bool i_guard)
{
    nlohmann::json callout;

    callout["LocationCode"] = util::pdbg::getLocationCode(i_target);
    callout["Priority"]     = callout::getString(i_priority);
    callout["Deconfigured"] = false;
    callout["Guarded"]      = false; // default

    // Check if guard info should be added.
    if (i_guard)
    {
        auto guardType = queryGuardPolicy();

        if (!(callout::GuardType::NONE == guardType))
        {
            callout["Guarded"]    = true;
            callout["EntityPath"] = util::pdbg::getPhysBinPath(i_target);
            callout["GuardType"]  = guardType.getString();
        }
    }

    addCallout(callout);
}

//------------------------------------------------------------------------------

void ServiceData::addBackplaneCallout(callout::Priority i_priority)
{
    // TODO: There isn't a device tree object for this. So will need to hardcode
    //       the location code for now. In the future, we will need a mechanism
    //       to make this data driven.

    nlohmann::json callout;

    callout["LocationCode"] = "P0";
    callout["Priority"]     = callout::getString(i_priority);
    callout["Deconfigured"] = false;
    callout["Guarded"]      = false;

    addCallout(callout);
}

//------------------------------------------------------------------------------

} // namespace analyzer
