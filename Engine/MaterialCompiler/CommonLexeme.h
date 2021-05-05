#pragma once
#include "Lexeme.h"
namespace gear {
    enum CommonType {
        BOOLEAN,
        STRING,
        NUMBER,
        COMMA,
        COLUMN
    };

    class CommonLexeme final: public Lexeme<CommonType> {
    public:
        CommonLexeme(CommonType type, const char* start, const char* end, size_t line, size_t pos) :
                Lexeme(type, start, end, line, pos) {
        }
    };
}