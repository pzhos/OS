#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <conio.h>
#include "Employee.h"

void replace(std::string& str, const std::string& from, const std::string& to) {
    if (from.empty())
        return;
    std::string::size_type start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

int main(int argc, char* argv[])
{
    std::string fileName;
    int numberOfRecords;
    setlocale(LC_ALL, "Rus");

    std::cout << "Write name of bin file: ";
    std::cin >> fileName;
    std::cout << "Write number of records: ";
    std::cin >> numberOfRecords;

    std::string path = argv[0];
    replace(path, "_1.exe", "");

    STARTUPINFOA si;
    PROCESS_INFORMATION piApp;
    ZeroMemory(&si, sizeof(STARTUPINFOA));
    si.cb = sizeof(STARTUPINFOA);
    if (CreateProcessA((path + "Creator.exe").c_str(),
        (LPSTR)((path + "Creator.exe " + fileName + " " + std::to_string(numberOfRecords)).c_str()),
        NULL, NULL, FALSE, NULL, NULL, NULL, &si, &piApp))
    {
        WaitForSingleObject(piApp.hProcess, INFINITE);
        CloseHandle(piApp.hThread);
        CloseHandle(piApp.hProcess);
    }
    else {
        std::cout << "Process Creator.exe not start";
        return 1;
    }

    std::fstream file_bin;
    file_bin.open(fileName, std::ios::in | std::ios::binary);
    employee emp;
    while (file_bin.read((char*)&emp, sizeof(emp))) {
        std::cout << emp.num << " " << emp.name << " " << emp.hours << "\n";
    }
    file_bin.close();

    std::string fileNameTxt;
    double salary;

    std::cout << "\nWrite name of report file: ";
    std::cin >> fileNameTxt;
    std::cout << "Write salary: ";
    std::cin >> salary;

    ZeroMemory(&si, sizeof(STARTUPINFOA));
    if (CreateProcessA((path + "Reporter.exe").c_str(),
        (LPSTR)((path + "Reporter.exe " + fileName + " " + fileNameTxt + " " + std::to_string(salary)).c_str()),
        NULL, NULL, FALSE, NULL, NULL, NULL, &si, &piApp))
    {
        WaitForSingleObject(piApp.hProcess, INFINITE);
        CloseHandle(piApp.hThread);
        CloseHandle(piApp.hProcess);
    }
    else {
        std::cout << "Process Reporter.exe not start";
        return 1;
    }

    // Открываем и выводим содержимое файла отчета на консоль
    std::fstream file_txt;
    file_txt.open(fileNameTxt, std::ios::in);
    std::string line;
    if (file_txt.is_open()) {
        while (std::getline(file_txt, line)) {
            std::cout << line << std::endl;
        }
        file_txt.close();
    }
    else {
        std::cout << "Unable to open report file: " << fileNameTxt << std::endl;
    }

    return 0;
}
