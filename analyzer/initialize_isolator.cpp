
#include <assert.h>

#include <hei_main.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

#include <filesystem>
#include <fstream>
#include <map>
#include <vector>

namespace fs = std::filesystem;

namespace analyzer
{

//------------------------------------------------------------------------------

void __getChipDataFiles(std::map<libhei::ChipType_t, fs::path>& o_files)
{
    o_files.clear();

    auto directory = "/usr/share/openpower-libhei/";

    for (const auto& entry : fs::directory_iterator(directory))
    {
        auto path = entry.path();

        std::ifstream file{path, std::ios::binary};
        if (!file.good())
        {
            trace::err("Unable to open file: %s", path.string().c_str());
            continue;
        }

        // The first 8-bytes is the file keyword and the next 4-bytes is the
        // chip type.
        libhei::FileKeyword_t keyword;
        libhei::ChipType_t chipType;

        const size_t sz_keyword  = sizeof(keyword);
        const size_t sz_chipType = sizeof(chipType);
        const size_t sz_buffer   = sz_keyword + sz_chipType;

        // Read the keyword and chip type from the file.
        char buffer[sz_buffer];
        file.read(buffer, sz_buffer);
        if (!file.good())
        {
            trace::err("Unable to read file: %s", path.string().c_str());
            continue;
        }

        // Get the keyword.
        memcpy(&keyword, &buffer[0], sz_keyword);
        keyword = be64toh(keyword);

        // Ensure the keyword value is correct.
        if (libhei::KW_CHIPDATA != keyword)
        {
            trace::err("Invalid chip data file: %s", path.string().c_str());
            continue;
        }

        // Get the chip type.
        memcpy(&chipType, &buffer[sz_keyword], sz_chipType);
        chipType = be32toh(chipType);

        // Trace each legitimate chip data file for debug.
        trace::inf("File found: type=0x%0" PRIx32 " path=%s", chipType,
                   path.string().c_str());

        // Ensure there aren't duplicate entries for this type. If so, this
        // would be a bug in the code that generates the chip data files.
        assert(0 == o_files.count(chipType));

        // So far, so good. Add the entry.
        o_files.emplace(chipType, path);
    }
}

//------------------------------------------------------------------------------

void __initialize(const fs::path& i_path)
{
    // Get file size.
    const auto sz_buffer = fs::file_size(i_path);

    // Create a buffer large enough to hold the entire file.
    std::vector<char> buffer(sz_buffer);

    // Read the entire file into the buffer.
    std::ifstream file{i_path, std::ios::binary};
    assert(file.good()); // We've opened it once before, so it should open now.
    file.read(buffer.data(), sz_buffer);
    file.close();

    // Initialize the isolator with this chip data file.
    libhei::initialize(buffer.data(), sz_buffer);
}

//------------------------------------------------------------------------------

void initializeIsolator(std::vector<libhei::Chip>& o_chips)
{
    // Get all of the active chips to be analyzed.
    util::pdbg::getActiveChips(o_chips);

    // Find all of the existing chip data files.
    std::map<libhei::ChipType_t, fs::path> files;
    __getChipDataFiles(files);

    // Keep track of models/levels that have already been initialized.
    std::vector<libhei::ChipType_t> types;

    for (const auto& chip : o_chips)
    {
        auto chipType = chip.getType();

        if (types.end() != std::find(types.begin(), types.end(), chipType))
        {
            // This type has already been initialized. Nothing more to do.
            continue;
        }

        // Get the file for this chip.
        auto itr = files.find(chipType);

        // Ensure a chip data file exist for this chip.
        assert(files.end() != itr);

        // Initialize this chip type.
        __initialize(itr->second);

        // Mark this chip type as initialized.
        types.push_back(chipType);
    }
}

//------------------------------------------------------------------------------

} // namespace analyzer
