/**
@file interconnect.tpp
@brief Implementation of the templated Interconnect class.
*/
#pragma once

template <typename ArbiterT>
Interconnect<ArbiterT>::Interconnect(int nodes, const std::string& routing)
    : total_nodes(nodes), routing_algo(routing) {
    assert(nodes > 0 && "Total nodes must be positive");
}

template <typename ArbiterT>
Router<ArbiterT>* Interconnect<ArbiterT>::get_router(int nodeId) {
    if (nodeId < 0 || nodeId >= total_nodes) return nullptr;
    if (nodeId >= static_cast<int>(routers_.size())) return nullptr;
    return routers_[nodeId].get();
}

template <typename ArbiterT>
SimStats Interconnect<ArbiterT>::run(int cycles) {
    build();
    SimStats stats;
    stats.total_cycles = cycles;

    for (int i = 0; i < cycles; ++i) {
        on_clock();
        // Here you can add a call to a virtual method collectStepStats(stats);
    }

    return stats;
}
