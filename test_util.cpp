// Copyright 2021, by Jay M. Coskey

#include <iostream>
#include <set>
#include <sstream>

#include "geometry.h"


using std::cout;


int main() {
    Dirs orthoDirs  = dirSignedPerms(Dir(1,0));
    Dirs diagDirs   = dirSigns(Dir(1,1));
    Dirs allDirs    = getUnion(orthoDirs, diagDirs);

    Dirs knightDirs = dirSignedPerms(Dir(1,2));

    assert(orthoDirs.size()  == 4);
    assert(diagDirs.size()   == 4);
    assert(allDirs.size())   == 8);

    assert(knightDirs.size() == 4);

    cout << "orthoDirs="  << showSet(orthoDirs) << "\n";
    cout << "diagDirs="   << showSet(diagDirs)  << "\n";
    cout << "adjDirs="    << showSet(adjDirs)   << "\n";

    cout << "knightDirs=" << showSet(knightDirs) << "\n";

    return 0;
}
