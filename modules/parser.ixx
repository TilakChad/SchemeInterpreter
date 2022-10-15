import Eval;
import Lexer;

#include "../include/types.hpp"
#include "../include/macros.hpp"

#include <variant>
#include <new>
// #include <memory>

export module Parser;

export class Parser
{
    Eval::Expression *ast;

  public:
    void ParseStart(Tokenizer &tokenizer)
    {
        auto next = tokenizer.next();
        Assert(next.type == TokenType::OParen);
        ast = CreateExpressionTree(tokenizer);
    }

    Eval::InternDataType EvalAST()
    {
        return Eval::EvaluateExpressionTree(ast);
    }

    void ParseList(Tokenizer &tokenizer)
    {
        // Keep parsing until closing brace is encountered.
    }

    Eval::Expression *CreateExpressionTree(Tokenizer &tokenizer)
    {
        Eval::Expression *expr  = new Eval::Expression();

        Token             token = tokenizer.next(); // This should be a function

        expr->op.begin          = token.ptrs.begin;
        expr->op.end            = token.ptrs.end;

        if (token.type != TokenType::OParen)
            token = tokenizer.next();

        while (token.type != TokenType::CParen && token.type != TokenType::End)
        {
            if (token.type == TokenType::OParen)
            {
                expr->childs.push_back(CreateExpressionTree(tokenizer));
            }
            else
            {
                // Parse it normally as a number or id for now
                auto nexpr = new Eval::Expression();

                /*nexpr->val.tag          = Eval::DataTypeTag::Int;
                nexpr->val.data.integer = Parse::ParseAs<u32>(token);*/
                if (token.type == TokenType::AlphaNumeric)
                {
                    nexpr->val.tag           = Eval::DataTypeTag::Id;
                    nexpr->val.data.id.begin = token.ptrs.begin;
                    nexpr->val.data.id.end   = token.ptrs.end;
                }
                else if (token.type == TokenType::Number)
                {
                    std::visit(
                        [&](auto &&arg) {
                            using T = std::decay_t<decltype(arg)>;
                            if constexpr (std::is_same_v<T, i64>)
                            {
                                nexpr->val.tag          = Eval::DataTypeTag::Int;
                                nexpr->val.data.integer = arg;
                            }
                            else if constexpr (std::is_same_v<T, f64>)
                            {
                                nexpr->val.tag       = Eval::DataTypeTag::Real;
                                nexpr->val.data.real = arg;
                            }
                        },
                        token.num.num);
                }
                else
                    Unimplemented();

                nexpr->leaf = true;
                expr->childs.push_back(nexpr);
            }
            token = tokenizer.next();
        }

        return expr;
    }
};