#ifndef LLCE_DEQUE_H
#define LLCE_DEQUE_H

#include <array>
#include <cmath>

namespace llce {

template <typename T, size_t N>
class deque {
public:

/// Class Functions ///

// Initializers //

deque() {
    mStartIndex = 1;
    mEndIndex = 0;
    mSize = 0;
}

// Modification Operators //

void push_front( const T& pValue ) {
    _spush();
    mBuffer[mStartIndex] = pValue;
}

void push_back( const T& pValue ) {
    _epush();
    mBuffer[mEndIndex] = pValue;
}

T pop_front() {
    T value = mBuffer[mStartIndex];
    _spop();
    return value;
}

T pop_back() {
    T value = mBuffer[mEndIndex];
    _epop();
    return value;
}

// Access Operators //

T& back( const size_t& pIndex ) {
    return mBuffer[_eindex(pIndex)];
}

const T& back( const size_t& pIndex ) const {
    return mBuffer[_eindex(pIndex)];
}

T& front( const size_t& pIndex ) {
    return mBuffer[_sindex(pIndex)];
}

const T& front( const size_t& pIndex ) const {
    return mBuffer[_sindex(pIndex)];
}

size_t size() const {
    return mSize;
}

private:

/// Private Functions ///

size_t _sindex( const size_t& pIndex ) const {
    return ( pIndex + mStartIndex ) % N;
}

size_t _eindex( const size_t& pIndex ) const {
    return ( mEndIndex >= pIndex ) ? mEndIndex - pIndex : N - ( pIndex - mEndIndex );
}

void _spush() {
    mSize = std::min( N, mSize + 1 );
    mStartIndex = ( mStartIndex != 0 ) ? mStartIndex - 1 : N - 1;
    if( mSize == N ) { mEndIndex = ( mEndIndex != 0 ) ? mEndIndex - 1 : N - 1; }
}

void _epush() {
    mSize = std::min( N, mSize + 1 );
    mEndIndex = ( mEndIndex + 1 ) % N;
    if( mSize == N ) { mStartIndex = ( mStartIndex + 1 ) % N; }
}

void _spop() {
    if( mSize > 0 ) {
        mSize--;
        mStartIndex = ( mStartIndex + 1 ) % N;
    }
}

void _epop() {
    if( mSize > 0 ) {
        mSize--;
        mEndIndex = ( mEndIndex != 0 ) ? mEndIndex - 1 : N - 1;
    }
}

/// Class Fields ///

std::array<T, N> mBuffer;
size_t mStartIndex, mEndIndex;
size_t mSize;

};

};

#endif
