#include <queue>

#include "dram_stat.h"
#include "gtest/gtest.h"
#include "gtest/internal/gtest-port.h"
#include "pim.h"
#include "test_pim_kernel.h"
#include <vector>
#include <string>

using namespace std;

namespace PimSim
{
class LP5PIMOp
{
  public:
    LP5PIMOp() {}

    // ~LP5PIMOp() override {}

    void SetUp(unsigned input_dim_arg, unsigned output_dim_arg, std::string ini_filename_arg)
    {
        output_dim_ = output_dim_arg;
        input_dim_ = input_dim_arg;
        ini_file_name_ = ini_filename_arg;

/*
#ifdef HAS_NO_GETARGVS
        // libgtest <= 1.7.0 has no GetArgvs function in the internal namespace
        auto argv = ::testing::internal::g_argvs;
#else
        auto argv = ::testing::internal::GetArgvs();
#endif
        for (int i = 0; i < argv.size(); i++)
        {
            if (argv[i].compare("--output_dim") == 0 || argv[i].compare("--o_dim") == 0)
            {
                output_dim_ = static_cast<unsigned>(atoi(argv[i + 1].c_str()));
            }
            if (argv[i].compare("--input_dim") == 0 || argv[i].compare("--i_dim") == 0)
            {
                input_dim_ = static_cast<unsigned>(atoi(argv[i + 1].c_str()));
            }
            if (argv[i].compare("--ini_file") == 0)
            {
                ini_file_name_ = argv[i + 1];
            }
        }

       for (int i = 0; i < argv.size(); i++)
        {
            if (argv[i].compare("--output_dim") == 0 || argv[i].compare("--o_dim") == 0)
            {
                output_dim_ = static_cast<unsigned>(atoi(argv[i + 1].c_str()));
            }
            if (argv[i].compare("--input_dim") == 0 || argv[i].compare("--i_dim") == 0)
            {
                input_dim_ = static_cast<unsigned>(atoi(argv[i + 1].c_str()));
            }
            if (argv[i].compare("--ini_file") == 0)
            {
                ini_file_name_ = argv[i + 1];
            }
        }*/

        std::cout << "ini_file = " << ini_file_name_ << "\n" << std::flush;

        conf_ = Config::getInstance();
        std::cout << "Point - 1 \n" << std::flush;
        dram_stat_ = DRAMStat::getInstance();
        std::cout << "Point - 2 \n" << std::flush;
        conf_->set(ini_file_name_);
        std::cout << "Point - 3 \n" << std::flush;
        conf_->setPIMAddr(40, 64, 4, 32);
        std::cout << "Point - 4 \n" << std::flush;

        dram = new LPDDR5(conf_);
        dram->initAll();
        read_cb_ = std::bind(&LP5PIMOp::DRAM_read_return_cb, this, std::placeholders::_1);
        write_cb_ = std::bind(&LP5PIMOp::DRAM_write_return_cb, this, std::placeholders::_1);
        dram_con_ = new LP5Controller(conf_, read_cb_, write_cb_);
        dram_con_->attachDRAM(dram);
        dram_con_->initAll();

        addr_gen_ = new AddressGen(conf_);
        dram_stat_->clear();

        lp5_pim_kernel_ = new LP5PIMKernel(dram_con_, addr_gen_);
        precision_bits_ = conf_->prec_mac_in_;  // TODO: precision_bits --> mac_precision

        cout << "------------------------------------------" << endl;
        cout << " Test Configuration" << endl;
        cout << " - output_dim : " << output_dim_ << endl;
        cout << " - input_dim  : " << input_dim_ << endl;
        cout << " - ini file   : " << ini_file_name_ << endl;
        cout << " - precision  : " << conf_->mac_precision_str_ << endl;
        cout << " - Protocol   : " << conf_->pim_protocol_str_ << endl;
        cout << "------------------------------------------" << endl;
    }

    //void TearDown() override
    ~LP5PIMOp()
    {
        delete dram;
        delete dram_con_;
        delete addr_gen_;
        delete lp5_pim_kernel_;
    }

    Config *conf_;
    DRAMStat *dram_stat_;
    DRAM *dram;
    DRAMController *dram_con_;
    AddressGen *addr_gen_;
    LP5PIMKernel *lp5_pim_kernel_;

    /* parameter */
    int bankgroups_;
    int pims_per_group_;
    int banks_per_pim_;
    int rows_;
    int columns_;
    int device_width_;
    int BL_;
    int channel_size_;
    int channels_;
    int bus_width_;
    std::string address_mapping_;
    int banks_per_group_;
    int banks_;
    enum PIMPrecision precision_bits_;

    /* input argument */
    unsigned output_dim_;        // --output_dim
    unsigned input_dim_;         // --input_dim
    std::string ini_file_name_;  // --ini_file

    /* callback*/
    std::function<void(uint64_t)> read_cb_;
    std::function<void(uint64_t)> write_cb_;
    void DRAM_read_return_cb(uint64_t addr);
    void DRAM_write_return_cb(uint64_t addr);

    bool isExponentOfTwo(uint32_t n);
    // Following are AK changes
    void parseCMDLineArgs(int argc, char* argv[], 
            unsigned& output_dim_arg, unsigned& input_dim_arg, std::string& ini_filename_arg);
    PIMPrecision getPIMPrecision(){
        return precision_bits_;
    };
    void printStats() {
        dram_stat_->printPower((LP5Controller *)dram_con_, true);
        dram_stat_->printTotalTime((LP5Controller *)dram_con_);
    };
    double getTotalExecutionTimeInNs() {
        return (dram_stat_->getTotalTime((LP5Controller *)dram_con_) * pow(10, 9));
    }
};

bool LP5PIMOp::isExponentOfTwo(uint32_t n)
{
    if (n & (n - 1)) return false;
    return true;
}

void LP5PIMOp::DRAM_read_return_cb(uint64_t addr) {}

void LP5PIMOp::DRAM_write_return_cb(uint64_t addr) {}

void LP5PIMOp::parseCMDLineArgs(int argc, char* argv[], unsigned& input_dim_arg, unsigned& output_dim_arg, std::string& ini_filename_arg){

    std::vector<std::string> str_args;
    for (int i = 0; i < argc; ++i) {
        str_args.push_back(std::string(argv[i]));
        std::cout << "Check-1 Arg-" << i << " = " << std::string(argv[i]) << "\n"; //<< " str_args=" << str_args[i] << "\n";
    }
    for (int i = 0; i < argc; ++i) {
        std::cout << " Arg-" << i << " = " << str_args[i] << "\n";
    }

    std::cout << " Size of str_args = " << str_args.size() << "\n";
    for (int i = 0; i < str_args.size(); i++)
    {
        if (str_args[i].compare("--output_dim") == 0 || str_args[i].compare("--o_dim") == 0)
        {
            output_dim_arg = static_cast<unsigned>(atoi(str_args[i + 1].c_str()));
        }
        if (str_args[i].compare("--input_dim") == 0 || str_args[i].compare("--i_dim") == 0)
        {
            input_dim_arg = static_cast<unsigned>(atoi(str_args[i + 1].c_str()));
        }
        if (str_args[i].compare("--ini_file") == 0)
        {
            ini_filename_arg = str_args[i + 1];
        }
    }
}

double doGemVOpAndGetExecTime(unsigned input_dim_arg, unsigned output_dim_arg, std::string ini_filename_arg) {
    LP5PIMOp  *simObj = new LP5PIMOp();

    simObj->SetUp(
        input_dim_arg,
        output_dim_arg,
        ini_filename_arg);

    simObj->lp5_pim_kernel_->GEMVKernel(
        output_dim_arg,
        input_dim_arg,
        simObj->getPIMPrecision(),
        PIMKernelType::GEMV_SRF
    );

    simObj->lp5_pim_kernel_->run();
    double execTimeInNs = simObj->getTotalExecutionTimeInNs();
    delete simObj;

    return execTimeInNs;
}
}  // namespace PimSim
