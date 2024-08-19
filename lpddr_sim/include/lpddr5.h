#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "dram.h"

namespace PimSim
{
class LPDDR5 : public DRAM
{
  public:
    enum class Command : int
    {
        ACT1,
        ACT2,
        PRE,
        PREA,
        ACTMB1,
        ACTMB2,
        RD,
        WR,
        RDA,
        WRA,
        RDP,
        WRP,
        WRPB,
        PIMX,
        MWR,
        MWRA,
        RD32,
        WR32,
        RD32A,
        WR32A,
        CAS_RD,
        CAS_WR,
        REFpb,
        REFab,
        MAX
    };

    enum class TimeoutTiming : int
    {
        WR,
        MWR,
        RD,
        MAX
    };

    enum class BankState : int
    {
        IDLE,
        ACTINT,
        ACT,
        MAX
    };

    enum class ACParam : int
    {
        tRC,    // ACTIVATE-to-ACTIVATE command period (same bank)
        tRAS,   // Row active time
        tRCD,   // RAS-to-CAS delay
        tRPab,  // Row precharge time (all banks)
        tRPpb,  // Row precharge time (single bank)
        tRRD,   // Active bank-A to active bank-B
        tPPD,   // Precharge to Precharge delay
        tFAW,   // Four-bank ACTIVATE window
        // Effective Burst Length definition (BL/n)
        BLn_min,       // 2
        BLn_max,       // 4
        BLn_samebg,    // 4
        BLn_diffbg,    // 2
        BL32n_min,     // 6
        BL32n_max,     // 8
        BL32n_samebg,  // 8
        BL32n_diffbg,  // 2

        tPIMX,  // currently same as BLn_samebg
        // no tCCDs, BL/n notation for LPDDR5
        // tCCDL,
        // tCCDS,
        // tCCDR, no CCDR, RTRR, WTWR have different values.
        tRTWL,  // READ command to WRITE command delay, same bank group
        tRTWS,  // READ command to WRITE command delay, different bank group
        tRBTP,  // READ burst end to PRECHARGE command delay
        WL,     // Write Latency
        RL,     // Read Latency
        tWTRL,  // Write burst to Read Command (same bank group)
        tWTRS,  // Write burst to Read Command (diff bank group)
        // This values stand for readability
        tRTRR,  // Read Command to Read Command different rank
        tRTWR,  // Read Command to Write Command different rank
        tWTRR,  // Write Command to Read Command different rank
        tWTWR,  // Write Command to Write Command different rank

        tR32TWL,  // READ32 command to WRITE command delay, same bank group
        tR32TWS,  // READ32 command to WRITE command delay, different bank group
        tR32TRR,  // Read32 Command to Read Command different rank
        tR32TWR,  // Read32 Command to Write Command different rank
        tW32TRR,  // Write32 Command to Read Command different rank
        tW32TWR,  // Write32 Command to Write Command different rank

        tWR,          // Write recovery time
        tRFCpb,       // Refresh cycle time (per-bank)
        tRFCab,       // Refresh cycle time (all-bank)
        tpbR2pbR,     // REFpb to REFpb to different bank
        tpbr2act,     // REFpb to Activate command to different bank
        tAAD,         // Maximum legal delay between ACT-1 and ACT-2
        tREFW,        // Refresh Window
        tREFI,        // Average Refresh Interval
        tREFIpb,      // Average Refresh Interval (per-bank)
        burst_cycle,  // 2 cycle assuming BL16
        MAX
    };

    LPDDR5(Config* conf_);
    ~LPDDR5(void);

    // states x number of commands
    std::vector<std::vector<int>> next_command_;
    std::vector<int> act_intermediate_cmds_;
    std::vector<int> sync_cmds_;
    std::vector<int> sync_wr_cmds_;
    std::vector<int> sync_rd_cmds_;

    // # of channel * # of banks, here we only assume single rank
    std::vector<std::vector<bool>> refresh_checker_;
    std::vector<int> act_taad_timing_;
    std::vector<std::vector<Node*>> act1_node_;

    int WCK_to_CK_ratio_;
    static const int tRAS_ns = 42;
    static const int tRP_ns = 18;
    static const int tRFCab_ns = 280;

    void setCommandInfo(void) override;
    void setStateInfo(void) override;
    void initTree(void) override;
    void initTiming(void) override;
    void initLambda(void) override;
    void initLogging(void) override;
    void initStateCheck(void) override;
    float getClockTiming(void) override
    {
        return clock_timing_;
    }
    int offset(int a, int b, int c) override;
    void modeChange(Node*, DRAMMode);

    void checkAndUpdateState(Node*, int);
    void setACT2(Node*);
    void setRDBank(Node*);
    void setWRaMWR(Node*);
    void setWRAaMWRA(Node*);
    void setPRE(Node*);
    void lambdaACT1(void);
    void lambdaACT2(void);
    void lambdaPRE(void);
    void lambdaRD(void);
    void lambdaWR(void);
    void lambdaMWR(void);
    void lambdaRD32(void);
    void lambdaWR32(void);
    void lambdaRDA(void);
    void lambdaWRA(void);
    void lambdaMWRA(void);
    void lambdaRDP(void);
    void lambdaWRP(void);
    void lambdaWRPB(void);
    void lambdaACTMB1(void);
    void lambdaACTMB2(void);
    void lambdaPREA(void);
    void lambdaPIMX(void);

    std::unordered_map<std::string, double> getPowerStats()
    {
        return energy_power_;
    }

    inline bool isEnabledBank(int id, int ba, metadata_t* metadata = nullptr)
    {
        if (conf_->banks_per_pim_ == 1)
        {
            return true;
        }
        else if (conf_->banks_per_pim_ == 2)
        {
            return (id % 2 == ba % 2);
        }
        else
        {
            std::cerr << "unknown: banks per pim" << std::endl;
            exit(-1);
        }
        // bool is_enabled_bank = (metadata->u32_data_ == 0) ? true
        //                        : (metadata->u32_data_ == 1)
        //                            ? ((nd->id % 2) ? true : false)
        //                            : ((nd->id % 2 == 0) ? true : false);
    }
};
}  // namespace PimSim
