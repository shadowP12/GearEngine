#pragma once
#include "CommonLexeme.h"
#include <vector>
#include <unordered_map>
namespace gear {
    class ArrayParser {
    public:
        ArrayParser(const std::vector<CommonLexeme>& lexemes): mCursor(0), mLexemes(lexemes) {
        }
        std::vector<std::string> parse() {
            std::vector<std::string> values;
            for (auto lexeme : mLexemes) {
                if (lexeme.getType() == CommonType::STRING) {
                    values.push_back(lexeme.getStringValue());
                }
            }
            return values;
        }
    private:
        size_t mCursor = 0;
        const std::vector<CommonLexeme>& mLexemes;
    };
}
