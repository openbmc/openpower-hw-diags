#include <libpdbg.h>

#include <hei_main.hpp>

#include <fstream>
#include <iostream>
#include <map>

namespace analyzer
{

// FIXME TEMP CODE - begin

static constexpr uint32_t chipTypeP10      = 0;
static constexpr uint32_t chipTypeExplorer = 1;

// FIXME TEMP CODE - end

/**
 * @brief send chip data file to isolator
 *
 * Read a chip data file into memory and then send it to the isolator via
 * the initialize interface.
 *
 * @param i_filePath The file path and name to read into memory
 *
 * @return Returns true if the isolator was successfully initialized with
 *         a single chip data file. Returns false otherwise.
 *
 */
bool initWithFile(const char* i_filePath)
{
    using namespace libhei;

    bool rc = true; // assume success

    // open the file and seek to the end to get length
    std::ifstream fileStream(i_filePath, std::ios::binary | std::ios::ate);

    if (!fileStream)
    {
        std::cout << "could not open file" << std::endl;
        rc = false;
    }
    else
    {
        // get file size based on seek position
        std::ifstream::pos_type fileSize = fileStream.tellg();

        // create a buffer large enough to hold the entire file
        std::vector<char> fileBuffer(fileSize);

        // seek to the beginning of the file
        fileStream.seekg(0, std::ios::beg);

        // read the entire file into the buffer
        fileStream.read(fileBuffer.data(), fileSize);

        // done with the file
        fileStream.close();

        // intialize the isolator with the chip data
        initialize(fileBuffer.data(), fileSize); // hei initialize
    }

    return rc;
}

/**
 * @brief Analyze using the hardware error isolator
 *
 * Query the hardware for each active chip that is a valid candidate for
 * error analyses. Based on the list of active chips initialize the
 * isolator with the associated chip data files. Finally request analyses
 * from the hardware error isolator and log the results.
 *
 * @param o_errors A map for storing information about erros that were
 *                 detected by the hardware error isolator.
 *
 * @return True if hardware error analyses was successful, false otherwise
 */
bool analyzeHardware(std::map<std::string, std::string>& o_errors)
{
    using namespace libhei;

    bool rc = true;

    std::vector<Chip> chipList; // chips that need to be analyzed

    IsolationData isoData{}; // data from isolato

    pdbg_target *targetProc, *targetOcmb; // P10 and explorer targets

    pdbg_targets_init(NULL); // initialize targets

    /** @brief gather list of chips to analyze */
    pdbg_for_each_class_target("proc", targetProc)
    {
        // add each processor chip to the chip list
        chipList.emplace_back(Chip(targetProc, chipTypeP10));

        if (PDBG_TARGET_ENABLED == pdbg_target_probe(targetProc))
        {
            pdbg_for_each_compatible(targetProc, targetOcmb, "ibm,power10-ocmb")
            {
                if (PDBG_TARGET_ENABLED == pdbg_target_probe(targetOcmb))
                {
                    // add each explorer chip (ocmb) to the chip list
                    chipList.emplace_back(Chip(targetOcmb, chipTypeExplorer));
                }
            }
        }
    }

    // TODO select chip data files based on chip types detected
    do
    {
        // TODO for now chip data files are local
        // hei initialize
        if (false == initWithFile("/usr/share/openpower-hw-diags/p10.cdb"))
        {
            rc = false;
            break;
        }

        // TODO for now chip data files are local
        // hei initialize
        if (false == initWithFile("/usr/share/openpower-hw-diags/explorer.cdb"))
        {
            rc = false;
            break;
        }

        // hei isolate
        isolate(chipList, isoData);

        if (!(isoData.getSignatureList().empty()))
        {
            // TODO parse signature list
            int numErrors = isoData.getSignatureList().size();

            std::cout << "isolated: " << numErrors << std::endl;
        }
        else
        {
            // FIXME TEMP CODE - begin

            o_errors["0xfeed"] = "0xbeef"; // [signature] = chip

            // FIXME TEMP CODE - end
        }

        // hei uninitialize
        uninitialize();

    } while (0);

    return rc;
} // namespace analyzer

} // namespace analyzer
