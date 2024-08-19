#pragma once

#include <list>
#include <vector>

#include "config.h"
#include "dram.h"

namespace PimSim
{
class DRAMController
{
  public:
    DRAMController(Config* conf, int command_max, std::function<void(uint64_t)> read_callback,
                   std::function<void(uint64_t)> write_callback);
    virtual ~DRAMController(void);

    std::vector<std::function<int(Node*, Address, metadata_t*)>> prereq_;
    std::vector<int> stiming_;

    std::function<void(uint64_t req_id)> read_callback_, write_callback_;
    void registerCallBack(std::function<void(uint64_t)> read_callback,
                          std::function<void(uint64_t)> write_callback);

    void initAll(void);
    void attachDRAM(DRAM* dram);
    int decode(int cmd, Address addr, Node* node, metadata_t*);
    bool check(int cmd, Address addr, Node* node);
    virtual void tick(int ch) = 0;
    void tick(void);
    void addTransaction(Request req);
    void addBarrierToLastReq(int ch);
    bool empty(void);
    bool empty(int);
    void runUntilEmpty(void);
    virtual void initPrereq(void) = 0;
    virtual void initStiming(void) = 0;
    virtual int offset(int a, int b, int c) = 0;
    int cmd_max_;
    Config* conf_;
    std::list<Request>* queue_;
    std::list<Request>* pending_rd_queue_;
    DRAM* dram_;
    std::vector<int> num_trans_;
    AddressGen* add_trans_;
};
}  // namespace PimSim
