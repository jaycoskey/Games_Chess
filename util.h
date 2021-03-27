// Copyright 2021, by Jay M. Coskey

#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <sstream>
#include <type_traits>
#include <vector>

using std::string;
using std::ostream, std::ostringstream;
using std::map, std::set, std::vector;

using Short = int16_t;
using Col   = Short;
using Row   = Short;
using Hash  = std::size_t;


// ---------- Color
enum class Color {
    Black,
    White
};
ostream& operator<<(ostream& os, Color c) {
    if (c == Color::Black) { os << 'B'; }
    else { os << 'W'; }
    return os;
}

Color opponent(Color color) {
    return color == Color::Black ? Color::White : Color::Black;
}

const vector<Color> colors{Color::White, Color::Black};

// ---------- Hash-related functions
string showHash(Hash h) {
    ostringstream oss;
    oss << std::hex << h;
    return oss.str();
}

// ---------- PRNG

// Used for Zobrist hashing in board.h.
// TODO: Replace default seed with one from high-resolution clock.
// TODO: using hr_clock = std::chrono::high_resolution_clock;
// TODO: using to_nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>;
Hash random_bitstring() {
    // TODO: static auto seed = to_nanoseconds(hr_clock.now().time_since_epoch()).count();
    // TODO: auto t = std::chrono::high_resolution_clock::now();
    // TODO: static auto seed = std::chrono::duration_cast<std::chrono::nanoseconds>(t.time_since_epoch()).count();
    static std::mt19937_64 prng;  // TODO: {seed};
    return prng();
}

// ---------- Collection functions
template <typename K, typename V>
vector<std::pair<K, V>> mapToVector(const map<K, V>& src)
{
    return vector<std::pair<K, V>>(src.begin(), src.end());
}

template <typename T>
std::string showSet(const set<T>& items) {
    ostringstream oss;
    oss << '{';
    int i = 0;
    for (auto& item : items) {
        if (i++ > 0) { oss << ", "; }
        oss << item;
    }
    oss << '}';
    return oss.str();
}

template <typename T>
std::string showVector(const vector<T>& items) {
    ostringstream oss;
    oss << '[';
    int i = 0;
    for (auto& item : items) {
        if (i++ > 0) { oss << ", "; }
        oss << item;
    }
    oss << ']';
    return oss.str();
}

template <typename T>
set<T> getUnion(const set<T>& a, const set<T>& b) {
    set<T> result{a};
    result.insert(b.begin(), b.end());
    return result;
}

// ---------- String
string repeatString(const string& input, int count) {
    string result;
    result.reserve(input.size() * count);
    while (count-- > 0) {
        result += input;
    }
    return result;
}

// ---------- Underlying type, from Scott Meyers, Effective Modern C++ (for enums)
template <typename E>
constexpr auto to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}
