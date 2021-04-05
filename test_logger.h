// Copyright 2021, by Jay M. Coskey

#pragma once

#include <iostream>

#include "logger.h"


void test_logger_file()
{
    ScopedTracer(__func__);
    Logger::init(LogLevel::LogWarn, "test_logger");
    std::cout << "test_logger_file: Check log file with a name that starts with 'test_logger'.\n";
    Logger::error("ERROR!");
}

void test_logger_stderr()
{
    ScopedTracer(__func__);
    std::cout << "This function should print exactly one line of output, with an urgent message.\n";
    Logger::init(LogLevel::LogWarn);
    Logger::error("\tExpected error message---", 123456789);
    Logger::info("Unimportant message!");
}

void test_logger()
{
   test_logger_file();    // Check output
   test_logger_stderr();  // Check output
}
