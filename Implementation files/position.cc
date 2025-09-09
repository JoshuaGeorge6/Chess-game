#include "position.h"
#include <sstream>

Position::Position(int row, int col) : row(row), col(col) {}

bool Position::isValid() const {
    return row >= 1 && row <= 8 && col >= 1 && col <= 8;
}

bool Position::operator==(const Position& other) const {
    return row == other.row && col == other.col;
}

bool Position::operator!=(const Position& other) const {
    return !(*this == other);
} 



