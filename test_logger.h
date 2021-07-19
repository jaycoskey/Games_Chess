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

#include "logger.h"

void test_logger_file() {
    ScopedTracer(__func__);
    Logger::init(LogLevel::LogWarn, "test_logger");
    std::cout
        << "test_logger_file: Check log file with a name that starts with "
           "'test_logger'.\n";
    Logger::error("ERROR!");
}

void test_logger_stderr() {
    ScopedTracer(__func__);
    std::cout
        << "This function should print exactly one line of output, with an "
           "urgent message.\n";
    Logger::init(LogLevel::LogWarn);
    Logger::error("\tExpected error message---", 123456789);
    Logger::info("Unimportant message!");
}

void test_logger() {
    test_logger_file();   // Check output
    test_logger_stderr(); // Check output
}
