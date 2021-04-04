// Copyright 2021, by Jay M. Coskey

#pragma once

#include <iostream>
#include <set>
#include <sstream>

#include "geometry.h"


using std::cout;


TEST(UtilTest, UtilMisc) {
    ScopedTracer(__func__);
    Dirs orthoDirs  = dirSignedPerms(Dir(1,0));
    Dirs diagDirs   = dirSigns(Dir(1,1));
    Dirs allDirs    = getUnion(orthoDirs, diagDirs);

    Dirs knightDirs = dirSignedPerms(Dir(1,2));

    EXPECT_EQ(orthoDirs.size(),  (unsigned long) 4);
    EXPECT_EQ(diagDirs.size(),   (unsigned long) 4);
    EXPECT_EQ(allDirs.size(),    (unsigned long) 8);

    EXPECT_EQ(knightDirs.size(), (unsigned long) 8);

    // cout << "orthoDirs="  << orthoDirs  << "\n";
    // cout << "diagDirs="   << diagDirs   << "\n";
    // cout << "allDirs="    << allDirs    << "\n";
 
    // cout << "knightDirs=" << knightDirs << "\n";
}
