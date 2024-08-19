#pragma once

#include "pim.h"
#include "pim_req_gen.h"

#define GEMV_INPUT_DIV

using namespace std;

namespace PimSim
{
class LP5PIMKernel
{
  protected:
    PIMPrecision mac_in_prec_;
    DRAMController *dram_con_;
    AddressGen *addr_gen_;
    std::mt19937 *ran_gen_;

    Config *conf_;
    LP5PIMMemReqGen *lp5_pim_req_gen_;
    burst_t *bst_inst_elt_, *bst_inst_gemv1_, *bst_inst_gemv2_;
    burst_t *b_dummy_, *bst_zero_;
    metadata_t *m_dummy_, *metadata_;

  public:
    LP5PIMKernel(DRAMController *dram_con, AddressGen *addr_gen)
        : dram_con_(dram_con), addr_gen_(addr_gen)
    {
        conf_ = Config::getInstance();

        std::random_device rd;  // Will be used to obtain a seed for the random number engine
        ran_gen_ = new std::mt19937(rd());  // Standard mersenne_twister_engine seeded with rd()

        m_dummy_ = new metadata_t;
        metadata_ = new metadata_t;
        m_dummy_->u32_data_ = 0;
        metadata_->u32_data_ = 0;
        bst_inst_elt_ = new burst_t;
        bst_inst_gemv1_ = new burst_t;
        bst_inst_gemv2_ = new burst_t;
        b_dummy_ = new burst_t;
        bst_zero_ = new burst_t;
        bst_zero_->bcast(0.0f);

        lp5_pim_req_gen_ = new LP5PIMMemReqGen(dram_con, addr_gen);
    };

    ~LP5PIMKernel(void)
    {
        delete m_dummy_;
        delete metadata_;
        delete bst_inst_elt_;
        delete bst_inst_gemv1_;
        delete bst_inst_gemv2_;
        delete b_dummy_;
        delete bst_zero_;
        delete ran_gen_;
        delete lp5_pim_req_gen_;
    };

    void setPrecision(PIMPrecision prec_bits)
    {
        mac_in_prec_ = prec_bits;
        lp5_pim_req_gen_->setPrecision(prec_bits);
    }

    static bool isGEMVwSRF(enum PIMKernelType pim_op)
    {
        if (pim_op == PIMKernelType::GEMV_SRF)
        {
            return true;
        }
        else
        {
            cerr << "unknown: PIMKernelType GEMV" << endl;
            exit(-1);
            return 0;
        }
    }

    void GEMVKernel(uint32_t output_vec_dim, uint32_t input_vec_dim, enum PIMPrecision mac_in_prec,
                    enum PIMKernelType pim_op, burst_t *bsts_input = nullptr)
    {
#ifdef GEMV_INPUT_DIV
        bool enable_gemv_input_div = false;
        uint32_t output_vec_dim_org = output_vec_dim;
        uint32_t input_vec_dim_org = input_vec_dim;
        size_t num_psum_burst = 0;

        if (pim_op == PIMKernelType::GEMV_SRF)
        {
            uint32_t minimum_required_output_dim =
                lp5_pim_req_gen_->total_num_pimblocks_ * lp5_pim_req_gen_->num_elts_in_burst_;
            uint32_t max_num_input_div = conf_->channels_;

            // FIXME: the current number of partitions is a multiple of two.
            // The maximum number of partitions is the number of channels.
            for (int num_partitions = 1; num_partitions <= max_num_input_div; num_partitions *= 2)
            {
                output_vec_dim = output_vec_dim_org * num_partitions;
                input_vec_dim = input_vec_dim_org / num_partitions;

                if (output_vec_dim * 2 > minimum_required_output_dim)
                {
                    if (num_partitions > 1)
                    {
                        enable_gemv_input_div = true;
                        num_psum_burst = output_vec_dim / lp5_pim_req_gen_->num_elts_in_burst_ *
                                         conf_->input_to_acc_ratio_;
                    }
                    break;
                }
            }
        }
#endif
        if (mac_in_prec_ != mac_in_prec) setPrecision(mac_in_prec);

        // set instruction
        if (conf_->pim_protocol_ == Config::Protocol::NEW)
        {
            *bst_inst_gemv1_ = lp5_pim_req_gen_->getGEMVwSRFInst(output_vec_dim);
        }
        else if (conf_->pim_protocol_ == Config::Protocol::BACKWARD)
        {
            // Get jump loop counter value.
            *bst_inst_gemv1_ =
                lp5_pim_req_gen_->getGEMVwSRFInstBackward(output_vec_dim, input_vec_dim);
        }
        else
        {
            cerr << "unknown: Config::Protocol" << endl;
            exit(-1);
        }

        if (conf_->pim_protocol_ == Config::Protocol::BACKWARD)
        {
            lp5_pim_req_gen_->parkIn(b_dummy_, m_dummy_);
            lp5_pim_req_gen_->modeChange(DRAMMode::SB, DRAMMode::AB, b_dummy_, m_dummy_);
        }

        lp5_pim_req_gen_->setInst(bst_inst_gemv1_, metadata_, 0);

        lp5_pim_req_gen_->doGEMV(output_vec_dim, input_vec_dim, b_dummy_, bst_zero_, m_dummy_,
                                 pim_op, bsts_input);

        if (conf_->pim_protocol_ == Config::Protocol::BACKWARD)
        {
            lp5_pim_req_gen_->modeChange(DRAMMode::AB, DRAMMode::SB, b_dummy_, m_dummy_);
            lp5_pim_req_gen_->parkOut(b_dummy_, m_dummy_);
        }

        bool enable_read_res = true;  // add results/psum read latency
        int num_rd_bursts = output_vec_dim;

        if (enable_read_res)
        {
            num_rd_bursts =
                output_vec_dim / lp5_pim_req_gen_->num_elts_in_burst_ * conf_->input_to_acc_ratio_;

#ifdef GEMV_INPUT_DIV
            if (enable_gemv_input_div)
            {
                num_rd_bursts = num_psum_burst;  // psum
            }
#endif
            lp5_pim_req_gen_->readBursts(num_rd_bursts, b_dummy_, m_dummy_);
        }
    }

    void parkIn()
    {
        lp5_pim_req_gen_->parkIn(b_dummy_, m_dummy_);
    }

    void parkOut()
    {
        lp5_pim_req_gen_->parkOut(b_dummy_, m_dummy_);
    }

    void modeChange(DRAMMode asis, DRAMMode tobe)
    {
        lp5_pim_req_gen_->modeChange(asis, tobe, b_dummy_, m_dummy_);
    }

    void run()
    {
        dram_con_->runUntilEmpty();
    }
};
}  // namespace PimSim
