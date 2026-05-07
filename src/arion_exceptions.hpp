#pragma once

#include <exception>
#include <sstream>
#include <string>

#include "lexer/token.hpp"

class ArionException : public std::exception {
 public:
  ArionException(const std::string& filename) : filename_(filename) {}

  const char* what() const noexcept override { return message_.c_str(); }

 protected:
  const std::string filename_;
  std::string message_;
};

class NoInputException : public ArionException {
 public:
  NoInputException() : ArionException("") {
    message_ = "arion interpreter: fatal error: no input file";
  }
};

class FileErrorException : public ArionException {
 public:
  FileErrorException(const std::string& path, const std::string& message)
      : ArionException(path), error_message_(message) {
    message_ = std::string("file error: cannot open file ")
                   .append(filename_)
                   .append(": ")
                   .append(error_message_);
  }

 private:
  const std::string error_message_;
};

class InvalidTokenException : public ArionException {
 public:
  InvalidTokenException(const std::string& filename, const lexer::Token& token)
      : ArionException(filename), token_(token) {
    message_ = std::string(filename_)
                   .append(":")
                   .append(std::to_string(token_.line_num))
                   .append(":")
                   .append(std::to_string(token_.col_num))
                   .append(": lexer error: ")
                   .append(token_.error_hint().first);
  }

 private:
  const lexer::Token token_;
};

class UnexpectedTokenException : public ArionException {
 public:
  UnexpectedTokenException(const std::string& filename,
                           const lexer::TokenType expected,
                           const lexer::Token unexpected)
      : ArionException(filename), unexpected_(unexpected) {
    std::ostringstream oss;
    oss << lexer::Token{expected, lexer::InvalidType::NotInvalid, "", 0, 0};
    auto exp = oss.str();

    oss.clear();
    oss << unexpected_;
    auto unex = oss.str();

    message_ = std::string(filename_)
                   .append(":")
                   .append(std::to_string(unexpected.line_num))
                   .append(":")
                   .append(std::to_string(unexpected.col_num))
                   .append(": parser error: expected '")
                   .append(exp)
                   .append("' found '")
                   .append(unex)
                   .append("'");
  }

 private:
  lexer::Token unexpected_;
};

class EOFTokenLookupException : public ArionException {
 public:
  EOFTokenLookupException(const std::string& filename)
      : ArionException(filename) {
    message_ =
        std::string(filename_).append(": expected more tokens than given.");
  }
};

class InvalidSyntaxException : public ArionException {};