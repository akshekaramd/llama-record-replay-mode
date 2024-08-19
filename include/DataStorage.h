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

extern bool token_generation_phase_has_started;	// AK - Addition

// Define the struct with the specified members
struct DataStruct {
    uint32_t iteration;
    uint32_t nrows;
    double pim_execution_time_in_ns;
    uint32_t output_matrix_size;
    float output_result_matrix[33000];  // Array of 15000 float values
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

// #endif // DATA_STORAGE_H
