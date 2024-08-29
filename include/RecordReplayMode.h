//#ifndef DATA_STORAGE_H
//#define DATA_STORAGE_H

#include <vector>
#include <iostream>
#include <cstring>   // for memcpy
#include <fstream>   // for file I/O
#include <iomanip>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <map>
#include <chrono>

#include "../ggml/include/ggml.h"

#define     ERROR_CODE      1
#define     SUCCESS_CODE    2

extern bool token_generation_phase_has_started;	// AK - Addition
extern uint32_t    gemv_iteration;

// Define the struct with the specified members
struct DataStruct {
    uint32_t iteration;
    uint32_t nrows;

    double pim_execution_time_in_ns;

    uint32_t output_matrix_size;
    std::vector<float> output_result_matrix;
};

// Singleton class to manage the globalDataVector
class DataStorage {
public:
    // Get the singleton instance
    static DataStorage& getInstance();

    // Add a DataStruct to the vector
    void addData(const DataStruct& data);

    // Get data by index
    const DataStruct& getData(size_t index) const;

    // Get the size of the vector
    size_t getSize() const;

private:
    // Private constructor to prevent instantiation
    DataStorage() = default;

    // Disable copy constructor and assignment operator
    DataStorage(const DataStorage&) = delete;
    DataStorage& operator=(const DataStorage&) = delete;

    // Vector to store DataStructs
    std::vector<DataStruct> dataVector;
};

void populateDataFromJson();
void readDataFromMemory(size_t index);
double simulate_gemv_on_pim(int input_dimension_arg, int output_dimension_arg);

class RecordReplayAPI {
public:
    // Get the singleton instance
    static RecordReplayAPI& getInstance() {
        static RecordReplayAPI instance;
        return instance;
    }

    // Delete copy constructor and assignment operator to prevent copying
    RecordReplayAPI(const RecordReplayAPI&) = delete;
    RecordReplayAPI& operator=(const RecordReplayAPI&) = delete;

    // Start or reset a timer with the given name
    void startTimer(const std::string& timerName) {
        timers[timerName] = std::chrono::steady_clock::now();
    }

    // Update the timer with the given name
    void updateTimer(const std::string& timerName) {
        auto now = std::chrono::steady_clock::now();
        if (elapsedTimes.find(timerName) != elapsedTimes.end()) {
            elapsedTimes[timerName] += std::chrono::duration_cast<std::chrono::nanoseconds>(now - timers[timerName]).count();
        } else {
            elapsedTimes[timerName] = std::chrono::duration_cast<std::chrono::nanoseconds>(now - timers[timerName]).count();
        }
        timers[timerName] = now;
    }

    // Reset a timer to zero
    void resetTimer(const std::string& timerName) {
        elapsedTimes[timerName] = 0;
    }

    // Get the elapsed time for a given timer
    long long getElapsedTime(const std::string& timerName) {
        return elapsedTimes[timerName];
    }

    // Increment the counter
    void increment_gemv_counter(const std::string& counter_name) {
        // Check if the counter exists
        if (gemv_counter.find(counter_name) == gemv_counter.end()) {
            // Initialize the counter to 0 if it doesn't exist
            gemv_counter[counter_name] = 0;
        }

        // Increment the counter
        ++gemv_counter[counter_name];
    }

    // Reset the counter
    void reset_gemv_counter(const std::string& counter_name) {
        gemv_counter[counter_name] = 0;
    }

    // Read the counter value
    uint64_t read_gemv_counter(const std::string& counter_name) {
        return gemv_counter[counter_name];
    }

private:
    RecordReplayAPI() = default;
    ~RecordReplayAPI() = default;

    std::map<std::string, std::chrono::steady_clock::time_point> timers;
    std::map<std::string, long long> elapsedTimes; // In nanoseconds

    // Following variables are used to keep track of number of GEMV ops
    std::map<std::string, uint64_t> gemv_counter;
};

double simualate_gemv_on_aie(uint32_t output_dimension);
double getGEMVCost(int tileSize);
int determineTileSize(int n);
int calculateGEMVOperations(int n, int tileSize);

// #endif // DATA_STORAGE_H
