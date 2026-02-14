#include <gtest/gtest.h>
#include <iostream>
using namespace std;

int add(int a, int b) {
    return a + b;
}

// pulled function logic from https://www.geeksforgeeks.org/dsa/check-for-prime-number/
bool IsPrime(int num){
    if (num <= 1)
        return false;

    // Check divisibility from 2 to n-1
    for (int i = 2; i < num; i++)
    {
        if (num % i == 0) return false;
    }

    return true;
}

TEST(FunctionTest, CheckAddition) {
    EXPECT_EQ(add(3, 5), 8);
    EXPECT_EQ(add(-1, 1), 0);
}

TEST(FunctionTest, CheckPrime){
    EXPECT_FALSE(IsPrime(6));
    EXPECT_TRUE(IsPrime(23));
}