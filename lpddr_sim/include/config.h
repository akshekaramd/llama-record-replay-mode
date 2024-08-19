#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "INIReader.h"

namespace PimSim
{
enum class PIMPrecision : int
{
    INT8,
    INT16,
    INT24,
    INT32,
    FP16,
    BF16,
    FP32,
};

enum class MACPrecision : int
{
    INT32_INT8INT8,
    FP32_FP16FP16,
    FP32_BF16BF16,
    FP32_FP32FP32
};

class Config
{
  private:
    static Config* instance_;
    Config(void) {}

  public:
    enum class Protocol : int
    {
        NEW,
        BACKWARD,
        MAX
    };

    static Config* getInstance(void);
    void set(std::string ini_file_name);
    void check();

    void setPIMAddr(int pim_addr_start, int num_cmd, int num_regtype, int num_reg);
    void inline setBitPos(std::vector<uint32_t>* bp, uint64_t* width, size_t num_count);

    INIReader* reader_;
    // address mapping numbers
    int shift_bits_;
    int ch_pos_, ra_pos_, bg_pos_, ba_pos_, ro_pos_, co_pos_;
    uint64_t ch_mask_, ra_mask_, bg_mask_, ba_mask_, ro_mask_, co_mask_;
    std::unordered_map<std::string, std::vector<uint64_t>> bit_pos_;
    std::vector<uint32_t> bp_ch_, bp_ra_, bp_bg_, bp_ba_, bp_ro_, bp_co_;

    Protocol pim_protocol_;
    std::string pim_protocol_str_;
    int pim_mb_ra_;
    int pim_sb_ra_;
    int pim_reg_ra_;

    int ranks_;
    int bankgroups_;
    int banks_per_group_;

    int pims_per_group_;
    int banks_per_pim_;
    std::string pim_structure_;
    unsigned int pim_pipe_tail_;

    int rows_;
    int columns_;
    int device_width_;
    int BL_;
    int channel_size_;
    int channels_;
    int bus_width_;
    std::string address_mapping_;

    int banks_;
    int devices_per_rank_;
    int request_size_bytes_;

    int reg_type_pos_;
    int reg_type_mask_;
    int reg_idx_pos_;
    int reg_idx_mask_;
    int cmd_idx_pos_;
    int cmd_idx_mask_;
    int enable_all_rank_pos_;

    int WCK_to_CK_ratio_;
    double CK_freq_;
    int RL_;
    int WL_;
    int fence_cycle_;

    int pim_ck_freq_;
    int input_to_acc_ratio_;

    int num_srfs_;
    int num_vrfs_;
    std::string mac_precision_str_;
    enum MACPrecision mac_precision_;
    enum PIMPrecision prec_mac_in_;
    enum PIMPrecision prec_mac_acc_;

    std::string power_model_;
    std::string power_log_file_;
    int power_sampling_period_;

  private:
    void generateAddressMapping(void);
    void calculateDRAMSize(void);
};
}  // namespace PimSim
