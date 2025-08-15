#include <chrono>
#include <cstddef>
#include <cstring>
#include <deque>
#include <gtest/gtest.h>
#include <BSP/telemetry.h>
#include <sstream>
#include <stdexcept>
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

TEST(BSP_Test, SIGSEV) {
    std::string str = "NumberOne:0,NumberTwo:-3";

    try {
        double x = std::stod(str);

        EXPECT_EQ(x, 0);
    } catch (const std::invalid_argument& e) {
        
    }
    
    EXPECT_EQ(1, 0);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
