#include "dfa_graph.hpp"

#include <cassert>
#include <stdexcept>

DFAGraphNode::DFAGraphNode(NodeType node_type, bool is_final)
    : node_type(node_type), final_state(is_final) {}

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

bool DFAGraphNode::is_final() const noexcept {
    return final_state;
}

DFAGraph::DFAGraph(std::vector<DFAGraphNode*> nodes, DFAGraphNode* start_node, CharMachine* char_machine)
    : nodes(std::move(nodes)), start_node(start_node), current_node(start_node), char_machine(char_machine) {
    if (start_node == nullptr) {
        throw std::invalid_argument("DFAGraph: start_node must not be null");
    }
}

DFAGraph::DFAGraph(CharMachine* char_machine) : char_machine(char_machine) {
    // TODO: implement DFA graph for Arion
    start_node = nullptr;
    current_node = nullptr;
}

DFAGraph::~DFAGraph() {
    for (DFAGraphNode* node : nodes) {
        delete node;
    }
}

int DFAGraph::adv() {
    if (current_node == nullptr || char_machine == nullptr) {
        throw std::runtime_error("DFAGraph::adv: DFA is not initialized");
    }
    if (char_machine->is_eof()) {
        return -2;
    }
    const char c = char_machine->curr();
    DFAGraphNode* next = current_node->get_transition(c);
    if (next == nullptr) {
        return -1;
    }
    current_node = next;
    char_machine->adv();
    return current_node->is_final() ? 0 : 1;
}

Token DFAGraph::next_token() {
    if (current_node == nullptr) {
        throw std::runtime_error("DFAGraph::next_token: DFA is not initialized");
    }
    if (char_machine->is_eof()) {
        return {NodeType::END_OF_FILE, ""};
    }

    std::string lexeme;

    while (!char_machine->is_eof()) {
        char c = char_machine->curr();
        int result = adv();
        if (result < 0) break;
        lexeme += c;
    }

    NodeType type = current_node->is_final()
        ? current_node->get_node_type()
        : NodeType::INVALID;
    reset();
    return {type, lexeme};
}

void DFAGraph::reset() noexcept {
    current_node = start_node;
}

DFAGraphNode* DFAGraph::get_current_node() const noexcept {
    return current_node;
}
