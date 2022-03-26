#include <util/data_file.hpp>
#include <util/temporary_file.hpp>
#include <util/trace.hpp>

#include "gtest/gtest.h"

using namespace std;
using namespace util;

using json = nlohmann::json;

TEST(UtilDataFile, TestFindFiles)
{
    // The parent folder is determined by the file system.
    // On Linux it is /tmp.
    fs::path dataDir = fs::temp_directory_path();
    // pattern of the temp files used by TemporaryFile class.
    auto regexPattern = R"(openpower\-hw\-diags\-.*)";
    // The vector to store existing temp files before creating new ones.
    vector<fs::path> existingDataPaths;
    // The vector to store new and existing temp files.
    vector<fs::path> dataPaths;

    trace::inf("parent path: %s", string(dataDir).c_str());
    // Before creating temp files, call the function under test.
    util::findFiles(dataDir, regexPattern, existingDataPaths);

    for (auto path : existingDataPaths)
    {
        trace::inf("path in existingDataPath vector: %s", path.c_str());
    }

    // Create two new temp file objects.
    TemporaryFile tempFile1{};
    TemporaryFile tempFile2{};

    string fullPathTempFile1 = tempFile1.getPath();
    string fullPathTempFile2 = tempFile2.getPath();
    EXPECT_NE(fullPathTempFile1, fullPathTempFile2);

    trace::inf("fullPathTempFile1: %s", fullPathTempFile1.c_str());
    trace::inf("fullPathTempFile2: %s", fullPathTempFile2.c_str());

    // Paths of two temp files.
    // path1 and path2 will be used to test later.
    fs::path path1 = fullPathTempFile1;
    fs::path path2 = fullPathTempFile2;

    // After creating new temp files, call the function under test.
    util::findFiles(dataDir, regexPattern, dataPaths);

    for (auto path : dataPaths)
    {
        trace::inf("path in dataPath vector: %s", path.c_str());
    }

    // TODO
    // Sometimes, by the time calling the function below,
    // the existing temp files have been deleted,
    // and the following testing functino will not be true.
    EXPECT_EQ((2 + existingDataPaths.size()), dataPaths.size());

    vector<fs::path>::iterator it;
    // Before removing the newly created temp files.
    it = find(dataPaths.begin(), dataPaths.end(), path1);
    EXPECT_TRUE(it != dataPaths.end());
    it = find(dataPaths.begin(), dataPaths.end(), path2);
    EXPECT_TRUE(it != dataPaths.end());

    // Remove the newly created temp files.
    tempFile1.remove();
    tempFile2.remove();

    // After removing the newly created temp files, the temp file path entries
    // are still in the dataPaths vector.
    // The test complete.
}

TEST(UtilDataFile, TestValidateJson)
{
    // The json object that is used as schema for other json objects.
    // Copied/modified from file:
    // ./analyzer/ras-data/schema/ras-data-schema-v01.json
    json schema_obj = R"({
    "$schema": "https://json-schema.org/draft/2020-12/schema",
    "title": "RAS Data schema for openpower-hw-diags",
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
})"_json;

    // The json objects
    // Copied/modified from file:
    // ./analyzer/ras-data/data/ras-data-p10-20.json
    json json_obj = R"({
    "model_ec" : "20da0020",
    "version" : 1
})"_json;

    EXPECT_TRUE(util::validateJson(schema_obj, json_obj));

    // Test negative scenario.
    // The key "version_1" does not exist in the schema.
    json json_obj1 = R"({
    "model_ec" : "20da0020",
    "version_1" : 1
})"_json;

    EXPECT_FALSE(util::validateJson(schema_obj, json_obj1));
}
