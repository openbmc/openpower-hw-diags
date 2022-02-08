#include <attn/ti_handler.hpp>
#include <util/pdbg.hpp>
#include <util/trace.hpp>

#include <map>
#include <string>

#include "gtest/gtest.h"

using namespace attn;

TEST(Resolution, TestSet1)
{
    TiDataArea* tiData = nullptr;
    std::map<std::string, std::string> tiMap;

    trace::inf("TestSet1: will test ti handler.");

    tiData = (TiDataArea*)defaultPhypTiInfo;

    trace::inf("set itInfo values to the map.");
    parsePhypOpalTiInfo(tiMap, tiData);

    // check this insertion in parsePhypOpalTiInfo() function.
    // i_map["0x00 TI Area Valid"] =
    //     fmt::format("{:02x}", i_tiDataArea->tiAreaValid);
    ASSERT_EQ(tiMap["0x00 TI Area Valid"], tiData->tiAreaValid);
}