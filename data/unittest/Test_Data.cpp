#include "../Data.hpp"
#include "gtest/gtest.h"

namespace Data {

TEST(DataTest, defaultConstructor)
{
    Data<std::string> data;

    EXPECT_EQ(std::string(""), data.get());
}

TEST(DataTest, constructor)
{
    const std::string str = "Test string with some data to make it not so short.";
    Data<std::string> data(str);

    EXPECT_EQ(str, data.get());
}

TEST(DataTest, moveConstructor)
{
    std::string str = "Test string with some data to make it not so short.";
    Data<std::string> data(std::move(str));

    EXPECT_EQ(std::string("Test string with some data to make it not so short."), data.get());
}

TEST(DataTest, setAndGet)
{
    Data<int> data;
    data.set(2);

    EXPECT_EQ(2, data.get());
}

} // namespace Data
