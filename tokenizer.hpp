#pragma once

#include <string>
#include <utility>
#include <vector>
#include <fstream>

enum TokenType {
    return_,
    open_paren,
    close_paren,
    semicolon,
    INT,
    ident,
    int_,
    assign,
};

struct Token {
    TokenType type;
    std::string value;
};

class Tokenizer {
public:
    explicit Tokenizer(std::string src_location): file_source(std::move(src_location)) {
    }

    [[nodiscard]] std::vector<Token> tokenize() const {
        std::vector<Token> tokens;
        std::ifstream file(file_source);
        char ch;
        std::string buffer;
        while (file.get(ch)) {
            if (isdigit(ch)) {
                buffer += ch;
                while (file.get(ch) && isdigit(ch)) {
                    buffer += ch;
                }
                tokens.push_back(Token{TokenType::INT, buffer});
                file.unget();
                buffer.clear();
            } else if (isalpha(ch)) {
                buffer += ch;
                while (file.get(ch) && isalpha(ch)) {
                    buffer += ch;
                }
                if (buffer == "exit") {
                    tokens.push_back(Token{TokenType::return_, buffer});
                } else if (buffer == "int") {
                    tokens.push_back(Token{TokenType::int_, buffer});
                } else {
                    tokens.push_back({TokenType::ident, buffer});
                }
                file.unget();
                buffer.clear();
            } else if (ch == '(') {
                tokens.push_back({TokenType::open_paren, "("});
            } else if (ch == ')') {
                tokens.push_back({TokenType::close_paren, ")"});
            } else if (ch == ';') {
                tokens.push_back({TokenType::semicolon, ";"});
            } else if (ch == '=') {
                tokens.push_back({TokenType::assign, "="});
            }
        }
        return tokens;
    }

private:
    std::vector<Token> tokens;
    const std::string file_source;
};
