#include <sstream>
#include <iostream>
#include <string>
#include <vector>

extern "C" {
#include "sha3.h"
}

std::string byteToHex(const uint8_t* input, size_t length) {
		std::string result;
		char buffer[4];

		for (auto i = 0; i < length; ++i) {
				sprintf(buffer, "%02x", input[i]);
				result.append(buffer);
		}

		return result;
}

int main() {
		static std::vector<std::string> staticTypes{ "uint256" };
		static std::vector<std::string> dynamicTypes{ "string" };

		std::string result;

		std::string input = "insertEntry(string,uint256)";
		uint8_t functionSelector[32] = { 0 };
		sha3_HashBuffer(256, SHA3_FLAGS_KECCAK, input.c_str(), input.length(), functionSelector, 32);

		result += byteToHex(functionSelector, 4);

		input.erase(0, input.find("(") + 1);
		input.erase(input.rfind(")"));
		
		std::istringstream stream(input);
		std::string temp;
		std::vector<std::string> params;
		while (getline(stream, temp, ',')) {
				params.emplace_back(temp);
		}

		std::cout << result;

		return 0;
}