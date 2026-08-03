#include <gtest/gtest.h>

extern "C" {
#include "iso14229.h"
}

TEST(SmokeTest, BasicAssertionsWork) {
    EXPECT_EQ(1, 1);
    EXPECT_TRUE(true);
}

TEST(UDSErrToStr, ReturnsCorrectStringForKnownError) {
    EXPECT_STREQ(UDSErrToStr(UDS_OK), "UDS_OK");
    EXPECT_STREQ(UDSErrToStr(UDS_NRC_RequestOutOfRange), "UDS_NRC_RequestOutOfRange");
}

TEST(UDSErrToStr, ReturnsUnknownForInvalidValue) {
    EXPECT_STREQ(UDSErrToStr((UDSErr_t)-999), "unknown");
}
