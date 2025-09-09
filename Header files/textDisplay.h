#ifndef TEXTDISPLAY_H
#define TEXTDISPLAY_H

#include "display.h"
#include <iostream>
#include <string>

class TextDisplay : public ChessDisplay {
public:
    TextDisplay(const Board* board);
    ~TextDisplay() = default;
    
    // Observer pattern methods
    void notify() override;

private:
    void displayBoard();
    char getEmptySquareChar(int row, int col) const;
};

#endif // TEXTDISPLAY_H

