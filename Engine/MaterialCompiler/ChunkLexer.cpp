#include "ChunkLexer.h"
namespace gear {
    void ChunkLexer::readBlock() {
        size_t braceCount = 0;
        while (hasMore()) {
            skipWhiteSpace();

            if (*mCursor == '{') {
                braceCount++;
            }  else if (*mCursor == '}') {
                braceCount--;
            }

            if (braceCount == 0) {
                consume();
                break;
            }

            consume();
        }
    }

    void ChunkLexer::readIdentifier() {
        while (hasMore() && isAphaNumericCharacter(*mCursor)) {
            consume();
        }
    }

    void ChunkLexer::readUnknown() {
        consume();
    }

    bool ChunkLexer::getCurrentType(ChunkType* type) {
        if (!hasMore()) return false;

        char c = peek();
        if (isAlphaCharacter(c)) {
            *type = ChunkType::IDENTIFIER;
        } else if (c == '{') {
            *type = ChunkType::BLOCK;
        } else {
            *type = ChunkType::UNKNOWN;
        }

        return true;
    }

    bool ChunkLexer::readLexeme() {
        skipWhiteSpace();

        ChunkType nextMaterialType;
        bool peeked = getCurrentType(&nextMaterialType);
        if (!peeked) {
            return true;
        }

        const char* lexemeStart = mCursor;
        size_t line = getLine();
        size_t cursor = getCursor();
        switch (nextMaterialType) {
            case ChunkType::BLOCK :     readBlock();      break;
            case ChunkType::IDENTIFIER: readIdentifier(); break;
            case ChunkType::UNKNOWN:    readUnknown();    break;
            default:
                break;
        }
        mLexemes.emplace_back(nextMaterialType, lexemeStart, mCursor - 1, line, cursor);

        return nextMaterialType != UNKNOWN;
    }
}