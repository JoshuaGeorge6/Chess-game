#include "textDisplay.h"
#include "board.h"
#include "piece.h"

TextDisplay::TextDisplay(const Board* board) : ChessDisplay(board) {}

void TextDisplay::notify() {
    displayBoard();
}

char TextDisplay::getEmptySquareChar(int row, int col) const {
    // Chess board pattern: light squares are spaces, dark squares are underscores
    // Light squares: (row + col) is odd
    // Dark squares: (row + col) is even
    bool isLightSquare = ((row + col) % 2 == 1);
    return isLightSquare ? ' ' : '_';
} 

void TextDisplay::displayBoard() {
    if (!board) return;
    
    // Display the board from rank 8 to rank 1 (top to bottom)
    for (int row = 8; row >= 1; row--) {
        std::cout << row << " ";
        
        for (int col = 1; col <= 8; col++) {
            Position pos(row, col);
            Piece* piece = board->getPiece(pos);
            
            if (piece) {
                std::cout << piece->getSymbol();
            } else {
                std::cout << getEmptySquareChar(row, col);
            }
        }
        std::cout << std::endl;
    }
    
    // Display column labels
    std::cout << std::endl << "  abcdefgh" << std::endl;
}

