#include "DataStorage.h"
#include <nlohmann/json.hpp>
#include <vector>
#include <iostream>
#include <cstring>   // for memcpy
#include <fstream>   // for file I/O
#include <iomanip>

// TODO - Can you avoid using namespaces ?
// using json = nlohmann::json;

std::map<std::pair<unsigned, unsigned>, double> global_dim_kv_table;

// Get the singleton instance
DataStorage& DataStorage::getInstance() {
    static DataStorage instance;
    return instance;
}

// Add a DataStruct to the vector
void DataStorage::addData(const DataStruct& data) {
    dataVector.push_back(data);
}

// Get data by index
const DataStruct& DataStorage::getData(size_t index) const {
    if (index < dataVector.size()) {
        return dataVector[index];
    } else {
        throw std::out_of_range("Index out of range");
    }
}

// Get the size of the vector
size_t DataStorage::getSize() const {
    return dataVector.size();
}

// ********************* AK - Changes go here *************************
// Function to check if a file exists using ifstream
bool fileExists(const std::string& filename) {
    std::ifstream file(filename.c_str());
    return file.good();
}

// Function to populate the DataStorage singleton from JSON files
void populateDataFromJson() {
    size_t index = 0;
    while (true) {
        std::string filename = "record_mode/output_" + std::to_string(index) + ".json";

        // Check if the file exists
        if (!fileExists(filename)) {
            std::cout << "File " << filename << " not found. Stopping data population." << std::endl;
            break;
        }

        // Open and parse the JSON file
        std::ifstream file(filename.c_str());
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << filename << std::endl;
            break;
        }

        nlohmann::json j;
        file >> j;

        // Populate the DataStruct with values from the JSON
        DataStruct data;
        data.iteration = j["iteration"].get<uint32_t>();
        data.nrows = j["nrows"].get<uint32_t>();
        data.pim_execution_time_in_ns = j["gemv_pim_exec_time_in_ns"].get<double>();
        // data.output_matrix_size = j["output_matrix_size"].get<uint32_t>();

        // Copy the output_result_matrix array from JSON
        // auto output_result_matrix = j["output_matrix"].get<std::vector<float>>();
        data.output_result_matrix = j["output_matrix"].get<std::vector<float>>();
        // data.output_matrix_size = output_result_matrix.size();
        // std::memcpy(data.output_result_matrix, output_result_matrix.data(), output_result_matrix.size() * sizeof(float));

        // Add the populated DataStruct to the DataStorage singleton
        DataStorage::getInstance().addData(data);

        std::cout << "Populated struct " << index << " with data from " << filename << std::endl;

        ++index;
    }
}

// Function to read data from a specific element in the DataStorage singleton
void readDataFromMemory(size_t index) {
    const DataStruct& data = DataStorage::getInstance().getData(index);

    // Read and print the data from the specified index in the vector
    std::cout << "Iteration: " << data.iteration << std::endl;
    std::cout << "Number of Rows: " << data.nrows << std::endl;
    std::cout << "PIM Execution Time (ns): " << data.pim_execution_time_in_ns << std::endl;
    std::cout << "Output Matrix Size: " << data.output_matrix_size << std::endl;
    std::cout << "First Element of Output Result Matrix: " << data.output_result_matrix[0] << std::endl; // Example of accessing array
}

// Function to find the nearest next highest power of 2
unsigned int roundToNearestPowerOf2(unsigned int n) {
    // If its 0, the answer is technically 0. 
    // But the simulator only accepts powers of 2 as input.
    // Hence, it should return 0 rather than an odd number. 
    if (n == 0) return 0;
    if (n == 1) return 2;

    // If n is already a power of 2, return n
    if (n && !(n & (n - 1))) {
        return n;
    }

    // Otherwise, find the next power of 2
    unsigned int power = 1;
    while (power < n) {
        power <<= 1;
    }
    
    return power;
}

int send_dimension_args(int input_arg, int output_arg) {
    printf("Sending inp=%d outp=%d to simulator \n", input_arg, output_arg);
    int fd;
    int send_packet[] = {input_arg, output_arg};

    // Open the named pipe for writing
    fd = open("/tmp/llama2simulator_dimesionArgs", O_WRONLY);

    if (fd == -1) {
        perror("Failed to open named pipe");
        return ERROR_CODE;
    }
    write(fd, send_packet, sizeof(send_packet));
    close(fd);
    return SUCCESS_CODE;
}

int receiveExecTimeInNsAndUpdateTiming(double *ExecTimeInNs) {
    const char* fifoPath = "/tmp/sim2llama_execTimeInNs";
    double receivedValue;

    // Open the named pipe for reading
    int fd = open(fifoPath, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open named pipe");
        return ERROR_CODE;
    }

    // Read the double value from the named pipe
    ssize_t bytes_read = read(fd, &receivedValue, sizeof(receivedValue));
    if (bytes_read == -1) {
        perror("Failed to read from named pipe");
        close(fd);
        return ERROR_CODE;
    }

    printf("Received: %f\n", receivedValue);

    *ExecTimeInNs = receivedValue;
    close(fd);
    return SUCCESS_CODE;
}

// Add a new set of Simulation result to the Table
// The function returns the individual execution time (ns) taken for this specific invocation
void addNewSimResultToTable(
                        double NewSimTimeInNs, 
                        unsigned input_dimension_arg, 
                        unsigned output_dimension_arg) {

    global_dim_kv_table[std::make_pair(input_dimension_arg, output_dimension_arg)] = NewSimTimeInNs;
}

// Function to find if similar dimensions (represented by key) already existed
double lookup_dim_value_table(
                const std::pair<unsigned, unsigned>& key,
                bool& lookup_result) {
    auto it = global_dim_kv_table.find(key);
    if (it != global_dim_kv_table.end()) {
        lookup_result = true;
        return it->second;
    } else {
        lookup_result = false;
        return 0;
    }
}

/* Assume, we are doing the following GEMV operation : 
 *  A . B = Y
 *
 * A is an input matrix of m rows and n cols (mxn)
 * B is an input matrix of n cols
 */
double simulate_gemv_on_pim(int input_dimension_arg, int output_dimension_arg) {
    bool lookup_result;
    std::pair<unsigned, unsigned> dimension_keys = std::make_pair(input_dimension_arg, output_dimension_arg);

    // Look up Dimension KV table    
    double time_to_execute_this_gemv_op = lookup_dim_value_table(dimension_keys, lookup_result);

    // If it exists, then return the value back
    if(lookup_result == true) {
        // std::cout << " DataStorage.cpp : Look Up result found! Reusing CACHED timings values ... \n";
        return time_to_execute_this_gemv_op;
    }
    // else
    //    std::cout << " DataStorage.cpp : New Arguments encountered. Running an actual simulation ... \n";
    // If not, send it to PIM and return the value back

    // If you are here, then this is a new set of dimensions that have not been encountered so far, 
    // We will send these dimensions to the PIM simulator and get the execution time for this.
    int ret_value = ERROR_CODE;

    // Now send the args to the simulator
    ret_value = send_dimension_args(
                        roundToNearestPowerOf2(input_dimension_arg),
                        roundToNearestPowerOf2(output_dimension_arg)
                );
    if(ret_value != SUCCESS_CODE) {
        printf("DataStorage.cpp: Error in sending dimension args to simulator. Exiting \n");
        exit(0);
    }

    // Recieve the exec time from the simulator
    time_to_execute_this_gemv_op = 0;
    ret_value = receiveExecTimeInNsAndUpdateTiming(&time_to_execute_this_gemv_op);
    // std::cout << " DataStorage.cpp : ret_value @228 = " << ret_value << "\n";
    if(ret_value != SUCCESS_CODE) {
        printf("DataStorage.cpp : Error in Recieving Execution Time from simulator. Exiting \n");
        exit(0);
    }

    // Cache the new timing results to the look up table and output this value
    addNewSimResultToTable(time_to_execute_this_gemv_op, input_dimension_arg, output_dimension_arg); 
    return time_to_execute_this_gemv_op;
}
// ********************* AK - Changes end here *************************
