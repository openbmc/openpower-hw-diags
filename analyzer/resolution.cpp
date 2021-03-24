#include <analyzer/resolution.hpp>
#include <util/pdbg.hpp>

namespace analyzer
{

void HardwareCalloutResolution::resolve(ServiceData& io_sd) const
{
    // Get the chip target from the root cause signature.
    auto trgt = util::pdbg::getTrgt(io_sd.getRootCause().getChip());

    // Get the unit target, if needed.
    if (!iv_path.empty())
    {
        std::string path{util::pdbg::getPath(trgt)};
        trgt = util::pdbg::getTrgt(path + "/" + iv_path);
    }

    // Add location code to callout list.
    auto locCode = util::pdbg::getLocationCode(trgt);
    io_sd.addCallout(std::make_shared<HardwareCallout>(locCode, iv_priority));

    // Add entity path to gard list.
    auto entityPath = util::pdbg::getPhysDevPath(trgt);
    io_sd.addGuard(std::make_shared<Guard>(entityPath, iv_guard));
}

} // namespace analyzer
