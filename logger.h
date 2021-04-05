// Copyright 2021, by Jay M. Coskey

#pragma once

// Note: Not thread-safe

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>


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
    static void init(LogLevel reportLevel /* =LogLevel::LogError */)
    {
        _foutP = nullptr;
        _outP = &std::cerr;
        _reportLevel = reportLevel;
    }

    static void init(LogLevel reportLevel, const std::string& base_filename)
    {
        _reportLevel = reportLevel;
        time_t now;
        time(&now);
        char suffix[sizeof "20201231_125959 "];
        strftime(suffix, sizeof suffix, "_%Y%m%d_%H%M%S\0", localtime(&now));
        _filename = base_filename + suffix + ".log";
        _setOfstream();
    }

    static void logToCerr() {
        assert(_outP);
        _outP = &std::cerr;
    }

    static void logToCout() {
        assert(_outP);
        _outP = &std::cout;
    }

    static void logToFile(const std::string& filename) {
        _closeFile();
        _filename = filename;
        _setOfstream();
    }

    template <typename Arg, typename... Args>
    static void log(LogLevel eventLevel, Arg&& arg, Args&&... args)
    {
        if (eventLevel <= _reportLevel) {
            write("ERROR: ", std::forward<Arg>(arg), std::forward<Args>(args)...);
        }
    }

    template <typename Arg, typename... Args>
    static void error(Arg&& arg, Args&&... args)
    {
        if (LogError <= _reportLevel) {
            write("ERROR: ", std::forward<Arg>(arg), std::forward<Args>(args)...);
        }
    }

    template <typename Arg, typename... Args>
    static void warn(Arg&& arg, Args&&... args)
    {
        if (LogWarn <= _reportLevel) {
            write("WARN : ", std::forward<Arg>(arg), std::forward<Args>(args)...);
        }
    }

    template <typename Arg, typename... Args>
    static void info(Arg&& arg, Args&&... args)
    {
        if (LogInfo <= _reportLevel) {
            write("INFO : ", std::forward<Arg>(arg), std::forward<Args>(args)...);
        }
    }

    template <typename Arg, typename... Args>
    static void debug(Arg&& arg, Args&&... args)
    {
        if (LogDebug <= _reportLevel) {
            write("DEBUG: ", std::forward<Arg>(arg), std::forward<Args>(args)...);
        }
    }

    template <typename Arg, typename... Args>
    static void trace(Arg&& arg, Args&&... args)
    {
        if (LogTrace <= _reportLevel) {
            write("TRACE: ", std::forward<Arg>(arg), std::forward<Args>(args)...);
        }
    }

    static void flush()
    {
        assert(_outP);
        (*_outP) << std::flush;
    }

    static LogLevel reportLevel()
    {
        assert(_outP);
        return _reportLevel;
    }

    static void setReportLevel(LogLevel newLogLevel)
    {
        assert(_outP);
        _reportLevel = newLogLevel;
    }

    template <typename Arg, typename... Args>
    static void write(Arg&& arg, Args&&... args)
    {
        assert(_outP);
        *_outP << std::forward<Arg>(arg);
        ((*_outP << std::forward<Args>(args)), ...);
        *_outP << "\n";
    }

    static void close()
    {
        assert(_outP);
        *(_outP) << std::flush;;
        _closeFile();
    }

private:
    static void _setOfstream() {
        _foutP = new std::ofstream{};
        _foutP->open(_filename, std::ios_base::out);
        _outP = _foutP;
    }

    static void _closeFile() {
        if (_foutP) {
            try { _foutP->close(); }
            catch (...) { }
        }
    }

    static std::string    _filename;
    static std::ofstream* _foutP;
    static std::ostream*  _outP;
    static LogLevel       _reportLevel;
};
