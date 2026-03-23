#include "dfa_graph.hpp"

#include <cassert>
#include <stdexcept>

DFAGraphNode::DFAGraphNode(NodeType node_type) : node_type(node_type) {}

DFAGraphNode::~DFAGraphNode() {}

void DFAGraphNode::add_transition(char input, DFAGraphNode* node) {
    if (node == nullptr) {
        throw std::invalid_argument("DFAGraphNode::add_transition: target node must not be null");
    }
    transitions[input] = node;
}

DFAGraphNode* DFAGraphNode::get_transition(char input) const noexcept {
    const auto it = transitions.find(input);
    if (it == transitions.end()) {
        return nullptr;
    }
    assert(it->second != nullptr && "transition map must not store null (use add_transition)");
    return it->second;
}

NodeType DFAGraphNode::get_node_type() const noexcept {
    return node_type;
}

DFAGraph::DFAGraph(std::vector<DFAGraphNode*> nodes, DFAGraphNode* start_node, std::set<DFAGraphNode*> final_nodes)
    : nodes(std::move(nodes)), start_node(start_node), final_nodes(std::move(final_nodes)), current_node(start_node) {
    if (start_node == nullptr) {
        throw std::invalid_argument("DFAGraph: start_node must not be null");
    }
}

DFAGraph::DFAGraph() {
    // hardcoded input goes here
    // TODO: implement DFA graph for Arion
    start_node = nullptr;
    current_node = nullptr;
}

DFAGraph::~DFAGraph() {
    for (DFAGraphNode* node : nodes) {
        if (node != nullptr) {
            delete node;
        }
    }
}

int DFAGraph::advance(char input) {
    if (current_node == nullptr) {
        return -1;
    }
    current_node = current_node->get_transition(input);
    if (current_node == nullptr) {
        return -1;
    }
    if (final_nodes.find(current_node) != final_nodes.end()) {
        return 0;
    }
    return 1;
}

void DFAGraph::reset() noexcept {
    current_node = start_node;
}

DFAGraphNode* DFAGraph::get_current_node() const noexcept {
    return current_node;
}
