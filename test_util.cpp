// Copyright 2021, by Jay M. Coskey

#include <iostream>
#include <set>
#include <sstream>


int main() {
    std::set<Dir> orthoDirs  = signedPerms(Dir(1,0));
    std::set<Dir> diagDirs   = signs(Dir(1,1));
    std::set<Dir> adjDirs    = getUnion(orthoDirs, diagDirs);

    std::set<Dir> knightDirs = signedPerms(Dir(1,2));

    std::cout << "orthoDirs="  << show(orthoDirs)  << std::endl;
    std::cout << "diagDirs="   << show(diagDirs)   << std::endl;
    std::cout << "adjDirs="    << show(adjDirs)    << std::endl;

    std::cout << "knightDirs=" << show(knightDirs) << std::endl;

    return 0;
}
