#include <analyzer/service_data.hpp>
#include <hei_main.hpp>
#include <nlohmann/json.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

namespace analyzer
{

//------------------------------------------------------------------------------

bool __getRasDataFile(const libhei::Signature& i_rootCause,
                      nlohmann::json& o_rasData)
{
    bool o_fileFound = false;

    auto directory = "/usr/share/openpower-hw-diags/";

    for (const auto& entry : fs::directory_iterator(directory))
    {
        auto path = entry.path();

        // Open the file.
        std::ifstream file{path};
        if (!file.good())
        {
            trace::err("Unable to open file: %s", path.string().c_str());
            continue;
        }

        // Parse the JSON and don't throw exceptions.
        o_rasData = nlohmann::json::parse(file, nullptr, false);
        if (o_rasData.is_discarded() || !o_rasData.is_object())
        {
            trace::err("File not valid JSON: %s", path.string().c_str());
            continue;
        }

        // Get the chip model/EC level.
        auto model_ec = o_rasData.find("model_ec");
        if (o_rasData.end() == model_ec || !model_ec->is_string())
        {
            trace::err("File does not contain 'model_ec': %s",
                       path.string().c_str());
            continue;
        }

        // Check if this is the file we are looking for.
        if (i_rootCause.getChip().getType() ==
            std::stoul(model_ec->get<std::string>(), 0, 16))
        {
            trace::inf("RAS data file found: %s", path.string().c_str());
            o_fileFound = true;
            break;
        }
    }

    return o_fileFound;
}

//------------------------------------------------------------------------------

void applyRasActions(ServiceData& io_servData)
{
    bool actionApplied = false;

    libhei::Signature rootCause = io_servData.getRootCause();

    // Look for the RAS data file associated with this signature.
    nlohmann::json rasData;
    if (__getRasDataFile(rootCause, rasData))
    {
        // TODO: Search RAS data files and apply service actions.
    }

    if (!actionApplied)
    {
        // The default callout is to callout level 2 support.
        io_servData.addCallout(std::make_shared<ProcedureCallout>(
            ProcedureCallout::NEXTLVL, Callout::Priority::HIGH));
    }
}

//------------------------------------------------------------------------------

} // namespace analyzer
