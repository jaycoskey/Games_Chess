#include <fstream>
#include <iostream>
#include <string>

#include "logger.h"


std::string    Logger::_filename = "";
std::ofstream* Logger::_foutP = nullptr;
std::ostream*  Logger::_outP = nullptr;
LogLevel       Logger::_reportLevel = LogLevel::LogError;
