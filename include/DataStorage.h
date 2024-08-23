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

class pim_timer {
public:
    // Delete copy constructor and assignment operator
    pim_timer(const pim_timer&) = delete;
    pim_timer& operator=(const pim_timer&) = delete;

    static pim_timer& getInstance() {
        static pim_timer instance;
        return instance;
    }

    void reset() {
        total_pim_time_ = 0;
    }

    void update_timer(double new_total_pim_time) {
        total_pim_time_ = new_total_pim_time;

        // If you are updating the timer, that indicates a new PIM execution
        // has taken place, hence we increase the num_of_pim_ops_ value
        ++num_of_pim_ops_;
    }

    double getTotalElapsedTime() {
        return total_pim_time_;
    }

    int64_t get_num_of_pim_ops() {
        return num_of_pim_ops_;
    }

private:
    pim_timer() : total_pim_time_(0.0) {
        total_pim_time_ = 0;
    }

    double total_pim_time_ = 0;
    int64_t num_of_pim_ops_ = 0;
};

class ExecutionTimer {
public:
    // Get the singleton instance
    static ExecutionTimer& getInstance() {
        static ExecutionTimer instance;
        return instance;
    }

    // Delete copy constructor and assignment operator to prevent copying
    ExecutionTimer(const ExecutionTimer&) = delete;
    ExecutionTimer& operator=(const ExecutionTimer&) = delete;

    // Start or reset a timer with the given name
    void startTimer(const std::string& timerName) {
        timers[timerName] = std::chrono::steady_clock::now();
    }

    // Update the timer with the given name
    void updateTimer(const std::string& timerName) {
        auto now = std::chrono::steady_clock::now();
        if (elapsedTimes.find(timerName) != elapsedTimes.end()) {
            elapsedTimes[timerName] += std::chrono::duration_cast<std::chrono::milliseconds>(now - timers[timerName]).count();
        } else {
            elapsedTimes[timerName] = std::chrono::duration_cast<std::chrono::milliseconds>(now - timers[timerName]).count();
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

private:
    ExecutionTimer() = default;
    ~ExecutionTimer() = default;

    std::map<std::string, std::chrono::steady_clock::time_point> timers;
    std::map<std::string, long long> elapsedTimes; // In nanoseconds
};
// #endif // DATA_STORAGE_H
