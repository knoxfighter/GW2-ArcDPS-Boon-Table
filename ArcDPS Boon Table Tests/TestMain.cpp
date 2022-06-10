#include <gtest/gtest.h>

int main(int pArgumentCount, char** pArgumentVector)
{
	::testing::InitGoogleTest(&pArgumentCount, pArgumentVector); 

	int result = RUN_ALL_TESTS();

	return result;
}
