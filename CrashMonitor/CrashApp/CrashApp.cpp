#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <filesystem>

int main()
{
    // Выводим рабочую директорию
    std::cout << "Current directory: "
        << std::filesystem::current_path() << std::endl;

    // Формируем путь к файлу лога
    std::filesystem::path logPath =
        std::filesystem::current_path() / "CrashApp.log";

    // Выводим полный путь к логу
    std::cout << "Log file: " << logPath << std::endl;

    // Открываем лог
    std::ofstream log(logPath);

    log << "Application started." << std::endl;

    std::cout << "Application started." << std::endl;
    std::cout << "The application will crash in 5 seconds..." << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(5));

    log << "Crash is about to happen." << std::endl;
    log.close();

    // Искусственный краш
    int* ptr = nullptr;
    *ptr = 67;

    return 0;
}