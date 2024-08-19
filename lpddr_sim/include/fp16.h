#pragma once

#include <cstdint>
#include <cstring>

#include "half.hpp"

namespace PimSim
{
typedef half_float::half float16;
typedef uint16_t bf16;
float convertH2F(float16 val);
float16 convertF2H(float val);
float convertBF2F(bf16 val);
bf16 convertF2BF(float val);

union fp16i
{
    float16 fval;
    uint16_t ival;

    fp16i(void)
    {
        ival = 0;
    }
    fp16i(float16 x)
    {
        fval = x;
    }
    fp16i(uint16_t x)
    {
        ival = x;
    }
};

union fp32bf16
{
    float fp;
    bf16 bf[2];
};
}  // namespace PimSim
