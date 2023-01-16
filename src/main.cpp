import Lexer;
import Parser;
import Eval;
import Log;
import Lib;

#include "../include/types.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <format>
#include <malloc.h>

#if defined(_MSC_VER) || defined(_WIN32)
#include <Windows.h>
#include <Psapi.h>

#endif
// void TestList()
//{
//     lib::List<u32> lst;
//     lst.AddNode(1);
//     lst.AddNode(3);
//     lst.AddNode(6);
//     lst.AddNode(8);
//
//     auto get_node = lst.SearchNode(6);
//     lst.RemoveNode(get_node);
//     lst.RemoveNode(8);
//     std::cout << lst << std::endl;
// }

int main(int argc, char **argv)
{
    /*Tokenizer   tokenizer((const u8 *)"(+ 3 (+ 1 2 3) (+ 3 2 1) (- 4 3) ) $", 0, 40);*/
    const char *default_file = "./lisp/cons.lisp";
    if (argc == 2)
        default_file = argv[1];
    std::ifstream src_code(default_file, std::ios::binary | std::ios::in);

    // This one's not good but meh
    srand(time(NULL));
    if (!src_code)
    {
        GetSingletonLogger().Log("Failed  to open file :  ", default_file, __LINE__, "   ");
    }

    std::string code;
    {
        std::stringstream stream;
        stream << src_code.rdbuf();
        code = stream.str();
        code.push_back('$');
    }

    Parser               parser;
    Tokenizer            tokenizer((const u8 *)code.data(), 0, (u32)code.size());

    Eval::InternDataType re = {Eval::DataTypeTag::None};
    while (true)
    {
        if (!parser.ParseStart(tokenizer))
            break;
        re = parser.EvalAST();
    }

    // TODO :: Cleanup memory from both symbol table and ast
    parser.DestroyAST();

    // Run garbage collector for the last time
    Eval::RunGarbageCollector();

    auto valprint = [&](auto &x) -> auto
    {
        auto print = [&](auto &x) {
            if (x.tag == Eval::DataTypeTag::Int)
                std::cout << x.data.integer;
            else if (x.tag == Eval::DataTypeTag::Real)
                std::cout << x.data.real;
        };
        print(x);
        if (x.tag == Eval::DataTypeTag::Cons)
        {
            std::cout << "( ";
            print(x.data.cons->car);
            std::cout << " , ";
            print(x.data.cons->cdr);
            std::cout << " )\n";
        }
    };

    std::cout << GetSingletonLogger().dump() << std::endl;

    std::cout << "Result of Evaluation : ";
    valprint(re);

#if defined(_MSC_VER) || defined(_WIN32)
    /*    PROCESS_MEMORY_COUNTERS info = {};
    GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
    std::cout << "Outputting process memory usage : " << std::endl;
    std::cout << std::format("Peak working : {} \n Active Working : {}\n\n", info.PeakPagefileUsage,
                info.PeakWorkingSetSize);
                */

    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
    // _CrtDumpMemoryLeaks();
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    return 0;
}