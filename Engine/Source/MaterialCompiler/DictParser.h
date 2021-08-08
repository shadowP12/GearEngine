#pragma once
#include "CommonLexeme.h"
#include <vector>
#include <unordered_map>
namespace gear {
    class DictParser {
    public:
        DictParser(const std::vector<CommonLexeme>& lexemes): mCursor(0), mLexemes(lexemes) {
        }
        std::unordered_map<std::string, std::string> parse() {
            std::unordered_map<std::string, std::string> dict;
            std::string key;
            bool isKey = true;
            for (auto lexeme : mLexemes) {
                if (lexeme.getType() == CommonType::STRING) {
                    if (isKey) {
                        isKey = false;
                        key = lexeme.getStringValue();
                    } else {
                        isKey = true;
                        dict[key] = lexeme.getStringValue();
                    }
                }
            }
            return dict;
        }
    private:
        size_t mCursor = 0;
        const std::vector<CommonLexeme>& mLexemes;
    };
}
