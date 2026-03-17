#include "client/client.h" 
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>

// ===================================================================
// UTILITY FUNCTIONS
// ===================================================================

void print_header(const std::string& text) {
    std::cout << "\n==================================================\n";
    std::cout << " " << text << "\n";
    std::cout << "==================================================\n";
}

template <typename T>
void print_vector(const std::vector<T>& vec) {
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i + 1 == vec.size() ? "" : ", ");
    }
    std::cout << "]";
}

template <typename T>
void assert_equal(const T& actual, const T& expected, const std::string& test_name) {
    if (actual == expected) {
        std::cout << "[PASS] " << test_name << "\n";
    } else {
        std::cerr << "[FAIL] " << test_name << "\n"
                  << "       -> Expected: " << expected << "\n"
                  << "       -> Got:      " << actual << "\n";
    }
}

template <typename T>
void assert_vector_equal(const std::vector<T>& actual, const std::vector<T>& expected, const std::string& test_name) {
    bool match = (actual.size() == expected.size());
    if (match) {
        for (size_t i = 0; i < actual.size(); ++i) {
            if (actual[i] != expected[i]) {
                match = false;
                break;
            }
        }
    }
    
    if (match) {
        std::cout << "[PASS] " << test_name << "\n";
    } else {
        std::cerr << "[FAIL] " << test_name << "\n";
        std::cerr << "       -> Expected: "; print_vector(expected); std::cerr << "\n";
        std::cerr << "       -> Got:      "; print_vector(actual); std::cerr << "\n";
    }
}

// Assertion helper for Obliviousness (Ensures two operations take the exact same accesses)
void assert_oblivious(size_t acc1, size_t acc2, const std::string& test_name) {
    if (acc1 == acc2) {
        std::cout << "[PASS] " << test_name << "\n       -> Both operations took exactly " << acc1 << " accesses.\n";
    } else {
        std::cerr << "[FAIL] " << test_name << "\n"
                  << "       -> SECURITY LEAK DETECTED!\n"
                  << "       -> Op 1 took: " << acc1 << " accesses\n"
                  << "       -> Op 2 took: " << acc2 << " accesses\n";
    }
}

// Helper to print basic access stats cleanly for small tests
void print_accesses(size_t accesses) {
    std::cout << "       -> Server Accesses (Buckets): " << accesses << "\n";
}

// Helper to print detailed performance statistics for benchmarks
void print_perf_stats(const std::string& op_name, int num_ops, size_t access_delta, double time_ms) {
    std::cout << "  [" << op_name << " Stats]\n"
              << "   - Total Time:      " << std::fixed << std::setprecision(2) << time_ms << " ms\n"
              << "   - Time per Op:     " << std::fixed << std::setprecision(4) << (time_ms / num_ops) << " ms\n"
              << "   - Total Accesses:  " << access_delta << " buckets\n"
              << "   - Accesses per Op: " << std::fixed << std::setprecision(2) << (access_delta / (double)num_ops) << " buckets\n";
}

int main() {
    print_header("Starting OSM Correctness & Scaling Tests");

    // Increased capacity to support up to 8192 elements safely
    uint64_t max_capacity = 8192; 
    Client *client = new Client(max_capacity, 4); 
    size_t before_acc, after_acc;

    // ===================================================================
    // TEST SUITE 1: Insert and Size 
    // ===================================================================
    print_header("Test Suite 1: Insert and Size");
    
    uint64_t KEY_A = 10;
    assert_equal(client->size(KEY_A), (size_t)0, "Initial size of a new key");
    
    before_acc = client->get_server_accesses();
    client->insert(KEY_A, 100);
    client->insert(KEY_A, 200);
    after_acc = client->get_server_accesses();
    
    assert_equal(client->size(KEY_A), (size_t)2, "Size updates correctly after insertions");
    print_accesses(after_acc - before_acc);

    // ===================================================================
    // TEST SUITE 2: Find and Sorted Order (0-Indexed)
    // ===================================================================
    print_header("Test Suite 2: Find & Sorted Order");
    
    uint64_t KEY_B = 20;
    
    client->insert(KEY_B, 50);
    client->insert(KEY_B, 10);
    client->insert(KEY_B, 40);
    client->insert(KEY_B, 20);
    client->insert(KEY_B, 30);
    
    assert_equal(client->size(KEY_B), (size_t)5, "Size is 5 after inserting 5 elements");

    before_acc = client->get_server_accesses();
    std::vector<uint64_t> res_0_to_2 = client->find(KEY_B, 0, 2);
    after_acc = client->get_server_accesses();
    assert_vector_equal(res_0_to_2, {10, 20, 30}, "Find [0, 2] returns the first 3 elements sorted");
    print_accesses(after_acc - before_acc);

    std::vector<uint64_t> res_1_to_3 = client->find(KEY_B, 1, 3);
    assert_vector_equal(res_1_to_3, {20, 30, 40}, "Find [1, 3] returns the middle 3 elements sorted");

    std::vector<uint64_t> res_all = client->find(KEY_B, 0, 4);
    assert_vector_equal(res_all, {10, 20, 30, 40, 50}, "Find [0, 4] returns all 5 elements correctly sorted");

    // ===================================================================
    // TEST SUITE 3: Remove Correctness
    // ===================================================================
    print_header("Test Suite 3: Remove Logic");

    before_acc = client->get_server_accesses();
    bool rm_success = client->remove(KEY_B, 30);
    after_acc = client->get_server_accesses();
    
    assert_equal(rm_success, true, "Removing an existing element returns true");
    assert_equal(client->size(KEY_B), (size_t)4, "Size decreases by 1 after successful removal");
    print_accesses(after_acc - before_acc);

    std::vector<uint64_t> res_after_rm = client->find(KEY_B, 0, 3);
    assert_vector_equal(res_after_rm, {10, 20, 40, 50}, "Find correctly skips the removed element and maintains order");

    bool rm_fail = client->remove(KEY_B, 999);
    assert_equal(rm_fail, false, "Removing a non-existent element returns false");
    assert_equal(client->size(KEY_B), (size_t)4, "Size remains unchanged after a failed remove attempt");

    // ===================================================================
    // TEST SUITE 4: Multi-Key Isolation
    // ===================================================================
    print_header("Test Suite 4: Multi-Key Isolation");

    uint64_t KEY_C = 30;
    client->insert(KEY_C, 1000);
    client->insert(KEY_C, 2000);

    assert_equal(client->size(KEY_C), (size_t)2, "Key C size is correct");
    assert_equal(client->size(KEY_A), (size_t)2, "Key A size remains unaffected by other keys");
    assert_equal(client->size(KEY_B), (size_t)4, "Key B size remains unaffected by other keys");

    // ===================================================================
    // TEST SUITE 5: Scale & Performance Benchmarks
    // ===================================================================
    print_header("Test Suite 5: Performance Benchmarks (Up to 5000 elements)");

    uint64_t PERF_KEY = 999;
    std::vector<int> checkpoints = {1000, 3000, 5000};
    int current_size = 0;

    for (int target_size : checkpoints) {
        std::cout << "\n--- Benchmarking at " << target_size << " Elements ---\n";
        
        int elements_to_add = target_size - current_size;
        
        // 1. Benchmark Inserts
        size_t acc_before_insert = client->get_server_accesses();
        auto start_time = std::chrono::high_resolution_clock::now();
        
        std::cout << "  [INFO] Starting " << elements_to_add << " inserts...\n";
        for (int i = 0; i < elements_to_add; ++i) {
            // Inserting reverse-sorted data to force tree rebalancing
            client->insert(PERF_KEY, 10000 - (current_size + i)); 
            
            // Print progress every 500 inserts
            if ((i + 1) % 500 == 0) {
                std::cout << "      ... inserted " << (i + 1) << " / " << elements_to_add << " elements.\n";
            }
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        size_t acc_after_insert = client->get_server_accesses();
        
        double time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        print_perf_stats("Bulk Insert (" + std::to_string(elements_to_add) + " items)", elements_to_add, acc_after_insert - acc_before_insert, time_ms);
        
        current_size = target_size;
        assert_equal(client->size(PERF_KEY), (size_t)current_size, "Verify current size matches target");

        // 2. Benchmark Single Element Find (Point Query)
        size_t acc_before_find = client->get_server_accesses();
        start_time = std::chrono::high_resolution_clock::now();
        
        // Find the middle element
        std::vector<uint64_t> point_res = client->find(PERF_KEY, target_size / 2, target_size / 2);
        
        end_time = std::chrono::high_resolution_clock::now();
        size_t acc_after_find = client->get_server_accesses();
        time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        
        print_perf_stats("Single Find", 1, acc_after_find - acc_before_find, time_ms);

        // 3. Benchmark Range Find (100 elements)
        int range_size = 100;
        size_t acc_before_range = client->get_server_accesses();
        start_time = std::chrono::high_resolution_clock::now();
        
        std::vector<uint64_t> range_res = client->find(PERF_KEY, 0, range_size - 1);
        
        end_time = std::chrono::high_resolution_clock::now();
        size_t acc_after_range = client->get_server_accesses();
        time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        
        print_perf_stats("Range Find (100 items)", 1, acc_after_range - acc_before_range, time_ms);
    }

    // ===================================================================
    // TEST SUITE 6: Bulk Deletion Stress Test
    // ===================================================================
    print_header("Test Suite 6: Bulk Deletion Stress Test");
    
    size_t acc_before_del = client->get_server_accesses();
    auto start_time_del = std::chrono::high_resolution_clock::now();
    
    // Remove the first 1000 elements
    int delete_count = 1000;
    std::cout << "  [INFO] Starting " << delete_count << " sequential deletions...\n";
    
    for (int i = 0; i < delete_count; ++i) {
        // Values were inserted as 10000 - index
        client->remove(PERF_KEY, 10000 - i); 
        
        // Print progress every 100 deletions
        if ((i + 1) % 100 == 0) {
            std::cout << "      ... deleted " << (i + 1) << " / " << delete_count << " elements.\n";
        }
    }
    
    auto end_time_del = std::chrono::high_resolution_clock::now();
    size_t acc_after_del = client->get_server_accesses();
    double time_ms_del = std::chrono::duration<double, std::milli>(end_time_del - start_time_del).count();
    
    print_perf_stats("Bulk Remove (1000 items)", delete_count, acc_after_del - acc_before_del, time_ms_del);
    assert_equal(client->size(PERF_KEY), (size_t)4000, "Size is exactly 4000 after 1000 removals");

    // ===================================================================
    // TEST SUITE 7: Pairwise Obliviousness Security Tests
    // ===================================================================
    print_header("Test Suite 7: Pairwise Obliviousness Security Tests");

    uint64_t OBLIV_KEY = 555;
    size_t acc1, acc2;

    // Pre-fill the tree so the max_tree_height is stable (doesn't jump mid-test)
    std::cout << "  [INFO] Pre-filling tree with 100 elements to stabilize height...\n";
    for (int i = 0; i < 100; ++i) {
        client->insert(OBLIV_KEY, i);
    }

    // --- PAIR 1: Insert vs Insert ---
    // Two inserts at the same tree height threshold should be identical.
    before_acc = client->get_server_accesses();
    client->insert(OBLIV_KEY, 200);
    acc1 = client->get_server_accesses() - before_acc;

    before_acc = client->get_server_accesses();
    client->insert(OBLIV_KEY, 201);
    acc2 = client->get_server_accesses() - before_acc;

    assert_oblivious(acc1, acc2, "Insert vs Insert (Stable Height)");

    // --- PAIR 2: Find Hit vs Find Miss (Point Query) ---
    // Finding an element right at the root vs finding an element that doesn't exist.
    before_acc = client->get_server_accesses();
    client->find(OBLIV_KEY, 50, 50); // Hit (Valid index)
    acc1 = client->get_server_accesses() - before_acc;

    before_acc = client->get_server_accesses();
    client->find(OBLIV_KEY, 999, 999); // Miss (Out of bounds index)
    acc2 = client->get_server_accesses() - before_acc;

    assert_oblivious(acc1, acc2, "Find (Hit) vs Find (Miss) [Same Range Size]");

    // --- PAIR 3: Remove Hit vs Remove Miss ---
    // Removing a node vs attempting to remove a node that isn't there.
    before_acc = client->get_server_accesses();
    client->remove(OBLIV_KEY, 99); // Hit
    acc1 = client->get_server_accesses() - before_acc;

    before_acc = client->get_server_accesses();
    client->remove(OBLIV_KEY, 999); // Miss
    acc2 = client->get_server_accesses() - before_acc;

    assert_oblivious(acc1, acc2, "Remove (Hit) vs Remove (Miss)");

    // --- PAIR 4: Remove Internal Node vs Remove Leaf Node ---
    // Internal nodes require the complex successor swap logic. Leaves do not.
    // To an outside observer, both should look exactly the same.
    before_acc = client->get_server_accesses();
    client->remove(OBLIV_KEY, 50); // Likely an internal node (inserted early)
    acc1 = client->get_server_accesses() - before_acc;

    before_acc = client->get_server_accesses();
    client->remove(OBLIV_KEY, 201); // Definitely a leaf (inserted last)
    acc2 = client->get_server_accesses() - before_acc;

    assert_oblivious(acc1, acc2, "Remove (Internal Node) vs Remove (Leaf Node)");

    print_header("ALL TESTS COMPLETE");

    delete client;
    return 0;
}