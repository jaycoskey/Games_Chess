// Copyright 2021, by Jay M. Coskey

#pragma once

// Note: Not thread-safe

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

using std::string;
using std::cout;
using std::ofstream, std::ostream;
using std::forward;


enum LogLevel {
        LogOff=0,
        // LogFatal=100,
        LogError=200,
        LogWarn=300,
        LogInfo=400,
        LogDebug=500,
        LogTrace=600
        // , LogAll=std::numeric_limits<int>::max()
    };

class Logger
{
public:
    Logger(LogLevel reportLevel)
        : _foutP{nullptr}
        , _outP{&std::cerr}
        , _reportLevel{reportLevel}
    { }

    Logger(LogLevel reportLevel, const string& base_filename)
        : _reportLevel{reportLevel}
    {
        time_t now;
        time(&now);
        char suffix[sizeof "20201231_125959 "];
        strftime(suffix, sizeof suffix, "_%Y%m%d_%H%M%S\0", localtime(&now));
        _filename = base_filename + suffix + ".log";
        _setOfstream();
    }

    void logToCerr() {
        _outP = &std::cerr;
    }

    void logToFile(const string& filename) {
        _closeFile();
        _filename = filename;
        _setOfstream();
    }

    template <typename Arg, typename... Args>
    void log(LogLevel eventLevel, Arg&& arg, Args&&... args)
    {
        if (eventLevel <= _reportLevel) {
            _write("ERROR: ", forward<Arg>(arg), forward<Args>(args)...);
        }
    }

    template <typename Arg, typename... Args>
    void error(Arg&& arg, Args&&... args)
    {
        if (LogError <= _reportLevel) {
            _write("ERROR: ", forward<Arg>(arg), forward<Args>(args)...);
        }
    }

    template <typename Arg, typename... Args>
    void warn(Arg&& arg, Args&&... args)
    {
        if (LogWarn <= _reportLevel) {
            _write("WARN : ", forward<Arg>(arg), forward<Args>(args)...);
        }
    }

    template <typename Arg, typename... Args>
    void info(Arg&& arg, Args&&... args)
    {
        if (LogInfo <= _reportLevel) {
            _write("INFO : ", forward<Arg>(arg), forward<Args>(args)...);
        }
    }

    template <typename Arg, typename... Args>
    void debug(Arg&& arg, Args&&... args)
    {
        if (LogDebug <= _reportLevel) {
            _write("DEBUG: ", forward<Arg>(arg), forward<Args>(args)...);
        }
    }

    template <typename Arg, typename... Args>
    void trace(Arg&& arg, Args&&... args)
    {
        if (LogTrace <= _reportLevel) {
            _write("TRACE: ", forward<Arg>(arg), forward<Args>(args)...);
        }
    }

    void flush()
    {
        (*_outP) << std::flush;
    }

    LogLevel getLogLevel() const
    {
        return _reportLevel;
    }

    void setReportLevel(LogLevel newLogLevel)
    {
        _reportLevel = newLogLevel;
    }

    ~Logger()
    {
        *(_outP) << std::flush;;
        _closeFile();
    }

private:
    void _setOfstream() {
        _foutP = new ofstream{};
        _foutP->open(_filename, std::ios_base::out);
        _outP = _foutP;
    }

    void _closeFile() {
        if (_foutP) {
            try { _foutP->close(); }
            catch (...) { }
        }
    }

    template <typename Arg, typename... Args>
    void _write(Arg&& arg, Args&&... args)
    {
        *_outP << forward<Arg>(arg);
        ((*_outP << forward<Args>(args)), ...);
        *_outP << "\n";
    }

    string    _filename;
    ofstream* _foutP;
    ostream*  _outP;
    LogLevel  _reportLevel;
};

Logger logger{LogLevel::LogError};
