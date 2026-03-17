#include "server.h"
#include <cstring>

// --------------------------------------------------------------------------
// Server Initialization
// Allocates the flat byte array representing the ORAM tree storage.
// --------------------------------------------------------------------------
Server::Server(size_t bucket_size_bytes, size_t num_buckets) {
    this->bucket_size_bytes = bucket_size_bytes;
    this->num_buckets = num_buckets;
    this->database.resize(bucket_size_bytes * num_buckets);
    this->read_count = 0;
    this->write_count = 0;
}

// --------------------------------------------------------------------------
// Write Buckets (Eviction)
// Writes a sequence of buckets (typically a root-to-leaf path)
// back into the server's storage.
// --------------------------------------------------------------------------
bool Server::write_buckets(std::vector<size_t> index_list,
                           std::vector<uint8_t> data) {
    // Validate total data size
    if (bucket_size_bytes * index_list.size() != data.size()) {
        return false;
    }

    // Validate bucket indices
    for (size_t idx : index_list) {
        if (idx >= num_buckets) {
            return false;
        }
    }

    // TRACKING: Increment write count by the number of buckets written
    this->write_count += index_list.size();

    // Copy data into database
    for (size_t i = 0; i < index_list.size(); i++) {
        size_t db_offset = index_list[i] * bucket_size_bytes;
        size_t data_offset = i * bucket_size_bytes;

        std::memcpy(&database[db_offset],
                    &data[data_offset],
                    bucket_size_bytes);
    }

    return true;
}

// --------------------------------------------------------------------------
// Read Buckets (Path Fetch)
// Reads a sequence of buckets (typically a root-to-leaf path)
// from server storage.
// --------------------------------------------------------------------------
std::vector<uint8_t> Server::read_buckets(std::vector<size_t> index_list) {
    // Validate bucket indices
    for (size_t idx : index_list) {
        if (idx >= num_buckets) {
            return {};
        }
    }

    // TRACKING: Increment read count by the number of buckets read
    this->read_count += index_list.size();

    std::vector<uint8_t> data(index_list.size() * bucket_size_bytes);

    // Copy data out of database
    for (size_t i = 0; i < index_list.size(); i++) {
        size_t db_offset = index_list[i] * bucket_size_bytes;
        size_t data_offset = i * bucket_size_bytes;

        std::memcpy(&data[data_offset],
                    &database[db_offset],
                    bucket_size_bytes);
    }

    return data;
}