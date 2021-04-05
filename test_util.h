// Copyright 2021, by Jay M. Coskey

#pragma once

#include "util.h"
#include "geometry.h"


TEST(UtilTest, UtilMisc) {
    ScopedTracer(__func__);
    const Dirs& orthoDirs  = dirSignedPerms(Dir(1,0));
    const Dirs& diagDirs   = dirSigns(Dir(1,1));
    const Dirs& allDirs    = getUnion(orthoDirs, diagDirs);
    const Dirs& knightDirs = dirSignedPerms(Dir(1,2));

    EXPECT_EQ(orthoDirs.size(),  (unsigned long) 4);
    EXPECT_EQ(diagDirs.size(),   (unsigned long) 4);
    EXPECT_EQ(allDirs.size(),    (unsigned long) 8);

    EXPECT_EQ(knightDirs.size(), (unsigned long) 8);
}
