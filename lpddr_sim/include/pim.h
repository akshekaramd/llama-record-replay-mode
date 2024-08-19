#pragma once

#include <iostream>
#include <string>
#include <unordered_map>

#include "burst_t.h"
#include "node.h"

namespace PimSim
{
enum class PIMRegType : int
{
    VRF,  // same as GRF in HBM2-PIM
    IRF,  // same as CRF in HBM2-PIM
    SRF,  // same as SRF in HBM2-PIM
    MAX
};

enum class PIMOP : int
{
    NOP,
    MAC_INT32_INT8INT8,
    MAC_FP32_FP16FP16,
    MAC_FP32_BF16BF16,
    MAC_FP32_FP32FP32,
    MOV,
    JUMP,  // For backward protocol
    MAX
};

enum class PIMOperand : int
{
    VRF,
    SRF,
    BANK,
    MAX
};

extern std::string g_pim_reg_type_name[int(PIMRegType::MAX)];
extern std::string g_pim_op_name[int(PIMOP::MAX)];
extern std::string g_pim_operand_name[int(PIMOperand::MAX)];
extern bool is_pim_operand_idxable[int(PIMOperand::MAX)];

// aa stands for aam flag
struct inst_seed
{
    PIMOP op;
    PIMOperand dst;
    PIMOperand src0;
    PIMOperand src1;
    PIMOperand src2;
    unsigned int dst_aa;
    unsigned int dst_idx;
    unsigned int src0_aa;
    unsigned int src0_idx;
    unsigned int reset_en;
};

union inst_t
{
    inst_t()
    {
        value = 0;
    }
#pragma pack(push, 1)
    struct
    {
        unsigned int resv : 4;  // reserved bit for future use
        unsigned int reset_en : 1;
        unsigned int src0_idx : 10;
        unsigned int src0_aa : 1;
        unsigned int dst_idx : 5;
        unsigned int dst_aa : 1;
        unsigned int src1 : 2;
        unsigned int src0 : 2;
        unsigned int dst : 2;
        unsigned int op : 4;
    } format;
    struct
    {
        unsigned int resv : 6;   // reserved bit for future use
        unsigned int imm1 : 6;   // negative jump offset
        unsigned int imm0 : 16;  // loop counter
        unsigned int op : 4;
    } format_imm;

    uint32_t value;

    inst_t(inst_seed sd)
    {
        set(sd);
    }

    void set(inst_seed sd)
    {
        format.op = int(sd.op);
        format.dst = int(sd.dst);
        format.src0 = int(sd.src0);
        format.src1 = int(sd.src1);
        format.dst_aa = sd.dst_aa;
        format.src0_aa = sd.src0_aa;
        format.dst_idx = sd.dst_idx;
        format.src0_idx = sd.src0_idx;
        format.reset_en = sd.reset_en;
    }
    void set_imm(inst_seed sd)
    {
        format_imm.op = int(sd.op);
        format_imm.imm0 = int(sd.src0_idx);  // loop counter
        format_imm.imm1 = int(sd.dst_idx);   // jump offset
    }
    std::string to_str()
    {
        std::string op = g_pim_op_name[int(format.op)];
        std::string dst = g_pim_operand_name[int(format.dst)] +
                          ((!format.dst_aa && is_pim_operand_idxable[int(format.dst)])
                               ? ("[" + std::to_string(format.dst_idx) + "]")
                               : "");
        std::string src0 = g_pim_operand_name[int(format.src0)] +
                           ((!format.src0_aa && is_pim_operand_idxable[int(format.src0)])
                                ? ("[" + std::to_string(format.src0_idx) + "]")
                                : "");
        std::string src1 = g_pim_operand_name[int(format.src1)] + "";
        return op + "," + dst + "," + src0 + "," + src1;
    }
#pragma pack(pop)
};

class PIM
{
  private:
    static PIM* instance_;
    PIM(void)
    {
        Config* conf = Config::getInstance();
        num_srfs_ = conf->num_srfs_;
        actual_col_bits_ = log2(conf->columns_) - log2(conf->BL_);
        for (int ch = 0; ch < conf->channels_; ch++)
        {
            pim_stat_.push_back({
                {"BANK_READ", 0},
                {"BANK_WRITE", 0},
                {"VRF_READ", 0},
                {"VRF_WRITE", 0},
                {"SRF_READ", 0},
                {"SRF_WRITE", 0},
                {"MAC_INT32_INT8INT8", 0},
                {"MAC_FP32_FP16FP16", 0},
                {"MAC_FP32_BF16BF16", 0},
                {"MAC_FP32_FP32FP32", 0},
                {"MOV", 0},
            });
        }
    }
    PIMPrecision pim_precision_;
    int getRegIdx(int aam, int idx, int ro, int co);
    inline int getRegOffset(int aam, int idx, int co);
    void updateStat(inst_t, int);
    int num_srfs_;
    int num_elts_in_burst_;
    int actual_col_bits_;

  public:
    static PIM* getInstance(void)
    {
        if (!instance_)
        {
            instance_ = new PIM();
            return instance_;
        }
        else
        {
            return instance_;
        }
    }

    void init(Node* node)
    {
        node->pim_pc = 0;
        node->last_jump_idx = -1;
        node->num_jumps_tobe_taken = -1;
        node->last_repeat_idx = -1;
        node->num_repeat_tobe_done = -1;
    }

    void setNumEltsBurst(int num_elts_in_burst)
    {
        num_elts_in_burst_ = num_elts_in_burst;
    }

    std::vector<std::unordered_map<std::string, uint64_t>> pim_stat_;

    void doPIM(Node* node, int cmd_idx, int ba, int ro, int co, burst_t* data);
    void doPIMBackward(Node* node, int ba, int ro, int co, burst_t* data);
};
}  // namespace PimSim
