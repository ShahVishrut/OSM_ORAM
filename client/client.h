#ifndef CLIENT_H
#define CLIENT_H

#include "../server/server.h"
#include <vector>
#include <cstdint>

// --------------------------------------------------------------------------
// ODSPointer: Oblivious Data Structure Pointer
// Replaces standard memory pointers to obscure access patterns.
// Contains the block identifier and the random leaf label specifying
// the path in the ORAM tree where the block currently resides.
// --------------------------------------------------------------------------
struct ODSPointer {
    uint32_t block_id; 
    uint32_t leaf_label;
    bool is_null;

    ODSPointer() : block_id(0), leaf_label(0), is_null(true) {}
    ODSPointer(uint32_t b, uint32_t l) : block_id(b), leaf_label(l), is_null(false) {}
};

// --------------------------------------------------------------------------
// OSMNode: Oblivious Sorted Multimap Node
// Extends an AVL search tree to store multiple values for the same key.
// Includes order-statistic style metadata for efficient indexed retrieval.
// --------------------------------------------------------------------------
struct OSMNode {
    // --- Core Key-Value Data ---
    uint64_t key;
    uint64_t value;

    // --- Child Pointers ---
    ODSPointer l_child_ptr;
    ODSPointer r_child_ptr;

    // --- AVL Heights & Duplicate Key Counts ---
    uint8_t l_height;
    uint8_t r_height;
    uint32_t l_same_key_size;
    uint32_t r_same_key_size;

    // --- Left Subtree Augmented Statistics ---
    uint64_t l_max_key_subtree;
    uint32_t l_max_key_count;

    uint64_t l_min_key_subtree;
    uint32_t l_min_key_count;

    // --- Right Subtree Augmented Statistics ---
    uint64_t r_max_key_subtree;
    uint32_t r_max_key_count;

    uint64_t r_min_key_subtree;
    uint32_t r_min_key_count;

    OSMNode()
        : key(0), value(0),
          l_height(0), r_height(0),
          l_same_key_size(0), r_same_key_size(0),
          l_max_key_subtree(0), l_max_key_count(0),
          l_min_key_subtree(0), l_min_key_count(0),
          r_max_key_subtree(0), r_max_key_count(0),
          r_min_key_subtree(0), r_min_key_count(0) {}
};

// --------------------------------------------------------------------------
// ORAMBlock: Fundamental storage unit in the Path ORAM tree.
// Combines the logical ODS pointer header with the tree node payload.
// --------------------------------------------------------------------------
struct ORAMBlock {
    ODSPointer header;
    OSMNode data;
};

// --------------------------------------------------------------------------
// Client: Oblivious Data Structure Client
// Manages the local stash, block assignments, and the oblivious
// data structure logic to hide memory access patterns.
// --------------------------------------------------------------------------
class Client {
public:
    // --- Initialization ---
    Client(size_t num_blocks, size_t blocks_per_bucket);

    // --- Oblivious Sorted Multimap (OSM) API ---
    void insert(uint64_t key, uint64_t value);
    size_t size(uint64_t key);
    std::vector<uint64_t> find(uint64_t key, uint32_t i, uint32_t j);
    bool remove(uint64_t key, uint64_t value);

    // Expose server metrics to the testing driver
    size_t get_server_accesses() const {
        return server->get_total_accesses(); // Adjust 'server' to match your member variable name
    }

private:
    // --- Core Path ORAM Operations ---
    ORAMBlock write_block(ORAMBlock to_write, bool write);
    void read_block_path(uint32_t leaf_num);
    void evict(uint32_t leaf_num);
    bool is_on_path(size_t index, size_t leaf_num);

    // --- AVL Rebalancing Helpers ---
    void rotate_right_right(std::vector<ORAMBlock>& avl_history, int cur_node_index);
    void rotate_left_left(std::vector<ORAMBlock>& avl_history, int cur_node_index);
    void rotate_right_left(std::vector<ORAMBlock>& avl_history, int cur_node_index);
    void rotate_left_right(std::vector<ORAMBlock>& avl_history, int cur_node_index);

    // --- ORAM Configuration ---
    size_t block_size_bytes = sizeof(ORAMBlock);
    size_t num_blocks;
    size_t blocks_per_bucket;
    size_t bucket_size_bytes;
    size_t tree_height;

    Server* server;

    // --- Client State & Local Stash ---
    ODSPointer root;
    std::vector<ORAMBlock> stash;
    std::vector<bool> stash_full;

    // --- Block ID Management ---
    uint32_t cur_block_id = 0;
    uint32_t next_available_block_id();

    // --- Padding ---
    size_t tree_size = 0;              // Tracks the total number of nodes in your AVL tree
    size_t current_op_accesses = 0;    // Tracks Path ORAM accesses during the current operation
    static const uint32_t DUMMY_BLOCK_ID = 0xFFFFFFFF; 
    void perform_dummy_access();
    size_t get_max_tree_height();
    void pad_operation(size_t target_accesses);
};

#endif