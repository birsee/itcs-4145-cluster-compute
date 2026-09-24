#pragma once
#include <string>
#include <vector>
#include <cstdlib>

enum FillType { 
    Same, 
    Sorted, 
    Reverse, 
    Random,
    Count
};

std::string fillTypeName(FillType type);

template <typename T> void fill(std::vector<T>& a, FillType type) {
    size_t n = a.size();
    switch (type) {
        case FillType::Same: 
            for (size_t i = 0; i < n; i++) a[i] = static_cast<T>(1);
            break;
        case FillType::Sorted:
            for (size_t i = 0; i < n; i++) a[i] = static_cast<T>(i);
            break;
        case FillType::Reverse:
            for (size_t i = 0; i < n; i++) a[i] = static_cast<T>(n - i);
            break;
        case FillType::Random: 
            for (size_t i = 0; i < n; i++) a[i] = static_cast<T>(std::rand() % 1000);
            break;
    }
}