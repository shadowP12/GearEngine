#include "CommonLexer.h"

namespace gear {
    void CommonLexer::readIdentifier() {
        while (hasMore() && isIdentifierCharacter(*mCursor)) {
            consume();
        }
    }

    void CommonLexer::readNumber() {
        if (hasMore() && (isNumericCharacter(*mCursor) || *mCursor == '-')) {
            consume();
        }

        while(hasMore() && isNumericCharacter(*mCursor)) {
            consume();
        }

        if (!hasMore()) {
            return;
        }

        if (*mCursor != '.') {
            return;
        }

        consume();
        while (hasMore() && isNumericCharacter(*mCursor)) {
            consume();
        }
    }

    bool CommonLexer::readLexeme() {
        skipWhiteSpace();

        if (!hasMore()) {
            return true;
        }

        const char* lexemeStart = mCursor;
        size_t line = getLine();
        size_t cursor = getCursor();

        CommonType lexemeType;
        switch (char c = peek()) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                readNumber();
                lexemeType = NUMBER;
                break;
            case ',':
                consume();
                lexemeType = COMMA;
                break;
            case ':':
                consume();
                lexemeType = COLUMN;
                break;
            default:
                if (isIdentifierCharacter(c)) {
                    readIdentifier();
                    lexemeType = STRING;
                } else {
                    return false;
                }
                break;
        }

        CommonLexeme lexeme(lexemeType, lexemeStart, mCursor - 1, line, cursor);
        mLexemes.push_back(lexeme);
        return true;
    }
}