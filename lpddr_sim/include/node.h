#pragma once

#include <vector>

#include "address.h"
#include "common.h"
#include "storage_t.h"

namespace PimSim
{
struct Node  // channel to bank
{
    int id;
    Level level;
    std::vector<uint64_t> pred;
    std::vector<uint64_t> next;
    int state;
    int opened_row;
    int sync_timeout;  // reserved for LPDDR5
    DRAMMode mode;
    const int CLOSED = -1;

    int pim_pc;
    int last_jump_idx;
    int num_jumps_tobe_taken;
    int last_repeat_idx;
    int num_repeat_tobe_done;

    Node(void)
    {
        state = 0;
        parent = nullptr;
        storage = nullptr;
        sync_timeout = 0;
        opened_row = -1;
        mode = DRAMMode::MAX;
        pim_pc = 0;
        last_jump_idx = -1;
        num_jumps_tobe_taken = -1;
        last_repeat_idx = -1;
        num_repeat_tobe_done = -1;
    }
    void initNode(int cmd_max)
    {
        pred = std::vector<uint64_t>(cmd_max, 0);
        next = std::vector<uint64_t>(cmd_max, 0);
    }

    virtual ~Node(void)
    {
        if (storage != nullptr)
        {
            delete storage;
        }
    }

    Node* parent;
    std::vector<Node*> children;
    storage_t* storage;

    Node* findParent(Level);
    bool traverse(std::function<bool(Node*)>, Level target_level);
    bool traverse(std::function<bool(Node*, int)>, Level target_level, int value);
    void traverse(std::function<void(Node*, Address, burst_t*)>, Level target_level, Address addr,
                  burst_t* data);
    void traverse(std::function<void(Node*, Address, burst_t*, metadata_t*)>, Level target_level,
                  Address addr, burst_t* data, metadata_t* mdata);
    DRAMMode getDRAMMode()
    {
        return findParent(Level::Rank)->mode;
    }
    void setDRAMMode(DRAMMode tobe)
    {
        findParent(Level::Rank)->mode = tobe;
    }
};
}  // namespace PimSim
