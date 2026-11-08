// Standalone updater for the portable Windows build.
//
// The running application downloads and verifies the new ZIP, extracts it, then
// launches this helper. The helper waits for the application to exit, copies
// the extracted files over the installation directory, and restarts the app.
// It deliberately uses no Qt so it can run from a temporary folder without
// depending on the Qt DLLs it is about to replace.

#include <windows.h>
#include <shellapi.h>

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <thread>

namespace fs = std::filesystem;

namespace {

std::string toUtf8(const std::wstring &text)
{
    if (text.empty()) {
        return {};
    }

    const int size = WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1,
                                         nullptr, 0, nullptr, nullptr);
    if (size <= 0) {
        return {};
    }

    std::string result(static_cast<size_t>(size - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1,
                        result.data(), size, nullptr, nullptr);
    return result;
}

fs::path temporaryDirectory()
{
    wchar_t buffer[MAX_PATH] = {};
    const DWORD length = GetTempPathW(MAX_PATH, buffer);
    return length > 0 ? fs::path(std::wstring(buffer, length))
                      : fs::temp_directory_path();
}

void logLine(const fs::path &path, const std::string &message)
{
    std::ofstream output(path, std::ios::app | std::ios::binary);
    if (!output) {
        return;
    }

    SYSTEMTIME time{};
    GetLocalTime(&time);

    char stamp[64] = {};
    std::snprintf(stamp, sizeof(stamp), "%04d-%02d-%02d %02d:%02d:%02d ",
                  time.wYear, time.wMonth, time.wDay,
                  time.wHour, time.wMinute, time.wSecond);
    output << stamp << message << '\n';
}

void waitForProcess(DWORD processId)
{
    if (processId == 0) {
        return;
    }

    HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, processId);
    if (!process) {
        return;
    }

    WaitForSingleObject(process, 60000);
    CloseHandle(process);
}

bool copyTree(const fs::path &source, const fs::path &target)
{
    if (!fs::exists(source)) {
        return false;
    }

    for (const fs::directory_entry &entry : fs::recursive_directory_iterator(source)) {
        const fs::path relative = fs::relative(entry.path(), source);
        const fs::path destination = target / relative;

        if (entry.is_directory()) {
            fs::create_directories(destination);
        } else if (entry.is_regular_file()) {
            fs::create_directories(destination.parent_path());

            // MinGW's std::filesystem::copy_file can report "File exists" even
            // with overwrite_existing; remove the old file first.
            std::error_code removeError;
            fs::remove(destination, removeError);
            removeError.clear();

            std::error_code copyError;
            fs::copy_file(entry.path(), destination,
                          fs::copy_options::overwrite_existing, copyError);
            if (copyError) {
                throw fs::filesystem_error("copy_file", entry.path(), destination,
                                           copyError);
            }
        }
    }

    return true;
}

bool startApplication(const fs::path &executable, const fs::path &workingDirectory)
{
    std::wstring command = L"\"" + executable.wstring() + L"\"";
    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};

    const BOOL started = CreateProcessW(
        executable.c_str(), command.data(),
        nullptr, nullptr, FALSE, 0, nullptr, workingDirectory.c_str(),
        &startup, &process);

    if (!started) {
        return false;
    }

    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    return true;
}

} // namespace

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int)
{
    int argumentCount = 0;
    LPWSTR *arguments = CommandLineToArgvW(GetCommandLineW(), &argumentCount);

    DWORD processId = 0;
    fs::path source;
    fs::path target;
    fs::path logPath;
    std::wstring executableName = L"DesktopHub.exe";

    for (int index = 1; arguments && index < argumentCount; ++index) {
        const std::wstring argument = arguments[index];
        const bool hasValue = index + 1 < argumentCount;

        if (argument == L"--pid" && hasValue) {
            processId = static_cast<DWORD>(std::wcstoul(arguments[++index], nullptr, 10));
        } else if (argument == L"--source" && hasValue) {
            source = arguments[++index];
        } else if (argument == L"--target" && hasValue) {
            target = arguments[++index];
        } else if (argument == L"--exe" && hasValue) {
            executableName = arguments[++index];
        } else if (argument == L"--log" && hasValue) {
            logPath = arguments[++index];
        }
    }

    if (logPath.empty()) {
        logPath = temporaryDirectory() / L"DesktopHubUpdate.log";
    }

    logLine(logPath, "Updater started.");
    logLine(logPath, "Source: " + toUtf8(source.wstring()));
    logLine(logPath, "Target: " + toUtf8(target.wstring()));

    if (source.empty() || target.empty()) {
        logLine(logPath, "Missing source or target path.");
        LocalFree(arguments);
        return 1;
    }

    waitForProcess(processId);

    bool copied = false;
    for (int attempt = 0; attempt < 30; ++attempt) {
        try {
            if (copyTree(source, target)) {
                copied = true;
                break;
            }
        } catch (const std::exception &error) {
            logLine(logPath, std::string("Copy attempt failed: ") + error.what());
        }

        // The old executable may remain locked for a moment after exit.
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if (!copied) {
        logLine(logPath, "Could not copy the update files.");
        LocalFree(arguments);
        return 1;
    }

    const fs::path executable = target / fs::path(executableName);
    if (!startApplication(executable, target)) {
        logLine(logPath, "Could not restart the application.");
        LocalFree(arguments);
        return 1;
    }

    logLine(logPath, "Update complete.");
    LocalFree(arguments);
    return 0;
}
