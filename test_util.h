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
