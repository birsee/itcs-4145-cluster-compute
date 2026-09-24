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

std::string fillTypeName(FillType type) {
    switch (type) {
        case FillType::Same: return "Same";
        case FillType::Sorted: return "Sorted";
        case FillType::Reverse: return "Reverse";
        case FillType::Random: return "Random";
        default: return "Unknown";
    }
}