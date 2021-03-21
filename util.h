// Copyright 2021, by Jay M. Coskey

#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <type_traits>

using std::string;
using std::ostringstream;
using std::map, std::set;

using Short = int16_t;
using Col   = Short;
using Row   = Short;
using Hash  = std::size_t;


// ---------- Color
enum class Color {
    Black,
    White
};

const string& showColor(Color color) {
    const map<Color, string> color2cstr {
        { Color::Black, string{"B"} },
        { Color::White, string{"W"} }
    };
    return color2cstr.find(color)->second;
}

// ---------- Hash-related functions
string showHash(Hash h) {
    ostringstream oss;
    oss << std::hex << h;
    return oss.str();
}

// ---------- Set operations
template <typename T>
std::string showSet(const std::set<T>& items) {
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
    set<T> result = a;
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
