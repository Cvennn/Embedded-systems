#include <gtest/gtest.h>
#include "../TimeParser.h"

// Test suite: TimeParserTest
TEST(TimeParserTest, TestCaseCorrectTime) {
    // Test with correct time string
    char time_test[] = "000005";
    EXPECT_EQ(time_parse(time_test), 5);

}

TEST(TimeParserTest, TestCaseIncorrectTime) {
    // Test with incorrect time string
    char time_test[] = "000066";
    ASSERT_EQ(time_parse(time_test), TIME_VALUE_ERROR);
}

TEST(TimeParserTest, TestCaseIncorrectLengthShort) {

    // Test with incorrect length
    char time_test[] = "14120";
    ASSERT_EQ(time_parse(time_test), TIME_LEN_ERROR);

}

TEST(TimeParserTest, TestCaseIncorrectLengthLong) {
    // Test with incorrect length
    char time_test[] = "1412000";
    ASSERT_EQ(time_parse(time_test), TIME_LEN_ERROR);
}

TEST(TimeParserTest, TestCaseNullptr) {
    // Test with nullptr
    char* time_test = nullptr;
    ASSERT_EQ(time_parse(time_test), TIME_ARRAY_ERROR);
}

// https://google.github.io/googletest/reference/testing.html
// https://google.github.io/googletest/reference/assertions.html
