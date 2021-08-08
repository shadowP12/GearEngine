#pragma once
#include "Lexer.h"
#include "CommonLexeme.h"

namespace gear {
    class CommonLexer : public Lexer<CommonLexeme> {
    private:
        bool readLexeme() override;
        bool getCurrentType(CommonType* type);
        void readNumber();
        void readIdentifier();
    };
}