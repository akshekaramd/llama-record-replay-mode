#pragma once

#define MAX(a, b) ((a) > (b)) ? (a) : (b);
#define MIN(a, b) ((a) < (b)) ? (a) : (b);

#include <cstdint>
#include <deque>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

#include "burst_t.h"

namespace PimSim
{
// this concept does not changes
enum class Level : int
{
    Channel,
    Rank,
    BankGroup,
    PIMBlock,
    Bank,
    MAX
};

enum class DRAMMode : int
{
    SB,
    AB,
    MAX
};

class Request
{
  public:
    uint64_t addr_;
    enum class Type : int
    {
        READ,
        WRITE,
        MWR,  // masked write
        PIM_READ,
        PIM_WRITE,
        PIM_X,
        PIM_BCAST,
        READ32,
        WRITE32,
        // REF,
        // RFM, // refresh management
        // PDE, // power down entry
        // PDX, // power down exit
        MAX
    } type_;
    burst_t* data_;
    metadata_t* mdata_;
    bool bar_;
    uint64_t return_cycle_;

    Request(void) : bar_(false), return_cycle_(0) {}

    Request(uint64_t addr, Type type, burst_t* data, metadata_t* mdata)
        : addr_(addr), type_(type), data_(data), mdata_(mdata), bar_(false), return_cycle_(0)
    {
    }

    Request(uint64_t addr, Type type, burst_t* data, metadata_t* mdata, bool bar)
        : addr_(addr), type_(type), data_(data), mdata_(mdata), bar_(bar), return_cycle_(0)
    {
    }

    Request(uint64_t addr, Type type, burst_t* data, metadata_t* mdata, bool bar, uint64_t rcycle)
        : addr_(addr), type_(type), data_(data), mdata_(mdata), bar_(bar), return_cycle_(rcycle)
    {
    }

    int print_request();
};
}  // namespace PimSim
