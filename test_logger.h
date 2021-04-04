// Copyright 2021, by Jay M. Coskey

#pragma once

#include <iostream>

#include "logger.h"

using std::cout;


void test_logger_file()
{
    ScopedTracer(__func__);
    Logger logger{LogWarn, "test_logger"};
    cout << "test_logger_file: Check log file with a name that starts with 'test_logger'.\n";
    logger.error("ERROR!");
}

void test_logger_stderr()
{
    ScopedTracer(__func__);
    cout << "This function should print exactly one line of output, with an urgent message.\n";
    Logger logger{LogWarn};
    logger.error("\tExpected error message---", 123456789);
    logger.info("Unimportant message!");
}

void test_logger()
{
   test_logger_file();    // Check output
   test_logger_stderr();  // Check output
}
