#pragma once
#include <iterator>
#include <utility>
#include <vector>
#include <variant>
#include <memory>
#include <fstream>
#include <iostream>

#include "tokenizer.hpp"
#include "parser.hpp"

class Generator {
public:
    explicit Generator(const NodeProg &node_prog) : m_prog(node_prog) {
        asm_out.open(file_dir, std::ios::out);
    }

    ~Generator() {
        asm_out.close();
    }

    void generate_ident(const NodeIdent &node_ident) {
        asm_out << "    sta %" << node_ident.ident.value << "\n";
        asm_out << "    ldi A, 0\n";
    }

    void generate_int_lit(const NodeIntLit &node_int_lit) {
        asm_out << "    ldi A " << node_int_lit.int_.value << "\n";
    }

    void generate_term(const std::unique_ptr<NodeTerm> &node_term) {
        if (std::holds_alternative<std::unique_ptr<NodeIdent>>(node_term->value)) {
            generate_ident(*std::get<std::unique_ptr<NodeIdent>>(node_term->value));
        } else if (std::holds_alternative<std::unique_ptr<NodeIntLit>>(node_term->value)) {
            generate_int_lit(*std::get<std::unique_ptr<NodeIntLit>>(node_term->value));
        } else {
            std::cerr << "Invalid term type!" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    void generate_expr(const std::unique_ptr<NodeExpr> &node_expr) {
        generate_term(node_expr->value);
    }

    void generate_exit(const std::unique_ptr<NodeStmtExit> &node_stmt_exit) {
        generate_expr(node_stmt_exit->expr);
        asm_out << "\n    lda %r\n    out 0\n";
        asm_out << "    hlt\n";
    }

    void generate_stmt_int(const std::unique_ptr<NodeStmtInt> &node_stmt_int) {
        generate_expr(node_stmt_int->value);
        generate_ident(*node_stmt_int->ident);
    }

    void generate() {
        asm_out << ".text\n\nstart:\n";

        for (const auto &stmt : m_prog.stmts) {
            if (std::holds_alternative<std::unique_ptr<NodeStmtExit>>(stmt->stmt)) {
                generate_exit(std::get<std::unique_ptr<NodeStmtExit>>(stmt->stmt));
            } else if (std::holds_alternative<std::unique_ptr<NodeStmtInt>>(stmt->stmt)) {
                generate_stmt_int(std::get<std::unique_ptr<NodeStmtInt>>(stmt->stmt));
            } else {
                std::cerr << "unrecognized stmt type" << std::endl;
                exit(EXIT_FAILURE);
            }
        }

        asm_out << "\n.data\n\n";
        asm_out << "r = 0" << std::endl;
    }

private:
    const std::string file_dir = "C:/Code/C++/Compiler/SimpleLang/8bit-computer/tests/asm_code.asm";
    std::ofstream asm_out;
    const NodeProg &m_prog;
};
