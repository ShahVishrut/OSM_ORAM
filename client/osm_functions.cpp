#include "client.h"

#include <cstdlib>

size_t Client::size(uint64_t key) {
  if (root.is_null) {
    return 0;
  }

  std::vector<ORAMBlock> history;
  ORAMBlock cur_read;
  cur_read.header = root;
  size_t result = 0;

  // --------------------------------------------------------------------------
  // Phase 1: Traverse the tree to find the target key
  // --------------------------------------------------------------------------
  while (true) {
    cur_read = write_block(cur_read, false);
    history.push_back(cur_read);

    if (key > cur_read.data.key) {
      if (cur_read.data.r_child_ptr.is_null) break;
      cur_read.header = cur_read.data.r_child_ptr;

    } else if (key < cur_read.data.key) {
      if (cur_read.data.l_child_ptr.is_null) break;
      cur_read.header = cur_read.data.l_child_ptr;

    } else {
      // Key found: result is 1 (the node itself) + left matches + right matches
      result = 1 + cur_read.data.l_same_key_size + cur_read.data.r_same_key_size;
      break;
    }
  }

  // --------------------------------------------------------------------------
  // Phase 2: PathORAM Write-back
  // Update parent pointers with new leaf_labels from bottom to top
  // --------------------------------------------------------------------------
  for (int index = (int)history.size() - 1; index >= 0; index--) {
    uint32_t new_leaf_label = write_block(history[index], true).header.leaf_label;

    if (index > 0) {
      if (!history[index - 1].data.l_child_ptr.is_null &&
          history[index - 1].data.l_child_ptr.block_id == history[index].header.block_id) {
        history[index - 1].data.l_child_ptr.leaf_label = new_leaf_label;

      } else if (!history[index - 1].data.r_child_ptr.is_null &&
                 history[index - 1].data.r_child_ptr.block_id == history[index].header.block_id) {
        history[index - 1].data.r_child_ptr.leaf_label = new_leaf_label;
      }

    } else {
      root.leaf_label = new_leaf_label;
    }
  }

  return result;
}

std::vector<uint64_t> Client::find(uint64_t key, uint32_t i, uint32_t j) {
  if (root.is_null) return {};

  std::vector<ORAMBlock> common_path;
  ORAMBlock cur_read;
  cur_read.header = root;

  uint32_t recursive_i = i;
  uint32_t recursive_j = j;

  bool found_i = false;
  bool found_j = false;
  bool abort_search = false;

  ODSPointer next_header_i;
  ODSPointer next_header_j;

  // --------------------------------------------------------------------------
  // Phase 1: Traverse together until i and j paths diverge
  // --------------------------------------------------------------------------
  while (true) {
    cur_read = write_block(cur_read, false);
    common_path.push_back(cur_read);

    uint32_t next_recursive_i = recursive_i;
    uint32_t next_recursive_j = recursive_j;

    // --- Determine next step for i ---
    if (key > cur_read.data.key) {
      if (cur_read.data.r_child_ptr.is_null) { abort_search = true; break; }
      next_header_i = cur_read.data.r_child_ptr;

    } else if (key < cur_read.data.key) {
      if (cur_read.data.l_child_ptr.is_null) { abort_search = true; break; }
      next_header_i = cur_read.data.l_child_ptr;

    } else {
      if (cur_read.data.l_same_key_size == recursive_i) {
        found_i = true;

      } else if (cur_read.data.l_same_key_size > recursive_i) {
        if (cur_read.data.l_child_ptr.is_null) { abort_search = true; break; }
        next_header_i = cur_read.data.l_child_ptr;

      } else {
        if (cur_read.data.r_child_ptr.is_null) { abort_search = true; break; }
        next_header_i = cur_read.data.r_child_ptr;
        next_recursive_i = recursive_i - (cur_read.data.l_same_key_size + 1);
      }
    }

    // --- Determine next step for j ---
    if (key > cur_read.data.key) {
      if (cur_read.data.r_child_ptr.is_null) { abort_search = true; break; }
      next_header_j = cur_read.data.r_child_ptr;

    } else if (key < cur_read.data.key) {
      if (cur_read.data.l_child_ptr.is_null) { abort_search = true; break; }
      next_header_j = cur_read.data.l_child_ptr;

    } else {
      if (cur_read.data.l_same_key_size == recursive_j) {
        found_j = true;

      } else if (cur_read.data.l_same_key_size > recursive_j) {
        if (cur_read.data.l_child_ptr.is_null) { abort_search = true; break; }
        next_header_j = cur_read.data.l_child_ptr;

      } else {
        if (cur_read.data.r_child_ptr.is_null) { abort_search = true; break; }
        next_header_j = cur_read.data.r_child_ptr;
        next_recursive_j = recursive_j - (cur_read.data.l_same_key_size + 1);
      }
    }

    // Continue shared path if both need the exact same next block
    if (!found_i && !found_j && next_header_i.block_id == next_header_j.block_id) {
      cur_read.header = next_header_i;
      recursive_i = next_recursive_i;
      recursive_j = next_recursive_j;
    } else {
      break; 
    }
  }

  std::vector<uint64_t> result;
  std::vector<ORAMBlock> left_bfs;
  std::vector<ORAMBlock> right_bfs;
  std::vector<ORAMBlock> full_bfs;

  // --------------------------------------------------------------------------
  // Phase 2: BFS Logic (Only run if nodes were actually found)
  // --------------------------------------------------------------------------
  if (!abort_search) {
    std::vector<uint32_t> i_values;
    std::vector<uint32_t> j_values;

    ORAMBlock divergence_node = common_path.back();

    // Check if the divergence node itself is a valid match
    if (divergence_node.data.key == key) {
      if (divergence_node.data.l_same_key_size >= recursive_i && 
          divergence_node.data.l_same_key_size <= recursive_j) {
        result.push_back(divergence_node.data.value);
      }
      
      // Seed left path BFS
      if (!divergence_node.data.l_child_ptr.is_null) {
        cur_read.header = divergence_node.data.l_child_ptr;
        left_bfs.push_back(write_block(cur_read, false));
        i_values.push_back(recursive_i);
      }
      
      // Seed right path BFS
      if (!divergence_node.data.r_child_ptr.is_null && divergence_node.data.l_same_key_size < recursive_j) {
        cur_read.header = divergence_node.data.r_child_ptr;
        right_bfs.push_back(write_block(cur_read, false));
        j_values.push_back(recursive_j - (divergence_node.data.l_same_key_size + 1));
      }
    }

    // --- 2a. Process Left Path BFS ---
    size_t left_idx = 0;
    while (left_idx < left_bfs.size()) {
      ORAMBlock node = left_bfs[left_idx];
      uint32_t target_i = i_values[left_idx];
      left_idx++;

      if (node.data.key != key) {
        if (!node.data.r_child_ptr.is_null) {
          cur_read.header = node.data.r_child_ptr;
          left_bfs.push_back(write_block(cur_read, false));
          i_values.push_back(target_i);
        }
        continue;
      }

      uint32_t L = node.data.l_same_key_size;
      if (L < target_i) {
        if (!node.data.r_child_ptr.is_null) {
          cur_read.header = node.data.r_child_ptr;
          left_bfs.push_back(write_block(cur_read, false));
          i_values.push_back(target_i - (L + 1));
        }
      } else {
        result.push_back(node.data.value);
        if (!node.data.l_child_ptr.is_null) {
          cur_read.header = node.data.l_child_ptr;
          left_bfs.push_back(write_block(cur_read, false));
          i_values.push_back(target_i);
        }
        if (!node.data.r_child_ptr.is_null) {
          cur_read.header = node.data.r_child_ptr;
          full_bfs.push_back(write_block(cur_read, false));
        }
      }
    }

    // --- 2b. Process Right Path BFS ---
    size_t right_idx = 0;
    while (right_idx < right_bfs.size()) {
      ORAMBlock node = right_bfs[right_idx];
      uint32_t target_j = j_values[right_idx];
      right_idx++;

      if (node.data.key != key) {
        if (!node.data.l_child_ptr.is_null) {
          cur_read.header = node.data.l_child_ptr;
          right_bfs.push_back(write_block(cur_read, false));
          j_values.push_back(target_j);
        }
        continue;
      }

      uint32_t L = node.data.l_same_key_size;
      if (L > target_j) {
        if (!node.data.l_child_ptr.is_null) {
          cur_read.header = node.data.l_child_ptr;
          right_bfs.push_back(write_block(cur_read, false));
          j_values.push_back(target_j);
        }
      } else {
        result.push_back(node.data.value);
        if (!node.data.l_child_ptr.is_null) {
          cur_read.header = node.data.l_child_ptr;
          full_bfs.push_back(write_block(cur_read, false));
        }
        if (L < target_j && !node.data.r_child_ptr.is_null) {
          cur_read.header = node.data.r_child_ptr;
          right_bfs.push_back(write_block(cur_read, false));
          j_values.push_back(target_j - (L + 1));
        }
      }
    }

    // --- 2c. Process Full Subtree BFS ---
    size_t full_idx = 0;
    while (full_idx < full_bfs.size()) {
      ORAMBlock node = full_bfs[full_idx];
      full_idx++;

      if (node.data.key == key) {
        result.push_back(node.data.value);
        if (!node.data.l_child_ptr.is_null) {
          cur_read.header = node.data.l_child_ptr;
          full_bfs.push_back(write_block(cur_read, false));
        }
        if (!node.data.r_child_ptr.is_null) {
          cur_read.header = node.data.r_child_ptr;
          full_bfs.push_back(write_block(cur_read, false));
        }
      }
    }
  }

  // --------------------------------------------------------------------------
  // Phase 3: Write-back logic
  // Helper to update leaf labels in parent nodes for BFS paths
  // --------------------------------------------------------------------------
  auto check_and_update = [](std::vector<ORAMBlock>& vec, uint32_t child_block_id, uint32_t new_leaf_label) {
    for (auto& p : vec) {
      if (!p.data.l_child_ptr.is_null && p.data.l_child_ptr.block_id == child_block_id) {
        p.data.l_child_ptr.leaf_label = new_leaf_label; 
        return true;
      }
      if (!p.data.r_child_ptr.is_null && p.data.r_child_ptr.block_id == child_block_id) {
        p.data.r_child_ptr.leaf_label = new_leaf_label; 
        return true;
      }
    }
    return false;
  };

  // Write back full_bfs
  for (int idx = (int)full_bfs.size() - 1; idx >= 0; idx--) {
    uint32_t new_leaf = write_block(full_bfs[idx], true).header.leaf_label;
    uint32_t cid = full_bfs[idx].header.block_id;
    if (check_and_update(full_bfs, cid, new_leaf)) continue;
    if (check_and_update(left_bfs, cid, new_leaf)) continue;
    check_and_update(right_bfs, cid, new_leaf);
  }

  // Write back right_bfs
  for (int idx = (int)right_bfs.size() - 1; idx >= 0; idx--) {
    uint32_t new_leaf = write_block(right_bfs[idx], true).header.leaf_label;
    uint32_t cid = right_bfs[idx].header.block_id;
    if (!check_and_update(right_bfs, cid, new_leaf)) {
      check_and_update(common_path, cid, new_leaf);
    }
  }

  // Write back left_bfs
  for (int idx = (int)left_bfs.size() - 1; idx >= 0; idx--) {
    uint32_t new_leaf = write_block(left_bfs[idx], true).header.leaf_label;
    uint32_t cid = left_bfs[idx].header.block_id;
    if (!check_and_update(left_bfs, cid, new_leaf)) {
      check_and_update(common_path, cid, new_leaf);
    }
  }
  
  // Write back common_path
  for (int idx = (int)common_path.size() - 1; idx >= 0; idx--) {
    uint32_t new_leaf = write_block(common_path[idx], true).header.leaf_label;
    if (idx > 0) {
      if (!common_path[idx - 1].data.l_child_ptr.is_null &&
          common_path[idx - 1].data.l_child_ptr.block_id == common_path[idx].header.block_id) {
        common_path[idx - 1].data.l_child_ptr.leaf_label = new_leaf;
      } else if (!common_path[idx - 1].data.r_child_ptr.is_null &&
                 common_path[idx - 1].data.r_child_ptr.block_id == common_path[idx].header.block_id) {
        common_path[idx - 1].data.r_child_ptr.leaf_label = new_leaf;
      }
    } else {
      root.leaf_label = new_leaf;
    }
  }

  return result;
}

void Client::insert(uint64_t key, uint64_t value) {
  // --------------------------------------------------------------------------
  // Phase 1: Base case - Empty tree
  // --------------------------------------------------------------------------
  if (root.is_null) {
    ORAMBlock to_write;
    to_write.header.block_id = next_available_block_id();
    to_write.header.is_null = false;
    to_write.data.key = key;
    to_write.data.value = value;
    root = write_block(to_write, true).header;
    return;
  }

  std::vector<ORAMBlock> avl_history;
  ORAMBlock cur_read;
  cur_read.header = root;
  bool duplicate = false;

  // --------------------------------------------------------------------------
  // Phase 2: Read AVL tree path from PathORAM and add new Node to end
  // Traverse downward until an empty spot is found or duplicate is handled
  // --------------------------------------------------------------------------
  while (true) {
    cur_read = write_block(cur_read, false);
    avl_history.push_back(cur_read);

    // Go Right
    if (key > cur_read.data.key ||
        key == cur_read.data.key && value > cur_read.data.value) {
      if (cur_read.data.r_child_ptr.is_null) {
        ORAMBlock to_write;
        to_write.header.block_id = next_available_block_id();
        to_write.header.is_null = false;
        to_write.data.key = key;
        to_write.data.value = value;

        avl_history.back().data.r_child_ptr = to_write.header;
        avl_history.push_back(to_write);
        break;
      } else {
        cur_read.header = cur_read.data.r_child_ptr;
      }

    // Go Left
    } else if (key < cur_read.data.key ||
               key == cur_read.data.key && value < cur_read.data.value) {
      if (cur_read.data.l_child_ptr.is_null) {
        ORAMBlock to_write;
        to_write.header.block_id = next_available_block_id();
        to_write.header.is_null = false;
        to_write.data.key = key;
        to_write.data.value = value;

        avl_history.back().data.l_child_ptr = to_write.header;
        avl_history.push_back(to_write);
        break;
      } else {
        cur_read.header = cur_read.data.l_child_ptr;
      }

    // Duplicate Found
    } else {
      duplicate = true;
      if (!cur_read.data.l_child_ptr.is_null) {
        cur_read.header = cur_read.data.l_child_ptr;
      } else {
        break;
      }
    }
  }

  // --------------------------------------------------------------------------
  // Phase 3: Reverse traverse and update heights/stats, and rebalance tree
  // --------------------------------------------------------------------------
  for (int height = 1; !duplicate && height < avl_history.size(); height++) {
    int cur_node_index = avl_history.size() - 1 - height;

    // --- Update Right Child Stats ---
    if (!avl_history[cur_node_index].data.r_child_ptr.is_null &&
        avl_history[cur_node_index].data.r_child_ptr.block_id ==
            avl_history[cur_node_index + 1].header.block_id) {
      
      avl_history[cur_node_index].data.r_height =
          1 + std::max(avl_history[cur_node_index + 1].data.l_height,
                       avl_history[cur_node_index + 1].data.r_height);
      
      if (key < avl_history[cur_node_index].data.r_min_key_subtree) {
        avl_history[cur_node_index].data.r_min_key_subtree = key;
        avl_history[cur_node_index].data.r_min_key_count = 1;
      } else if (key == avl_history[cur_node_index].data.r_min_key_subtree) {
        avl_history[cur_node_index].data.r_min_key_count++;
      }
      
      if (key > avl_history[cur_node_index].data.r_max_key_subtree) {
        avl_history[cur_node_index].data.r_max_key_subtree = key;
        avl_history[cur_node_index].data.r_max_key_count = 1;
      } else if (key == avl_history[cur_node_index].data.r_max_key_subtree) {
        avl_history[cur_node_index].data.r_max_key_count++;
      }
      
      if (key == avl_history[cur_node_index].data.key) {
        avl_history[cur_node_index].data.r_same_key_size++;
      }

    // --- Update Left Child Stats ---
    } else {
      avl_history[cur_node_index].data.l_height =
          1 + std::max(avl_history[cur_node_index + 1].data.l_height,
                       avl_history[cur_node_index + 1].data.r_height);
      
      if (key < avl_history[cur_node_index].data.l_min_key_subtree) {
        avl_history[cur_node_index].data.l_min_key_subtree = key;
        avl_history[cur_node_index].data.l_min_key_count = 1;
      } else if (key == avl_history[cur_node_index].data.l_min_key_subtree) {
        avl_history[cur_node_index].data.l_min_key_count++;
      }
      
      if (key > avl_history[cur_node_index].data.l_max_key_subtree) {
        avl_history[cur_node_index].data.l_max_key_subtree = key;
        avl_history[cur_node_index].data.l_max_key_count = 1;
      } else if (key == avl_history[cur_node_index].data.l_max_key_subtree) {
        avl_history[cur_node_index].data.l_max_key_count++;
      }
      
      if (key == avl_history[cur_node_index].data.key) {
        avl_history[cur_node_index].data.l_same_key_size++;
      }
    }

    // --- Check Balance Factor & Perform Rotations ---
    int balance_factor = avl_history[cur_node_index].data.r_height -
                         avl_history[cur_node_index].data.l_height;

    if (std::abs(balance_factor) > 1) {
      bool child_is_right =
          (!avl_history[cur_node_index].data.r_child_ptr.is_null &&
           avl_history[cur_node_index].data.r_child_ptr.block_id ==
               avl_history[cur_node_index + 1].header.block_id);

      bool grandchild_is_right =
          (!avl_history[cur_node_index + 1].data.r_child_ptr.is_null &&
           avl_history[cur_node_index + 1].data.r_child_ptr.block_id ==
               avl_history[cur_node_index + 2].header.block_id);

      if (child_is_right && grandchild_is_right) {
        rotate_right_right(avl_history, cur_node_index);
      } else if (!child_is_right && !grandchild_is_right) {
        rotate_left_left(avl_history, cur_node_index);
      } else if (child_is_right && !grandchild_is_right) {
        rotate_right_left(avl_history, cur_node_index);
      } else {
        rotate_left_right(avl_history, cur_node_index);
      }
    }
  }

  // --------------------------------------------------------------------------
  // Phase 4: PathORAM Write-back
  // Write back to PathORAM in reverse order, updating new leaf references
  // --------------------------------------------------------------------------
  for (int index = avl_history.size() - 1; index >= 0; index--) {
    uint32_t new_leaf_label =
        write_block(avl_history[index], true).header.leaf_label;

    if (index > 0 && !avl_history[index - 1].data.l_child_ptr.is_null &&
        avl_history[index - 1].data.l_child_ptr.block_id ==
            avl_history[index].header.block_id) {
      avl_history[index - 1].data.l_child_ptr.leaf_label = new_leaf_label;

    } else if (index > 0 && !avl_history[index - 1].data.r_child_ptr.is_null &&
               avl_history[index - 1].data.r_child_ptr.block_id ==
                   avl_history[index].header.block_id) {
      avl_history[index - 1].data.r_child_ptr.leaf_label = new_leaf_label;

    } else if (index > 1 && !avl_history[index - 2].data.l_child_ptr.is_null &&
               avl_history[index - 2].data.l_child_ptr.block_id ==
                   avl_history[index].header.block_id) {
      avl_history[index - 2].data.l_child_ptr.leaf_label = new_leaf_label;

    } else if (index > 1 && !avl_history[index - 2].data.r_child_ptr.is_null &&
               avl_history[index - 2].data.r_child_ptr.block_id ==
                   avl_history[index].header.block_id) {
      avl_history[index - 2].data.r_child_ptr.leaf_label = new_leaf_label;

    } else if (index == 0) {
      root.leaf_label = new_leaf_label;
    }
  }
}