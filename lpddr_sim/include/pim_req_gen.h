#pragma once

#include <cassert>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "dram_controller.h"
#include "pim.h"

// #define PIM_REQ_LOG

namespace PimSim
{
enum class PIMKernelType : int
{
    GEMV_SRF,
    MAX
};

template <typename T>
class vector4d
{
  public:
    vector4d(size_t d1 = 0, size_t d2 = 0, size_t d3 = 0, size_t d4 = 0, T const &t = T())
        : d1_(d1), d2_(d2), d3_(d3), d4_(d4), data(d1 * d2 * d3 * d4, t)
    {
    }

    T &operator()(size_t i, size_t j, size_t k, size_t l)
    {
        return data[i * d2_ * d3_ * d4_ + j * d3_ * d4_ + k * d4_ + l];
    }

  private:
    size_t d1_, d2_, d3_, d4_;
    std::vector<T> data;
};

class LP5PIMMemReqGen
{
  private:
    DRAMController *dram_con_;
    AddressGen *addr_gen_;
    Config *conf_;
    int channels_;
    int ranks_;
    int bankgroups_;
    int banks_per_group_;
    int rows_;
    int columns_;
    int pims_per_group_;
    int banks_per_pim_;
    int cols_per_row_;
    std::string address_mapping_;
    int prev_gemv_wgt_bst_num_;
    int prev_gemv_out_bst_num_;

    int indexToChannelID(int idx);
    int indexToRankID(int idx);
    int indexToBankgroupID(int idx);
    int indexToBankID(int idx);
    int indexToPIMBlockID(int idx);
    int indexToRowID(int idx);
    int indexToColumnID(int idx);
    int indexToPIMRowID(int idx);
    int indexToPIMColumnID(int idx);
    int tileIndexToBankID(uint32_t tile_idx, int pim_blk_id = 0);
    int indexInPIMBlockToRowID(uint32_t tile_idx, uint32_t num_pimx);
    int indexInPIMBlockToColumnID(uint32_t tile_idx, uint32_t num_pimx);

    int set_prev_gemv_bst_num(int bst_num);

    const int out_offset_ = 10000;  // initial row address for output
    bool enable_single_mac_instruction_;

  public:
    LP5PIMMemReqGen(DRAMController *dram_con, AddressGen *addr_gen)
        : dram_con_(dram_con), addr_gen_(addr_gen)
    {
        conf_ = Config::getInstance();

        channels_ = conf_->channels_;
        ranks_ = conf_->ranks_;
        bankgroups_ = conf_->bankgroups_;
        banks_per_group_ = conf_->banks_per_group_;
        rows_ = conf_->rows_;
        columns_ = conf_->columns_;
        pims_per_group_ = conf_->pims_per_group_;
        banks_per_pim_ = conf_->banks_per_pim_;

        setPrecision(conf_->prec_mac_in_);
        cols_per_row_ = columns_ / conf_->BL_;

        // GEMV(SRF)
        mac_precision_ = conf_->mac_precision_;
        num_srfs_ = conf_->num_srfs_;
        num_vrfs_ = conf_->num_vrfs_;
        num_vrfsets_out_ = conf_->num_vrfs_ / conf_->input_to_acc_ratio_;
        // use as an accumulator when operating gemv(srf)
        // example) mac(F16F32acc): 8 * num_vrfs(32B) == 4 * num_vrfsets_out_(64B)

        prev_gemv_wgt_bst_num_ = 0;
        prev_gemv_out_bst_num_ = 0;
        total_num_pimblocks_ = channels_ * ranks_ * bankgroups_ * pims_per_group_;
        assert(banks_per_pim_ * pims_per_group_ == banks_per_group_);
    }

    void setPrecision(PIMPrecision precision_bits)
    {
        num_elts_in_burst_ = (conf_->bus_width_ * conf_->BL_) / getPrecisionSize(precision_bits);
        PIM::getInstance()->setNumEltsBurst(num_elts_in_burst_);
    }

    int num_vrfs_;         // register for gemv(srf), gemv(vrf) and eltwise
    int num_srfs_;         // input register for gemv(srf)
    int num_vrfsets_out_;  // output register set for gemv(srf)

    int num_vrfs_input_;   // input register for gemv(vrf)
    int num_vrfs_output_;  // output register for gemv(vrf)

    enum MACPrecision mac_precision_;

    int total_num_pimblocks_;
    int num_elts_in_burst_;

    static unsigned getPrecisionSize(enum PIMPrecision p);
    size_t getNumInputTiles(PIMKernelType pim_op, uint32_t input_vec_dim);
    size_t getNumOutputTiles(PIMKernelType pim_op, uint32_t output_vec_dim);
    size_t getInputTileDim(PIMKernelType pim_op, size_t num_used_srfs = 8);
    size_t getOutputTileDim(PIMKernelType pim_op, uint32_t output_vec_dim);
    size_t getNumUsedVRFSets(uint32_t output_vec_dim);
    size_t getNumUsedSRFs(uint32_t input_vec_dim);
    int getBanksPerPIM(void);
    uint32_t getBanksPerPIMBlockFlag(void);
    uint32_t dimToNumBursts(uint32_t dim);

    /* common */
    void setInst(burst_t *bst_inst, metadata_t *mdata, int irf_idx);
    void parkIn(burst_t *bst, metadata_t *mdata);
    void parkOut(burst_t *bst, metadata_t *mdata);
    void modeChange(DRAMMode asis, DRAMMode tobe, burst_t *bst, metadata_t *mdata);

    /* gemv */
    burst_t getGEMVwSRFInst(uint32_t output_vec_dim);
    burst_t getGEMVwSRFInstBackward(uint32_t output_vec_dim, uint32_t input_vec_dim);
    inst_t getMACwSRFInst(MACPrecision mac_precision, uint32_t output_vec_dim);
    burst_t getGEMVwVRFInst(void);
    void setGEMVwSRFWeight(uint32_t output_vec_dim, uint32_t input_vec_dim, burst_t **bsts_weight);
    burst_t *getGEMVOutput(burst_t *bsts_output, int output_bst_num, int num_bursts_output);
    void doGEMV(uint32_t output_vec_dim, uint32_t input_vec_dim, burst_t *bst1, burst_t *bst_zero,
                metadata_t *mdata, PIMKernelType pim_op, burst_t *bsts_input = nullptr);

    void doGEMVforSameOutputTIleIndex(uint32_t output_vec_dim, uint32_t input_vec_dim,
                                      burst_t *bst1, burst_t *bst_zero, metadata_t *dummy,
                                      int output_tile_idx, burst_t *bsts_input = nullptr);
    void readBursts(uint32_t num_bursts, burst_t *bst1, metadata_t *dummy);
    void clearVRF(burst_t *bst_zero, metadata_t *dummy, uint32_t num_used_vrfs_output);
    void storeWeightData(vector4d<std::queue<std::pair<int, int>>> wgt_indices,
                         burst_t **bsts_weight);
};
}  // namespace PimSim
