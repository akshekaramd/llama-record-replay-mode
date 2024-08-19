#pragma once

#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "INIReader.h"
#include "config.h"
#include "lpddr5.h"
#include "pim.h"

namespace PimSim
{
class PowerCal
{
  public:
    PowerCal(void);
    ~PowerCal(void);

    double getPower(double total_time, std::vector<std::vector<uint64_t>> &num_cmd, bool is_final);
    bool getBankActive(int ch, int bank);
    void setBankActive(int ch, int bank, bool active);
    void cntBankActiveTime(int ch, int bank);
    uint32_t getPowerSamplingInterval(void);
    bool isPowerSampling(void);

  private:
    void initIni(void);
    void calPower(double total_time, std::vector<std::vector<uint64_t>> &num_cmd, bool is_final);

    double BL_ns_;
    double tRAS_ns_;
    double tRP_ns_;
    double tRC_ns_;
    double tRFC_ns_;
#ifndef POWER_RELEASE
    INIReader *reader_;
    double prev_energy_;
#endif
    Config *conf_;
    std::ofstream ofs_;
    double avg_power_;
    uint32_t power_sampling_period_;
    double power_sampling_period_ns_;
    std::vector<std::vector<bool>> is_bank_active_;
    std::vector<std::vector<uint64_t>> bank_active_time_;
};  // class PowerCal
}  // namespace PimSim
