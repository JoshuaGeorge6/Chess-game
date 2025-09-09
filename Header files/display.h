#ifndef DISPLAY_H
#define DISPLAY_H

#include "colour.h"

// Forward declaration
class Board;

// Abstract Display class (Observer)
class ChessDisplay {
protected:
    const Board* board;  // Pointer to the board being observed

public:
    ChessDisplay(const Board* board);
    virtual ~ChessDisplay() = default;
    
    // Pure virtual methods that concrete displays must implement
    virtual void notify() = 0;  // Called when board changes - no parameter needed
};

#endif // DISPLAY_H
