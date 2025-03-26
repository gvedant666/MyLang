#pragma once

#include <iostream>
#include <memory>
#include <optional>
#include <utility>
#include <variant>

#include "tokenizer.hpp"

enum StmtType {
    Exit,
    Ident,
};

struct NodeIdent {
    Token ident;
    Token Int;
};

struct NodeIntLit {
    Token int_;
};

struct NodeTerm {
    std::variant<std::unique_ptr<NodeIdent>, std::unique_ptr<NodeIntLit>> value;
};

struct NodeExpr {
    std::unique_ptr<NodeTerm> value;
};

struct NodeStmtExit {
    std::unique_ptr<NodeExpr> expr;
};

struct NodeStmtInt {
    std::unique_ptr<NodeIdent> ident;
    std::unique_ptr<NodeExpr> value;
};

struct NodeStmt {
    std::variant<std::unique_ptr<NodeIdent>, std::unique_ptr<NodeStmtExit>, std::unique_ptr<NodeStmtInt>> stmt;
    StmtType stmt_type;
};

struct NodeProg {
    std::vector<std::unique_ptr<NodeStmt>> stmts;
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens) : tokens(std::move(tokens)), index(0) {}

    std::optional<std::unique_ptr<NodeTerm>> parse_term() {

        if (get_token_type(index) == TokenType::INT) {
            const Token token = get_token();
            index++;
            auto node_int_lit = std::make_unique<NodeIntLit>(NodeIntLit{token});
            return std::make_unique<NodeTerm>(NodeTerm{std::move(node_int_lit)});
        }

        std::cerr << "parse_term_fail" << std::endl;
        return std::nullopt;
    }

    std::optional<std::unique_ptr<NodeExpr>> parse_expr() {
        if (auto node_term = parse_term()) {
            return std::make_unique<NodeExpr>(NodeExpr{std::move(node_term.value())});
        }

        std::cerr << "parse_expr_fail" << std::endl;
        return std::nullopt;
    }

    std::optional<std::unique_ptr<NodeStmt>> parse_stmt() {

        if (get_token_type(index) == TokenType::return_ &&
            index + 1 < tokens.size() && get_token_type(index + 1) == TokenType::open_paren) {
            index += 2;
            auto expr = parse_expr();
            if (!expr) return std::nullopt;

            index += 2;
            auto exit_stmt = std::make_unique<NodeStmtExit>(NodeStmtExit{std::move(expr.value())});
            return std::make_unique<NodeStmt>(NodeStmt{std::move(exit_stmt), StmtType::Exit});
        }

        if (get_token_type(index) == TokenType::int_ &&
            index + 1 < tokens.size() && get_token_type(index + 1) == TokenType::ident) {
            index++;
            Token token = get_token();
            auto ident = std::make_unique<NodeIdent>(NodeIdent{token});
            index++;

            if (get_token_type(index) != TokenType::assign) {
                std::cerr << "expecting assignment sign '=' " << std::endl;
                return std::nullopt;
            }
            index++;

            auto expr = parse_expr();
            if (!expr) return std::nullopt;

            if (get_token_type(index) == TokenType::semicolon) {
                index++;
            } else {
                std::cerr << "expecting ';' " << std::endl;
                return std::nullopt;
            }

            auto int_stmt = std::make_unique<NodeStmtInt>(NodeStmtInt{std::move(ident), std::move(expr.value())});
            return std::make_unique<NodeStmt>(NodeStmt{std::move(int_stmt), StmtType::Ident});
        }

        std::cerr << "parse_stmt_fail" << std::endl;
        return std::nullopt;
    }

    std::optional<NodeProg> parse_program() {
        NodeProg prog;

        while (index < tokens.size()) {
            if (auto stmt = parse_stmt()) {
                prog.stmts.push_back(std::move(stmt.value()));
            } else {
                std::cerr << "parse_prog_fail" << std::endl;
                return std::nullopt;
            }
        }

        return prog;
    }

private:
    std::vector<Token> tokens;
    size_t index;

    [[nodiscard]] TokenType get_token_type(const size_t index) const {
        if (index < tokens.size()) {
            return tokens[index].type;
        } else {
            std::cerr << "get_token_type fail" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    [[nodiscard]] Token get_token() const {
        if (index < tokens.size()) {
            return tokens[index];
        }
        std::cerr << "no more tokens" << std::endl;
        exit(EXIT_FAILURE);
    }
};
