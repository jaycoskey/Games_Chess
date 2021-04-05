// Games_Chess
// Copyright (C) 2021, by Jay M. Coskey
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <iostream>
#include <map>
#include <random>
#include <set>
#include <vector>

using Short = int;
using Col   = Short;
using Row   = Short;
using Hash  = std::size_t;


enum class Color {
    Black,
    White
};

constexpr Short VECTOR_CAPACITY_INCR = 25;

// ---------- Collections
template<typename T>
bool doesContain(const std::vector<T>& vec, const T& val)
{
    return find(vec.begin(), vec.end(), val) != vec.end();
}

template <typename K, typename V>
std::vector<std::pair<K, V>> mapToVector(const std::map<K, V>& src)
{
    return std::vector<std::pair<K, V>>(src.begin(), src.end());
}

template <typename K, typename V>
std::ostream& operator<<(std::ostream& os, const std::map<K, V>& m)
{
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
std::ostream& operator<<(std::ostream& os, const std::pair<T, U>& p)
{
    os << '(' << p.first << ", " << p.second << ')';
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::set<T>& items)
{
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
std::ostream& operator<<(std::ostream& os, const std::vector<T>& items)
{
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
const std::set<T> getUnion(const std::set<T>& a, const std::set<T>& b)
{
    std::set<T> result{a};
    result.insert(b.begin(), b.end());
    return result;
}

template <typename K, typename V>
std::vector<V> concatMap(const std::map<K, std::vector<V>>& m)
{
    std::vector<V> result;
    for (auto& [key, vals] : m) {
        std::copy(vals.begin(), vals.end(), std::back_inserter(result));
    }
    return result;
}

// ---------- Color
std::string to_string(Color c);

std::ostream& operator<<(std::ostream& os, Color c);

Color opponent(Color color);

const std::vector<Color> allColors{Color::White, Color::Black};
constexpr Short COLORS_COUNT = 2;

// ---------- Hash
std::string test_to_string(Hash h);

// ---------- PRNG
std::mt19937& prng();

// Used for Zobrist hashing in board.h.
Hash random_bitstring();

// ---------- String
std::string repeatString(const std::string& input, int count);
