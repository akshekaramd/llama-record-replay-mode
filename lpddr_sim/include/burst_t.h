#pragma once

#include <bitset>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

#include "fp16.h"

namespace PimSim
{
union metadata_t
{
    metadata_t()
    {
        u32_data_ = 0;
    }
    uint32_t u32_data_;
    uint16_t u16_data_[2];
    uint8_t u8_data_[4];
};

union burst_t
{
    burst_t(void)
    {
        clear();
    }
    burst_t(float *x)
    {
        memcpy(u8_data_, x, 32);
    }

    burst_t(float x0, float x1, float x2, float x3, float x4, float x5, float x6, float x7)
    {
        set(x0, x1, x2, x3, x4, x5, x6, x7);
    }

    void bcast(float x)
    {
        set(x, x, x, x, x, x, x, x);
    }
    void bcast(uint32_t x)
    {
        set(x, x, x, x, x, x, x, x);
    }
    void bcast(int8_t x)
    {
        set(x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x,
            x, x, x);
    }
    void bcast(int16_t x)
    {
        set(x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x);
    }
    void bcast(bf16 x)
    {
        set(x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x);
    }
    void bcast(float16 x)
    {
        set(x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x);
    }
    void set(float x0, float x1, float x2, float x3, float x4, float x5, float x6, float x7)
    {
        fp32_data_[0] = x0;
        fp32_data_[1] = x1;
        fp32_data_[2] = x2;
        fp32_data_[3] = x3;
        fp32_data_[4] = x4;
        fp32_data_[5] = x5;
        fp32_data_[6] = x6;
        fp32_data_[7] = x7;
    }

    void set(uint32_t x0, uint32_t x1, uint32_t x2, uint32_t x3, uint32_t x4, uint32_t x5,
             uint32_t x6, uint32_t x7)
    {
        u32_data_[0] = x0;
        u32_data_[1] = x1;
        u32_data_[2] = x2;
        u32_data_[3] = x3;
        u32_data_[4] = x4;
        u32_data_[5] = x5;
        u32_data_[6] = x6;
        u32_data_[7] = x7;
    }

    void set(int8_t x0, int8_t x1, int8_t x2, int8_t x3, int8_t x4, int8_t x5, int8_t x6, int8_t x7,
             int8_t x8, int8_t x9, int8_t x10, int8_t x11, int8_t x12, int8_t x13, int8_t x14,
             int8_t x15, int8_t x16, int8_t x17, int8_t x18, int8_t x19, int8_t x20, int8_t x21,
             int8_t x22, int8_t x23, int8_t x24, int8_t x25, int8_t x26, int8_t x27, int8_t x28,
             int8_t x29, int8_t x30, int8_t x31)
    {
        i8_data_[0] = x0;
        i8_data_[1] = x1;
        i8_data_[2] = x2;
        i8_data_[3] = x3;
        i8_data_[4] = x4;
        i8_data_[5] = x5;
        i8_data_[6] = x6;
        i8_data_[7] = x7;
        i8_data_[8] = x8;
        i8_data_[9] = x9;
        i8_data_[10] = x10;
        i8_data_[11] = x11;
        i8_data_[12] = x12;
        i8_data_[13] = x13;
        i8_data_[14] = x14;
        i8_data_[15] = x15;
        i8_data_[16] = x16;
        i8_data_[17] = x17;
        i8_data_[18] = x18;
        i8_data_[19] = x19;
        i8_data_[20] = x20;
        i8_data_[21] = x21;
        i8_data_[22] = x22;
        i8_data_[23] = x23;
        i8_data_[24] = x24;
        i8_data_[25] = x25;
        i8_data_[26] = x26;
        i8_data_[27] = x27;
        i8_data_[28] = x28;
        i8_data_[29] = x29;
        i8_data_[30] = x30;
        i8_data_[31] = x31;
    }

    void set(int16_t x0, int16_t x1, int16_t x2, int16_t x3, int16_t x4, int16_t x5, int16_t x6,
             int16_t x7, int16_t x8, int16_t x9, int16_t x10, int16_t x11, int16_t x12, int16_t x13,
             int16_t x14, int16_t x15)
    {
        i16_data_[0] = x0;
        i16_data_[1] = x1;
        i16_data_[2] = x2;
        i16_data_[3] = x3;
        i16_data_[4] = x4;
        i16_data_[5] = x5;
        i16_data_[6] = x6;
        i16_data_[7] = x7;
        i16_data_[8] = x8;
        i16_data_[9] = x9;
        i16_data_[10] = x10;
        i16_data_[11] = x11;
        i16_data_[12] = x12;
        i16_data_[13] = x13;
        i16_data_[14] = x14;
        i16_data_[15] = x15;
    }

    void set(bf16 x0, bf16 x1, bf16 x2, bf16 x3, bf16 x4, bf16 x5, bf16 x6, bf16 x7, bf16 x8,
             bf16 x9, bf16 x10, bf16 x11, bf16 x12, bf16 x13, bf16 x14, bf16 x15)
    {
        bf16_data_[0] = x0;
        bf16_data_[1] = x1;
        bf16_data_[2] = x2;
        bf16_data_[3] = x3;
        bf16_data_[4] = x4;
        bf16_data_[5] = x5;
        bf16_data_[6] = x6;
        bf16_data_[7] = x7;
        bf16_data_[8] = x8;
        bf16_data_[9] = x9;
        bf16_data_[10] = x10;
        bf16_data_[11] = x11;
        bf16_data_[12] = x12;
        bf16_data_[13] = x13;
        bf16_data_[14] = x14;
        bf16_data_[15] = x15;
    }

    void set(float16 x0, float16 x1, float16 x2, float16 x3, float16 x4, float16 x5, float16 x6,
             float16 x7, float16 x8, float16 x9, float16 x10, float16 x11, float16 x12, float16 x13,
             float16 x14, float16 x15)
    {
        fp16_data_[0] = x0;
        fp16_data_[1] = x1;
        fp16_data_[2] = x2;
        fp16_data_[3] = x3;
        fp16_data_[4] = x4;
        fp16_data_[5] = x5;
        fp16_data_[6] = x6;
        fp16_data_[7] = x7;
        fp16_data_[8] = x8;
        fp16_data_[9] = x9;
        fp16_data_[10] = x10;
        fp16_data_[11] = x11;
        fp16_data_[12] = x12;
        fp16_data_[13] = x13;
        fp16_data_[14] = x14;
        fp16_data_[15] = x15;
    }

    void set(int8_t *x)
    {
        memcpy(i8_data_, x, 32);
    }

    void set(burst_t &b)
    {
        memcpy(u8_data_, b.u8_data_, 32);
    }

    void initializeRandom(std::mt19937 &gen)
    {
        std::uniform_real_distribution<float> dis(-16, 16);
        for (size_t i = 0; i < 8; i++)
        {
            fp32_data_[i] = dis(gen);
        }
    }

    void initializeRandomI8(std::mt19937 &gen)
    {
        std::uniform_int_distribution<int8_t> dis(-16, 16);
        for (size_t i = 0; i < 32; i++)
        {
            i8_data_[i] = dis(gen);
        }
    }

    void initializeRandomI16(std::mt19937 &gen)
    {
        std::uniform_int_distribution<int16_t> dis(-16, 16);
        for (size_t i = 0; i < 16; i++)
        {
            i16_data_[i] = dis(gen);
        }
    }

    void initializeRandomF16(std::mt19937 &gen)
    {
        std::uniform_real_distribution<float> dis(-16, 16);
        for (size_t i = 0; i < 16; i++)
        {
            fp16_data_[i] = convertF2H(dis(gen));
        }
    }

    void clear()
    {
        for (size_t i = 0; i < 8; i++)
        {
            u32_data_[i] = 0;
        }
    }

    std::string binToStr() const
    {
        std::stringstream ss;
        ss << "[";
        for (size_t i = 0; i < 8; i++)
        {
            ss << std::bitset<32>(u32_data_[i]);
        }
        ss << "]";
        return ss.str();
    }

    std::string hexToStr() const
    {
        std::stringstream ss;
        ss << "[";
        for (size_t i = 0; i < 8; i++)
        {
            ss << std::setfill('0') << std::setw(8) << std::hex << u32_data_[i];
        }
        ss << "]" << std::dec;
        return ss.str();
    }

    std::string hexToStrRev() const
    {
        std::stringstream ss;
        for (int i = 7; i >= 0; i--)
        {
            ss << std::setfill('0') << std::setw(8) << std::hex << u32_data_[i];
        }
        return ss.str();
    }

    void hexFromStrRev(std::string str)
    {
        int idx = 8;
        for (unsigned i = 0; i < str.length(); i += 8)
        {
            u32_data_[idx--] = std::stoi(str.substr(i, 8), nullptr, 16);
        }
    }

    std::string fp32ToStr() const
    {
        std::stringstream ss;
        ss << "[ ";
        for (size_t i = 0; i < 8; i++)
        {
            ss << fp32_data_[i] << " ";
        }
        ss << "]";
        return ss.str();
    }

    std::string int8ToStr() const
    {
        std::stringstream ss;
        ss << "[ ";
        for (size_t i = 0; i < 32; i++)
        {
            ss << static_cast<int16_t>(i8_data_[i]) << " ";
        }
        ss << "]";
        return ss.str();
    }

    std::string int16ToStr() const
    {
        std::stringstream ss;
        ss << "[ ";
        for (size_t i = 0; i < 16; i++)
        {
            ss << i16_data_[i] << " ";
        }
        ss << "]";
        return ss.str();
    }

    std::string int32ToStr() const
    {
        std::stringstream ss;
        ss << "[ ";
        for (size_t i = 0; i < 8; i++)
        {
            ss << u32_data_[i] << " ";
        }
        ss << "]";
        return ss.str();
    }

    std::string fp16ToStr() const
    {
        std::stringstream ss;
        ss << "[ ";
        for (size_t i = 0; i < 16; i++)
        {
            ss << fp16_data_[i] << " ";
        }
        ss << "]";
        return ss.str();
    }

    std::string bf16ToStr() const
    {
        std::stringstream ss;
        ss << "[ ";
        for (size_t i = 0; i < 16; i++)
        {
            ss << bf16_data_[i] << " ";
        }
        ss << "]";
        return ss.str();
    }

    std::string int32accToStr(burst_t acc0, burst_t acc1, burst_t acc2) const
    {
        std::stringstream ss;
        ss << "[ ";
        for (size_t i = 0; i < 8; i++)
        {
            ss << acc0.u32_data_[i] << " ";
        }
        for (size_t i = 0; i < 8; i++)
        {
            ss << acc1.u32_data_[i] << " ";
        }
        for (size_t i = 0; i < 8; i++)
        {
            ss << acc2.u32_data_[i] << " ";
        }
        for (size_t i = 0; i < 8; i++)
        {
            ss << u32_data_[i] << " ";
        }

        ss << "]";
        return ss.str();
    }

    std::string int32accToStr(burst_t acc_pre) const
    {
        std::stringstream ss;
        ss << "[ ";
        for (size_t i = 0; i < 8; i++)
        {
            ss << acc_pre.u32_data_[i] << " ";
        }
        for (size_t i = 0; i < 8; i++)
        {
            ss << u32_data_[i] << " ";
        }

        ss << "]";
        return ss.str();
    }

    std::string fp32accToStr(burst_t acc_pre) const
    {
        std::stringstream ss;
        ss << "[ ";
        for (size_t i = 0; i < 8; i++)
        {
            ss << acc_pre.fp32_data_[i] << " ";
        }
        for (size_t i = 0; i < 8; i++)
        {
            ss << fp32_data_[i] << " ";
        }

        ss << "]";
        return ss.str();
    }

    bool fp32Similar(const burst_t &rhs, float epsilon)
    {
        for (size_t i = 0; i < 8; i++)
        {
            if ((fp32_data_[i] - rhs.fp32_data_[i]) / fp32_data_[i] > epsilon)
            {
                return false;
            }
        }
        return true;
    }

    burst_t addInt8(burst_t &arg1, burst_t &arg2)
    {
        burst_t ret;
        for (size_t i = 0; i < 32; i++)
        {
            ret.i8_data_[i] = arg1.i8_data_[i] + arg2.i8_data_[i];
        }
        return ret;
    }

    burst_t mulInt8(burst_t &arg1, burst_t &arg2)
    {
        burst_t ret;
        for (size_t i = 0; i < 32; i++)
        {
            ret.i8_data_[i] = arg1.i8_data_[i] * arg2.i8_data_[i];
        }
        return ret;
    }

    burst_t macInt8(burst_t &arg1, burst_t &arg2, burst_t &arg3)
    {
        burst_t ret;
        for (size_t i = 0; i < 32; i++)
        {
            ret.i8_data_[i] = arg1.i8_data_[i] + (arg2.i8_data_[i] * arg3.i8_data_[i]);
        }
        return ret;
    }

    burst_t addFp32(burst_t &arg1, burst_t &arg2)
    {
        burst_t ret;
        for (size_t i = 0; i < 8; i++)
        {
            ret.fp32_data_[i] = arg1.fp32_data_[i] + arg2.fp32_data_[i];
        }
        return ret;
    }

    burst_t mulFp32(burst_t &arg1, burst_t &arg2)
    {
        burst_t ret;
        for (size_t i = 0; i < 8; i++)
        {
            ret.fp32_data_[i] = arg1.fp32_data_[i] * arg2.fp32_data_[i];
        }
        return ret;
    }

    burst_t macFp32(burst_t &arg1, burst_t &arg2, burst_t &arg3)
    {
        burst_t ret;
        for (size_t i = 0; i < 8; i++)
        {
            ret.fp32_data_[i] = arg1.fp32_data_[i] + (arg2.fp32_data_[i] * arg3.fp32_data_[i]);
        }
        return ret;
    }

    burst_t addInt16(burst_t &arg1, burst_t &arg2)
    {
        burst_t ret;
        for (size_t i = 0; i < 16; i++)
        {
            ret.i16_data_[i] = arg1.i16_data_[i] + arg2.i16_data_[i];
        }
        return ret;
    }

    burst_t mulInt16(burst_t &arg1, burst_t &arg2)
    {
        burst_t ret;
        for (size_t i = 0; i < 16; i++)
        {
            ret.i16_data_[i] = arg1.i16_data_[i] * arg2.i16_data_[i];
        }
        return ret;
    }

    burst_t macInt16(burst_t &arg1, burst_t &arg2, burst_t &arg3)
    {
        burst_t ret;
        for (size_t i = 0; i < 16; i++)
        {
            ret.i16_data_[i] = arg1.i16_data_[i] + (arg2.i16_data_[i] * arg3.i16_data_[i]);
        }
        return ret;
    }
    burst_t addFp16(burst_t &arg1, burst_t &arg2)
    {
        burst_t ret;
        for (size_t i = 0; i < 16; i++)
        {
            ret.fp16_data_[i] = arg1.fp16_data_[i] + arg2.fp16_data_[i];
        }
        return ret;
    }

    burst_t mulFp16(burst_t &arg1, burst_t &arg2)
    {
        burst_t ret;
        for (size_t i = 0; i < 16; i++)
        {
            ret.fp16_data_[i] = arg1.fp16_data_[i] * arg2.fp16_data_[i];
        }
        return ret;
    }

    burst_t macFp16(burst_t &arg1, burst_t &arg2, burst_t &arg3)
    {
        burst_t ret;
        for (size_t i = 0; i < 16; i++)
        {
            ret.fp16_data_[i] = arg1.fp16_data_[i] + (arg2.fp16_data_[i] * arg3.fp16_data_[i]);
        }
        return ret;
    }

    int8_t sumInt8()
    {
        int8_t ret = 0;
        for (size_t i = 0; i < 32; i++)
        {
            ret += i8_data_[i];
        }
        return ret;
    }

    float16 sumFp16()
    {
        float16 ret = convertF2H(0.0f);
        for (size_t i = 0; i < 16; i++)
        {
            ret += fp16_data_[i];
        }
        return ret;
    }

    float sumFp32()
    {
        float ret = 0;
        for (size_t i = 0; i < 8; i++)
        {
            ret += fp32_data_[i];
        }
        return ret;
    }

    int16_t sumInt16()
    {
        int16_t ret = 0;
        for (size_t i = 0; i < 16; i++)
        {
            ret += i16_data_[i];
        }
        return ret;
    }

    int16_t sumInt32()
    {
        int16_t ret = 0;
        for (size_t i = 0; i < 8; i++)
        {
            ret += u32_data_[i];
        }
        return ret;
    }

    burst_t macInt32accInt8Int8(burst_t &arg1, burst_t &arg2, burst_t &arg3, unsigned s_idx)
    {
        burst_t ret;
        for (size_t i = s_idx; i < s_idx + 8; i++)
        {
            ret.u32_data_[i - s_idx] =
                arg1.u32_data_[i - s_idx] + (arg2.i8_data_[i] * arg3.i8_data_[i]);
        }
        return ret;
    }

    burst_t macInt16accInt32(burst_t &arg1, burst_t &arg2, burst_t &arg3, unsigned s_idx)
    {
        burst_t ret;
        for (size_t i = s_idx; i < s_idx + 8; i++)
        {
            ret.u32_data_[i - s_idx] =
                arg1.u32_data_[i - s_idx] + (arg2.i16_data_[i] * arg3.i16_data_[i]);
        }
        return ret;
    }

    burst_t macFp32accFp16Fp16(burst_t &arg1, burst_t &arg2, burst_t &arg3, unsigned s_idx)
    {
        burst_t ret;
        for (size_t i = s_idx; i < s_idx + 8; i++)
        {
            ret.fp32_data_[i - s_idx] =
                arg1.fp32_data_[i - s_idx] + convertH2F(arg2.fp16_data_[i] * arg3.fp16_data_[i]);
        }
        return ret;
    }

    burst_t macFp32accBf16Bf16(burst_t &arg1, burst_t &arg2, burst_t &arg3, unsigned s_idx)
    {
        burst_t ret;
        for (size_t i = s_idx; i < s_idx + 8; i++)
        {
            ret.fp32_data_[i - s_idx] =
                arg1.fp32_data_[i - s_idx] +
                convertBF2F(arg2.bf16_data_[i]) * convertBF2F(arg3.bf16_data_[i]);
        }
        return ret;
    }

    bool operator==(const burst_t &rhs) const
    {
        return !(memcmp(this, &rhs, 32));
    }
    bool operator!=(const burst_t &rhs) const
    {
        return (memcmp(this, &rhs, 32));
    }

    burst_t operator+(const burst_t &rhs) const
    {
        burst_t ret;
        for (size_t i = 0; i < 8; i++)
        {
            ret.fp32_data_[i] = fp32_data_[i] + rhs.fp32_data_[i];
        }
        return ret;
    }

    burst_t operator*(const burst_t &rhs) const
    {
        burst_t ret;
        for (size_t i = 0; i < 8; i++)
        {
            ret.fp32_data_[i] = fp32_data_[i] * rhs.fp32_data_[i];
        }
        return ret;
    }

    float fp32_data_[8];
    uint32_t u32_data_[8];
    uint16_t u16_data_[16];
    uint8_t u8_data_[32];
    int8_t i8_data_[32];
    int16_t i16_data_[16];
    float16 fp16_data_[16];
    bf16 bf16_data_[16];
};
}  // namespace PimSim
