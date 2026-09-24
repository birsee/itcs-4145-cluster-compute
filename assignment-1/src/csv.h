#pragma once
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <type_traits>

// little csv header i made for practice :)

template<typename First, typename... Args> void csvRow(std::ofstream& file, const First& first, const Args&... args) {
    file << first;
    ((file << "," << args), ...);
    file << '\n';
    file.flush();
}

template<typename First, typename... Args> std::ofstream csvStart(const std::string& path, const First& first, const Args&... args) {
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "failed to open file: " << path << "\n";
        return file;
    }
    file << std::fixed << std::setprecision(5);
    csvRow(file, first, args...);
    return file;
}

inline void csvEnd(std::ofstream& file) {
    file.close();
}