#include "util/diagnostics.h"

#include <utility>
#include <cstdio>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

void DiagnosticSink::SetListener(Listener l)
{
    std::lock_guard lock(mutex_);
    listener_ = std::move(l);
}

void DiagnosticSink::Info(std::string msg, std::filesystem::path file, const int line)
{
    Report({Severity::Info, std::move(msg), std::move(file), line});
}

void DiagnosticSink::Warning(std::string msg, std::filesystem::path file, const int line)
{
    Report({Severity::Warning, std::move(msg), std::move(file), line});
}

void DiagnosticSink::Error(std::string msg, std::filesystem::path file, const int line)
{
    Report({Severity::Error, std::move(msg), std::move(file), line});
}

void DiagnosticSink::Report(Diagnostic d)
{
    std::lock_guard lock(mutex_);
    if (d.severity == Severity::Error) ++errors_;
    if (d.severity == Severity::Warning) ++warnings_;
    if (listener_) listener_(d);
    all_.push_back(std::move(d));
}

void EnableConsoleColors()
{
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &mode))
        SetConsoleMode(h, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
}

void PrintDiagnostic(const Diagnostic& d)
{
    auto color = "\x1b[0m";
    auto label = "Info";
    switch (d.severity)
    {
        case Severity::Info:    color = "\x1b[36m"; label = "Info";    break;
        case Severity::Warning: color = "\x1b[33m"; label = "Warning"; break;
        case Severity::Error:   color = "\x1b[31m"; label = "Error";   break;
    }
    std::string loc;
    if (!d.file.empty())
    {
        loc = d.file.generic_string();
        if (d.line > 0) loc += ":" + std::to_string(d.line);
        loc += ": ";
    }
    std::printf("%s%s%s\x1b[0m: %s\n", loc.c_str(), color, label, d.message.c_str());
    std::fflush(stdout);
}