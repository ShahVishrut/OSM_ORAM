#include "client/client.h" // Assuming this is your header
#include <iostream>
#include <vector>
#include <string>
#include <iomanip>

// Helper to easily print vectors returned by find()
void print_vector(const std::string& label, const std::vector<uint64_t>& vec) {
    std::cout << label << ": [";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << (i + 1 == vec.size() ? "" : ", ");
    }
    std::cout << "]\n";
}

void print_header(const std::string& text) {
    std::cout << "\n==================================================\n";
    std::cout << " " << text << "\n";
    std::cout << "==================================================\n";
}

int main() {
    // Increase capacity slightly to handle the stress tests
    Client *my_client = new Client(500, 4); 
    std::cout << std::boolalpha; // Print true/false instead of 1/0

    print_header("SUITE 1: Basic Insertions & Duplicates");
    my_client->insert(50, 66);
    my_client->insert(10, 78);
    my_client->insert(30, 88);
    my_client->insert(75, 987);
    my_client->insert(40, 984);
    my_client->insert(45, 982);
    my_client->insert(5, 981);
    
    // Duplicate islands
    my_client->insert(50, 67);
    my_client->insert(50, 68);
    my_client->insert(50, 69);
    my_client->insert(50, 70);
    my_client->insert(10, 79);
    my_client->insert(10, 80);

    std::cout << "Size of 10 (expected 3): " << my_client->size(10) << "\n";
    std::cout << "Size of 50 (expected 5): " << my_client->size(50) << "\n";
    print_vector("Find key 50, range [1, 3] (expected [67, 68, 69])", my_client->find(50, 1, 3));


    print_header("SUITE 2: Forced AVL Rotations (Insertions)");
    // 1. Left-Left (LL) Rotation: Insert decreasing order
    my_client->insert(300, 1);
    my_client->insert(200, 1);
    my_client->insert(100, 1);
    std::cout << "LL Rotation Test (Size 100 expected 1): " << my_client->size(100) << "\n";

    // 2. Right-Right (RR) Rotation: Insert increasing order
    my_client->insert(400, 1);
    my_client->insert(500, 1);
    my_client->insert(600, 1);
    std::cout << "RR Rotation Test (Size 600 expected 1): " << my_client->size(600) << "\n";

    // 3. Left-Right (LR) Rotation
    my_client->insert(700, 1);
    my_client->insert(650, 1);
    my_client->insert(675, 1);
    std::cout << "LR Rotation Test (Size 675 expected 1): " << my_client->size(675) << "\n";

    // 4. Right-Left (RL) Rotation
    my_client->insert(800, 1);
    my_client->insert(900, 1);
    my_client->insert(850, 1);
    std::cout << "RL Rotation Test (Size 850 expected 1): " << my_client->size(850) << "\n";


    print_header("SUITE 3: Basic & Duplicate Removals");
    std::cout << "Remove(99, 100) non-existent (expected false): " << my_client->remove(99, 100) << "\n";
    
    std::cout << "Remove(5, 981) unique leaf  (expected true):  " << my_client->remove(5, 981) << "\n";
    std::cout << " -> Size of 5   (expected 0):     " << my_client->size(5) << "\n";

    std::cout << "Remove(50, 68) middle dup   (expected true):  " << my_client->remove(50, 68) << "\n";
    std::cout << " -> Size of 50  (expected 4):     " << my_client->size(50) << "\n";
    print_vector(" -> Find key 50, range [0, 3] (expected [66, 67, 69, 70])", my_client->find(50, 0, 3));


    print_header("SUITE 4: Total Duplicate Wipeout");
    // Ensure that completely deleting an island doesn't break augmented stats for parents
    my_client->insert(55, 1);
    my_client->insert(55, 2);
    my_client->insert(55, 3);
    std::cout << "Initial Size of 55 (expected 3): " << my_client->size(55) << "\n";
    std::cout << "Remove(55, 1) (expected true): " << my_client->remove(55, 1) << "\n";
    std::cout << "Remove(55, 2) (expected true): " << my_client->remove(55, 2) << "\n";
    std::cout << "Remove(55, 3) (expected true): " << my_client->remove(55, 3) << "\n";
    std::cout << "Final Size of 55 (expected 0): " << my_client->size(55) << "\n";


    print_header("SUITE 5: Complex 2-Child Node Removal");
    // Build a specific subtree to force a complex in-order successor swap
    my_client->insert(2000, 1);
    my_client->insert(1500, 1);
    my_client->insert(2500, 1);
    my_client->insert(2200, 1);
    my_client->insert(2700, 1);
    my_client->insert(2300, 1); // 2300 is the right child of the successor (2200)

    std::cout << "Size of 2000 before removal (expected 1): " << my_client->size(2000) << "\n";
    std::cout << "Size of 2200 before removal (expected 1): " << my_client->size(2200) << "\n";
    
    // Removing 2000 forces the tree to swap 2200 into 2000's spot, and shift 2300 up.
    std::cout << "Remove(2000, 1) (expected true): " << my_client->remove(2000, 1) << "\n";
    
    std::cout << "Size of 2000 after removal (expected 0): " << my_client->size(2000) << "\n";
    std::cout << "Size of 2200 after removal (expected 1): " << my_client->size(2200) << "\n";
    std::cout << "Size of 2300 after removal (expected 1): " << my_client->size(2300) << "\n";


    print_header("SUITE 6: Sequential Stress Test");
    // Forcing continuous Right-Right insertions and cascading rebalances
    std::cout << "Inserting keys 3000 to 3020...\n";
    for (int i = 0; i <= 20; i++) {
        my_client->insert(3000 + i, i);
    }
    
    bool stress_insert_success = true;
    for (int i = 0; i <= 20; i++) {
        if (my_client->size(3000 + i) != 1) stress_insert_success = false;
    }
    std::cout << "All stress inserts found? (expected true): " << stress_insert_success << "\n";

    // Forcing continuous removals and backward rebalances
    std::cout << "Removing keys 3000 to 3020...\n";
    bool stress_remove_success = true;
    for (int i = 0; i <= 20; i++) {
        if (!my_client->remove(3000 + i, i)) stress_remove_success = false;
    }
    std::cout << "All stress removals returned true? (expected true): " << stress_remove_success << "\n";
    
    bool stress_cleanup_success = true;
    for (int i = 0; i <= 20; i++) {
        if (my_client->size(3000 + i) != 0) stress_cleanup_success = false;
    }
    std::cout << "All stress keys completely gone? (expected true): " << stress_cleanup_success << "\n";

    print_header("TESTS COMPLETE");

    delete my_client;
    return 0;
}