#pragma once
// ============================================================================
// Scheduler.hpp - Diagonal CGRA Scheduling (Algorithm 1 of the paper).
// ============================================================================
#include "CGRA.hpp"
#include "Graph.hpp"
#include "ReservationTable.hpp"
#include "Routing.hpp"
#include <vector>

namespace ufm {

// Final placement of one instruction.
struct Placement {
    InstrId instr = -1;
    PEId    pe    = -1;
    Cycle   cycle = -1;   // Absolute cycle in the first iteration (diagonal).
    int     slot  = -1;   // cycle % II (configuration-memory slot of the PE).
};

class Scheduler {
public:
    explicit Scheduler(const CGRA& cgra) : cgra_(cgra) {}

    // [PAPER-DERIVED] Algorithm 1: for II = MinII..MaxII, greedily place
    // instructions in topological order on the pre-determined diagonal PE
    // order; BFS-route every dependency; retry the next slot on routing
    // failure; increment II and restart on exhaustion.
    bool schedule(const DFG& dfg);

    int achievedII() const { return achievedII_; }
    const std::vector<Placement>& placements() const { return placements_; }
    const std::vector<Route>&     routes()     const { return routes_; }
    const ReservationTable& reservationTable() const { return rt_; }

private:
    const CGRA& cgra_;
    ReservationTable rt_;
    int achievedII_ = 0;
    std::vector<Placement> placements_;
    std::vector<Route>     routes_;
};

} // namespace ufm
