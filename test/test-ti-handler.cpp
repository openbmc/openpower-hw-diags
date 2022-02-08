// TODO: will use fmt::format later.
// #include <fmt/format.h>

#include <attn/ti_handler.hpp>
#include <util/trace.hpp>

#include <map>
#include <string>

#include "gtest/gtest.h"

TEST(TiHandler, TestSet1)
{
    uint8_t* tiInfo          = nullptr;
    attn::TiDataArea* tiData = nullptr;

    std::map<std::string, std::string> tiMap;

    trace::inf("TestSet1: will test ti handler.");

    tiInfo = (uint8_t*)attn::defaultPhypTiInfo;
    tiData = (attn::TiDataArea*)tiInfo;

    trace::inf("set value in tiData to the map by parsePhypOpalTiInfo().");
    attn::parsePhypOpalTiInfo(tiMap, tiData);

    // TODO: will use fmt::format later.
    // check this insertion in parsePhypOpalTiInfo() function.
    // i_map["0x00 TI Area Valid"] =
    //     fmt::format("{:02x}", i_tiDataArea->tiAreaValid);
    // ASSERT_EQ(tiMap["0x00 TI Area Valid"],
    //           fmt::format("{:02x}", tiData->tiAreaValid));
    ASSERT_EQ(tiMap["0x00 TI Area Valid"], "00");
}