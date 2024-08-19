#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "config.h"
#include "lp5_controller.h"
#include "power_cal.h"

namespace PimSim
{
class DRAMStat
{
  public:
    // static DRAMStat &instance()
    // {
    //     static DRAMStat* instance_ = new DRAMStat();
    //     return *instance_;
    // }

    static DRAMStat* getInstance(void)
    {
        if (!instance_)
        {
            instance_ = new DRAMStat();
            return instance_;
        }
        else
        {
            return instance_;
        }
    }

    void clear(void)
    {
        data_to_read_ = 0;
        data_to_write_ = 0;
        std::vector<int>().swap(cmd_);
        Config* conf_ = Config::getInstance();
        num_total_banks_ = conf_->banks_ * conf_->ranks_ * conf_->channels_;
        power_cal_ = new PowerCal();
        cur_cycle_.resize(conf_->channels_, 0);
    }

    std::vector<uint64_t> cur_cycle_;
    uint64_t data_to_read_;
    uint64_t data_to_write_;
    std::vector<int> cmd_;
    std::vector<std::vector<uint64_t>> num_cmd_;

    uint64_t getCurCycle(Node* nd)
    {
        int ch = nd->findParent(Level::Channel)->id;
        return cur_cycle_[ch];
    }

    uint64_t getMaxCycle()
    {
        return *std::max_element(cur_cycle_.begin(), cur_cycle_.end());
    }

    float getBW(LP5Controller* dram_con)
    {
        float clock_timing = ((LPDDR5*)dram_con->dram_)->getClockTiming();  // 1.25ns (LPDDR5-6400)
        return (data_to_read_ + data_to_write_) / getMaxCycle() / clock_timing;
    }

    inline double getClockPeriod(LP5Controller* dram_con)
    {
        return ((LPDDR5*)dram_con->dram_)->getClockTiming() * pow(10, -9);
    }

    inline double getTotalTime(LP5Controller* dram_con)
    {
        double clk_period = getClockPeriod(dram_con);
        return getMaxCycle() * clk_period;
    }

    void printTotalTime(LP5Controller* dram_con)
    {
        std::cout << "------------------------------------" << std::endl;
        std::cout << "Exe Time (ns): " << std::setw(19) << getTotalTime(dram_con) * pow(10, 9)
                  << std::endl;
        std::cout << "Exe Time (cyc): " << std::setw(18) << getMaxCycle() << std::endl;
        std::cout << "------------------------------------" << std::endl;
    }

    double printPower(LP5Controller* dram_con, bool is_final)
    {
        double total_time = getTotalTime(dram_con);
        return power_cal_->getPower(total_time, num_cmd_, is_final);
    }

    uint32_t getPowerSamplingInterval(void)
    {
        return power_cal_->getPowerSamplingInterval();
    }

    bool isPowerSampling(void)
    {
        return power_cal_->isPowerSampling();
    }

    void setNumCmdSize(uint32_t num_cmd_types)
    {
        Config* conf_ = Config::getInstance();
        num_cmd_.resize(conf_->channels_, std::vector<uint64_t>(num_cmd_types, 0));
    }

    bool getBankActive(int ch, int bank)
    {
        return power_cal_->getBankActive(ch, bank);
    }

    void setBankActive(int ch, int bank, bool active)
    {
        power_cal_->setBankActive(ch, bank, active);
    }

    void cntBankActiveTime(int ch, int bank)
    {
        power_cal_->cntBankActiveTime(ch, bank);
    }

  private:
    static DRAMStat* instance_;
    PowerCal* power_cal_;

    DRAMStat(void) {}
    ~DRAMStat(void)
    {
        delete power_cal_;
    }

    int num_total_banks_;
};
}  // namespace PimSim
