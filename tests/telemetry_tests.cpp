#include "LP/telemetry.h"
#include <cstring>
#include <gtest/gtest.h>
#include <string>
#include <vector>

class TelemetryTest : public ::testing::Test {
  protected:
    LP::Telemetry tel;
};

// === SERIAL BUFFER PARSING ===
TEST_F(TelemetryTest, SerialParsing_Correct) {
    std::vector<char> buffer = {'6', ' ', '9', '\n', '1', '2'};

    std::string frame_stream = tel.parse_serial(buffer);

    EXPECT_EQ(frame_stream, "6 9\n");
    EXPECT_EQ(tel.get_frame_fragments(), "12");
}

TEST_F(TelemetryTest, SerialParsing_Empty) {
    // missing frame end chars
    std::vector<char> buffer = {'6', ' ', '9'};

    std::string frame_stream = tel.parse_serial(buffer);

    EXPECT_EQ(frame_stream, "");
    EXPECT_EQ(tel.get_frame_fragments(), "6 9");
}

// === FRAME PARSING ===
TEST_F(TelemetryTest, ParseFrame_Unnamed) {
    std::vector<double> expected_ch1 = {15, 14, 4};
    std::vector<double> expected_ch2 = {12, 5, 3};
    std::vector<double> expected_ch3 = {3, 2, 0};

    std::string frame_stream = "15 12 3\n14 5 2\n4 3 0\n";

    tel.parse_frame(frame_stream);

    auto data = *tel.get_data();

    EXPECT_EQ(data[1].values, expected_ch1);
    EXPECT_EQ(data[2].values, expected_ch2);
    EXPECT_EQ(data[3].values, expected_ch3);
}

TEST_F(TelemetryTest, ParseFrame_Named) {
    std::vector<double> expected_ch1 = {15, 14, 4};
    std::vector<double> expected_ch2 = {12, 5, 3};
    std::vector<double> expected_ch3 = {3, 2, 0};

    std::string frame_stream = "x = 15 y = 12 z = 3\nx = 14 y = 5 z = 2\nx = 4 y = 3 z = 0\n";

    tel.frame_format.named = true;
    std::strcpy(tel.frame_format.name_sep, " = ");

    tel.parse_frame(frame_stream);

    auto data = *tel.get_data();

    EXPECT_EQ(data[1].name, "x");
    EXPECT_EQ(data[2].name, "y");
    EXPECT_EQ(data[3].name, "z");

    EXPECT_EQ(data[1].values, expected_ch1);
    EXPECT_EQ(data[2].values, expected_ch2);
    EXPECT_EQ(data[3].values, expected_ch3);
}
