#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstring>
#include <deque>
#include <gtest/gtest.h>
#include <BSP/telemetry.h>
#include <iterator>
#include <sstream>
#include <string>
#include <termios.h>
#include <thread>
#include <unordered_map>
#include <vector>

// Test for baud_rate_t array functionality
TEST(BSP_Test, baud_test) {
    typedef struct baud_rate_t {
        const char* str;
        const int   value;
    } baud_rate_t;

    baud_rate_t baud_rates[] = {
        {"9600", B9600},
        {"115200", B115200}
    };

    const size_t array_size = sizeof(baud_rates) / sizeof(*baud_rates);

    EXPECT_EQ(baud_rates[0].str, "9600");
    EXPECT_EQ(baud_rates[1].value, B115200);
    EXPECT_EQ(array_size, 2);
}

// Test for real time data storage on a std::deque
TEST(BSP_Test, plot_data_test) {
    std::deque<int> data;
    std::deque<int> expected_data = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

    for (int i = 0; i < 12; i++) {
        data.push_back(i);

        if (data.size() > 10) {
            data.pop_front();
        }
    }

    EXPECT_EQ(data, expected_data);
}

TEST(BSP_Test, data_processing) {
    std::vector<double> res;
    std::vector<double> expected_res = {1, 2, 3, 4};

    std::string str = "1\n2\n3\n4\n";
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, '\n')) {
        res.push_back(std::stod(token));
    }

    EXPECT_EQ(res, expected_res);
}

TEST(BSP_Test, time_test) {
    auto start_time = std::chrono::steady_clock::now();

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    auto now = std::chrono::steady_clock::now();
    auto duration = now - start_time;

    int elapsed = std::chrono::duration<double>(duration).count();

    EXPECT_EQ(1, elapsed);
}

TEST(BSP_Test, vec_tel_test) {
    std::vector<double> data;
    std::vector<double> expected_res = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19 };

    for (size_t i = 0; i < 20; i++) {
        data.push_back(i);

        if (data.size() > 10) {
            data.erase(data.begin());
        }
    }

    EXPECT_EQ(data, expected_res);
}

TEST(BSP_Test, parse_serial) {
    BSP::Telemetry tel;

    std::vector<char> buffer = { '6', ' ', '9' };

    std::string frame_stream = tel.parse_serial(buffer);

    // EXPECT_EQ(frame_stream, "6 9\n");
    EXPECT_EQ(tel.get_frame_fragments(), "6 9");
}

TEST(BSP_Test, parse_frame) {
    BSP::Telemetry tel;

    std::vector<double> expected_x = { 15, 14, 4 };
    std::vector<double> expected_y = { 12,  5, 3 };
    std::vector<double> expected_z = { 3,   2, 0 };

    std::string frame_stream = "15 12 3\n\r14 5 2\n\r4 3 0\n\r";

    tel.frame_format.named = false;

    tel.parse_frame(frame_stream);

    auto data = *tel.getData();

    // check names
    // EXPECT_EQ(data[1].name, "X");
    // EXPECT_EQ(data[2].name, "Y");
    // EXPECT_EQ(data[3].name, "Z");

    // check values
    EXPECT_EQ(data[1].values, expected_x);
    EXPECT_EQ(data[2].values, expected_y);
    EXPECT_EQ(data[3].values, expected_z);
}

TEST(BSP_Test, special_chars_formatting) {
    const char* str = "Hello\\n";

    std::string res = BSP::Telemetry::format_special_chars(str);

    EXPECT_EQ(res, "Hello\n");
}

TEST(BSP_Test, dump_test) {
    std::vector<double> vec      = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12 };
    std::vector<double> expected = { 9, 10 };
    std::vector<double> res;

    double min_x = 9;
    double max_x = 11;

    auto min_it = std::lower_bound(vec.begin(), vec.end(), min_x);
    auto max_it = std::upper_bound(vec.begin(), vec.end(), max_x);

    // check if found min is within the range
    if (min_it != vec.end() && *min_it < min_x || *min_it > max_x) {
        min_it = vec.end();
    }

    max_it = std::prev(max_it);

    if (min_it != vec.end() && max_it != vec.end()) {
        for (auto i = min_it; i <= max_it; i++) {
            res.push_back(*i);
        }
    }

    EXPECT_EQ(res, expected);
    EXPECT_EQ(*min_it, 9);
    EXPECT_EQ(*max_it, 10);
}

TEST(BSP_Test, regex_test) {
    BSP::Telemetry tel;

    std::string frame1 = "10 4033 1200\n";
    std::string frame2 = "x:-10 y:20 z:-10.233\n";

    ASSERT_TRUE(tel.validate_frame(frame1));

    tel.frame_format.named = true;

    ASSERT_TRUE(tel.validate_frame(frame2));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
