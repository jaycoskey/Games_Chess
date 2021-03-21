// Copyright 2021, by Jay M. Coskey

#pragma once

#include "util.h"
#include "geometry.h"

using std::string;


class Board;

// TODO: User interactivity
class Player {
public:
    Player(Color color, const Dir& forward, const string& name)
        : _color{color}
        , _forward{forward}
        , _name{name}
        {}
    Color color() const { return _color; }
    const Dir& forward() const { return _forward; }
    const std::string& name() const { return _name; }

private:
    Color       _color;
    Dir         _forward;
    std::string _name;
};
