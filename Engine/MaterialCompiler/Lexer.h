#pragma once
#include <string>
#include <vector>
namespace gear {
    // 词法分析器模板
    template <class T>
    class Lexer {
    public:
        virtual ~Lexer() = default;

        void lex(const char* text, size_t size, size_t lineOffset = 1) {
            mCursor = text;
            mEnd = mCursor + size;
            mLineCounter = lineOffset;
            while (hasMore()) {
                if (!readLexeme()) {
                    return;
                }
            }
        }

        const std::vector<T>& getLexemes() {
            return mLexemes;
        }

    protected:

        size_t getLine() const noexcept {
            return mLineCounter;
        }

        size_t getCursor() const noexcept {
            return mLinePosition;
        }

        inline bool isWhiteSpaceCharacter(char c) const noexcept {
            return c < 33 || c > 127;
        }

        inline bool isAlphaCharacter(char c) const noexcept {
            return (c >= 'A' && c <= 'Z') ||  (c >= 'a' && c <= 'z');
        }

        inline bool isNumericCharacter(char c) const noexcept {
            return (c >= '0' && c <= '9');
        }

        inline bool isAphaNumericCharacter(char c) const noexcept {
            return  isAlphaCharacter(c) || isNumericCharacter(c);
        }

        inline bool isIdentifierCharacter(char c) const noexcept {
            return isAphaNumericCharacter(c) || (c == '_');
        }

        inline bool hasMore() const noexcept {
            return mCursor < mEnd;
        }

        inline void skipUntilEndOfLine() {
            while (*mCursor != '\n' && hasMore()) {
                consume();
            }
        }

        inline char consume() noexcept {
            if (*mCursor == '\n') {
                mLineCounter++;
                mLinePosition = 0;
            }
            mLinePosition++;
            return *mCursor++;
        }

        inline char peek() const noexcept {
            return *mCursor;
        }

        // 读取下一个Lexeme
        virtual bool readLexeme() = 0;

        // 跳过空格
        void skipWhiteSpace() {
            while (mCursor < mEnd && isWhiteSpaceCharacter(*mCursor)) {
                consume();
            }

            if (hasMore() && *mCursor == '/' && mCursor < (mEnd - 1) && *(mCursor + 1) == '/') {
                skipUntilEndOfLine();
                skipWhiteSpace();
            }
        }

        const char* mEnd = nullptr;
        const char* mCursor = nullptr;
        std::vector<T> mLexemes;

        size_t mLineCounter = 1;
        size_t mLinePosition = 0;
    };
}