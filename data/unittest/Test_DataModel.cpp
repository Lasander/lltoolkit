#include "../DataModel.h"
#include "gtest/gtest.h"

TEST(Test_DataModel, setter) 
{
    Data::DataModel<int> data;
    data.set(2);
    
    EXPECT_EQ(data.get(), 2);
}