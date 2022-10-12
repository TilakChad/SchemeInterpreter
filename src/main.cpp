import Lexer;
import Parser;
import Eval;
import Log;

#include "../include/types.hpp"
#include <iostream>;
#include <fstream>
#include <sstream>

int main(int argc, char **argv)
{
    // Tokenizer     tokenizer((const u8 *)"(+ 3 (+ 1 2 3) (+ 3 2 1) (- 4 3) ) $", 0, 40);
    std::ifstream src_code("./lisp/test.lisp", std::ios::binary | std::ios::in);

    if (src_code.is_open())
    {
    }
    else
    {
        GetSingletonLogger().Log("Failed  to open file :  ", "../lisp/test.lisp", __LINE__);
    }

    // Token     next = tokenizer.next();
    // auto      z    = Parse::ParseAs<i16>(next);
    // auto      y    = Parse::ParseAs<u32>(tokenizer.next());
    // std::cout << z << y << std::endl;
    std::string code;
    {
        std::stringstream stream;
        stream << src_code.rdbuf();
        code = stream.str();
        code.push_back('$');
    }

    Parser                  parser;
    Tokenizer tokenizer((const u8 *)code.data(), 0, code.size()); 

    parser.ParseStart(tokenizer);
    // Eval::InternDataType a1 = {Eval::DataTypeTag::Int, 50};
    // Eval::InternDataType a2 = {Eval::DataTypeTag::Int, 6};
    // auto                 result = Eval::ApplyOpIntern(a1, a2, [](i32 x, i32 y) { return x + y;
    // },Eval::DataTypeTag::Int); std::cout << "Result : " << result.data.integer;
    std::cout << "Result of Evaluation : " << parser.EvalAST().data.integer << std::endl;

    std::cout << GetSingletonLogger().dump();
    return 0;
}