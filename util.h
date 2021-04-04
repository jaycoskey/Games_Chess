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
// #include <variant>
#include <vector>

using std::string;
using std::ostream, std::ostringstream;
using std::map, std::pair, std::set, std::vector;

using Short = int;
using Col   = Short;
using Row   = Short;
using Hash  = std::size_t;


const Short VECTOR_CAPACITY_INCR = 25;

// ---------- Collections
template<typename T>
bool doesContain(const vector<T>& vec, const T& val) {
    return find(vec.begin(), vec.end(), val) != vec.end();
}

template <typename K, typename V>
vector<pair<K, V>> mapToVector(const map<K, V>& src)
{
    return vector<pair<K, V>>(src.begin(), src.end());
}

template <typename K, typename V>
ostream& operator<<(ostream& os, const map<K, V>& m) {
    os << '{';
    int i = 0;
    for (const auto& [key, val] : m) {
        if (i++ > 0) { os << ", "; }
        os << key << ": " << val;
    }
    os << '}';
    return os;
}

template <typename T, typename U>
ostream& operator<<(ostream& os, const pair<T, U>& p) {
    os << '(' << p.first << ", " << p.second << ')';
    return os;
}

template <typename T>
ostream& operator<<(ostream& os, const set<T>& items) {
    os << '{';
    int i = 0;
    for (const auto& item : items) {
        if (i++ > 0) { os << ", "; }
        os << item;
    }
    os << '}';
    return os;
}

template <typename T>
ostream& operator<<(ostream& os, const vector<T>& items) {
    os << '[';
    int i = 0;
    for (const auto& item : items) {
        if (i++ > 0) { os << ", "; }
        os << item;
    }
    os << ']';
    return os;
}

template <typename T>
set<T> getUnion(const set<T>& a, const set<T>& b) {
    set<T> result{a};
    result.insert(b.begin(), b.end());
    return result;
}

template <typename K, typename V>
vector<V> concatMap(const map<K, vector<V>>& m) {
    vector<V> result;
    for (auto& [key, vals] : m) {
        std::copy(vals.begin(), vals.end(), std::back_inserter(result));
    }
    return result;
}

// ---------- Color
enum class Color {
    Black,
    White
};

string to_string(Color c)
{
    return c == Color::Black ? "Black" : "White";
}

ostream& operator<<(ostream& os, Color c) {
    if (c == Color::Black) { os << 'B'; }
    else { os << 'W'; }
    return os;
}

Color opponent(Color color)
{
    return color == Color::Black ? Color::White : Color::Black;
}

const vector<Color> allColors{Color::White, Color::Black};
const Short COLORS_COUNT = 2;

// ---------- Hash
string test_to_string(Hash h)
{
    ostringstream oss;
    oss << std::hex << h;
    return oss.str();
}

// ---------- PRNG
std::mt19937& prng() {
    static std::random_device rd;
    static std::mt19937 gen{rd()};
    return gen;
}

// Used for Zobrist hashing in board.h.
Hash random_bitstring() {
    static std::mt19937_64 prng;
    return prng();
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

// ---------- Variant
//  template<typename... Ts>
//  std::ostream& operator<<(ostream& os, const std::variant<Ts...>& v)
//  {
//      std::visit( [&os](auto&& arg) { os << arg; }
//                , v
//                );
//      return os;
//  }

// ---------- Underlying type, from Scott Meyers, Effective Modern C++ (for enums)
template <typename E>
constexpr auto to_underlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}
