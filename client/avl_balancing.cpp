#include "client.h"

void Client::rotate_right_right(std::vector<ORAMBlock>& avl_history,
                                int cur_node_index) {
  // --------------------------------------------------------------------------
  // Phase 1: Node A (cur_node_index) adopts Node B's (cur_node_index + 1)
  // left child as its new right child.
  // --------------------------------------------------------------------------
  avl_history[cur_node_index].data.r_child_ptr =
      avl_history[cur_node_index + 1].data.l_child_ptr;
  avl_history[cur_node_index].data.r_height =
      avl_history[cur_node_index + 1].data.l_height;
  avl_history[cur_node_index].data.r_max_key_count =
      avl_history[cur_node_index + 1].data.l_max_key_count;
  avl_history[cur_node_index].data.r_max_key_subtree =
      avl_history[cur_node_index + 1].data.l_max_key_subtree;
  avl_history[cur_node_index].data.r_min_key_count =
      avl_history[cur_node_index + 1].data.l_min_key_count;
  avl_history[cur_node_index].data.r_min_key_subtree =
      avl_history[cur_node_index + 1].data.l_min_key_subtree;

  // Check if the min of the new right subtree straddles A's key
  if (avl_history[cur_node_index].data.r_min_key_subtree ==
      avl_history[cur_node_index].data.key) {
    avl_history[cur_node_index].data.r_same_key_size =
        avl_history[cur_node_index].data.r_min_key_count;
  } else {
    avl_history[cur_node_index].data.r_same_key_size = 0;
  }

  // --------------------------------------------------------------------------
  // Phase 2: Node B (cur_node_index + 1) adopts Node A as its new left child.
  // --------------------------------------------------------------------------
  avl_history[cur_node_index + 1].data.l_child_ptr =
      avl_history[cur_node_index].header;
  avl_history[cur_node_index + 1].data.l_height =
      1 + std::max(avl_history[cur_node_index].data.l_height,
                   avl_history[cur_node_index].data.r_height);

  // Calculate B's new l_same_key_size based on A's updated state
  if (avl_history[cur_node_index].data.r_max_key_count != 0) {
    if (avl_history[cur_node_index].data.r_max_key_subtree ==
        avl_history[cur_node_index + 1].data.key) {
      if (avl_history[cur_node_index].data.key ==
          avl_history[cur_node_index].data.r_max_key_subtree) {
        avl_history[cur_node_index + 1].data.l_same_key_size =
            1 + avl_history[cur_node_index].data.l_same_key_size +
            avl_history[cur_node_index].data.r_same_key_size;
      } else {
        avl_history[cur_node_index + 1].data.l_same_key_size =
            avl_history[cur_node_index].data.r_max_key_count;
      }
    } else {
      avl_history[cur_node_index + 1].data.l_same_key_size = 0;
    }
  } else {
    if (avl_history[cur_node_index].data.key ==
        avl_history[cur_node_index + 1].data.key) {
      avl_history[cur_node_index + 1].data.l_same_key_size =
          1 + avl_history[cur_node_index].data.l_same_key_size +
          avl_history[cur_node_index].data.r_same_key_size;
    } else {
      avl_history[cur_node_index + 1].data.l_same_key_size = 0;
    }
  }

  // --------------------------------------------------------------------------
  // Phase 3: Calculate MAX Key for B's new Left Child (A)
  // Logic: Check A's Right Subtree (which was just updated).
  // If it exists, that's the Max. Otherwise, A itself is the Max.
  // --------------------------------------------------------------------------
  avl_history[cur_node_index + 1].data.l_max_key_subtree =
      (avl_history[cur_node_index].data.r_max_key_count > 0)
          ? avl_history[cur_node_index].data.r_max_key_subtree
          : avl_history[cur_node_index].data.key;

  // Calculate Count for that Max Key
  if (avl_history[cur_node_index].data.r_max_key_count > 0) {
    // STRADDLE CHECK: Is the Right Subtree's Max Key equal to A's Key?
    if (avl_history[cur_node_index].data.r_max_key_subtree ==
        avl_history[cur_node_index].data.key) {
      // Yes: The 'Max' group includes A. Sum A's self(1) + Left Matches + Right
      // Matches. Note: Ensure A.r_same_key_size was updated to
      // B.l_same_key_size prior to this if relying on it.
      avl_history[cur_node_index + 1].data.l_max_key_count =
          1 + avl_history[cur_node_index].data.l_same_key_size +
          avl_history[cur_node_index].data.r_same_key_size;
    } else {
      // No: The Max is strictly in the right subtree.
      avl_history[cur_node_index + 1].data.l_max_key_count =
          avl_history[cur_node_index].data.r_max_key_count;
    }
  } else {
    // No right child: A is the Max. Count is Self(1) + Left Matches.
    avl_history[cur_node_index + 1].data.l_max_key_count =
        1 + avl_history[cur_node_index].data.l_same_key_size;
  }

  // --------------------------------------------------------------------------
  // Phase 4: Calculate MIN Key for B's new Left Child (A)
  // Logic: Check A's Left Subtree (untouched by rotation).
  // --------------------------------------------------------------------------
  avl_history[cur_node_index + 1].data.l_min_key_subtree =
      (avl_history[cur_node_index].data.l_min_key_count > 0)
          ? avl_history[cur_node_index].data.l_min_key_subtree
          : avl_history[cur_node_index].data.key;

  // Calculate Count for that Min Key
  if (avl_history[cur_node_index].data.l_min_key_count > 0) {
    // STRADDLE CHECK: Is the Left Subtree's Min Key equal to A's Key?
    if (avl_history[cur_node_index].data.l_min_key_subtree ==
        avl_history[cur_node_index].data.key) {
      // Yes: The 'Min' group includes A. Sum Total Matches.
      avl_history[cur_node_index + 1].data.l_min_key_count =
          1 + avl_history[cur_node_index].data.l_same_key_size +
          avl_history[cur_node_index].data.r_same_key_size;
    } else {
      // No: The Min is strictly in the left subtree.
      avl_history[cur_node_index + 1].data.l_min_key_count =
          avl_history[cur_node_index].data.l_min_key_count;
    }
  } else {
    // No left child: A is the Min. Count is Self(1) + Right Matches.
    avl_history[cur_node_index + 1].data.l_min_key_count =
        1 + avl_history[cur_node_index].data.r_same_key_size;
  }

  // --------------------------------------------------------------------------
  // Phase 5: Swap nodes in history to reflect new structure, and update
  // parent pointers (or root).
  // --------------------------------------------------------------------------
  std::swap(avl_history[cur_node_index + 1], avl_history[cur_node_index]);

  if (cur_node_index == 0) {
    // Node is at the top, so it becomes the new root
    root = avl_history[cur_node_index].header;
  } else {
    // Link the parent node (cur_node_index - 1) to the new subtree root
    if (!avl_history[cur_node_index - 1].data.r_child_ptr.is_null &&
        avl_history[cur_node_index - 1].data.r_child_ptr.block_id ==
            avl_history[cur_node_index + 1].header.block_id) {
      avl_history[cur_node_index - 1].data.r_child_ptr =
          avl_history[cur_node_index].header;
    } else {
      avl_history[cur_node_index - 1].data.l_child_ptr =
          avl_history[cur_node_index].header;
    }
  }
}


void Client::rotate_left_left(std::vector<ORAMBlock>& avl_history,
                              int cur_node_index) {
  // --------------------------------------------------------------------------
  // Phase 1: Node A (old root/cur_node_index) adopts Node B's right child
  // as its new left child.
  // --------------------------------------------------------------------------
  avl_history[cur_node_index].data.l_child_ptr =
      avl_history[cur_node_index + 1].data.r_child_ptr;
  avl_history[cur_node_index].data.l_height =
      avl_history[cur_node_index + 1].data.r_height;
  avl_history[cur_node_index].data.l_max_key_count =
      avl_history[cur_node_index + 1].data.r_max_key_count;
  avl_history[cur_node_index].data.l_max_key_subtree =
      avl_history[cur_node_index + 1].data.r_max_key_subtree;
  avl_history[cur_node_index].data.l_min_key_count =
      avl_history[cur_node_index + 1].data.r_min_key_count;
  avl_history[cur_node_index].data.l_min_key_subtree =
      avl_history[cur_node_index + 1].data.r_min_key_subtree;

  // Check if the max of the new left subtree straddles A's key
  if (avl_history[cur_node_index].data.l_max_key_subtree ==
      avl_history[cur_node_index].data.key) {
    avl_history[cur_node_index].data.l_same_key_size =
        avl_history[cur_node_index].data.l_max_key_count;
  } else {
    avl_history[cur_node_index].data.l_same_key_size = 0;
  }

  // --------------------------------------------------------------------------
  // Phase 2: Node B (new root/cur_node_index + 1) adopts Node A as its
  // new right child.
  // --------------------------------------------------------------------------
  avl_history[cur_node_index + 1].data.r_child_ptr =
      avl_history[cur_node_index].header;
  avl_history[cur_node_index + 1].data.r_height =
      1 + std::max(avl_history[cur_node_index].data.l_height,
                   avl_history[cur_node_index].data.r_height);

  // Calculate B's new r_same_key_size by looking at A's min key
  if (avl_history[cur_node_index].data.l_min_key_count != 0) {
    if (avl_history[cur_node_index].data.l_min_key_subtree ==
        avl_history[cur_node_index + 1].data.key) {
      if (avl_history[cur_node_index].data.key ==
          avl_history[cur_node_index].data.l_min_key_subtree) {
        avl_history[cur_node_index + 1].data.r_same_key_size =
            1 + avl_history[cur_node_index].data.l_same_key_size +
            avl_history[cur_node_index].data.r_same_key_size;
      } else {
        avl_history[cur_node_index + 1].data.r_same_key_size =
            avl_history[cur_node_index].data.l_min_key_count;
      }
    } else {
      avl_history[cur_node_index + 1].data.r_same_key_size = 0;
    }
  } else {
    if (avl_history[cur_node_index].data.key ==
        avl_history[cur_node_index + 1].data.key) {
      avl_history[cur_node_index + 1].data.r_same_key_size =
          1 + avl_history[cur_node_index].data.l_same_key_size +
          avl_history[cur_node_index].data.r_same_key_size;
    } else {
      avl_history[cur_node_index + 1].data.r_same_key_size = 0;
    }
  }

  // --------------------------------------------------------------------------
  // Phase 3: Calculate MIN Key for B's new Right Child (A)
  // Logic: Check A's Left Subtree (which was just updated).
  // --------------------------------------------------------------------------
  avl_history[cur_node_index + 1].data.r_min_key_subtree =
      (avl_history[cur_node_index].data.l_min_key_count > 0)
          ? avl_history[cur_node_index].data.l_min_key_subtree
          : avl_history[cur_node_index].data.key;

  if (avl_history[cur_node_index].data.l_min_key_count > 0) {
    // STRADDLE CHECK: Is the Left Subtree's Min Key equal to A's Key?
    if (avl_history[cur_node_index].data.l_min_key_subtree ==
        avl_history[cur_node_index].data.key) {
      avl_history[cur_node_index + 1].data.r_min_key_count =
          1 + avl_history[cur_node_index].data.l_same_key_size +
          avl_history[cur_node_index].data.r_same_key_size;
    } else {
      avl_history[cur_node_index + 1].data.r_min_key_count =
          avl_history[cur_node_index].data.l_min_key_count;
    }
  } else {
    // No left child: A is the Min. Count is Self(1) + Right Matches.
    avl_history[cur_node_index + 1].data.r_min_key_count =
        1 + avl_history[cur_node_index].data.r_same_key_size;
  }

  // --------------------------------------------------------------------------
  // Phase 4: Calculate MAX Key for B's new Right Child (A)
  // Logic: Check A's Right Subtree (untouched by rotation).
  // --------------------------------------------------------------------------
  avl_history[cur_node_index + 1].data.r_max_key_subtree =
      (avl_history[cur_node_index].data.r_max_key_count > 0)
          ? avl_history[cur_node_index].data.r_max_key_subtree
          : avl_history[cur_node_index].data.key;

  if (avl_history[cur_node_index].data.r_max_key_count > 0) {
    // STRADDLE CHECK: Is the Right Subtree's Max Key equal to A's Key?
    if (avl_history[cur_node_index].data.r_max_key_subtree ==
        avl_history[cur_node_index].data.key) {
      avl_history[cur_node_index + 1].data.r_max_key_count =
          1 + avl_history[cur_node_index].data.l_same_key_size +
          avl_history[cur_node_index].data.r_same_key_size;
    } else {
      avl_history[cur_node_index + 1].data.r_max_key_count =
          avl_history[cur_node_index].data.r_max_key_count;
    }
  } else {
    // No right child: A is the Max. Count is Self(1) + Left Matches.
    avl_history[cur_node_index + 1].data.r_max_key_count =
        1 + avl_history[cur_node_index].data.l_same_key_size;
  }

  // --------------------------------------------------------------------------
  // Phase 5: Swap nodes in history to reflect new structure, and update
  // parent pointers (or root).
  // --------------------------------------------------------------------------
  std::swap(avl_history[cur_node_index + 1], avl_history[cur_node_index]);

  if (cur_node_index == 0) {
    root = avl_history[cur_node_index].header;
  } else {
    if (!avl_history[cur_node_index - 1].data.r_child_ptr.is_null &&
        avl_history[cur_node_index - 1].data.r_child_ptr.block_id ==
            avl_history[cur_node_index + 1].header.block_id) {
      avl_history[cur_node_index - 1].data.r_child_ptr =
          avl_history[cur_node_index].header;
    } else {
      avl_history[cur_node_index - 1].data.l_child_ptr =
          avl_history[cur_node_index].header;
    }
  }
}


void Client::rotate_right_left(std::vector<ORAMBlock>& avl_history,
                               int cur_node_index) {
  // --------------------------------------------------------------------------
  // Phase 1: Node A (cur_node_index) adopts Node C's left child
  // as its new right child.
  // --------------------------------------------------------------------------
  avl_history[cur_node_index].data.r_child_ptr =
      avl_history[cur_node_index + 2].data.l_child_ptr;
  avl_history[cur_node_index].data.r_height =
      avl_history[cur_node_index + 2].data.l_height;
  avl_history[cur_node_index].data.r_min_key_subtree =
      avl_history[cur_node_index + 2].data.l_min_key_subtree;
  avl_history[cur_node_index].data.r_min_key_count =
      avl_history[cur_node_index + 2].data.l_min_key_count;
  avl_history[cur_node_index].data.r_max_key_subtree =
      avl_history[cur_node_index + 2].data.l_max_key_subtree;
  avl_history[cur_node_index].data.r_max_key_count =
      avl_history[cur_node_index + 2].data.l_max_key_count;

  if (avl_history[cur_node_index].data.r_min_key_subtree ==
      avl_history[cur_node_index].data.key) {
    avl_history[cur_node_index].data.r_same_key_size =
        avl_history[cur_node_index].data.r_min_key_count;
  } else {
    avl_history[cur_node_index].data.r_same_key_size = 0;
  }

  // --------------------------------------------------------------------------
  // Phase 2: Node B (cur_node_index + 1) adopts Node C's right child
  // as its new left child.
  // --------------------------------------------------------------------------
  avl_history[cur_node_index + 1].data.l_child_ptr =
      avl_history[cur_node_index + 2].data.r_child_ptr;
  avl_history[cur_node_index + 1].data.l_height =
      avl_history[cur_node_index + 2].data.r_height;
  avl_history[cur_node_index + 1].data.l_min_key_subtree =
      avl_history[cur_node_index + 2].data.r_min_key_subtree;
  avl_history[cur_node_index + 1].data.l_min_key_count =
      avl_history[cur_node_index + 2].data.r_min_key_count;
  avl_history[cur_node_index + 1].data.l_max_key_subtree =
      avl_history[cur_node_index + 2].data.r_max_key_subtree;
  avl_history[cur_node_index + 1].data.l_max_key_count =
      avl_history[cur_node_index + 2].data.r_max_key_count;

  if (avl_history[cur_node_index + 1].data.l_max_key_subtree ==
      avl_history[cur_node_index + 1].data.key) {
    avl_history[cur_node_index + 1].data.l_same_key_size =
        avl_history[cur_node_index + 1].data.l_max_key_count;
  } else {
    avl_history[cur_node_index + 1].data.l_same_key_size = 0;
  }

  // --------------------------------------------------------------------------
  // Phase 3: Node C (cur_node_index + 2) adopts Node A as its left child.
  // Calculate C's new left statistics (same key size, max, min).
  // --------------------------------------------------------------------------
  avl_history[cur_node_index + 2].data.l_child_ptr =
      avl_history[cur_node_index].header;
  avl_history[cur_node_index + 2].data.l_height =
      1 + std::max(avl_history[cur_node_index].data.l_height,
                   avl_history[cur_node_index].data.r_height);

  // Calculate C's l_same_key_size
  if (avl_history[cur_node_index].data.r_max_key_count != 0) {
    if (avl_history[cur_node_index].data.r_max_key_subtree ==
        avl_history[cur_node_index + 2].data.key) {
      if (avl_history[cur_node_index].data.key ==
          avl_history[cur_node_index].data.r_max_key_subtree) {
        avl_history[cur_node_index + 2].data.l_same_key_size =
            1 + avl_history[cur_node_index].data.l_same_key_size +
            avl_history[cur_node_index].data.r_same_key_size;
      } else {
        avl_history[cur_node_index + 2].data.l_same_key_size =
            avl_history[cur_node_index].data.r_max_key_count;
      }
    } else {
      avl_history[cur_node_index + 2].data.l_same_key_size = 0;
    }
  } else {
    if (avl_history[cur_node_index].data.key ==
        avl_history[cur_node_index + 2].data.key) {
      avl_history[cur_node_index + 2].data.l_same_key_size =
          1 + avl_history[cur_node_index].data.l_same_key_size +
          avl_history[cur_node_index].data.r_same_key_size;
    } else {
      avl_history[cur_node_index + 2].data.l_same_key_size = 0;
    }
  }

  // Calculate C's l_max
  avl_history[cur_node_index + 2].data.l_max_key_subtree =
      (avl_history[cur_node_index].data.r_max_key_count > 0)
          ? avl_history[cur_node_index].data.r_max_key_subtree
          : avl_history[cur_node_index].data.key;

  if (avl_history[cur_node_index].data.r_max_key_count > 0) {
    if (avl_history[cur_node_index].data.r_max_key_subtree ==
        avl_history[cur_node_index].data.key) {
      avl_history[cur_node_index + 2].data.l_max_key_count =
          1 + avl_history[cur_node_index].data.l_same_key_size +
          avl_history[cur_node_index].data.r_same_key_size;
    } else {
      avl_history[cur_node_index + 2].data.l_max_key_count =
          avl_history[cur_node_index].data.r_max_key_count;
    }
  } else {
    avl_history[cur_node_index + 2].data.l_max_key_count =
        1 + avl_history[cur_node_index].data.l_same_key_size;
  }

  // Calculate C's l_min
  avl_history[cur_node_index + 2].data.l_min_key_subtree =
      (avl_history[cur_node_index].data.l_min_key_count > 0)
          ? avl_history[cur_node_index].data.l_min_key_subtree
          : avl_history[cur_node_index].data.key;

  if (avl_history[cur_node_index].data.l_min_key_count > 0) {
    if (avl_history[cur_node_index].data.l_min_key_subtree ==
        avl_history[cur_node_index].data.key) {
      avl_history[cur_node_index + 2].data.l_min_key_count =
          1 + avl_history[cur_node_index].data.l_same_key_size +
          avl_history[cur_node_index].data.r_same_key_size;
    } else {
      avl_history[cur_node_index + 2].data.l_min_key_count =
          avl_history[cur_node_index].data.l_min_key_count;
    }
  } else {
    avl_history[cur_node_index + 2].data.l_min_key_count =
        1 + avl_history[cur_node_index].data.r_same_key_size;
  }

  // --------------------------------------------------------------------------
  // Phase 4: Node C (cur_node_index + 2) adopts Node B as its right child.
  // Calculate C's new right statistics (same key size, max, min).
  // --------------------------------------------------------------------------
  avl_history[cur_node_index + 2].data.r_child_ptr =
      avl_history[cur_node_index + 1].header;
  avl_history[cur_node_index + 2].data.r_height =
      1 + std::max(avl_history[cur_node_index + 1].data.l_height,
                   avl_history[cur_node_index + 1].data.r_height);

  // Calculate C's r_same_key_size
  if (avl_history[cur_node_index + 1].data.l_min_key_count != 0) {
    if (avl_history[cur_node_index + 1].data.l_min_key_subtree ==
        avl_history[cur_node_index + 2].data.key) {
      if (avl_history[cur_node_index + 1].data.key ==
          avl_history[cur_node_index + 1].data.l_min_key_subtree) {
        avl_history[cur_node_index + 2].data.r_same_key_size =
            1 + avl_history[cur_node_index + 1].data.l_same_key_size +
            avl_history[cur_node_index + 1].data.r_same_key_size;
      } else {
        avl_history[cur_node_index + 2].data.r_same_key_size =
            avl_history[cur_node_index + 1].data.l_min_key_count;
      }
    } else {
      avl_history[cur_node_index + 2].data.r_same_key_size = 0;
    }
  } else {
    if (avl_history[cur_node_index + 1].data.key ==
        avl_history[cur_node_index + 2].data.key) {
      avl_history[cur_node_index + 2].data.r_same_key_size =
          1 + avl_history[cur_node_index + 1].data.l_same_key_size +
          avl_history[cur_node_index + 1].data.r_same_key_size;
    } else {
      avl_history[cur_node_index + 2].data.r_same_key_size = 0;
    }
  }

  // Calculate C's r_min
  avl_history[cur_node_index + 2].data.r_min_key_subtree =
      (avl_history[cur_node_index + 1].data.l_min_key_count > 0)
          ? avl_history[cur_node_index + 1].data.l_min_key_subtree
          : avl_history[cur_node_index + 1].data.key;

  if (avl_history[cur_node_index + 1].data.l_min_key_count > 0) {
    if (avl_history[cur_node_index + 1].data.l_min_key_subtree ==
        avl_history[cur_node_index + 1].data.key) {
      avl_history[cur_node_index + 2].data.r_min_key_count =
          1 + avl_history[cur_node_index + 1].data.l_same_key_size +
          avl_history[cur_node_index + 1].data.r_same_key_size;
    } else {
      avl_history[cur_node_index + 2].data.r_min_key_count =
          avl_history[cur_node_index + 1].data.l_min_key_count;
    }
  } else {
    avl_history[cur_node_index + 2].data.r_min_key_count =
        1 + avl_history[cur_node_index + 1].data.r_same_key_size;
  }

  // Calculate C's r_max
  avl_history[cur_node_index + 2].data.r_max_key_subtree =
      (avl_history[cur_node_index + 1].data.r_max_key_count > 0)
          ? avl_history[cur_node_index + 1].data.r_max_key_subtree
          : avl_history[cur_node_index + 1].data.key;

  if (avl_history[cur_node_index + 1].data.r_max_key_count > 0) {
    if (avl_history[cur_node_index + 1].data.r_max_key_subtree ==
        avl_history[cur_node_index + 1].data.key) {
      avl_history[cur_node_index + 2].data.r_max_key_count =
          1 + avl_history[cur_node_index + 1].data.l_same_key_size +
          avl_history[cur_node_index + 1].data.r_same_key_size;
    } else {
      avl_history[cur_node_index + 2].data.r_max_key_count =
          avl_history[cur_node_index + 1].data.r_max_key_count;
    }
  } else {
    avl_history[cur_node_index + 2].data.r_max_key_count =
        1 + avl_history[cur_node_index + 1].data.l_same_key_size;
  }

  // --------------------------------------------------------------------------
  // Phase 5: Swap nodes in history and update parent pointers (or root)
  // for the double rotation.
  // --------------------------------------------------------------------------
  std::swap(avl_history[cur_node_index + 2], avl_history[cur_node_index]);

  if (cur_node_index == 0) {
    root = avl_history[cur_node_index].header;
  } else {
    if (!avl_history[cur_node_index - 1].data.r_child_ptr.is_null &&
        avl_history[cur_node_index - 1].data.r_child_ptr.block_id ==
            avl_history[cur_node_index + 2].header.block_id) {
      avl_history[cur_node_index - 1].data.r_child_ptr =
          avl_history[cur_node_index].header;
    } else {
      avl_history[cur_node_index - 1].data.l_child_ptr =
          avl_history[cur_node_index].header;
    }
  }
}


void Client::rotate_left_right(std::vector<ORAMBlock>& avl_history,
                               int cur_node_index) {
  // --------------------------------------------------------------------------
  // Phase 1: Node A (cur_node_index) adopts Node C's right child
  // as its new left child.
  // --------------------------------------------------------------------------
  avl_history[cur_node_index].data.l_child_ptr =
      avl_history[cur_node_index + 2].data.r_child_ptr;
  avl_history[cur_node_index].data.l_height =
      avl_history[cur_node_index + 2].data.r_height;
  avl_history[cur_node_index].data.l_min_key_subtree =
      avl_history[cur_node_index + 2].data.r_min_key_subtree;
  avl_history[cur_node_index].data.l_min_key_count =
      avl_history[cur_node_index + 2].data.r_min_key_count;
  avl_history[cur_node_index].data.l_max_key_subtree =
      avl_history[cur_node_index + 2].data.r_max_key_subtree;
  avl_history[cur_node_index].data.l_max_key_count =
      avl_history[cur_node_index + 2].data.r_max_key_count;

  // Check if the max of the new left subtree straddles A's key
  if (avl_history[cur_node_index].data.l_max_key_subtree ==
      avl_history[cur_node_index].data.key) {
    avl_history[cur_node_index].data.l_same_key_size =
        avl_history[cur_node_index].data.l_max_key_count;
  } else {
    avl_history[cur_node_index].data.l_same_key_size = 0;
  }

  // --------------------------------------------------------------------------
  // Phase 2: Node B (cur_node_index + 1) adopts Node C's left child
  // as its new right child.
  // --------------------------------------------------------------------------
  avl_history[cur_node_index + 1].data.r_child_ptr =
      avl_history[cur_node_index + 2].data.l_child_ptr;
  avl_history[cur_node_index + 1].data.r_height =
      avl_history[cur_node_index + 2].data.l_height;
  avl_history[cur_node_index + 1].data.r_min_key_subtree =
      avl_history[cur_node_index + 2].data.l_min_key_subtree;
  avl_history[cur_node_index + 1].data.r_min_key_count =
      avl_history[cur_node_index + 2].data.l_min_key_count;
  avl_history[cur_node_index + 1].data.r_max_key_subtree =
      avl_history[cur_node_index + 2].data.l_max_key_subtree;
  avl_history[cur_node_index + 1].data.r_max_key_count =
      avl_history[cur_node_index + 2].data.l_max_key_count;

  // Check if the min of the new right subtree straddles B's key
  if (avl_history[cur_node_index + 1].data.r_min_key_subtree ==
      avl_history[cur_node_index + 1].data.key) {
    avl_history[cur_node_index + 1].data.r_same_key_size =
        avl_history[cur_node_index + 1].data.r_min_key_count;
  } else {
    avl_history[cur_node_index + 1].data.r_same_key_size = 0;
  }

  // --------------------------------------------------------------------------
  // Phase 3: Node C (cur_node_index + 2) adopts Node B as its left child.
  // Calculate C's new left statistics (same key size, max, min).
  // --------------------------------------------------------------------------
  avl_history[cur_node_index + 2].data.l_child_ptr =
      avl_history[cur_node_index + 1].header;
  avl_history[cur_node_index + 2].data.l_height =
      1 + std::max(avl_history[cur_node_index + 1].data.l_height,
                   avl_history[cur_node_index + 1].data.r_height);

  // Calculate C's l_same_key_size (looking at B's right side)
  if (avl_history[cur_node_index + 1].data.r_max_key_count != 0) {
    if (avl_history[cur_node_index + 1].data.r_max_key_subtree ==
        avl_history[cur_node_index + 2].data.key) {
      if (avl_history[cur_node_index + 1].data.key ==
          avl_history[cur_node_index + 1].data.r_max_key_subtree) {
        avl_history[cur_node_index + 2].data.l_same_key_size =
            1 + avl_history[cur_node_index + 1].data.l_same_key_size +
            avl_history[cur_node_index + 1].data.r_same_key_size;
      } else {
        avl_history[cur_node_index + 2].data.l_same_key_size =
            avl_history[cur_node_index + 1].data.r_max_key_count;
      }
    } else {
      avl_history[cur_node_index + 2].data.l_same_key_size = 0;
    }
  } else {
    if (avl_history[cur_node_index + 1].data.key ==
        avl_history[cur_node_index + 2].data.key) {
      avl_history[cur_node_index + 2].data.l_same_key_size =
          1 + avl_history[cur_node_index + 1].data.l_same_key_size +
          avl_history[cur_node_index + 1].data.r_same_key_size;
    } else {
      avl_history[cur_node_index + 2].data.l_same_key_size = 0;
    }
  }

  // Calculate C's l_max
  avl_history[cur_node_index + 2].data.l_max_key_subtree =
      (avl_history[cur_node_index + 1].data.r_max_key_count > 0)
          ? avl_history[cur_node_index + 1].data.r_max_key_subtree
          : avl_history[cur_node_index + 1].data.key;

  if (avl_history[cur_node_index + 1].data.r_max_key_count > 0) {
    if (avl_history[cur_node_index + 1].data.r_max_key_subtree ==
        avl_history[cur_node_index + 1].data.key) {
      avl_history[cur_node_index + 2].data.l_max_key_count =
          1 + avl_history[cur_node_index + 1].data.l_same_key_size +
          avl_history[cur_node_index + 1].data.r_same_key_size;
    } else {
      avl_history[cur_node_index + 2].data.l_max_key_count =
          avl_history[cur_node_index + 1].data.r_max_key_count;
    }
  } else {
    avl_history[cur_node_index + 2].data.l_max_key_count =
        1 + avl_history[cur_node_index + 1].data.l_same_key_size;
  }

  // Calculate C's l_min
  avl_history[cur_node_index + 2].data.l_min_key_subtree =
      (avl_history[cur_node_index + 1].data.l_min_key_count > 0)
          ? avl_history[cur_node_index + 1].data.l_min_key_subtree
          : avl_history[cur_node_index + 1].data.key;

  if (avl_history[cur_node_index + 1].data.l_min_key_count > 0) {
    if (avl_history[cur_node_index + 1].data.l_min_key_subtree ==
        avl_history[cur_node_index + 1].data.key) {
      avl_history[cur_node_index + 2].data.l_min_key_count =
          1 + avl_history[cur_node_index + 1].data.l_same_key_size +
          avl_history[cur_node_index + 1].data.r_same_key_size;
    } else {
      avl_history[cur_node_index + 2].data.l_min_key_count =
          avl_history[cur_node_index + 1].data.l_min_key_count;
    }
  } else {
    avl_history[cur_node_index + 2].data.l_min_key_count =
        1 + avl_history[cur_node_index + 1].data.r_same_key_size;
  }

  // --------------------------------------------------------------------------
  // Phase 4: Node C (cur_node_index + 2) adopts Node A as its right child.
  // Calculate C's new right statistics (same key size, max, min).
  // --------------------------------------------------------------------------
  avl_history[cur_node_index + 2].data.r_child_ptr =
      avl_history[cur_node_index].header;
  avl_history[cur_node_index + 2].data.r_height =
      1 + std::max(avl_history[cur_node_index].data.l_height,
                   avl_history[cur_node_index].data.r_height);

  // Calculate C's r_same_key_size (looking at A's left side)
  if (avl_history[cur_node_index].data.l_min_key_count != 0) {
    if (avl_history[cur_node_index].data.l_min_key_subtree ==
        avl_history[cur_node_index + 2].data.key) {
      if (avl_history[cur_node_index].data.key ==
          avl_history[cur_node_index].data.l_min_key_subtree) {
        avl_history[cur_node_index + 2].data.r_same_key_size =
            1 + avl_history[cur_node_index].data.l_same_key_size +
            avl_history[cur_node_index].data.r_same_key_size;
      } else {
        avl_history[cur_node_index + 2].data.r_same_key_size =
            avl_history[cur_node_index].data.l_min_key_count;
      }
    } else {
      avl_history[cur_node_index + 2].data.r_same_key_size = 0;
    }
  } else {
    if (avl_history[cur_node_index].data.key ==
        avl_history[cur_node_index + 2].data.key) {
      avl_history[cur_node_index + 2].data.r_same_key_size =
          1 + avl_history[cur_node_index].data.l_same_key_size +
          avl_history[cur_node_index].data.r_same_key_size;
    } else {
      avl_history[cur_node_index + 2].data.r_same_key_size = 0;
    }
  }

  // Calculate C's r_min
  avl_history[cur_node_index + 2].data.r_min_key_subtree =
      (avl_history[cur_node_index].data.l_min_key_count > 0)
          ? avl_history[cur_node_index].data.l_min_key_subtree
          : avl_history[cur_node_index].data.key;

  if (avl_history[cur_node_index].data.l_min_key_count > 0) {
    if (avl_history[cur_node_index].data.l_min_key_subtree ==
        avl_history[cur_node_index].data.key) {
      avl_history[cur_node_index + 2].data.r_min_key_count =
          1 + avl_history[cur_node_index].data.l_same_key_size +
          avl_history[cur_node_index].data.r_same_key_size;
    } else {
      avl_history[cur_node_index + 2].data.r_min_key_count =
          avl_history[cur_node_index].data.l_min_key_count;
    }
  } else {
    avl_history[cur_node_index + 2].data.r_min_key_count =
        1 + avl_history[cur_node_index].data.r_same_key_size;
  }

  // Calculate C's r_max
  avl_history[cur_node_index + 2].data.r_max_key_subtree =
      (avl_history[cur_node_index].data.r_max_key_count > 0)
          ? avl_history[cur_node_index].data.r_max_key_subtree
          : avl_history[cur_node_index].data.key;

  if (avl_history[cur_node_index].data.r_max_key_count > 0) {
    if (avl_history[cur_node_index].data.r_max_key_subtree ==
        avl_history[cur_node_index].data.key) {
      avl_history[cur_node_index + 2].data.r_max_key_count =
          1 + avl_history[cur_node_index].data.l_same_key_size +
          avl_history[cur_node_index].data.r_same_key_size;
    } else {
      avl_history[cur_node_index + 2].data.r_max_key_count =
          avl_history[cur_node_index].data.r_max_key_count;
    }
  } else {
    avl_history[cur_node_index + 2].data.r_max_key_count =
        1 + avl_history[cur_node_index].data.l_same_key_size;
  }

  // --------------------------------------------------------------------------
  // Phase 5: Swap nodes in history and update parent pointers (or root)
  // for the double rotation.
  // --------------------------------------------------------------------------
  std::swap(avl_history[cur_node_index + 2], avl_history[cur_node_index]);

  if (cur_node_index == 0) {
    root = avl_history[cur_node_index].header;
  } else {
    if (!avl_history[cur_node_index - 1].data.r_child_ptr.is_null &&
        avl_history[cur_node_index - 1].data.r_child_ptr.block_id ==
            avl_history[cur_node_index + 2].header.block_id) {
      avl_history[cur_node_index - 1].data.r_child_ptr =
          avl_history[cur_node_index].header;
    } else {
      avl_history[cur_node_index - 1].data.l_child_ptr =
          avl_history[cur_node_index].header;
    }
  }
}