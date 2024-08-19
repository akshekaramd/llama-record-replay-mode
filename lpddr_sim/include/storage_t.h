#pragma once

#include <memory>
#include <unordered_map>
#include <utility>

#include "burst_t.h"

namespace PimSim
{
class row_t
{
  public:
    burst_t* col_;
    row_t(int num_col)
    {
        col_ = new burst_t[num_col];
    }
    virtual ~row_t(void)
    {
        delete[] col_;
    }
};

class storage_t
{
  public:
    std::unordered_map<int, row_t*> cells_;
    int num_col_;
    storage_t(int num_col) : num_col_(num_col) {}

    virtual ~storage_t(void)
    {
        for (const auto& e : cells_)
        {
            delete e.second;
        }
    }

#ifndef NO_STORAGE
    void read(int ro, int co, burst_t* bst)
    {
        if (!bst) return;
        if (!cells_.count(ro))
        {
            cells_.insert(std::make_pair(ro, new row_t(num_col_)));
        }
        memcpy(bst, &(cells_[ro]->col_[co]), sizeof(burst_t));
    }

    void read32(int ro, int co, burst_t* bst)
    {
        if (!bst) return;
        if (!cells_.count(ro))
        {
            cells_.insert(std::make_pair(ro, new row_t(num_col_)));
        }
        memcpy(bst, &(cells_[ro]->col_[co]), 2 * sizeof(burst_t));
    }

    void write(int ro, int co, burst_t* bst)
    {
        if (!bst) return;
        if (!cells_.count(ro))
        {
            cells_.insert(std::make_pair(ro, new row_t(num_col_)));
        }
        memcpy(&(cells_[ro]->col_[co]), bst, sizeof(burst_t));
    };

    void write32(int ro, int co, burst_t* bst)
    {
        if (!bst) return;
        if (!cells_.count(ro))
        {
            cells_.insert(std::make_pair(ro, new row_t(num_col_)));
        }
        memcpy(&(cells_[ro]->col_[co]), bst, 2 * sizeof(burst_t));
    }

    void maskedWrite(int ro, int co, burst_t* bst, metadata_t* metadata)
    {
        if (!bst) return;
        if (!cells_.count(ro))
        {
            cells_.insert(std::make_pair(ro, new row_t(num_col_)));
        }
        // write if all masks are set 1
        if (metadata->u32_data_ == 0xffffffff)
        {
            memcpy(&(cells_[ro]->col_[co]), bst, sizeof(burst_t));
            return;
        }

        std::bitset<32> mask(metadata->u32_data_);
        for (int i = 0; i < 32; i++)
        {
            if (mask[i])
            {
                memcpy(&(cells_[ro]->col_[co].u8_data_[i]), &(bst->u8_data_[i]), sizeof(uint8_t));
            }
        }
    };
#else
    void read(int ro, int co, burst_t* bst) {}
    void read32(int ro, int co, burst_t* bst) {}
    void write(int ro, int co, burst_t* bst)
    {
        if (!bst) return;
        if (!cells_.count(ro))
        {
            cells_.insert(std::make_pair(ro, new row_t(num_col_)));
        }
        memcpy(&(cells_[ro]->col_[co]), bst, sizeof(burst_t));
    };
    void read_inst(int ro, int co, burst_t* bst)
    {
        if (!bst) return;
        if (!cells_.count(ro))
        {
            cells_.insert(std::make_pair(ro, new row_t(num_col_)));
        }
        memcpy(bst, &(cells_[ro]->col_[co]), sizeof(burst_t));
    };

    void write32(int ro, int co, burst_t* bst) {}
    void maskedWrite(int ro, int co, burst_t* bst, metadata_t* metadata) {}
#endif
};
}  // namespace PimSim
