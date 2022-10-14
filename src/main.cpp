import Lexer;
import Parser;
import Eval;
import Log;
import Lib;

#include "../include/types.hpp"
#include <iostream>;
#include <fstream>
#include <sstream>

void TestList()
{
    lib::List<u32> lst;
    lst.AddNode(1);
    lst.AddNode(3);
    lst.AddNode(6);
    lst.AddNode(8);

    auto get_node = lst.SearchNode(6);
    lst.RemoveNode(get_node);
    lst.RemoveNode(8);
    std::cout << lst << std::endl;
}

int main(int argc, char **argv)
{
    // Tokenizer     tokenizer((const u8 *)"(+ 3 (+ 1 2 3) (+ 3 2 1) (- 4 3) ) $", 0, 40);
    std::ifstream src_code("./lisp/test.lisp", std::ios::binary | std::ios::in);

    if (!src_code)
    {
        GetSingletonLogger().Log("Failed  to open file :  ", "../lisp/test.lisp ", __LINE__, "   ");
    }

    std::string code;
    {
        std::stringstream stream;
        stream << src_code.rdbuf();
        code = stream.str();
        code.push_back('$');
    }

    Parser        parser;
    Tokenizer     tokenizer((const u8 *)code.data(), 0, code.size());

    constexpr u32 N = 3;

    for (auto i = 0; i < N; ++i)
    {
        parser.ParseStart(tokenizer);
        parser.EvalAST();
    }

    parser.ParseStart(tokenizer);

    auto re = parser.EvalAST();
    std::cout << "Result of Evaluation : " << re.data.integer << std::endl;

    std::cout << GetSingletonLogger().dump() << std::endl;
    return 0;
}