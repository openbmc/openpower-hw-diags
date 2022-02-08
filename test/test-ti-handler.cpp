#include <fmt/format.h>

#include <attn/ti_handler.hpp>
#include <util/trace.hpp>

#include <map>
#include <string>

#include "gtest/gtest.h"

TEST(TiHandler, TestSet1)
{
    attn::TiDataArea* tiData = nullptr;
    std::map<std::string, std::string> tiMap;

    trace::inf("TestSet1: will test ti handler.");
    // Use the pre-defined array as test data.
    tiData = (attn::TiDataArea*)attn::defaultPhypTiInfo;

    trace::inf("Set values in tiData to the map by parsePhypOpalTiInfo().");
    attn::parsePhypOpalTiInfo(tiMap, tiData);

    EXPECT_EQ(tiMap["0x00 TI Area Valid"],
              fmt::format("{:02x}", tiData->tiAreaValid));
    EXPECT_EQ(tiMap["0x01 Command"], fmt::format("{:02x}", tiData->command));
    EXPECT_EQ(tiMap["0x02 Num. Data Bytes"],
              fmt::format("{:04x}", be16toh(tiData->numDataBytes)));
    EXPECT_EQ(tiMap["0x04 Reserved"], fmt::format("{:02x}", tiData->reserved1));
    EXPECT_EQ(tiMap["0x06 HWDump Type"],
              fmt::format("{:04x}", be16toh(tiData->hardwareDumpType)));
    EXPECT_EQ(tiMap["0x08 SRC Format"],
              fmt::format("{:02x}", tiData->srcFormat));
    EXPECT_EQ(tiMap["0x09 SRC Flags"], fmt::format("{:02x}", tiData->srcFlags));
    EXPECT_EQ(tiMap["0x0a Num. ASCII Words"],
              fmt::format("{:02x}", tiData->numAsciiWords));
    EXPECT_EQ(tiMap["0x0b Num. Hex Words"],
              fmt::format("{:02x}", tiData->numHexWords));
    EXPECT_EQ(tiMap["0x0e Length of SRC"],
              fmt::format("{:04x}", be16toh(tiData->lenSrc)));
    EXPECT_EQ(tiMap["0x10 SRC Word 12"],
              fmt::format("{:08x}", be32toh(tiData->srcWord12HbWord0)));
    EXPECT_EQ(tiMap["0x14 SRC Word 13"],
              fmt::format("{:08x}", be32toh(tiData->srcWord13HbWord2)));
    EXPECT_EQ(tiMap["0x18 SRC Word 14"],
              fmt::format("{:08x}", be32toh(tiData->srcWord14HbWord3)));
    EXPECT_EQ(tiMap["0x1c SRC Word 15"],
              fmt::format("{:08x}", be32toh(tiData->srcWord15HbWord4)));
    EXPECT_EQ(tiMap["0x20 SRC Word 16"],
              fmt::format("{:08x}", be32toh(tiData->srcWord16HbWord5)));
    EXPECT_EQ(tiMap["0x24 SRC Word 17"],
              fmt::format("{:08x}", be32toh(tiData->srcWord17HbWord6)));
    EXPECT_EQ(tiMap["0x28 SRC Word 18"],
              fmt::format("{:08x}", be32toh(tiData->srcWord18HbWord7)));
    EXPECT_EQ(tiMap["0x2c SRC Word 19"],
              fmt::format("{:08x}", be32toh(tiData->srcWord19HbWord8)));
    EXPECT_EQ(tiMap["0x30 ASCII Data"],
              fmt::format("{:08x}", be32toh(tiData->asciiData0)));
    EXPECT_EQ(tiMap["0x34 ASCII Data"],
              fmt::format("{:08x}", be32toh(tiData->asciiData1)));
    EXPECT_EQ(tiMap["0x38 ASCII Data"],
              fmt::format("{:08x}", be32toh(tiData->asciiData2)));
    EXPECT_EQ(tiMap["0x3c ASCII Data"],
              fmt::format("{:08x}", be32toh(tiData->asciiData3)));
    EXPECT_EQ(tiMap["0x40 ASCII Data"],
              fmt::format("{:08x}", be32toh(tiData->asciiData4)));
    EXPECT_EQ(tiMap["0x44 ASCII Data"],
              fmt::format("{:08x}", be32toh(tiData->asciiData5)));
    EXPECT_EQ(tiMap["0x48 ASCII Data"],
              fmt::format("{:08x}", be32toh(tiData->asciiData6)));
    EXPECT_EQ(tiMap["0x4c ASCII Data"],
              fmt::format("{:08x}", be32toh(tiData->asciiData7)));
    EXPECT_EQ(tiMap["0x50 Location"], fmt::format("{:02x}", tiData->location));
    EXPECT_EQ(tiMap["0x51 Code Sections"],
              fmt::format("{:02x}", tiData->codeSection));
    EXPECT_EQ(tiMap["0x52 Additional Size"],
              fmt::format("{:02x}", tiData->additionalSize));
    EXPECT_EQ(tiMap["0x53 Additional Data"],
              fmt::format("{:02x}", tiData->andData));
}

TEST(TiHandler, TestSet2)
{
    attn::TiDataArea* tiData = nullptr;
    std::map<std::string, std::string> tiMap;

    trace::inf("TestSet2: will test ti handler.");
    // Use the pre-defined array as test data.
    tiData = (attn::TiDataArea*)attn::defaultHbTiInfo;

    trace::inf("Set values in tiData to the map by defaultHbTiInfo().");
    attn::parseHbTiInfo(tiMap, tiData);

    EXPECT_EQ(tiMap["0x00 TI Area Valid"],
              fmt::format("{:02x}", tiData->tiAreaValid));
    EXPECT_EQ(tiMap["0x04 Reserved"], fmt::format("{:02x}", tiData->reserved1));
    EXPECT_EQ(tiMap["0x05 HB_Term. Type"],
              fmt::format("{:02x}", tiData->hbTerminateType));
    EXPECT_EQ(tiMap["0x0c HB Dump Flag"],
              fmt::format("{:02x}", tiData->hbDumpFlag));
    EXPECT_EQ(tiMap["0x0d Source"], fmt::format("{:02x}", tiData->source));
    EXPECT_EQ(tiMap["0x10 HB Word 0"],
              fmt::format("{:08x}", be32toh(tiData->srcWord12HbWord0)));
    EXPECT_EQ(tiMap["0x14 HB Word 2"],
              fmt::format("{:08x}", be32toh(tiData->srcWord13HbWord2)));
    EXPECT_EQ(tiMap["0x18 HB Word 3"],
              fmt::format("{:08x}", be32toh(tiData->srcWord14HbWord3)));
    EXPECT_EQ(tiMap["0x1c HB Word 4"],
              fmt::format("{:08x}", be32toh(tiData->srcWord15HbWord4)));
    EXPECT_EQ(tiMap["0x20 HB Word 5"],
              fmt::format("{:08x}", be32toh(tiData->srcWord16HbWord5)));
    EXPECT_EQ(tiMap["0x24 HB Word 6"],
              fmt::format("{:08x}", be32toh(tiData->srcWord17HbWord6)));
    EXPECT_EQ(tiMap["0x28 HB Word 7"],
              fmt::format("{:08x}", be32toh(tiData->srcWord18HbWord7)));
    EXPECT_EQ(tiMap["0x2c HB Word 8"],
              fmt::format("{:08x}", be32toh(tiData->srcWord19HbWord8)));
    EXPECT_EQ(tiMap["0x30 error_data"],
              fmt::format("{:08x}", be32toh(tiData->asciiData0)));
    EXPECT_EQ(tiMap["0x34 EID"],
              fmt::format("{:08x}", be32toh(tiData->asciiData1)));
}