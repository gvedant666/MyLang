#include <fstream>
#include <iostream>
#include <string>

#include "asm_generator.hpp"
#include "parser.hpp"
#include "tokenizer.hpp"


int main() {
    const std::string filePath = "C:/Code/C++/Compiler/SimpleLang/text.sl";
    std::ifstream file(filePath, std::ios::in);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return 1;
    }
    std::vector<Token> tokens = Tokenizer(filePath).tokenize();
    if (file.is_open()) {
        file.close();
    }
    Parser parser(tokens);
    NodeProg node_prog = parser.parse_program().value();
    std::cout << node_prog.stmts.size() << std::endl;
    for (auto& node : node_prog.stmts) {
        std::cout << node->stmt_type << std::endl;
    }
    Generator generator(node_prog);
    generator.generate();

    std::cout << node_prog.stmts.size() << std::endl;

    //mingw32-make clean && mingw32-make run
}
