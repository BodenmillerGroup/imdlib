#include <gtest/gtest.h>
#include <iostream>

#include <IMDFile.h>

#define IMD_FILE_PATH ""

using namespace imd;

TEST(IMDFile, read) {
    IMDFile imdFile(IMD_FILE_PATH);
    auto data = imdFile.readData();
}

TEST(IMDFile, readMetadata) {
    IMDFile imdFile(IMD_FILE_PATH);
    auto metadata = imdFile.readMetadata();
}