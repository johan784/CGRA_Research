#include "CGRA.hpp"
#include "Graph.hpp"
#include "Scheduler.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {
void require(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(1);
    }
}

const ufm::Placement& placementOf(const ufm::Scheduler& scheduler, ufm::InstrId id) {
    for (const auto& p : scheduler.placements())
        if (p.instr == id) return p;
    std::cerr << "FAIL: missing placement for instruction " << id << '\n';
    std::exit(1);
}

void acyclicRecurrenceEdgeRoutesAndReservesModuloLink() {
    using namespace ufm;
    CGRA cgra(1, 2, false, 4);
    DFG dfg;
    const InstrId a = dfg.addInstruction(OpType::COMPUTE, "A");
    const InstrId b = dfg.addInstruction(OpType::COMPUTE, "B");
    dfg.addEdge(a, b, 1); // A(i) -> B(i+1)

    require(dfg.computeRecMII() == 0, "single recurrence edge has no recurrence cycle");
    Scheduler scheduler(cgra);
    require(scheduler.schedule(dfg), "A(i)->B(i+1) schedules on a two-PE mesh");
    require(scheduler.achievedII() == 1, "resource lower bound permits II=1");

    const Placement& pa = placementOf(scheduler, a);
    const Placement& pb = placementOf(scheduler, b);
    require(pa.pe == 0 && pa.cycle == 0 && pa.slot == 0, "A is placed at PE0, cycle 0");
    require(pb.pe == 1 && pb.cycle == 1 && pb.slot == 0, "B is placed at PE1, cycle 1");
    require(pa.cycle + 1 <= pb.cycle + scheduler.achievedII(),
            "loop-carried timing inequality holds");

    require(scheduler.routes().size() == 1, "one BFS route is emitted for recurrence edge");
    const Route& route = scheduler.routes().front();
    require(route.source == a && route.modCycle == 0, "route uses producer modulo cycle");
    require(route.path.size() == 2 && route.path.front() == pa.pe && route.path.back() == pb.pe,
            "BFS route connects producer PE to consumer PE");
    require(route.newLinks.size() == 1, "adjacent PEs require one reserved link");
    require(scheduler.reservationTable().linkOwner(route.newLinks.front(), route.modCycle) == a,
            "MRT reserves the recurrence link for producer A in slot 0");
    require(scheduler.reservationTable().peOccupant(pa.pe, pa.slot) == a &&
            scheduler.reservationTable().peOccupant(pb.pe, pb.slot) == b,
            "MRT reserves both placed PEs in their modulo slots");
}

void recurrenceCycleContributesToRecMII() {
    using namespace ufm;
    DFG dfg;
    const InstrId a = dfg.addInstruction(OpType::COMPUTE, "A");
    const InstrId b = dfg.addInstruction(OpType::COMPUTE, "B");
    dfg.addEdge(a, b, 0);
    dfg.addEdge(b, a, 1);
    require(dfg.computeRecMII() == 2,
            "two unit-latency operations over distance one require RecMII=2");

    CGRA cgra(1, 2, false, 4);
    Scheduler scheduler(cgra);
    require(scheduler.schedule(dfg), "two-node recurrence cycle is schedulable");
    require(scheduler.achievedII() >= 2, "schedule respects recurrence lower bound");

    const auto& pa = placementOf(scheduler, a);
    const auto& pb = placementOf(scheduler, b);
    require(pa.cycle + 1 <= pb.cycle, "zero-distance A->B timing is satisfied");
    require(pb.cycle + 1 <= pa.cycle + scheduler.achievedII(),
            "loop-carried B->A timing is satisfied");
}
} // namespace

int main() {
    acyclicRecurrenceEdgeRoutesAndReservesModuloLink();
    recurrenceCycleContributesToRecMII();
    std::cout << "All recurrence tests passed.\n";
}
