#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "address.h"
#include "address_gen.h"
#include "burst_t.h"
#include "common.h"
#include "config.h"
#include "node.h"
#include "storage_t.h"

namespace PimSim
{
class DRAM
{
  public:
    std::vector<std::function<void(Node*, Address, burst_t*, metadata_t*)>> lambda_;
    std::vector<std::function<void(Node*, Address, burst_t*, metadata_t*)>> logging_;

    std::vector<std::string> cmd_str_;
    std::vector<std::string> state_str_;

    std::vector<Level> scope_range_;
    std::vector<Level> scope_apply_;

    std::vector<bool> cmd_par_;
    std::vector<bool> cmd_wr_;
    std::vector<bool> cmd_rd_;
    std::vector<int> cmd_equivalent_;

    std::unordered_map<std::string, double> energy_power_;
    std::vector<int> ACtiming_;
    std::vector<int> timeouts_;

    DRAM(Config*, int);
    virtual ~DRAM(void);

    // initialization virtual function for all
    void initAll(void);

    virtual void setCommandInfo(void) = 0;
    virtual void setStateInfo(void) = 0;
    virtual void initTree(void) = 0;
    virtual void initTiming(void) = 0;
    virtual void initLambda(void) = 0;
    virtual void initLogging(void) = 0;
    virtual void initStateCheck(void) = 0;

    virtual int offset(int a, int b, int c) = 0;
    // to sync with another
    virtual float getClockTiming(void) = 0;

    Node* nodes_[(int)Level::MAX];
    int num_nodes_[(int)Level::MAX];
    Config* conf_;
    int cmd_max_;

  protected:
    float clock_timing_;
    int BL_;
    int RL_;
    int WL_;
};
}  // namespace PimSim
