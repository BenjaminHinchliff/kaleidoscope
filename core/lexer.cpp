#include "lexer.h"

namespace
{
    bool isDecimal(wchar_t c)
    {
        return std::isdigit(c) || c == L'.';
    }
}

namespace lexer
{
    Lexer::Lexer(std::wistream& input)
    {
        tokens::Token token;
        do
        {
            token = extractToken(input);
            tokens.push_back(token);
        } while (!std::holds_alternative<tokens::Eof>(token));

    }

    tokens::Token Lexer::peek(size_t forward) const
    {
        auto it = tokens.cbegin();
        if (it == tokens.cend()) return tokens::Eof{};
        std::advance(it, forward);
        return *it;
    }

    tokens::Token Lexer::pop()
    {
        if (tokens.size() == 0) return tokens::Eof{};
        tokens::Token front = tokens.front();
        tokens.pop_front();
        return front;
    }

    tokens::Token Lexer::extractToken(std::wistream& input)
    {
        wchar_t next;
        input >> std::skipws >> next;

        if (!input) return tokens::Eof{};

        if (std::isalpha(next))
        {
            std::wstring iden;
            iden += next;
            while (input.peek() != WEOF && std::isalpha(input.peek()))
            {
                input >> std::noskipws >> next;
                iden += next;
            }

            if (iden == L"def")
            {
                return tokens::Def{};
            }
            if (iden == L"extern")
            {
                return tokens::Extern{};
            }
            return tokens::Identifier{ iden };
        }

        if (isDecimal(next))
        {
            // TODO: Avoid string parsing for performance?
            std::wstring numStr;
            numStr += next;
            
            while (input.peek() != WEOF && isDecimal(input.peek()))
            {
                input >> std::noskipws >> next;
                numStr += next;
            }

            double num = std::stod(numStr);
            return tokens::Number{ num };
        }

        if (next == '#')
        {
            input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return extractToken(input);
        }

        return tokens::Character{ next };
    }
}