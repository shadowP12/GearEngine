#pragma once
#include "Lexeme.h"
namespace gear {
    enum ChunkType {
        BLOCK,
        IDENTIFIER,
        UNKNOWN
    };

    class ChunkLexeme final: public Lexeme<ChunkType> {
    public:
        ChunkLexeme(ChunkType type, const char* start, const char* end, size_t line, size_t pos) :
                Lexeme(type, start, end, line, pos) {
        }

        ChunkLexeme trimBlockMarkers() const {
            return { mType, mStart + 1, mEnd - 1, mLineNumber, mPosition };
        }
    };
}