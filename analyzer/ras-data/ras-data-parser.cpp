#include <analyzer/ras-data/ras-data-parser.hpp>
#include <util/data_file.hpp>
#include <util/trace.hpp>

#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

namespace analyzer
{

//------------------------------------------------------------------------------

std::shared_ptr<Resolution>
    RasDataParser::getResolution(const libhei::Signature& i_signature)
{
    nlohmann::json data;

    try
    {
        data = iv_dataFiles.at(i_signature.getChip().getType());
    }
    catch (const std::out_of_range& e)
    {
        trace::err("No RAS data defined for chip type: 0x%08x",
                   i_signature.getChip().getType());
        throw; // caught later downstream
    }

    const auto action = parseSignature(data, i_signature);

    std::shared_ptr<Resolution> resolution;

    try
    {
        resolution = parseAction(data, action);
    }
    catch (...)
    {
        trace::err("Unable to get resolution for action: %s", action.c_str());
        throw; // caught later downstream
    }

    return resolution;
}

//------------------------------------------------------------------------------

void RasDataParser::initDataFiles()
{
    iv_dataFiles.clear(); // initially empty

    // Get the RAS data schema files from the package `schema` subdirectory.
    fs::path schemaDir{PACKAGE_DIR "schema"};
    auto schemaRegex = R"(ras-data-schema-v[0-9]{2}\.json)";
    std::vector<fs::path> schemaPaths;
    util::findFiles(schemaDir, schemaRegex, schemaPaths);

    // Parse each of the schema files.
    std::map<unsigned int, nlohmann::json> schemaFiles;
    for (const auto& path : schemaPaths)
    {
        // Trace each data file for debug.
        trace::inf("File found: path=%s", path.string().c_str());

        // Open the file.
        std::ifstream file{path};
        assert(file.good()); // The file must be readable.

        try
        {
            // Parse the JSON.
            auto schema = nlohmann::json::parse(file);

            // Get the schema version.
            auto version = schema.at("version").get<unsigned int>();

            // Keep track of the schemas.
            auto ret = schemaFiles.emplace(version, schema);
            assert(ret.second); // Should not have duplicate entries
        }
        catch (...)
        {
            trace::err("Failed to parse file: %s", path.string().c_str());
            throw; // caught later downstream
        }
    }

    // Get the RAS data files from the package `data` subdirectory.
    fs::path dataDir{PACKAGE_DIR "ras-data"};
    std::vector<fs::path> dataPaths;
    util::findFiles(dataDir, R"(.*\.json)", dataPaths);

    // Parse each of the data files.
    for (const auto& path : dataPaths)
    {
        // Trace each data file for debug.
        trace::inf("File found: path=%s", path.string().c_str());

        // Open the file.
        std::ifstream file{path};
        assert(file.good()); // The file must be readable.

        try
        {
            // Parse the JSON.
            const auto data = nlohmann::json::parse(file);

            // Get the data version.
            auto version = data.at("version").get<unsigned int>();

            // Get the schema for this file.
            auto schema = schemaFiles.at(version);

            // Validate the data against the schema.
            assert(util::validateJson(schema, data));

            // Get the chip model/EC level from the data. The value is currently
            // stored as a string representation of the hex value. So it will
            // have to be converted to an integer.
            libhei::ChipType_t chipType =
                std::stoul(data.at("model_ec").get<std::string>(), 0, 16);

            // So far, so good. Add the entry.
            auto ret = iv_dataFiles.emplace(chipType, data);
            assert(ret.second); // Should not have duplicate entries
        }
        catch (...)
        {
            trace::err("Failed to parse file: %s", path.string().c_str());
            throw; // caught later downstream
        }
    }
}

//------------------------------------------------------------------------------

std::string RasDataParser::parseSignature(const nlohmann::json& i_data,
                                          const libhei::Signature& i_signature)
{
    // Get the signature keys. All are hex (lower case) with no prefix.
    char buf[5];
    sprintf(buf, "%04x", i_signature.getId());
    std::string id{buf};

    sprintf(buf, "%02x", i_signature.getBit());
    std::string bit{buf};

    sprintf(buf, "%02x", i_signature.getInstance());
    std::string inst{buf};

    std::string action;

    try
    {
        action =
            i_data.at("signatures").at(id).at(bit).at(inst).get<std::string>();
    }
    catch (const std::out_of_range& e)
    {
        trace::err("No action defined for signature: %s %s %s", id.c_str(),
                   bit.c_str(), inst.c_str());
        throw; // caught later downstream
    }

    // Return the action.
    return action;
}

//------------------------------------------------------------------------------

std::tuple<callout::BusType, std::string>
    RasDataParser::parseBus(const nlohmann::json& i_data,
                            const std::string& i_name)
{
    auto bus = i_data.at("buses").at(i_name);

    // clang-format off
    static const std::map<std::string, callout::BusType> m =
    {
        {"SMP_BUS", callout::BusType::SMP_BUS},
        {"OMI_BUS", callout::BusType::OMI_BUS},
    };
    // clang-format on

    auto busType = m.at(bus.at("type").get<std::string>());

    std::string unitPath{}; // default empty if unit does not exist
    if (bus.contains("unit"))
    {
        auto unit = bus.at("unit").get<std::string>();
        unitPath  = i_data.at("units").at(unit).get<std::string>();
    }

    return std::make_tuple(busType, unitPath);
}

//------------------------------------------------------------------------------

std::shared_ptr<Resolution>
    RasDataParser::parseAction(const nlohmann::json& i_data,
                               const std::string& i_action)
{
    auto o_list = std::make_shared<ResolutionList>();

    // This function will be called recursively and we want to prevent cyclic
    // recursion.
    static std::vector<std::string> stack;
    assert(stack.end() == std::find(stack.begin(), stack.end(), i_action));
    stack.push_back(i_action);

    // Iterate the action list and apply the changes.
    for (const auto& a : i_data.at("actions").at(i_action))
    {
        auto type = a.at("type").get<std::string>();

        if ("action" == type)
        {
            auto name = a.at("name").get<std::string>();

            o_list->push(parseAction(i_data, name));
        }
        else if ("callout_self" == type)
        {
            auto priority = a.at("priority").get<std::string>();
            auto guard    = a.at("guard").get<bool>();

            std::string path{}; // Must be empty to callout the chip.

            o_list->push(std::make_shared<HardwareCalloutResolution>(
                path, getPriority(priority), guard));
        }
        else if ("callout_unit" == type)
        {
            auto name     = a.at("name").get<std::string>();
            auto priority = a.at("priority").get<std::string>();
            auto guard    = a.at("guard").get<bool>();

            auto path = i_data.at("units").at(name).get<std::string>();

            o_list->push(std::make_shared<HardwareCalloutResolution>(
                path, getPriority(priority), guard));
        }
        else if ("callout_connected" == type)
        {
            auto name     = a.at("name").get<std::string>();
            auto priority = a.at("priority").get<std::string>();
            auto guard    = a.at("guard").get<bool>();

            auto busData = parseBus(i_data, name);

            o_list->push(std::make_shared<ConnectedCalloutResolution>(
                std::get<0>(busData), std::get<1>(busData),
                getPriority(priority), guard));
        }
        else if ("callout_bus" == type)
        {
            auto name     = a.at("name").get<std::string>();
            auto priority = a.at("priority").get<std::string>();
            auto guard    = a.at("guard").get<bool>();

            auto busData = parseBus(i_data, name);

            o_list->push(std::make_shared<BusCalloutResolution>(
                std::get<0>(busData), std::get<1>(busData),
                getPriority(priority), guard));
        }
        else if ("callout_clock" == type)
        {
            auto name     = a.at("name").get<std::string>();
            auto priority = a.at("priority").get<std::string>();
            auto guard    = a.at("guard").get<bool>();

            // clang-format off
            static const std::map<std::string, callout::ClockType> m =
            {
                {"OSC_REF_CLOCK_0", callout::ClockType::OSC_REF_CLOCK_0},
                {"OSC_REF_CLOCK_1", callout::ClockType::OSC_REF_CLOCK_1},
                {"TOD_CLOCK",       callout::ClockType::TOD_CLOCK},
            };
            // clang-format on

            o_list->push(std::make_shared<ClockCalloutResolution>(
                m.at(name), getPriority(priority), guard));
        }
        else if ("callout_procedure" == type)
        {
            auto name     = a.at("name").get<std::string>();
            auto priority = a.at("priority").get<std::string>();

            // clang-format off
            static const std::map<std::string, callout::Procedure> m =
            {
                {"LEVEL2",   callout::Procedure::NEXTLVL},
                {"SUE_SEEN", callout::Procedure::SUE_SEEN},
            };
            // clang-format on

            o_list->push(std::make_shared<ProcedureCalloutResolution>(
                m.at(name), getPriority(priority)));
        }
        else if ("callout_part" == type)
        {
            auto name     = a.at("name").get<std::string>();
            auto priority = a.at("priority").get<std::string>();

            // clang-format off
            static const std::map<std::string, callout::PartType> m =
            {
                {"PNOR", callout::PartType::PNOR},
            };
            // clang-format on

            o_list->push(std::make_shared<PartCalloutResolution>(
                m.at(name), getPriority(priority)));
        }
        else if ("plugin" == type)
        {
            auto name = a.at("name").get<std::string>();
            auto inst = a.at("instance").get<unsigned int>();

            o_list->push(std::make_shared<PluginResolution>(name, inst));
        }
        else
        {
            throw std::logic_error("Unsupported action type: " + type);
        }
    }

    // Done with this action pop it off the stack.
    stack.pop_back();

    return o_list;
}

//------------------------------------------------------------------------------

callout::Priority RasDataParser::getPriority(const std::string& i_priority)
{
    // clang-format off
    static const std::map<std::string, callout::Priority> m =
    {
        {"HIGH",  callout::Priority::HIGH},
        {"MED",   callout::Priority::MED},
        {"MED_A", callout::Priority::MED_A},
        {"MED_B", callout::Priority::MED_B},
        {"MED_C", callout::Priority::MED_C},
        {"LOW",   callout::Priority::LOW},
    };
    // clang-format on

    return m.at(i_priority);
}

//------------------------------------------------------------------------------

} // namespace analyzer
