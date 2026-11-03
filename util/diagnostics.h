#pragma once

#ifndef NETCOMP_DIAGNOSTICS_H
#define NETCOMP_DIAGNOSTICS_H

#include <filesystem>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

enum class Severity { Info, Warning, Error };

struct Diagnostic
{
    Severity severity = Severity::Info;
    std::string message;
    std::filesystem::path file;
    int line = 0;
};

class DiagnosticSink
{
public:

    using Listener = std::function<void(const Diagnostic&)>;

    void SetListener(Listener l);

    void Info(std::string msg, std::filesystem::path file = {}, int line = 0);
    void Warning(std::string msg, std::filesystem::path file = {}, int line = 0);
    void Error(std::string msg, std::filesystem::path file = {}, int line = 0);

private:

    mutable std::mutex mutex_;
    Listener listener_;
    std::vector<Diagnostic> all_;
    size_t errors_ = 0, warnings_ = 0;

    void Report(Diagnostic d);
};

void EnableConsoleColors();
void PrintDiagnostic(const Diagnostic& d);

#endif //NETCOMP_DIAGNOSTICS_H