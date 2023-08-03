#include "domain.h"

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}