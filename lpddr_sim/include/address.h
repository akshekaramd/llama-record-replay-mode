#pragma once

#include <iostream>

#include "common.h"

namespace PimSim
{
class Address
{
  public:
    int ch_;
    int ra_;
    int bg_;
    int ba_;
    int ro_;
    int co_;

    int cmd_idx_;
    int reg_type_;
    int reg_idx_;
    int enable_all_rank_ = 0;

    int local_id_[int(Level::MAX)];
    Address(int ch, int ra, int bg, int ba, int ro, int co);
    bool equal(const Address& addr) const;
    bool operator==(const Address& rhs) const
    {
        return (equal(rhs));
    }
    bool operator!=(const Address& rhs) const
    {
        return !(equal(rhs));
    }
};
}  // namespace PimSim
