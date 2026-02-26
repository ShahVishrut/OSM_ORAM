#include "client.h"

#include <cmath>
#include <cstring>
#include <random>
#include <stdexcept>

Client::Client(size_t num_blocks, size_t blocks_per_bucket) {
  if (num_blocks > UINT32_MAX) {
    throw std::invalid_argument("Too many blocks!");
  }
  this->num_blocks = num_blocks;
  this->tree_height = std::ceil(std::log2(this->num_blocks));
  this->blocks_per_bucket = blocks_per_bucket;
  this->bucket_size_bytes = this->block_size_bytes * this->blocks_per_bucket;

  this->server = new Server(this->bucket_size_bytes, (1u << (tree_height + 1)));

  stash.resize(3 * (tree_height + 1));
  stash_full.resize(stash.size());

  ORAMBlock dummy;
  std::vector<uint8_t> data(bucket_size_bytes);
  for (size_t j = 0; j < blocks_per_bucket; j++) {
    std::memcpy(&data[j * block_size_bytes], &dummy, block_size_bytes);
  }
  for (size_t i = 1; i < (1u << (tree_height + 1)); i++) {
    server->write_buckets({i}, data);
  }
}

ORAMBlock Client::write_block(ORAMBlock to_write, bool write) {
  uint32_t initial_leaf_num = to_write.header.leaf_label;
  read_block_path(initial_leaf_num);

  std::random_device rd;
  std::mt19937 gen(rd());

  std::uniform_int_distribution<> distr(0, (1u << tree_height) - 1);

  bool block_found = false;
  for (size_t i = 0; i < stash_full.size(); i++) {
    if (stash_full[i]) {
      if (stash[i].header.block_id == to_write.header.block_id) {
        block_found = true;
        if (write) {
          stash[i] = to_write;
        }
        stash[i].header.leaf_label = distr(gen);
        to_write = stash[i];
      }
    }
  }
  if (!block_found) {
    if (!write) {
      to_write.header.is_null = true;
    } else {
      for (size_t i = 0; i < stash_full.size(); i++) {
        if (!stash_full[i]) {
          stash[i] = to_write;
          stash[i].header.leaf_label = distr(gen);
          stash_full[i] = true;
          to_write = stash[i];
          break;
        }
      }
    }
  }
  evict(initial_leaf_num);
  return to_write;
}

void Client::read_block_path(uint32_t leaf_num) {
  std::vector<size_t> index_list;
  size_t leaf_path = (1u << tree_height) + leaf_num;
  while (leaf_path > 0) {
    index_list.push_back(leaf_path);
    leaf_path /= 2;
  }
  std::vector<uint8_t> temp = server->read_buckets(index_list);
  size_t stash_entry = 0;
  for (int cur_bucket = 0; cur_bucket < index_list.size(); cur_bucket++) {
    for (int cur_block = 0; cur_block < blocks_per_bucket; cur_block++) {
      ORAMBlock temp_block;
      std::memcpy(&temp_block,
                  &temp[(cur_bucket * blocks_per_bucket + cur_block) *
                        block_size_bytes],
                  sizeof(ORAMBlock));
      if (temp_block.header.is_null) {
        continue;
      }
      while (stash_entry < stash.size() && stash_full[stash_entry]) {
        stash_entry++;
      }
      if (stash_entry < stash.size()) {
        stash[stash_entry] = temp_block;
        stash_full[stash_entry] = true;
      } else {
        throw std::runtime_error("Stash overflow");
      }
    }
  }
}

bool Client::is_on_path(size_t index, size_t leaf_num) {
  size_t leaf_path = (1u << tree_height) + leaf_num;
  while (leaf_path >= index && leaf_path > 0) {
    if (leaf_path == index) {
      return true;
    }
    leaf_path /= 2;
  }
  return false;
}

void Client::evict(uint32_t leaf_num) {
  std::vector<size_t> index_list;
  std::vector<uint8_t> data((tree_height + 1) * bucket_size_bytes);
  size_t leaf_path = (1u << tree_height) + leaf_num;

  while (leaf_path > 0) {
    index_list.push_back(leaf_path);
    leaf_path /= 2;
  }
  for (int i = 0; i < index_list.size(); i++) {
    size_t cur_block_in_bucket = 0;
    for (int j = 0;
         j < stash_full.size() && cur_block_in_bucket < blocks_per_bucket;
         j++) {
      if (stash_full[j]) {
        uint32_t mapped_to_leaf = stash[j].header.leaf_label;
        if (is_on_path(index_list[i], mapped_to_leaf)) {
          std::memcpy(&data[(i * blocks_per_bucket + cur_block_in_bucket) *
                            block_size_bytes],
                      &stash[j], block_size_bytes);
          stash_full[j] = false;
          cur_block_in_bucket++;
        }
      }
    }
    while (cur_block_in_bucket < blocks_per_bucket) {
      ORAMBlock dummy;
      std::memcpy(&data[(i * blocks_per_bucket + cur_block_in_bucket) *
                        block_size_bytes],
                  &dummy, block_size_bytes);
      cur_block_in_bucket++;
    }
  }
  server->write_buckets(index_list, data);
}

uint32_t Client::next_available_block_id() { return cur_block_id++; }