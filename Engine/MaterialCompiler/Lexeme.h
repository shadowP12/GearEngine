#pragma once
#include <string>
namespace gear {
    // Lexeme模板
    template <class T>
    class Lexeme {
    public:
        Lexeme(T type, const char* start, const char* end, size_t line, size_t pos) :
                mStart(start), mEnd(end), mLineNumber(line), mPosition(pos), mType(type) {
        }

        const char* getStart() const noexcept {
            return mStart;
        }
        const char* getEnd() const noexcept {
            return mEnd;
        }

        const size_t getSize() const noexcept {
            return mEnd - mStart + 1;
        }

        const T getType() const noexcept {
            return mType;
        }
        const size_t getLine() const noexcept {
            return mLineNumber;
        }
        const size_t getLinePosition() const noexcept {
            return mPosition;
        }

        const std::string getStringValue() const noexcept {
            return std::string(mStart, mEnd - mStart + 1);
        }

    protected:
        const char* mStart; // included
        const char* mEnd;   // included
        size_t mLineNumber;
        size_t mPosition; // The offset in the line.
        T mType;
    };
}