#include <iostream>
#include <fstream>
#include <filesystem>
#include <windows.h>

int main()
{
    std::cout << "Agent started." << std::endl;

    // Формируем путь к файлу Agent.log
    std::filesystem::path logPath =
        std::filesystem::current_path() / "Agent.log";

    
    std::ofstream log(logPath);

    log << "Agent started." << std::endl;

    std::cout << "Log file created: " << logPath << std::endl;


    

    STARTUPINFO si{};
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi{};


    // Командная строка для запуска CrashApp
    wchar_t commandLine[] = L"CrashApp.exe";


    // Запуск CrashApp
    BOOL result = CreateProcessW(
        nullptr,        // имя исполняемого файла
        commandLine,    // командная строка
        nullptr,        // атрибуты безопасности процесса
        nullptr,        // атрибуты безопасности потока
        FALSE,          // наследование дескрипторов
        0,              // флаги создания
        nullptr,        // окружение
        nullptr,        // рабочая директория
        &si,            // настройки запуска
        &pi             // информация о созданном процессе
    );


    if (result)
    {
        std::cout << "CrashApp started successfully." << std::endl;

        log << "CrashApp started successfully." << std::endl;


        std::cout << "Waiting for CrashApp..." << std::endl;

        log << "Waiting for CrashApp..." << std::endl;


        WaitForSingleObject(
            pi.hProcess,
            INFINITE
        );


        std::cout << "CrashApp finished." << std::endl;

        log << "CrashApp finished." << std::endl;

        DWORD exitCode = 0;

        if (GetExitCodeProcess(pi.hProcess, &exitCode))
        {
            std::cout << "Exit code: 0x"
                << std::uppercase
                << std::hex
                << exitCode
                << std::dec
                << std::endl;


            log << "Exit code: 0x"
                << std::uppercase
                << std::hex
                << exitCode
                << std::dec
                << std::endl;


            if (exitCode == 0)
            {
                std::cout << "Application closed normally."
                    << std::endl;

                log << "Application closed normally."
                    << std::endl;
            }
            else if (exitCode == 0xC0000005)
            {
                std::cout << "Application crashed: access violation."
                    << std::endl;

                log << "Application crashed: access violation."
                    << std::endl;
            }
            else
            {
                std::cout << "Application terminated with error."
                    << std::endl;

                log << "Application terminated with error."
                    << std::endl;
            }
        }
    }
    else
    {
        std::cout << "Failed to start CrashApp." << std::endl;

        log << "Failed to start CrashApp." << std::endl;
    }
    




    log.close();

    return 0;
}