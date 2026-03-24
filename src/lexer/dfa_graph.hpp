#pragma once
#include "char_machine.hpp"
#include <string>
#include <unordered_map>
#include <vector>

enum class NodeType {
    END_OF_FILE,
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

struct Token {
    NodeType type;
    std::string lexeme;
};

class DFAGraphNode {
    private:
        std::unordered_map<char, DFAGraphNode*> transitions;
        const NodeType node_type;
        const bool final_state;

    public:
        DFAGraphNode(NodeType node_type, bool is_final = false);
        ~DFAGraphNode();

        DFAGraphNode(const DFAGraphNode&) = delete;
        DFAGraphNode& operator=(const DFAGraphNode&) = delete;

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
        bool is_final() const noexcept;
};


class DFAGraph {
    private:
        std::vector<DFAGraphNode*> nodes;

        DFAGraphNode* start_node = nullptr;
        DFAGraphNode* current_node = nullptr;

        CharMachine* char_machine = nullptr; // non-owning

    public:
        DFAGraph(std::vector<DFAGraphNode*> nodes, DFAGraphNode* start_node, CharMachine* char_machine);
        DFAGraph(CharMachine* char_machine);
        ~DFAGraph();

        DFAGraph(const DFAGraph&) = delete;
        DFAGraph& operator=(const DFAGraph&) = delete;

        /**
         * @brief Advance the DFA by consuming one character from the character machine.
         *        On a successful transition, the character machine is advanced.
         *        On failure (no transition or EOF), the character machine and current node are unchanged.
         *
         * @return int             -2 if the machine is at the end of the input (nothing consumed)
         *                         -1 if there is no transition for the current character (nothing consumed)
         *                         0  if the state after this transition is an accepting (final) state
         *                         1  if the state after this transition is not accepting
         *
         * @note A state can be accepting and still have outgoing transitions (e.g. longest-match lexing).
         */
        int adv();

        /**
         * @brief Determine the next token by advancing through DFA transitions from the current input position.
         *        Collects the matched lexeme and returns it along with the token type.
         *        Returns END_OF_FILE immediately if the character machine is already at EOF.
         *        Resets the DFA to the start node before returning.
         *
         * @return Token           The recognized token (type + lexeme), or END_OF_FILE at input end
         */
        Token next_token();

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
