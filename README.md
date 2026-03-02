## Repository Structure

### client/

This directory contains the logic for the trusted client, which maintains local state and executes the oblivious algorithms.

**client.h**
Defines the core data structures and the `Client` class, including `ORAMBlock`, `OSMNode`, and `ODSPointer`.

**path_oram.cpp**
Implements the core Path ORAM operations, including reading full root-to-leaf paths into the client stash and evicting blocks back down the tree to the server.

**osm_functions.cpp**
Contains the main logic for the Oblivious Sorted Multimap (OSM). This includes secure insert, size, and range-based find operations over data outsourced to the untrusted server.

**avl_balancing.cpp**
Implements AVL tree rotations and updates augmented order-statistic metadata, ensuring that OSM operations maintain logarithmic worst-case time complexity.

---

### server/

This directory contains the logic for the untrusted external storage provider, which stores the encrypted ORAM tree.

**server.h**
Defines the `Server` class and its storage interface for reading and writing ORAM buckets.

**server.cpp**
Implements the low-level APIs used to read and write buckets from the flat byte array representing the ORAM tree.

---

### Root Directory

**driver.cpp**
The main entry point. It initializes the `Client` and `Server` instances and runs application logic or performance tests.