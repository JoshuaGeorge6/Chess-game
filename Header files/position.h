#ifndef POSITION_H
#define POSITION_H

#include <string>

class Position {
private:
    int row;
    int col;

public:
    Position(int row, int col);
    
    // Getters
    int getRow() const { return row; }
    int getCol() const { return col; }
    
    // Utility methods
    bool isValid() const; // Check if we went out of bounds on the board later.
    
    // Operators
    bool operator==(const Position& other) const;
    bool operator!=(const Position& other) const;
};

#endif // POSITION_H



