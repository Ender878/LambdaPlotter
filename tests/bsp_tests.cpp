#include <deque>
#include <gtest/gtest.h>
#include <print>
#include <sstream>
#include <string>
#include <termios.h>
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

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
