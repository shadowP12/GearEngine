#pragma once
#include "ChunkLexeme.h"
#include <vector>
#include <unordered_map>
namespace gear {
    class ChunkParser {
    public:
        ChunkParser(const std::vector<ChunkLexeme>& lexemes): mCursor(0), mLexemes(lexemes) {
        }
        std::unordered_map<std::string, std::string> parse() {
            std::unordered_map<std::string, std::string> chunkDict;
            std::string identifier;
            for (auto lexeme : mLexemes) {
                if (lexeme.getType() == ChunkType::IDENTIFIER) {
                    identifier = lexeme.getStringValue();
                } else if (lexeme.getType() == ChunkType::BLOCK) {
                    ChunkLexeme trimmedLexeme = lexeme.trimBlockMarkers();
                    chunkDict[identifier] = trimmedLexeme.getStringValue();
                }
            }
            return chunkDict;
        }
    private:
        size_t mCursor = 0;
        const std::vector<ChunkLexeme>& mLexemes;
    };
}
