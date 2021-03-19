// Copyright 2021, by Jay M. Coskey

#pragma once

#include "util.h"


class Board;

// TODO: Privacy
// TODO: User interactivity
class Player {
public:
    Color getColor() { return color; }
    const std::string& getName() const { return name; }
    const Dir& getForward() const { return forward; }

    Color  color; 
private:
    std::string name; 
    Dir    forward;
    // friend struct std::hash<Board>;
};
