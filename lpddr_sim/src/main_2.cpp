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

// Add a new set of Simulation result to the Table
// The function returns the individual execution time (ns) taken for this specific invocation
double addNewSimResultToTable(std::map<std::pair<unsigned, unsigned>, double>& lookupTable, 
                        double NewSimTimeInNs, 
                        unsigned input_dimension_arg, 
                        unsigned output_dimension_arg) {
    
    /* The Simulator basically reports cumulative time for each invocation and does not split report 
     * per invocation time. For ex. invocation-1 reports 5ms and invocation-2 reports 8 ms. The way 
     * to actually interpret this is that invocation-2 actually took 3 ms (8ms-5ms). We need to do 
     * this subtraction out here in this function before adding the value into the table. 
     * We need to subtract the value with every existing table value and then add it into the table.
     */

    double finalNewSimTimeInNs = NewSimTimeInNs;
    for (const auto& entry : lookupTable) {
        double existingSimTime = entry.second;
        finalNewSimTimeInNs = finalNewSimTimeInNs - existingSimTime;
    }

    std::cout << " Adding New Sim Time = " << finalNewSimTimeInNs << " (ns) at position=" 
                << lookupTable.size() << " into the table \n";
    
    lookupTable[std::make_pair(input_dimension_arg, output_dimension_arg)] = finalNewSimTimeInNs;
    return finalNewSimTimeInNs;
}

// Function to find if similar dimensions (represented by key) already existed
double lookup_dim_value_table(const std::map<std::pair<unsigned, unsigned>, double>& lookupTable,
                const std::pair<unsigned, unsigned>& key,
                bool& lookup_result) {
    auto it = lookupTable.find(key);
    if (it != lookupTable.end()) {
        lookup_result = true;
        return it->second;
    } else {
        lookup_result = false;
        return 0;
    }
}

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
    double temp_execTimeInNs = 0;
    double totalExecTimeInNs = 0;
    std::map<std::pair<unsigned, unsigned>, double> dim_kv_table;

    while(1) {
        std::cout << "********************** Size of Lookup Table = " << dim_kv_table.size() << "***********************\n";
        // Recieve Args from the Llama file
        ret_value = recieve_dimension_args(input_dimension_arg, output_dimension_arg);

        if (ret_value == 1) {
            std::cout << "Some error in reciveing dimension args from llama.c. Simulator exiting \n";
            exit(0);
        }
        ++num_matmuls_invoked;
        
        std::pair<unsigned, unsigned> dim_key = std::make_pair(input_dimension_arg, output_dimension_arg);

        bool lookup_result;
        execTimeInNs = lookup_dim_value_table(dim_kv_table, dim_key, lookup_result);

        // If found, then just write the exec time (value) into the FIFO file
        if(lookup_result == true) {
            std::cout << "This value of <" << input_dimension_arg << ", " << output_dimension_arg << "> is already found. Using CACHED results ... \n";
        }
        else {
            std::cout << "--> Value not found in the lookup table... Running actual simulation! \n";
            execTimeInNs = doGemVOpAndGetExecTime(input_dimension_arg, output_dimension_arg, "../ini/LPDDR5X-8533_1P1B.ini");
            std::cout << "****** Num Of Matmuls Invoked=" << num_matmuls_invoked << " and Exec_Time(ns)=" << execTimeInNs << " ****** \n";

            // Add this new dimension values into the lookup table
            // Generally the previous instance of execTimeInNs in this code block holds total ExecTime. 
            // After sending it through the addNewSimResultToTable(), we get ExecTime (ns) for this specific invocation.
            // execTimeInNs would be equal to the execution time for this specific invovation.
            execTimeInNs = addNewSimResultToTable(dim_kv_table, execTimeInNs, input_dimension_arg, output_dimension_arg);
        }

        //Send the Execution Time to the simulator
        totalExecTimeInNs = totalExecTimeInNs + execTimeInNs;
        send_exec_time(totalExecTimeInNs);
        if(ret_value != 0) {
            std::cout << "Some error in sending execution time to llama.c. Simulator exiting \n";
            exit(0);
        }
    }
    //std::cout << "Execution Time (ns) = " << doGemVOpAndGetExecTime(1024, 1024, "../ini/LPDDR5X-8533_1P1B.ini");
}
