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

#include <ctlhashutils.h>
#include <boost/hash2/sha2.hpp>

namespace dynarithmic
{
    template <typename hashType>
    static std::vector<unsigned char> GenericHashSHA2(std::string_view message)
    {
        std::vector<unsigned char> data(message.begin(), message.end());
        std::vector<unsigned char> retVal;
        // Create a hash object
        hashType hasher;

        // Update the hash object with the input data
        hasher.update(data.data(), data.size());

        // Obtain the hash result
        typename hashType::result_type digest = hasher.result();

        for (unsigned char byte : digest)
            retVal.push_back(byte);
        return retVal;
    }

    std::vector<unsigned char> SHA2Hash(std::string_view message, SHA2HashType hashType)
    {
        std::vector<unsigned char> data(message.begin(), message.end());
        std::vector<unsigned char> retVal;
        switch (hashType)
        {
            case SHA2HashType::SHA256:
                return GenericHashSHA2<boost::hash2::sha2_256>(message);
            case SHA2HashType::SHA384:
                return GenericHashSHA2<boost::hash2::sha2_384>(message);
            case SHA2HashType::SHA512:
                return GenericHashSHA2<boost::hash2::sha2_512>(message);
        }
        return {};
    }
}