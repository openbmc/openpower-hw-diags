#include <fmt/format.h>

#include <attn/ti_handler.hpp>
#include <util/trace.hpp>

#include <bit>
#include <map>
#include <string>

#include "gtest/gtest.h"

namespace
{

template <typename T>
std::string uint_to_str(T data)
{
    std::string ret = "";
    throw std::invalid_argument("Invalid type of argument used.");
    return ret;
}

template <>
std::string uint_to_str<uint8_t>(uint8_t data)
{
    std::string ret = fmt::format("{:02x}", data);
    return ret;
}

template <>
std::string uint_to_str<uint16_t>(uint16_t data)
{
    std::string ret = fmt::format("{:04x}", be16toh(data));
    return ret;
}

template <>
std::string uint_to_str<uint32_t>(uint32_t data)
{
    std::string ret = fmt::format("{:08x}", be32toh(data));
    return ret;
}
} // namespace

TEST(TiHandler, TestSet0)
{
    trace::inf("TestSet0: will test ti handler.");

    union TestData
    {
        uint8_t data1;
        uint16_t data2;
        uint32_t data3;
        uint8_t data4[4];
    } data = {.data4 = {0x01, 0x23, 0x45, 0x67}};

    EXPECT_EQ("01", uint_to_str<uint8_t>(data.data1));
    if constexpr (std::endian::native == std::endian::big)
    {
        EXPECT_EQ("2301", uint_to_str<uint16_t>(data.data2));
        EXPECT_EQ("67452301", uint_to_str<uint32_t>(data.data3));
        EXPECT_EQ("2301", uint_to_str<uint16_t>(
                              *reinterpret_cast<uint16_t*>(&data.data4[0])));
        EXPECT_EQ("67452301",
                  uint_to_str<uint32_t>(
                      *reinterpret_cast<uint32_t*>(&data.data4[0])));
    }
    else if constexpr (std::endian::native == std::endian::little)
    {
        EXPECT_EQ("0123", uint_to_str<uint16_t>(data.data2));
        EXPECT_EQ("01234567", uint_to_str<uint32_t>(data.data3));
        EXPECT_EQ("0123", uint_to_str<uint16_t>(
                              *reinterpret_cast<uint16_t*>(&data.data4[0])));
        EXPECT_EQ("01234567",
                  uint_to_str<uint32_t>(
                      *reinterpret_cast<uint32_t*>(&data.data4[0])));
    }
}

TEST(TiHandler, TestSet1)
{
    trace::inf("TestSet1: will test ti handler.");

    uint8_t tiInfo[0x58] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
        0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
        0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
        0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
        0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
        0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41,
        0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c,
        0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57};

    std::map<std::string, std::string> tiMap;
    attn::TiDataArea* tiData = (attn::TiDataArea*)tiInfo;

    trace::inf("Set values in tiData to the map by parsePhypOpalTiInfo().");
    attn::parsePhypOpalTiInfo(tiMap, tiData);

    EXPECT_EQ(tiMap["0x00 TI Area Valid"], uint_to_str<uint8_t>(tiInfo[0x00]));
    EXPECT_EQ(tiMap["0x01 Command"], uint_to_str<uint8_t>(tiInfo[0x01]));
    EXPECT_EQ(
        tiMap["0x02 Num. Data Bytes"],
        uint_to_str<uint16_t>(*reinterpret_cast<uint16_t*>(&tiInfo[0x02])));
    EXPECT_EQ(tiMap["0x04 Reserved"], uint_to_str<uint8_t>(tiInfo[0x04]));
    EXPECT_EQ(
        tiMap["0x06 HWDump Type"],
        uint_to_str<uint16_t>(*reinterpret_cast<uint16_t*>(&tiInfo[0x06])));
    EXPECT_EQ(tiMap["0x08 SRC Format"], uint_to_str<uint8_t>(tiInfo[0x08]));
    EXPECT_EQ(tiMap["0x09 SRC Flags"], uint_to_str<uint8_t>(tiInfo[0x09]));
    EXPECT_EQ(tiMap["0x0a Num. ASCII Words"],
              uint_to_str<uint8_t>(tiInfo[0x0a]));
    EXPECT_EQ(tiMap["0x0b Num. Hex Words"], uint_to_str<uint8_t>(tiInfo[0x0b]));
    EXPECT_EQ(
        tiMap["0x0e Length of SRC"],
        uint_to_str<uint16_t>(*reinterpret_cast<uint16_t*>(&tiInfo[0x0e])));
    EXPECT_EQ(
        tiMap["0x10 SRC Word 12"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x10])));
    EXPECT_EQ(
        tiMap["0x14 SRC Word 13"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x14])));
    EXPECT_EQ(
        tiMap["0x18 SRC Word 14"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x18])));
    EXPECT_EQ(
        tiMap["0x1c SRC Word 15"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x1c])));
    EXPECT_EQ(
        tiMap["0x20 SRC Word 16"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x20])));
    EXPECT_EQ(
        tiMap["0x24 SRC Word 17"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x24])));
    EXPECT_EQ(
        tiMap["0x28 SRC Word 18"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x28])));
    EXPECT_EQ(
        tiMap["0x2c SRC Word 19"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x2c])));
    EXPECT_EQ(
        tiMap["0x30 ASCII Data"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x30])));
    EXPECT_EQ(
        tiMap["0x34 ASCII Data"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x34])));
    EXPECT_EQ(
        tiMap["0x38 ASCII Data"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x38])));
    EXPECT_EQ(
        tiMap["0x3c ASCII Data"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x3c])));
    EXPECT_EQ(
        tiMap["0x40 ASCII Data"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x40])));
    EXPECT_EQ(
        tiMap["0x44 ASCII Data"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x44])));
    EXPECT_EQ(
        tiMap["0x48 ASCII Data"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x48])));
    EXPECT_EQ(
        tiMap["0x4c ASCII Data"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x4c])));
    EXPECT_EQ(tiMap["0x50 Location"], uint_to_str<uint8_t>(tiInfo[0x50]));
    EXPECT_EQ(tiMap["0x51 Code Sections"], uint_to_str<uint8_t>(tiInfo[0x51]));
    EXPECT_EQ(tiMap["0x52 Additional Size"],
              uint_to_str<uint8_t>(tiInfo[0x52]));
    EXPECT_EQ(tiMap["0x53 Additional Data"],
              uint_to_str<uint8_t>(tiInfo[0x53]));
}

TEST(TiHandler, TestSet2)
{
    trace::inf("TestSet2: will test ti handler.");

    uint8_t tiInfo[0x58] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
        0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
        0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
        0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
        0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
        0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41,
        0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c,
        0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57};

    attn::TiDataArea* tiData = nullptr;
    std::map<std::string, std::string> tiMap;

    // Use the pre-defined array as test data.
    tiData = (attn::TiDataArea*)tiInfo;

    trace::inf("Set values in tiData to the map by defaultHbTiInfo().");
    attn::parseHbTiInfo(tiMap, tiData);

    EXPECT_EQ(tiMap["0x00 TI Area Valid"], uint_to_str<uint8_t>(tiInfo[0x00]));
    EXPECT_EQ(tiMap["0x04 Reserved"], uint_to_str<uint8_t>(tiInfo[0x04]));
    EXPECT_EQ(tiMap["0x05 HB_Term. Type"], uint_to_str<uint8_t>(tiInfo[0x05]));
    EXPECT_EQ(tiMap["0x0c HB Dump Flag"], uint_to_str<uint8_t>(tiInfo[0x0c]));
    EXPECT_EQ(tiMap["0x0d Source"], uint_to_str<uint8_t>(tiInfo[0x0d]));
    EXPECT_EQ(
        tiMap["0x10 HB Word 0"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x10])));
    EXPECT_EQ(
        tiMap["0x14 HB Word 2"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x14])));
    EXPECT_EQ(
        tiMap["0x18 HB Word 3"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x18])));
    EXPECT_EQ(
        tiMap["0x1c HB Word 4"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x1c])));
    EXPECT_EQ(
        tiMap["0x20 HB Word 5"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x20])));
    EXPECT_EQ(
        tiMap["0x24 HB Word 6"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x24])));
    EXPECT_EQ(
        tiMap["0x28 HB Word 7"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x28])));
    EXPECT_EQ(
        tiMap["0x2c HB Word 8"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x2c])));
    EXPECT_EQ(
        tiMap["0x30 error_data"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x30])));
    EXPECT_EQ(
        tiMap["0x34 EID"],
        uint_to_str<uint32_t>(*reinterpret_cast<uint32_t*>(&tiInfo[0x34])));
}
