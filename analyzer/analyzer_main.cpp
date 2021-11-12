#include <assert.h>
#include <unistd.h>

#include <analyzer/ras-data/ras-data-parser.hpp>
#include <analyzer/service_data.hpp>
#include <attn/attn_dump.hpp>
#include <hei_main.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

namespace analyzer
{

//------------------------------------------------------------------------------

// Forward references for externally defined functions.

/**
 * @brief Will get the list of active chip and initialize the isolator.
 * @param o_chips The returned list of active chips.
 */
void initializeIsolator(std::vector<libhei::Chip>& o_chips);

/**
 * @brief  Will get the list of active chip and initialize the isolator.
 * @param  i_isoData   The data gathered during isolation (for FFDC).
 * @param  o_rootCause The returned root cause signature.
 * @return True, if root cause has been found. False, otherwise.
 */
bool filterRootCause(const libhei::IsolationData& i_isoData,
                     libhei::Signature& o_rootCause);

/**
 * @brief Will create and submit a PEL using the given data.
 * @param i_isoData   The data gathered during isolation (for FFDC).
 * @param i_servData  Data regarding service actions gathered during analysis.
 * @return Tuple of BMC log id, platform log id
 */
std::tuple<uint32_t, uint32_t> createPel(const libhei::IsolationData& i_isoData,
                                         const ServiceData& i_servData);

//------------------------------------------------------------------------------

const char* __attn(libhei::AttentionType_t i_attnType)
{
    const char* str = "";
    switch (i_attnType)
    {
        case libhei::ATTN_TYPE_CHECKSTOP:
            str = "CHECKSTOP";
            break;
        case libhei::ATTN_TYPE_UNIT_CS:
            str = "UNIT_CS";
            break;
        case libhei::ATTN_TYPE_RECOVERABLE:
            str = "RECOVERABLE";
            break;
        case libhei::ATTN_TYPE_SP_ATTN:
            str = "SP_ATTN";
            break;
        case libhei::ATTN_TYPE_HOST_ATTN:
            str = "HOST_ATTN";
            break;
        default:
            trace::err("Unsupported attention type: %u", i_attnType);
            assert(0);
    }
    return str;
}

//------------------------------------------------------------------------------

bool analyzeHardware(attn::DumpParameters& o_dumpParameters)
{
    bool attnFound = false;

    if (!util::pdbg::queryHardwareAnalysisSupported())
    {
        trace::err("Hardware error analysis is not supported on this system");
        return attnFound;
    }

    trace::inf(">>> enter analyzeHardware()");

    // Initialize the isolator and get all of the chips to be analyzed.
    trace::inf("Initializing the isolator...");
    std::vector<libhei::Chip> chips;
    initializeIsolator(chips);

    // Isolate attentions.
    trace::inf("Isolating errors: # of chips=%u", chips.size());
    libhei::IsolationData isoData{};
    libhei::isolate(chips, isoData);

    // For debug, trace out the original list of signatures before filtering.
    for (const auto& sig : isoData.getSignatureList())
    {
        trace::inf("Signature: %s 0x%0" PRIx32 " %s",
                   util::pdbg::getPath(sig.getChip()), sig.toUint32(),
                   __attn(sig.getAttnType()));
    }

    // Filter for root cause attention.
    libhei::Signature rootCause{};
    attnFound = filterRootCause(isoData, rootCause);

    if (!attnFound)
    {
        // It is possible for TI handling, or manually initiated analysis via
        // the command line, that there will not be an active attention. In
        // which case, we will do nothing and let the caller of this function
        // determine if this is the expected behavior.
        trace::inf("No active attentions found");
    }
    else
    {
        trace::inf("Root cause attention: %s 0x%0" PRIx32 " %s",
                   util::pdbg::getPath(rootCause.getChip()),
                   rootCause.toUint32(), __attn(rootCause.getAttnType()));

        // Resolve any service actions required by the root cause.
        RasDataParser rasData{};
        ServiceData servData{rootCause, isoData.queryCheckstop()};
        rasData.getResolution(rootCause)->resolve(servData);

        // Create and commit a PEL.
        uint32_t logId = std::get<1>(createPel(isoData, servData));

        trace::inf("PEL created: PLID=0x%0" PRIx32, logId);

        // Gather/return information needed for dump.
        // TODO: Need ID from root cause. At the moment, HUID does not exist in
        //       devtree. Will need a better ID definition.
        // TODO: HW dump is default, but some attentions may require something
        //       different. Will need to investigate adding that information to
        //       the RAS data files.
        o_dumpParameters.logId    = logId;
        o_dumpParameters.unitId   = 0;
        o_dumpParameters.dumpType = attn::DumpType::Hardware;
    }

    // All done, clean up the isolator.
    trace::inf("Uninitializing isolator...");
    libhei::uninitialize();

    trace::inf("<<< exit analyzeHardware()");

    return attnFound;
}

//------------------------------------------------------------------------------

/**
 * @brief Get error isolator build information
 *
 * @return Pointer to build information
 */
const char* getBuildInfo()
{
    return libhei::getBuildInfo();
}

} // namespace analyzer
