#include <redis++.h>
#include <iostream>
#include <cstdio>
#include <unordered_set>
#include <Windows.h>

using namespace sw::redis;

int main(int argc, char** argv)
{
	try {
		Redis redis = Redis("tcp://127.0.0.1:6379");
		StringView key = "key";
		StringView value = "test rcc";
		redis.set(key, value);
		OptionalString val = redis.get("key");
		if (val) {
			std::cout << *val << std::endl;
		}
	}
	catch (const Error& e) {

	}
	return 0;
}
