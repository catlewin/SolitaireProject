#include <gtest/gtest.h>
#include "BoardConfig.h"

// -----------------------------------------------------------------------
// AC 1.1 - Default board size is 7
// -----------------------------------------------------------------------
TEST(BoardConfigTest, DefaultSizeIsSeven) {
    BoardConfig config;
    EXPECT_EQ(config.size, 7);
}

// AC 1.6 - Default board type is English
TEST(BoardConfigTest, DefaultTypeIsEnglish) {
    BoardConfig config;
    EXPECT_EQ(config.type, BoardType::English);
}

// AC 2.1 - Default game mode is Manual
TEST(BoardConfigTest, DefaultModeIsManual) {
    BoardConfig config;
    EXPECT_EQ(config.mode, GameMode::Manual);
}

// -----------------------------------------------------------------------
// AC 1.2 - Valid board sizes are accepted
// -----------------------------------------------------------------------
TEST(BoardConfigTest, ValidSizeMinBoundary) {
    BoardConfig config;
    EXPECT_EQ(config.validateAndSetSize("5"), ConfigError::None);
    EXPECT_EQ(config.size, 5);
}

TEST(BoardConfigTest, ValidSizeMaxBoundary) {
    BoardConfig config;
    EXPECT_EQ(config.validateAndSetSize("10"), ConfigError::None);
    EXPECT_EQ(config.size, 10);
}

TEST(BoardConfigTest, ValidSizeMidRange) {
    BoardConfig config;
    EXPECT_EQ(config.validateAndSetSize("7"), ConfigError::None);
    EXPECT_EQ(config.size, 7);
}

// -----------------------------------------------------------------------
// AC 1.3 - Out of range size resets to 7 and returns OutOfRange
// -----------------------------------------------------------------------
TEST(BoardConfigTest, SizeTooSmallResetsToDefault) {
    BoardConfig config;
    EXPECT_EQ(config.validateAndSetSize("4"), ConfigError::OutOfRange);
    EXPECT_EQ(config.size, 7);
}

TEST(BoardConfigTest, SizeTooLargeResetsToDefault) {
    BoardConfig config;
    EXPECT_EQ(config.validateAndSetSize("11"), ConfigError::OutOfRange);
    EXPECT_EQ(config.size, 7);
}

TEST(BoardConfigTest, SizeZeroResetsToDefault) {
    BoardConfig config;
    EXPECT_EQ(config.validateAndSetSize("0"), ConfigError::OutOfRange);
    EXPECT_EQ(config.size, 7);
}

// -----------------------------------------------------------------------
// AC 1.4 - Non-numeric input resets to 7 and returns NonNumeric
// -----------------------------------------------------------------------
TEST(BoardConfigTest, NonNumericLetterResetsToDefault) {
    BoardConfig config;
    EXPECT_EQ(config.validateAndSetSize("abc"), ConfigError::NonNumeric);
    EXPECT_EQ(config.size, 7);
}

TEST(BoardConfigTest, NonNumericMixedResetsToDefault) {
    BoardConfig config;
    EXPECT_EQ(config.validateAndSetSize("7a"), ConfigError::NonNumeric);
    EXPECT_EQ(config.size, 7);
}

TEST(BoardConfigTest, NonNumericEmptyStringResetsToDefault) {
    BoardConfig config;
// Empty string has no digits — treated as non-numeric
    EXPECT_NE(config.validateAndSetSize(""), ConfigError::None);
    EXPECT_EQ(config.size, 7);
}

TEST(BoardConfigTest, NonNumericSpecialCharResetsToDefault) {
    BoardConfig config;
    EXPECT_EQ(config.validateAndSetSize("!"), ConfigError::NonNumeric);
    EXPECT_EQ(config.size, 7);
}