/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2025 Dynarithmic Software.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

    FOR ANY PART OF THE COVERED WORK IN WHICH THE COPYRIGHT IS OWNED BY
    DYNARITHMIC SOFTWARE. DYNARITHMIC SOFTWARE DISCLAIMS THE WARRANTY OF NON INFRINGEMENT
    OF THIRD PARTY RIGHTS.
 */
#ifndef CTLRANDOMNUMUTILS_H
#define CTLRANDOMNUMUTILS_H

#include <vector>
#include <random>
#include <algorithm>
#include <iterator>

namespace dynarithmic
{
    template <typename T>
	T GenerateRandomString(size_t length, size_t lowchar = 0, size_t highchar = 255)
	{
		static std::random_device rd;
		static std::mt19937 generator(rd());
		static std::uniform_int_distribution<int> distrib(static_cast<int>(lowchar), static_cast<int>(highchar));

		T randomString;
		randomString.reserve(length); 
		std::generate_n(std::back_inserter(randomString), length, [&]() {
			return static_cast<unsigned char>(distrib(generator));
			});
		return randomString;
	}

    inline std::vector<unsigned char> CreateRandomDigits(size_t numDigits)
    {
        return GenerateRandomString<std::vector<unsigned char>>(numDigits);
    }

	inline std::string CreateRandomLatin1String(size_t length)
	{
		return GenerateRandomString<std::string>(length, 32, 255);
	}
}
#endif


