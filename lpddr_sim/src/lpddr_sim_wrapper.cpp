// This was testing program. This is depracated. Just kept it here, for any future testing

#include "lpddr_sim_wrapper.h"

using namespace PimSim;

/**********************
 * GEMV SRF
 **********************/
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
    return simObj->getTotalExecutionTimeInNs();
}
