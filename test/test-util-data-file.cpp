#include <util/data_file.hpp>
#include <util/temporary_file.hpp>
#include <util/trace.hpp>

#include "gtest/gtest.h"

using namespace std;
using namespace util;

using json = nlohmann::json;

TEST(UtilDataFile, TestFindFiles)
{
    TemporaryFile tempFile1{};
    TemporaryFile tempFile2{};

    string fullPathTempFile1 = tempFile1.getPath();
    string fullPathTempFile2 = tempFile2.getPath();
    EXPECT_NE(fullPathTempFile1, fullPathTempFile2);

    trace::inf("fullPathTempFile1: %s", fullPathTempFile1.c_str());
    trace::inf("fullPathTempFile2: %s", fullPathTempFile2.c_str());

    // path1 and path2 will be used to test later.
    fs::path path1 = fullPathTempFile1;
    fs::path path2 = fullPathTempFile2;

    // parent pathes for path1 and path2 are the same.
    fs::path dataDir  = path1.parent_path();
    auto regexPattern = R"(openpower\-hw\-diags\-.*)";
    vector<fs::path> dataPaths;

    trace::inf("parent path: %s", string(dataDir).c_str());
    // call the function under test.
    util::findFiles(dataDir, regexPattern, dataPaths);

    for (auto path : dataPaths)
    {
        trace::inf("path in dataPath vector: %s", path.c_str());
    }

    EXPECT_EQ(2, dataPaths.size());

    vector<fs::path>::iterator it;
    it = find(dataPaths.begin(), dataPaths.end(), path1);
    EXPECT_TRUE(it != dataPaths.end());
    it = find(dataPaths.begin(), dataPaths.end(), path2);
    EXPECT_TRUE(it != dataPaths.end());
}

TEST(UtilDataFile, TestValidateJson)
{
    json schema_obj;
    json json_obj;

    string schema_text    = R"({
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "title": "UT copied and updated from RAS Data schema for openpower-hw-diags",
    "version": 1,
    "type": "object",
    "additionalProperties": false,
    "required": [ "model_ec" ],
    "properties": {
        "model_ec": {
            "type": "string",
            "pattern": "^[0-9A-Fa-f]{8}$"
        },
        "version": {
            "type": "integer",
            "minimum": 1
        }
    }
})";
    std::string json_text = R"({
   "model_ec" : "20da0020",
   "version" : 1
})";

    schema_obj = json::parse(schema_text);
    json_obj   = json::parse(json_text);

    EXPECT_TRUE(util::validateJson(schema_obj, json_obj));
}
