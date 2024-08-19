#include "lpddr_sim_wrapper.hpp"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <array>

using namespace PimSim;

/*
This is working perfectly!
int main(int argc, char* argv[])
{
    LP5PIMOp  *simObj = new LP5PIMOp();
    unsigned input_dim_arg, output_dim_arg;
    std::string ini_filename_arg;

    simObj->parseCMDLineArgs(
        argc,
        argv,
        input_dim_arg,
        output_dim_arg,
        ini_filename_arg);
    std::cout << "AK - simObj created! \n";
    std::cout << "ini_filename_arg = " << ini_filename_arg << "\n";

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
    simObj->printStats();
    std::cout << "AK - My print -- Total Time = " << simObj->getTotalExecutionTimeInNs() << " \n";
}*/

int recieve_dimension_args(unsigned& input_dim_arg, unsigned& output_dim_arg) {
    int fd;
    std::array<int, 2> recvd_args;
    // Open the named pipe for reading
    fd = open("/tmp/llama2simulator_dimesionArgs", O_RDONLY);

    if (fd == -1) {
        perror("Failed to open named pipe");
        return 1;
    }
    while (true) {
        ssize_t bytes_read = read(fd, recvd_args.data(), recvd_args.size() * sizeof(int));
        if (bytes_read > 0) {
            std::cout << "Received: " << recvd_args[0] << ", " << recvd_args[1] << std::endl;
            input_dim_arg = (unsigned) recvd_args[0];
            output_dim_arg = (unsigned) recvd_args[1];

            // Exit after recieving the two args
            return 0;
        } else if (bytes_read == 0) {
            // End of file, wait for more data
            sleep(1);
        } else {
            perror("Read error");
            break;
        }
    }

    close(fd);

    // Control shouldnt come here!
    return 1;
}

int send_exec_time(double execution_time) { 
    const char* fifoPath = "/tmp/sim2llama_execTimeInNs";
    
    // Open the named pipe for writing
    int fd = open(fifoPath, O_WRONLY);
    if (fd == -1) {
        perror("Failed to open named pipe");
        return 1;
    }

    // Write the double value to the named pipe
    ssize_t bytes_written = write(fd, &execution_time, sizeof(execution_time));
    if (bytes_written == -1) {
        perror("Failed to write to named pipe");
        close(fd);
        return 1;
    }

    std::cout << "Sent: " << execution_time << std::endl;

    close(fd);
    return 0;
}

int main()
{
    unsigned input_dimension_arg, output_dimension_arg;
    int ret_value = 1;
    int num_matmuls_invoked = 0;
    double execTimeInNs = 0;

    while(1) {
        // Recieve Args from the Llama file
        ret_value = recieve_dimension_args(input_dimension_arg, output_dimension_arg);
        std::cout << " AK - 1 : ret_value = " << ret_value << "\n";
        if(ret_value == 0) {
            ++num_matmuls_invoked;
            execTimeInNs = doGemVOpAndGetExecTime(input_dimension_arg, output_dimension_arg, "../ini/LPDDR5X-8533_1P1B.ini");
            std::cout << "****** Num Of Matmuls Invoked=" << num_matmuls_invoked << " and Exec_Time(ns)=" << execTimeInNs << " ****** \n";
        }
        else {
            std::cout << "Some error in reciveing dimension args from llama.c. Simulator exiting \n";
            exit(0);
        }

        //Send the Execution Time to the simulator
        send_exec_time(execTimeInNs);
        if(ret_value != 0) {
            std::cout << "Some error in sending execution time to llama.c. Simulator exiting \n";
            exit(0);
        }
    }
    //std::cout << "Execution Time (ns) = " << doGemVOpAndGetExecTime(1024, 1024, "../ini/LPDDR5X-8533_1P1B.ini");
}
