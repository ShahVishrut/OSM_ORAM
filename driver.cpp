#include "client/client.h"
#include <iostream>
#include <vector>
#include <string>

// Helper to easily print vectors returned by find()
void print_vector(const std::string& label, const std::vector<uint64_t>& vec) {
    std::cout << label << ": [";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i + 1 == vec.size() ? "" : ", ");
    }
    std::cout << "]\n";
}

int main() {
    Client *my_client = new Client(200, 4);
    
    // 1. Initial Inserts
    my_client->insert(50, 66);
    my_client->insert(10, 78);
    my_client->insert(30, 88);
    my_client->insert(75, 987);
    my_client->insert(40, 984);
    my_client->insert(45, 982);
    my_client->insert(5, 981);
    
    // 2. Insert Duplicates to test the BFS 'island' logic
    // Key 50 will now have 5 total items
    my_client->insert(50, 67);
    my_client->insert(50, 68);
    my_client->insert(50, 69);
    my_client->insert(50, 70);

    // Key 10 will now have 3 total items
    my_client->insert(10, 79);
    my_client->insert(10, 80);

    std::cout << "--- Testing Size ---\n";
    std::cout << "Size of 10 (expected 3): " << my_client->size(10) << "\n";
    std::cout << "Size of 50 (expected 5): " << my_client->size(50) << "\n";
    std::cout << "Size of 40 (expected 1): " << my_client->size(40) << "\n";
    std::cout << "Size of 99 (expected 0): " << my_client->size(99) << "\n";
    
    std::cout << "\n--- Testing Find ---\n";
    
    // Find all 10s (indices 0 to 2)
    print_vector("Find key 10, range [0, 2]", my_client->find(10, 0, 2));

    // Find all 50s (indices 0 to 4)
    print_vector("Find key 50, range [0, 4]", my_client->find(50, 0, 4));

    // Find a subset of 50s (indices 1 to 3)
    print_vector("Find key 50, range [1, 3]", my_client->find(50, 1, 3));

    // Find a single 50 (index 2)
    print_vector("Find key 50, range [2, 2]", my_client->find(50, 2, 2));

    // Find a non-existent key
    print_vector("Find key 99, range [0, 1]", my_client->find(99, 0, 1));

    delete my_client;
    return 0;
}