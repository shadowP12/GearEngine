#pragma once
#include "Lexer.h"
#include "ChunkLexeme.h"
namespace gear {
    class ChunkLexer : public Lexer<ChunkLexeme> {
    private:
        bool readLexeme() override;
        bool getCurrentType(ChunkType* type);
        void readBlock();
        void readIdentifier();
        void readUnknown();
    };
}