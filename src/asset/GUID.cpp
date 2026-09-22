#include "asset/GUID.hpp"

#include <random>

namespace Droplet
{
    bool GuidUtils::IsValid(GUID p_guid)
    {
        return p_guid != C_INVALID_GUID;
    }

    GUID GuidUtils::Generate()
    {
        thread_local std::random_device rd;
        thread_local std::mt19937_64 gen(rd());
        thread_local std::uniform_int_distribution<uint64_t> dis;
        
        GUID guid = dis(gen);
        while (guid == C_INVALID_GUID) // 0 invalid guid
        {
            guid = dis(gen);
        }
        
        return guid;
    }
}
