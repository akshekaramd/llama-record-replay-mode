#include "lpddr_sim_wrapper.hpp"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <array>

#define     ERROR_CODE      1
#define     SUCCESS_CODE    2

using namespace PimSim;

bool createOrReplaceFIFO(const std::string& fifoName) {
    // Check if the FIFO file exists
    if (access(fifoName.c_str(), F_OK) == 0) {
        // FIFO exists, remove it
        if (unlink(fifoName.c_str()) != 0) {
            std::cerr << "Error: Could not remove existing FIFO " << fifoName << std::endl;
            return false;
        }
    }

    // Create a new FIFO with the specified name
    if (mkfifo(fifoName.c_str(), 0666) != 0) {
        std::cerr << "Error: Could not create FIFO " << fifoName << std::endl;
        return false;
    }

    return true;
}

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
        return ERROR_CODE;
    }
    while (true) {
        ssize_t bytes_read = read(fd, recvd_args.data(), recvd_args.size() * sizeof(int));
        if (bytes_read > 0) {
            std::cout << "Received: " << recvd_args[0] << ", " << recvd_args[1] << std::endl;
            input_dim_arg = (unsigned) recvd_args[0];
            output_dim_arg = (unsigned) recvd_args[1];

            // Exit after recieving the two args
            return SUCCESS_CODE;
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
        return ERROR_CODE;
    }

    // Write the double value to the named pipe
    ssize_t bytes_written = write(fd, &execution_time, sizeof(execution_time));
    if (bytes_written == -1) {
        perror("Failed to write to named pipe");
        close(fd);
        return ERROR_CODE;
    }

    std::cout << "Sent: " << execution_time << std::endl;

    close(fd);
    return SUCCESS_CODE;
}

void setup_fifo_pipes() {
    // Create this pipe to enable Llama.cpp --> PIM Simulator interface
    if (createOrReplaceFIFO("/tmp/llama2simulator_dimesionArgs")) {
        std::cout << "FIFO pipe @ /tmp/llama2simulator_dimesionArgs is ready for use." << std::endl;
    } else {
        std::cout << "Failed to create FIFO pipe @ /tmp/llama2simulator_dimesionArgs \n";
        std::cout << "Try deleting the existing one using 'rm -rfv /tmp/llama2simulator_dimesionArgs' and then run this program \n";
        exit(0);
    }

    // Create this pipe to enable PIM Simulator --> Llama.cpp interface
    if (createOrReplaceFIFO("/tmp/sim2llama_execTimeInNs")) {
        std::cout << "FIFO pipe @ /tmp/sim2llama_execTimeInNs is ready for use." << std::endl;
    } else {
        std::cout << "Failed to create FIFO pipe @ /tmp/sim2llama_execTimeInNs \n";
        std::cout << "Try deleting the existing one using 'rm -rfv /tmp/sim2llama_execTimeInNs' and then run this program \n";
        exit(0);
    }
}

int main()
{
    unsigned input_dimension_arg, output_dimension_arg;
    int ret_value = 1;
    int num_matmuls_invoked = 0;
    double execTimeInNsForThisGemvOp = 0;
    double temp_execTimeInNs = 0;
    double totalExecTimeInNs = 0;
    std::map<std::pair<unsigned, unsigned>, double> dim_kv_table;

    setup_fifo_pipes();

    while(1) {
        std::cout << "********************** Size of Lookup Table = " << dim_kv_table.size() << "***********************\n";
        // Recieve Args from the Llama file
        ret_value = recieve_dimension_args(input_dimension_arg, output_dimension_arg);

        if (ret_value != SUCCESS_CODE) {
            std::cout << "Some error in reciveing dimension args from llama.c. Simulator exiting \n";
            exit(0);
        }
        ++num_matmuls_invoked;
        
        std::pair<unsigned, unsigned> dim_key = std::make_pair(input_dimension_arg, output_dimension_arg);

        bool lookup_result;
        execTimeInNsForThisGemvOp = lookup_dim_value_table(dim_kv_table, dim_key, lookup_result);

        // If found, then just write the exec time (value) into the FIFO file
        if(lookup_result == true) {
            std::cout << "This value of <" << input_dimension_arg << ", " << output_dimension_arg << "> is already found. Using CACHED results. WAIT YOU SHOULDNT BE HERE!!!!!!!!!!!!!!!!!!! ... \n";
        }
        else {
            std::cout << "--> Value not found in the lookup table... Running actual simulation! \n";
            double totalPimSimexecTimeInNs = doGemVOpAndGetExecTime(input_dimension_arg, output_dimension_arg, "../ini/LPDDR5X-8533_1P1B.ini");
            std::cout << "****** Num Of Matmuls Invoked=" << num_matmuls_invoked << " and Exec_Time(ns)=" << totalPimSimexecTimeInNs << " ****** \n";

            // Add this new dimension values into the lookup table
            // Generally totalPimSimexecTimeInNs in this code block holds total ExecTime of all GEMV ops simulated in the PIM Simulator. 
            // After sending it through the addNewSimResultToTable(), we get ExecTime (ns) for this specific GEMV op.
            // execTimeInNsForThisGemvOp would be equal to the execution time for this specific invovation.
            execTimeInNsForThisGemvOp = addNewSimResultToTable(dim_kv_table, totalPimSimexecTimeInNs, input_dimension_arg, output_dimension_arg);
        }

        //Send the Execution Time to the simulator
        send_exec_time(execTimeInNsForThisGemvOp);
        if(ret_value != SUCCESS_CODE) {
            std::cout << "Some error in sending execution time to llama.cpp. Simulator exiting \n";
            exit(0);
        }
    }
    //std::cout << "Execution Time (ns) = " << doGemVOpAndGetExecTime(1024, 1024, "../ini/LPDDR5X-8533_1P1B.ini");
}
