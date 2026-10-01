#include "Graph.hpp"
#include <algorithm>
#include <functional>
#include <queue>
#include <stdexcept>

namespace ufm {

InstrId DFG::addInstruction(OpType op, const std::string& label) {
    Instruction in;
    in.id = static_cast<InstrId>(instrs_.size());
    in.op = op;
    in.label = label;
    instrs_.push_back(std::move(in));
    return in.id;
}

void DFG::addEdge(InstrId src, InstrId dst, int distance) {
    edges_.push_back(Edge{src, dst, distance});
    instrs_.at(dst).sources.push_back(src);
}

std::vector<InstrId> DFG::topologicalSort() const {
    // [REASONABLE INFERENCE] Recurrence edges (distance > 0) are back-edges of
    // the modulo-scheduled loop and must not constrain the topological order.
    const int n = static_cast<int>(instrs_.size());
    std::vector<int> indeg(n, 0);
    std::vector<std::vector<InstrId>> adj(n);
    for (const Edge& e : edges_) {
        if (e.distance == 0) {
            adj[e.src].push_back(e.dst);
            ++indeg[e.dst];
        }
    }
    // Deterministic Kahn: smallest free id first.
    std::priority_queue<InstrId, std::vector<InstrId>, std::greater<InstrId>> ready;
    for (int i = 0; i < n; ++i)
        if (indeg[i] == 0) ready.push(i);

    std::vector<InstrId> order;
    order.reserve(n);
    while (!ready.empty()) {
        const int u = ready.top(); ready.pop();
        order.push_back(u);
        for (const InstrId v : adj[u])
            if (--indeg[v] == 0) ready.push(v);
    }
    if (order.size() != static_cast<std::size_t>(n))
        throw std::invalid_argument("DFG contains a zero-distance cycle");
    return order;
}

int DFG::computeRecMII() const {
    // [REASONABLE INFERENCE] The paper names RecurrentEdges() but omits the
    // computation; use the standard maximum recurrence-cycle ratio. Enumerate
    // each simple directed cycle once (starting at its smallest instruction
    // id), so a DFS spanning-tree choice cannot hide another cycle.
    const int n = static_cast<int>(instrs_.size());
    std::vector<std::vector<const Edge*>> adj(n);
    for (const Edge& e : edges_) adj[e.src].push_back(&e);

    int recMII = 0;
    std::vector<bool> onPath(n, false);
    for (int start = 0; start < n; ++start) {
        onPath[start] = true;
        std::function<void(int, int, long long)> enumerate =
            [&](int u, int length, long long distance) {
                for (const Edge* e : adj[u]) {
                    const int v = e->dst;
                    const long long cycleDistance = distance + e->distance;
                    if (v == start) {
                        if (cycleDistance > 0) {
                            const long long ratio =
                                (length + 1LL + cycleDistance - 1) / cycleDistance;
                            recMII = std::max(recMII, static_cast<int>(ratio));
                        }
                    } else if (v > start && !onPath[v]) {
                        onPath[v] = true;
                        enumerate(v, length + 1, cycleDistance);
                        onPath[v] = false;
                    }
                }
            };
        enumerate(start, 0, 0);
        onPath[start] = false;
    }
    return recMII;
}

} // namespace ufm
