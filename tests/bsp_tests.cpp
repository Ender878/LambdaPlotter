#include <gtest/gtest.h>
#include <termios.h>

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

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
