#pragma once
#include "CommonLexeme.h"
#include <vector>
#include <unordered_map>
namespace gear {
    struct ParamParseValue {
        std::string type;
        std::string name;
    };

    class ParamParser {
    public:
        ParamParser(const std::vector<CommonLexeme>& lexemes): mCursor(0), mLexemes(lexemes) {
        }
        std::vector<ParamParseValue> parse() {
            std::vector<ParamParseValue> params;
            std::string type;
            bool isType = true;
            for (auto lexeme : mLexemes) {
                if (lexeme.getType() == CommonType::STRING) {
                    if (isType) {
                        isType = false;
                        type = lexeme.getStringValue();
                    } else {
                        isType = true;
                        ParamParseValue p;
                        p.type = type;
                        p.name = lexeme.getStringValue();
                        params.push_back(p);
                    }
                }
            }
            return params;
        }
    private:
        size_t mCursor = 0;
        const std::vector<CommonLexeme>& mLexemes;
    };
}
