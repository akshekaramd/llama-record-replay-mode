#pragma once

#include <list>
#include <vector>

#include "dram_controller.h"
#include "lpddr5.h"
#include "pim.h"

namespace PimSim
{
class LP5Controller : public DRAMController
{
  public:
    LPDDR5::Command translate_[int(Request::Type::MAX)] = {
        LPDDR5::Command::RD,   LPDDR5::Command::WR,  LPDDR5::Command::MWR,
        LPDDR5::Command::RDP,  LPDDR5::Command::WRP, LPDDR5::Command::PIMX,
        LPDDR5::Command::WRPB, LPDDR5::Command::RD,  LPDDR5::Command::WR,
    };

    LP5Controller(Config *conf_, std::function<void(uint64_t)> read_callback,
                  std::function<void(uint64_t)> write_callback);
    ~LP5Controller(void);

    using DRAMController::tick;
    void tick(int ch) override;

  private:
    bool checkCmd(int, Address &, int);
    void update(int, Address, burst_t *, metadata_t *, Node *);
    void initPrereq(void) override;
    void initStiming(void) override;
    int offset(int a, int b, int c) override;

    int getColCmdState(Node *nd, Address addr, bool is_cas_readable);
    int getMultiBankPIMCmdState(Node *nd, Address addr, metadata_t *metadata, bool is_pimx);
    int getRefreshState(Node *nd, Address addr, bool is_ref_ab);

    std::vector<bool> multi_cycle_;
    std::vector<bool> cas_check_;
    std::vector<std::vector<int>> bl32_timing_;
    std::vector<Address> prev_addr_;
    std::list<Request> *bl32_queue_;
    std::vector<uint64_t> fence_end_cycle_;

    // refresh feature
    std::vector<int> refresh_counter_;
    std::vector<bool> is_refresh_needed_;

    // command param indices
    int c_ACT1_;
    int c_ACT2_;
    int c_PRE_;
    int c_PREA_;
    int c_ACTMB1_;
    int c_ACTMB2_;
    int c_RD_;
    int c_WR_;
    int c_RDA_;
    int c_WRA_;
    int c_RDP_;
    int c_WRP_;
    int c_WRPB_;
    int c_PIMX_;
    int c_MWR_;
    int c_MWRA_;
    int c_RD32_;
    int c_WR32_;
    int c_RD32A_;
    int c_WR32A_;
    int c_CAS_RD_;
    int c_CAS_WR_;
    int c_REFpb_;
    int c_REFab_;

    // bankdstate param indices
    int bs_IDLE_;
    int bs_ACTINT_;

    std::vector<std::vector<bool>> waiting_colcmd_;

    // debug purpose
    std::vector<int> dbg_read_time;
};
}  // namespace PimSim
