#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <cstdint>

// --------------------------------------------------------------------------
// Server: Untrusted ORAM Storage Provider
// Holds the external memory arranged logically as a binary tree of buckets.
// The server is assumed to be untrusted and stores encrypted data only.
// --------------------------------------------------------------------------
class Server {
public:
    // --- Initialization ---
    // Initializes the server storage using tree configuration parameters.
    Server(size_t bucket_size_bytes, size_t num_buckets);

    // --- Path ORAM I/O Operations ---
    // Writes a sequence of buckets (typically a root-to-leaf path).
    bool write_buckets(std::vector<size_t> index_list,
                       std::vector<uint8_t> data);

    // Reads a sequence of buckets (typically a root-to-leaf path).
    std::vector<uint8_t> read_buckets(std::vector<size_t> index_list);

    // --- Performance Tracking ---
    size_t get_total_accesses() const { return read_count + write_count; }
    void reset_counters() { read_count = 0; write_count = 0; }

private:
    // --- Physical Storage ---
    // Raw byte array representing the ORAM tree storage.
    std::vector<uint8_t> database;

    // --- Tree Configuration ---
    size_t bucket_size_bytes;
    size_t num_buckets;

    // --- Metrics ---
    size_t read_count = 0;
    size_t write_count = 0;
};

#endif