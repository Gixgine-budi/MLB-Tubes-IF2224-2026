#pragma once
#include <unordered_map>
#include <set>
#include <vector>

enum class NodeType {
    INVALID,
    INTCON,
    REALCON,
    CHARCON,
    STRING,
    NOTSY,
    PLUS,
    MINUS,
    TIMES,
    IDIV,
    RDIV,
    IMOD,
    ANDSY,
    ORSY,
    EQL,
    NEG,
    GTR,
    GEQ,
    LSS,
    LEQ,
    LPARENT,
    RPARENT,
    LBRACK,
    RBRACK,
    COMMA,
    SEMICOLON,
    PERIOD,
    COLON,
    BECOMES,
    CONSTSY,
    TYPESY,
    VARSY,
    FUNCTIONSY,
    PROCEDURESY,
    ARRAYSY,
    RECORDSY,
    PROGRAMSY,
    IDENT,
    BEGINSY,
    IFSY,
    CASESY,
    REPEATSY,
    WHILESY,
    FORSY,
    ENDSY,
    ELSESY,
    UNTILSY,
    OFSY,
    DOSY,
    TOSY,
    DOWNTOSY,
    THENSY,
    COMMENT
};

class DFAGraphNode {
    private:
        std::unordered_map<char, DFAGraphNode*> transitions;
        const NodeType node_type;

    public:
        DFAGraphNode(NodeType node_type);
        virtual ~DFAGraphNode();

        /**
         * @brief Add a transition to the DFA graph
         *        If a transition already exists for the given input character, replace it
         *
         * @param input            The input character
         * @param node             The transition node (must not be null)
         */
        void add_transition(char input, DFAGraphNode* node);

        /**
         * @brief Get the transition object for a given input character
         *        If no transition exists for the given input character, return nullptr
         *
         * @param input            The input character
         * @return DFAGraphNode*   The transition node
         */
        DFAGraphNode* get_transition(char input) const noexcept;

        NodeType get_node_type() const noexcept;
};


class DFAGraph {
    private:
        std::vector<DFAGraphNode*> nodes;
        DFAGraphNode* start_node = nullptr;
        std::set<DFAGraphNode*> final_nodes;
        DFAGraphNode* current_node = nullptr;
    public:
        DFAGraph(std::vector<DFAGraphNode*> nodes, DFAGraphNode* start_node, std::set<DFAGraphNode*> final_nodes);
        DFAGraph();
        virtual ~DFAGraph();

        /**
         * @brief Advance the DFA by consuming one character
         *
         * @param input            The input character
         * @return int             -1 if there is no transition (or the machine was not reset / start is null)
         *                         0 if the state after this transition is an accepting (final) state
         *                         1 if the state after this transition is not accepting
         *
         * @note A state can be accepting and still have outgoing transitions (e.g. longest-match lexing).
         */
        int advance(char input);

        /**
         * @brief Reset the DFA graph to the start node
         */
        void reset() noexcept;

        /**
         * @brief Get the current node of the DFA graph
         *
         * @return DFAGraphNode*   The current node
         */
        DFAGraphNode* get_current_node() const noexcept;
};
