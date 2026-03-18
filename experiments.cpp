#include "client/client.h" // Assuming your OSM client header
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>

void print_header(const std::string& text) {
    std::cout << "\n==================================================\n";
    std::cout << " " << text << "\n";
    std::cout << "==================================================\n";
}

int main() {
    std::cout << "Starting OSM (Oblivious Sorted Multimap) Experiments...\n";

    // We will use a single target key to host the values we are querying, 
    // simulating a keyword search that returns multiple ranked documents.
    const uint64_t TARGET_KEY = 42; 

    // ===================================================================
    // EXPERIMENT 1: Fixed Dataset Size (2^20), Varying Range Query Size
    // ===================================================================
    print_header("EXPERIMENT 1: Varying Range Size (n = 2^20)");
    
    uint64_t n_exp1 = 1 << 20; // 1,048,576
    
    // Initialize client. (Adjust to match your OSM initialization)
    Client *client_exp1 = new Client(n_exp1, 4); 

    std::cout << "Inserting " << n_exp1 << " elements into OSM...\n";
    for (uint64_t i = 1; i <= n_exp1; ++i) {
        // Insert(k, v) adds v to the list Map[k] keeping its values sorted.
        // We map all values to TARGET_KEY to build a massive list.
        client_exp1->insert(TARGET_KEY, i); 
        
        // Update the same line every 100 insertions
        if (i % 100 == 0) {
            std::cout << "\r  -> Inserted " << i << " records..." << std::flush;
        }
    }
    // Print a final newline so the next text doesn't overwrite your final count
    std::cout << std::endl;

    std::cout << "\nExecuting varying range queries on TARGET_KEY...\n";
    std::cout << "Index Range [i, j]\tTime (ms)\tResults Fetched\n";
    std::cout << "--------------------------------------------------\n";

    // Test range sizes: 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024
    for (uint64_t range_size = 1; range_size <= 1024; range_size *= 2) {
        uint64_t start_index = 1; 
        uint64_t end_index = range_size;

        auto start_time = std::chrono::high_resolution_clock::now();
        
        // OSM.Find(k, i, j) retrieves the i-th through j-th values for key k.
        std::vector<uint64_t> results = client_exp1->find(TARGET_KEY, start_index, end_index);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        std::cout << "[" << start_index << ", " << std::setw(4) << end_index << "]\t\t" 
                  << duration.count() << " ms\t\t" 
                  << results.size() << "\n";
    }
    
    delete client_exp1;


    // ===================================================================
    // EXPERIMENT 2: Varying Dataset Size, Fixed Range Query Size
    // ===================================================================
    print_header("EXPERIMENT 2: Varying Input Size (Fixed Interval = [1, 64])");

    uint64_t fixed_start = 1;
    uint64_t fixed_end = 64;
    
    std::cout << "Dataset (n)\tTime (ms)\tResults Fetched\n";
    std::cout << "--------------------------------------------------\n";

    // Test dataset sizes: 2^10, 2^12, 2^14, 2^16, 2^18, 2^20
    for (int power = 10; power <= 20; power += 2) {
        uint64_t current_n = 1 << power;
        
        Client *client_exp2 = new Client(current_n, 4); 

        // Insert current_n elements under TARGET_KEY
        for (uint64_t i = 1; i <= current_n; ++i) {
            client_exp2->insert(TARGET_KEY, i);
        }

        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Execute the fixed-size query: Find(TARGET_KEY, 1, 64)
        std::vector<uint64_t> results = client_exp2->find(TARGET_KEY, fixed_start, fixed_end);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        std::cout << "2^" << power << " (" << current_n << ")\t" 
                  << duration.count() << " ms\t\t" 
                  << results.size() << "\n";

        delete client_exp2;
    }

    print_header("EXPERIMENTS COMPLETE");
    return 0;
}