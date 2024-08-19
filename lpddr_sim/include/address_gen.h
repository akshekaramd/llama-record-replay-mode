#pragma once

#include <vector>

#include "address.h"
#include "config.h"

namespace PimSim
{
class AddressGen
{
    Config *conf_;

  public:
    AddressGen(Config *conf) : conf_(conf) {}
    Address translate(uint64_t hex_addr);

    struct AddrSeed
    {
        int ch;
        int ra;
        int bg;
        int ba;
        int ro;
        int co;
        int reg_type;
        int reg_idx;
        int cmd_idx;
        int enable_all_ranks;
    };

    uint64_t memAddr(AddrSeed sd);
    uint64_t pimAddr(AddrSeed sd);
    uint64_t pimxAddr(AddrSeed sd);
    uint64_t inline localAddr2Addr(uint64_t hex_addr, std::vector<uint32_t> bp, int index);
};
}  // namespace PimSim
