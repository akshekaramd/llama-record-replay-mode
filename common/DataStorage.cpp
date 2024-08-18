#include "DataStorage.h"
#include <nlohmann/json.hpp>
#include <vector>
#include <iostream>
#include <cstring>   // for memcpy
#include <fstream>   // for file I/O
#include <iomanip>

// TODO - Can you avoid using namespaces ?
// using json = nlohmann::json;

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
        // Construct the file name based on the index
        std::ostringstream oss;
        oss << "record_mode/output_" << index << ".json";
        std::string filename = oss.str();

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
        // data.pim_execution_time_in_ns = j["pim_execution_time_in_ns"].get<double>(); TODO
        // data.output_matrix_size = j["output_matrix_size"].get<uint32_t>();

        // Copy the output_result_matrix array from JSON
        auto output_result_matrix = j["output_matrix"].get<std::vector<float>>();
        data.output_matrix_size = output_result_matrix.size();
        std::memcpy(data.output_result_matrix, output_result_matrix.data(), output_result_matrix.size() * sizeof(float));

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

// ********************* AK - Changes end here *************************
