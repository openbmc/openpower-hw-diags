#include <util/data_file.hpp>
#include <util/temporary_file.hpp>
#include <util/trace.hpp>

#include "gtest/gtest.h"

using namespace std;
using namespace util;

TEST(UtileDataFile, TestFindFiles)
{
    TemporaryFile tempFile1{};
    TemporaryFile tempFile2{};

    string fullPathTempFile1 = tempFile1.getPath();
    string fullPathTempFile2 = tempFile2.getPath();
    EXPECT_NE(fullPathTempFile1, fullPathTempFile2);

    trace::inf("fullPathTempFile1: %s", fullPathTempFile2.c_str());
    trace::inf("fullPathTempFile2: %s", fullPathTempFile2.c_str());

    fs::path path1 = fullPathTempFile1;
    fs::path path2 = fullPathTempFile2;

    trace::inf("parent path: %s", string(path1).c_str());

    fs::path dataDir  = path1.parent_path();
    auto regexPattern = R"(openpower\-hw\-diags\-.*)";
    vector<fs::path> dataPaths;
    trace::inf("Will call function findFiles()");

    util::findFiles(dataDir, regexPattern, dataPaths);

    for (auto path : dataPaths)
    {
        trace::inf("path: %s", path.c_str());
    }

    EXPECT_EQ(2, dataPaths.size());

    vector<fs::path>::iterator it;
    it = find(dataPaths.begin(), dataPaths.end(), path1);
    EXPECT_TRUE(it != dataPaths.end());
    it = find(dataPaths.begin(), dataPaths.end(), path2);
    EXPECT_TRUE(it != dataPaths.end());
}
