#pragma once

#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

#include "diagnoser/diagnoser.hpp"
#include "lexer/token.hpp"
#include "parser/parse_node.hpp"

namespace parser {

using ParsePtr = std::unique_ptr<ParseNode>;

class Parser {
 public:
  /**
   * @brief Construct a new Parser from a list of tokens
   *
   * @param filename original source file name
   * @param tokens reference to a valid list of tokens
   * @param diagnoser reference to the diagnostic accumulator
   */
  Parser(const std::string& filename, const std::vector<lexer::Token>& tokens,
         diag::Diagnoser& diagnoser);

  /**
   * @brief Parser Public API. Parse using recursive descent
   *
   */
  void parse();

  /**
   * @brief Returns the program parse tree root
   *
   * @return const ParseNode& reference to the parse tree root
   */
  const ParseNode& program() { return program_; }

 private:
  const std::string filename_;               //< Original source file name
  const std::vector<lexer::Token>& tokens_;  //< List of token from lexer
  size_t position_ = 0;                      //< Current pointer position
  ParseNode program_;                        //< Parse tree root
  diag::Diagnoser& diagnoser_;               //< Diagnostic accumulator

  /**
   * @brief Checks if last token has consumed
   *
   * @return true true if positition >= tokens.size, false otherwise
   */
  bool is_done() const;

  /**
   * @brief Check what the current token pointed is. On EOF reports a diagnostic
   * and returns a sentinel invalid token.
   *
   * @return const lexer::Token& reference to current token
   */
  const lexer::Token& current() const;

  /**
   * @brief Peek what the next token is. On EOF reports a diagnostic and returns
   * a sentinel invalid token.
   *
   * @param ahead How many position ahead needs to be looked up
   * @return const lexer::Token& reference to peeked token
   */
  const lexer::Token& peek(int ahead = 1) const;

  /**
   * @brief Consume the expected token, increment position, and return a heap
   * allocated ParseNode. On mismatch reports a diagnostic via diagnoser_,
   * advances past the bad token, and returns an Error node.
   *
   * @param expected  The expected token type
   * @param context   Optional human-readable context shown in the error message
   *                  (e.g. "after program name")
   * @return ParsePtr a heap allocated ParseNode
   */
  ParsePtr consume(lexer::TokenType expected, const char* context = nullptr);

  /**
   * @brief Advance past tokens until one of the given stop tokens is found (or
   * EOF). Used after reporting a parse error to resynchronize.
   *
   * @param stops  Set of token types to stop at (inclusive, the stop token
   *               is NOT consumed)
   */
  void sync(std::initializer_list<lexer::TokenType> stops);

  // ---------------------------- parse_header.cpp ----------------------------

  /**
   * @brief program → program-header + declaration-part + compound-statement +
   * period
   *
   */
  void parseProgram();

  /**
   * @brief programsy + ident + semicolon
   *
   * @return ParsePtr
   */
  ParsePtr parseProgramHeader();

  /**
   * @brief (const-declaration)* + (type-declaration)* + (var-declaration)* +
   * (subprogram-declaration)*
   *
   * @return ParsePtr
   */
  ParsePtr parseDeclarationPart();

  /**
   * @brief constsy + (ident + eql + constant + semicolon)+
   *
   * @return ParsePtr
   */
  ParsePtr parseConstDeclaration();

  /**
   * @brief charcon | string | [(plus | minus)? + (ident | intcon | realcon)]
   *
   * @return ParsePtr
   */
  ParsePtr parseConstant();

  /**
   * @brief typesy + (ident + eql + type + semicolon)+
   *
   * @return ParsePtr
   */
  ParsePtr parseTypeDeclaration();

  /**
   * @brief varsy + (identifier-list + colon + type + semicolon)+
   *
   * @return ParsePtr
   */
  ParsePtr parseVarDeclaration();

  /**
   * @brief ident (comma + ident)*
   *
   * @return ParsePtr
   */
  ParsePtr parseIdentifierList();

  /**
   * @brief ident | array-type | range | enumerated | record-type
   *
   * @return ParsePtr
   */
  ParsePtr parseType();

  /**
   * @brief arraysy + lbrack + (range | ident) + rbrack + ofsy + type
   *
   * @return ParsePtr
   */
  ParsePtr parseArrayType();

  /**
   * @brief constant + period + period + constant
   *
   * @return ParsePtr
   */
  ParsePtr parseRange();

  /**
   * @brief lparent + ident + (comma + ident)* + rparent
   *
   * @return ParsePtr
   */
  ParsePtr parseEnumerated();

  /**
   * @brief recordsy + field-list + endsy
   *
   * @return ParsePtr
   */
  ParsePtr parseRecordType();

  /**
   * @brief field-part + (semicolon + field-part)* + semicolon?
   *
   * @return ParsePtr
   */
  ParsePtr parseFieldList();

  /**
   * @brief identifier-list + colon + type
   *
   * @return ParsePtr
   */
  ParsePtr parseFieldPart();

  // -------------------------- parse_function.cpp --------------------------

  /**
   * @brief procedure-declaration | function-declaration
   *
   * @return ParsePtr
   */
  ParsePtr parseSubprogramDeclaration();

  /**
   * @brief proceduresy + ident + (formal-parameter-list)? + semicolon + block +
   * semicolon
   *
   * @return ParsePtr
   */
  ParsePtr parseProcedureDeclaration();

  /**
   * @brief functionsy + ident + (formal-parameter-list)? + colon + ident +
   * semicolon+ block + semicolon
   *
   * @return ParsePtr
   */
  ParsePtr parseFunctionDeclaration();

  /**
   * @brief declaration-part + compound-statement
   *
   * @return ParsePtr
   */
  ParsePtr parseBlock();

  /**
   * @brief lparent + parameter-group + (semicolon + parameter-group)* + rparent
   *
   * @return ParsePtr
   */
  ParsePtr parseFormalParameterList();

  /**
   * @brief identifier-list + colon + (ident | array-type)
   *
   * @return ParsePtr
   */
  ParsePtr parseParameterGroup();

  // -------------------------- parse_statement.cpp --------------------------

  /**
   * @brief beginsy + statement-list + endsy
   *
   * @return ParsePtr
   */
  ParsePtr parseCompoundStatement();

  /**
   * @brief statement (semicolon + statement)*
   *
   * @return ParsePtr
   */
  ParsePtr parseStatementList();

  /**
   * @brief (assignment-statement | if-statement | case-statement |
   * while-statement | repeat-statement | for-statement |
   * procedure/function-call)?
   *
   * @return ParsePtr
   */
  ParsePtr parseStatement();

  /**
   * @brief ident | component-variable
   *
   * @return ParsePtr
   */
  ParsePtr parseVariable();

  /**
   * @brief variable + (lbrack + index-list + rbrack) | (period + ident)
   *
   * @return ParsePtr
   */
  ParsePtr parseComponentVariable();

  /**
   * @brief ( intcon | charcon | ident ) + ( comma + index-list )*
   *
   * @return ParsePtr
   */
  ParsePtr parseIndexList();

  /**
   * @brief variable + becomes + expression
   *
   * @return ParsePtr
   */
  ParsePtr parseAssignmentStatement();

  /**
   * @brief ifsy + expression + thensy + statement + (elsy + statement)?
   *
   * @return ParsePtr
   */
  ParsePtr parseIfStatement();

  /**
   * @brief casesy + expression + ofsy + case-block + endsy
   *
   * @return ParsePtr
   */
  ParsePtr parseCaseStatement();

  /**
   * @brief constant + (comma + constant)* + colon + statement + (semicolon +
   * case-block?)*
   *
   * @return ParsePtr
   */
  ParsePtr parseCaseBlock();

  /**
   * @brief whilesy + expression + dosy + compound-statement
   *
   * @return ParsePtr
   */
  ParsePtr parseWhileStatement();

  /**
   * @brief repeatsy + statement-list + untilsy + expression
   *
   * @return ParsePtr
   */
  ParsePtr parseRepeatStatement();

  /**
   * @brief forsy + ident + becomes + expression + (tosy | downtosy) +
   * expression + dosy + compound-statement
   *
   * @return ParsePtr
   */
  ParsePtr parseForStatement();

  /**
   * @brief ident + (lparent + parameter-list? + rparent)?
   *
   * @return ParsePtr
   */
  ParsePtr parseFunctionCall();

  /**
   * @brief expression (comma + expression)*
   *
   * @return ParsePtr
   */
  ParsePtr parseParameterList();

  // -------------------------- parse_expression.cpp --------------------------

  /**
   * @brief simple-expression (relational-operator + simple-expression)?
   *
   * @return ParsePtr
   */
  ParsePtr parseExpression();

  /**
   * @brief (plus | minus)? term (additive-operator + term)*
   *
   * @return ParsePtr
   */
  ParsePtr parseSimpleExpression();

  /**
   * @brief factor (multiplicative-operator + factor)*
   *
   * @return ParsePtr
   */
  ParsePtr parseTerm();

  /**
   * @brief ident | intcon | realcon | charcon | string | (lparent + expression
   * + rparent) | (notsy + factor) | procedure/function-call | variable
   *
   * @return ParsePtr
   */
  ParsePtr parseFactor();

  /**
   * @brief eql | neq | gtr | geq | lss | leq
   *
   * @return ParsePtr
   */
  ParsePtr parseRelationalOperator();

  /**
   * @brief plus | minus | orsy
   *
   * @return ParsePtr
   */
  ParsePtr parseAdditiveOperator();

  /**
   * @brief times | rdiv | idiv | imod | andsy
   *
   * @return ParsePtr
   */
  ParsePtr parseMultiplicativeOperator();
};

}  // namespace parser