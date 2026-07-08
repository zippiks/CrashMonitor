#include <iostream>
#include <fstream>
#include <filesystem>
#include <windows.h>
#include <DbgHelp.h>
#include <ctime>
#include <string>


std::string GetCurrentDateTime()
{
    std::time_t now = std::time(nullptr);

    std::tm localTime{};
    localtime_s(&localTime, &now);

    char buffer[20];

    std::strftime(
        buffer,
        sizeof(buffer),
        "%Y-%m-%d %H:%M:%S",
        &localTime
    );

    return std::string(buffer);
}


std::string GetFileTime()
{
    std::time_t now = std::time(nullptr);

    std::tm localTime{};
    localtime_s(&localTime, &now);

    char buffer[20];

    std::strftime(
        buffer,
        sizeof(buffer),
        "%Y-%m-%d_%H-%M-%S",
        &localTime
    );

    return std::string(buffer);
}



void WriteLog(
    std::ofstream& log,
    const std::string& message
)
{
    log
        << "["
        << GetCurrentDateTime()
        << "] "
        << message
        << std::endl;
}



void WriteLog(
    std::ofstream& log,
    const std::string& message,
    DWORD code
)
{
    log
        << "["
        << GetCurrentDateTime()
        << "] "
        << message
        << ": 0x"
        << std::uppercase
        << std::hex
        << code
        << std::dec
        << std::endl;
}



std::string GetExceptionDescription(DWORD code)
{
    switch (code)
    {
    case EXCEPTION_ACCESS_VIOLATION:
        return "Access violation";

    case EXCEPTION_BREAKPOINT:
        return "Breakpoint";

    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        return "Integer divide by zero";

    default:
        return "Unknown exception";
    }
}



bool CreateDump(
    HANDLE processHandle,
    DWORD processId,
    const std::filesystem::path& dumpPath
)
{
    HANDLE dumpFile = CreateFileW(
        dumpPath.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );


    if (dumpFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }


    BOOL result = MiniDumpWriteDump(
        processHandle,
        processId,
        dumpFile,
        MiniDumpWithFullMemory,
        nullptr,
        nullptr,
        nullptr
    );


    CloseHandle(dumpFile);


    return result == TRUE;
}



bool AskRestart()
{
    int result = MessageBoxW(
        nullptr,
        L"CrashApp завершила работу с ошибкой.\n\nПерезапустить программу?",
        L"Crash Monitor",
        MB_YESNO | MB_ICONERROR
    );


    return result == IDYES;
}



bool StartCrashApp(
    PROCESS_INFORMATION& pi
)
{
    STARTUPINFO si{};
    si.cb = sizeof(si);


    wchar_t commandLine[] = L"CrashApp.exe";


    return CreateProcessW(
        nullptr,
        commandLine,
        nullptr,
        nullptr,
        FALSE,
        DEBUG_ONLY_THIS_PROCESS,
        nullptr,
        nullptr,
        &si,
        &pi
    );
}



int main()
{
    std::filesystem::path basePath =
        std::filesystem::current_path();


    std::ofstream log(
        basePath / "Agent.log"
    );


    std::filesystem::path dumpDirectory =
        basePath / "CrashReports";


    if (!std::filesystem::exists(dumpDirectory))
    {
        std::filesystem::create_directory(dumpDirectory);
    }


    WriteLog(
        log,
        "Agent started."
    );



    bool restart = true;


    while (restart)
    {

        PROCESS_INFORMATION pi{};


        if (!StartCrashApp(pi))
        {
            WriteLog(
                log,
                "Failed to start CrashApp."
            );

            break;
        }



        WriteLog(
            log,
            "CrashApp started."
        );


        WriteLog(
            log,
            "Process ID: "
            +
            std::to_string(pi.dwProcessId)
        );



        DEBUG_EVENT debugEvent{};

        bool running = true;

        bool crashed = false;

        bool dumpCreated = false;



        while (running)
        {

            if (WaitForDebugEvent(
                &debugEvent,
                INFINITE
            ))
            {

                switch (debugEvent.dwDebugEventCode)
                {

                case CREATE_PROCESS_DEBUG_EVENT:

                    if (debugEvent.u.CreateProcessInfo.hFile)
                    {
                        CloseHandle(
                            debugEvent.u.CreateProcessInfo.hFile
                        );
                    }

                    break;



                case EXCEPTION_DEBUG_EVENT:
                {
                    DWORD code =
                        debugEvent
                        .u
                        .Exception
                        .ExceptionRecord
                        .ExceptionCode;



                    WriteLog(
                        log,
                        "Exception",
                        code
                    );


                    WriteLog(
                        log,
                        GetExceptionDescription(code)
                    );



                    if (code == EXCEPTION_ACCESS_VIOLATION
                        &&
                        !dumpCreated)
                    {

                        crashed = true;


                        std::filesystem::path dumpPath =
                            dumpDirectory
                            /
                            (
                                "CrashApp_"
                                +
                                GetFileTime()
                                +
                                ".dmp"
                                );



                        if (CreateDump(
                            pi.hProcess,
                            pi.dwProcessId,
                            dumpPath
                        ))
                        {

                            WriteLog(
                                log,
                                "Dump created: "
                                +
                                dumpPath.string()
                            );


                            dumpCreated = true;
                        }
                    }

                    break;
                }



                case EXIT_PROCESS_DEBUG_EVENT:

                    running = false;

                    break;
                }



                DWORD status =
                    DBG_CONTINUE;


                if (debugEvent.dwDebugEventCode
                    ==
                    EXCEPTION_DEBUG_EVENT)
                {
                    status =
                        DBG_EXCEPTION_NOT_HANDLED;
                }



                ContinueDebugEvent(
                    debugEvent.dwProcessId,
                    debugEvent.dwThreadId,
                    status
                );
            }
        }



        CloseHandle(
            pi.hProcess
        );


        CloseHandle(
            pi.hThread
        );



        if (crashed)
        {
            restart = AskRestart();
        }
        else
        {
            restart = false;
        }
    }



    WriteLog(
        log,
        "Agent finished."
    );


    log.close();


    return 0;
}