#include "board.h"
#include "piece.h"
#include "display.h"
#include <iostream>
#include <algorithm>

Board::Board() : lastMoveFrom(0, 0), lastMoveTo(0, 0),
                 whiteKingMoved(false), blackKingMoved(false),
                 whiteRookKingMoved(false), whiteRookQueenMoved(false),
                 blackRookKingMoved(false), blackRookQueenMoved(false) {
    // Initialize 8x8 board with nullptr
    grid.resize(8);
    for (int i = 0; i < 8; i++) {
        grid[i].resize(8);
    }
}

Board::~Board() = default;

void Board::notifyObservers() const {
    for (ChessDisplay* observer : observers) {
        if (observer) {
            observer->notify();
        }
    }
}

void Board::setupStartingPosition() {
    // Clear the board first
    clear();
    
    // Place white pieces (row 1-2)
    grid[0][0] = std::make_unique<Rook>(Colour::WHITE);
    grid[0][1] = std::make_unique<Knight>(Colour::WHITE);
    grid[0][2] = std::make_unique<Bishop>(Colour::WHITE);
    grid[0][3] = std::make_unique<Queen>(Colour::WHITE);
    grid[0][4] = std::make_unique<King>(Colour::WHITE);
    grid[0][5] = std::make_unique<Bishop>(Colour::WHITE);
    grid[0][6] = std::make_unique<Knight>(Colour::WHITE);
    grid[0][7] = std::make_unique<Rook>(Colour::WHITE);
    
    // White pawns
    for (int col = 0; col < 8; col++) {
        grid[1][col] = std::make_unique<Pawn>(Colour::WHITE);
    }
    
    // Place black pieces (row 7-8)
    grid[7][0] = std::make_unique<Rook>(Colour::BLACK);
    grid[7][1] = std::make_unique<Knight>(Colour::BLACK);
    grid[7][2] = std::make_unique<Bishop>(Colour::BLACK);
    grid[7][3] = std::make_unique<Queen>(Colour::BLACK);
    grid[7][4] = std::make_unique<King>(Colour::BLACK);
    grid[7][5] = std::make_unique<Bishop>(Colour::BLACK);
    grid[7][6] = std::make_unique<Knight>(Colour::BLACK);
    grid[7][7] = std::make_unique<Rook>(Colour::BLACK);
    
    // Black pawns
    for (int col = 0; col < 8; col++) {
        grid[6][col] = std::make_unique<Pawn>(Colour::BLACK);
    }
    
    // Notify observers of board setup
    notifyObservers();
}

void Board::clearGameHistory() { // Extra, look when needed. 
    lastMoveFrom = Position(0, 0);
    lastMoveTo = Position(0, 0);
}

void Board::resetSpecialRules() {
    whiteKingMoved = false;
    blackKingMoved = false;
    whiteRookKingMoved = false;
    whiteRookQueenMoved = false;
    blackRookKingMoved = false;
    blackRookQueenMoved = false;
    
    // Reset en passant flags for all pawns
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (grid[row][col] && grid[row][col]->getType() == "Pawn") {
                auto* pawn = dynamic_cast<Pawn*>(grid[row][col].get());
                if (pawn) {
                    pawn->setCanEnPassant(false);
                }
            }
        }
    }
}

Piece* Board::getPiece(const Position& pos) const {
    if (!pos.isValid()) return nullptr;
    return grid[pos.getRow() - 1][pos.getCol() - 1].get();  // Convert to 0-indexed
}

bool Board::isInCheck(Colour colour) const {
    // Find the king
    Position kingPos(0, 0);
    bool kingFound = false;
    
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (grid[row][col] && grid[row][col]->getType() == "King" && 
                grid[row][col]->getColour() == colour) {
                kingPos = Position(row + 1, col + 1);  // Convert to 1-indexed
                kingFound = true;
                break;
            }
        }
        if (kingFound) break;
    }
    
    if (!kingFound) return false; // Dumb shii
    
    // Check if any enemy piece can attack the king
    Colour enemyColour = (colour == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;
    
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (grid[row][col] && grid[row][col]->getColour() == enemyColour) { // Look for each piece of the opposite colour, then check if that piece can legally capture the King, using that Piece's individual overloaded isValid() move. 
                Position from(row + 1, col + 1);  // Convert to 1-indexed
                if (grid[row][col]->isValidMove(from, kingPos, *this)) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

bool Board::canCastleKingSide(Colour colour) const {
    if (colour == Colour::WHITE) {
        if (whiteKingMoved || whiteRookKingMoved) return false;
        
        // Check if king and rook are still in their original positions
        Piece* king = grid[0][4].get();  // e1
        Piece* rook = grid[0][7].get();  // h1
        if (!king || king->getType() != "King" || king->getColour() != Colour::WHITE) return false;
        if (!rook || rook->getType() != "Rook" || rook->getColour() != Colour::WHITE) return false;
        
        // Check if path is clear
        if (grid[0][5] || grid[0][6]) return false;  // f1, g1 must be empty
        
        // King must not be in check, and must not pass through check
        if (isInCheck(colour)) return false;
        // TODO: Check if king passes through check at f1
        return true;
    } else {
        if (blackKingMoved || blackRookKingMoved) return false;
        
        // Check if king and rook are still in their original positions
        Piece* king = grid[7][4].get();  // e8
        Piece* rook = grid[7][7].get();  // h8
        if (!king || king->getType() != "King" || king->getColour() != Colour::BLACK) return false;
        if (!rook || rook->getType() != "Rook" || rook->getColour() != Colour::BLACK) return false;
        
        // Check if path is clear
        if (grid[7][5] || grid[7][6]) return false;  // f8, g8 must be empty
        
        // King must not be in check
        if (isInCheck(colour)) return false;
        return true;
    }
}

bool Board::canCastleQueenSide(Colour colour) const {
    if (colour == Colour::WHITE) {
        if (whiteKingMoved || whiteRookQueenMoved) return false;
        
        // Check if king and rook are still in their original positions
        Piece* king = grid[0][4].get();  // e1
        Piece* rook = grid[0][0].get();  // a1
        if (!king || king->getType() != "King" || king->getColour() != Colour::WHITE) return false;
        if (!rook || rook->getType() != "Rook" || rook->getColour() != Colour::WHITE) return false;
        
        // Check if path is clear
        if (grid[0][1] || grid[0][2] || grid[0][3]) return false;  // b1, c1, d1 must be empty
        
        // King must not be in check
        if (isInCheck(colour)) return false;
        return true;
    } else {
        if (blackKingMoved || blackRookQueenMoved) return false;
        
        // Check if king and rook are still in their original positions
        Piece* king = grid[7][4].get();  // e8
        Piece* rook = grid[7][0].get();  // a8
        if (!king || king->getType() != "King" || king->getColour() != Colour::BLACK) return false;
        if (!rook || rook->getType() != "Rook" || rook->getColour() != Colour::BLACK) return false;
        
        // Check if path is clear
        if (grid[7][1] || grid[7][2] || grid[7][3]) return false;  // b8, c8, d8 must be empty
        
        // King must not be in check
        if (isInCheck(colour)) return false;
        return true;
    }
}

bool Board::isEnPassant(const Position& from, const Position& to, Colour turn) const {// Checks for a pawn, and makes sure it can EnPassant
    // Check if this is a valid en passant move
    if (!getPiece(from) || getPiece(from)->getType() != "Pawn") return false;
    
    int direction = (turn == Colour::WHITE) ? 1 : -1;
    
    // Must be diagonal move
    if (abs(to.getCol() - from.getCol()) != 1 || (to.getRow() - from.getRow()) != direction) return false;
    
    // Target square must be empty
    if (getPiece(to)) return false;
    
    // There must be an enemy pawn beside our pawn
    Position enemyPawnPos(from.getRow(), to.getCol());
    Piece* enemyPawn = getPiece(enemyPawnPos);
    
    if (!enemyPawn || enemyPawn->getType() != "Pawn" || enemyPawn->getColour() == turn) return false;
    
    // The enemy pawn must have just moved two squares
    if (lastMoveFrom.getRow() == (turn == Colour::WHITE ? 7 : 2) && 
        lastMoveTo == enemyPawnPos && 
        abs(lastMoveFrom.getRow() - lastMoveTo.getRow()) == 2) {
        return true;
    }
    
    return false;
}

bool Board::isValidMove(const Position& from, const Position& to, Colour turn) const {
    if (!from.isValid() || !to.isValid()) return false; // Check position out of bounds. 
    
    Piece* piece = getPiece(from);
    if (!piece || piece->getColour() != turn) return false; // Can't move another colour's piece/ empty square
    
    Piece* targetPiece = getPiece(to);
    if (targetPiece && targetPiece->getColour() == turn) return false;  // Can't capture own piece
    
    // Check if the piece can make this move
    return piece->isValidMove(from, to, *this); // // Checks specific piece's validity
}

bool Board::wouldBeInCheck(const Position& from, const Position& to, Colour turn) const { // We simulate the move, then see if that would make the current King in check, then revert the board back to the OG state. 
    // Make a temporary copy of the move
    Piece* movingPiece = getPiece(from);
    if (!movingPiece) return true;  // Invalid move (Empty Square) 
    
    // Clone the captured piece BEFORE making the move
    std::unique_ptr<Piece> capturedPieceClone = nullptr;
    Piece* capturedPiece = getPiece(to);
    if (capturedPiece) {
        capturedPieceClone = capturedPiece->clone();
    }
    
    // Temporarily make the move
    const_cast<Board*>(this)->grid[to.getRow() - 1][to.getCol() - 1] = std::move(const_cast<Board*>(this)->grid[from.getRow() - 1][from.getCol() - 1]);
    const_cast<Board*>(this)->grid[from.getRow() - 1][from.getCol() - 1] = nullptr;
    
    bool inCheck = isInCheck(turn);
    
    // Restore the board
    const_cast<Board*>(this)->grid[from.getRow() - 1][from.getCol() - 1] = std::move(const_cast<Board*>(this)->grid[to.getRow() - 1][to.getCol() - 1]);
    if (capturedPieceClone) {
        const_cast<Board*>(this)->grid[to.getRow() - 1][to.getCol() - 1] = std::move(capturedPieceClone);
    } else {
        const_cast<Board*>(this)->grid[to.getRow() - 1][to.getCol() - 1] = nullptr;
    }
    
    return inCheck;
}

// by the time we get here, we made sure the move is valid in all aspects.
void Board::makeMove(const Position& from, const Position& to, char promotion) {
    Piece* piece = getPiece(from);
    if (!piece) return;
    
    // Handle en passant capture
    if (piece->getType() == "Pawn" && isEnPassant(from, to, piece->getColour())) {
        Position enemyPawnPos(from.getRow(), to.getCol());
        grid[enemyPawnPos.getRow() - 1][enemyPawnPos.getCol() - 1] = nullptr;
        // When we capture a piece, it goes off the board, and becomes nullptr
    }
    
    // Handle castling
    if (piece->getType() == "King" && abs(to.getCol() - from.getCol()) == 2) {
        // King-side castling
        if (to.getCol() > from.getCol() && canCastleKingSide(piece->getColour())) {
            grid[from.getRow() - 1][5] = std::move(grid[from.getRow() - 1][7]);  // Move rook
            grid[from.getRow() - 1][7] = nullptr;
        }
        // Queen-side castling
        else if (to.getCol() < from.getCol() && canCastleQueenSide(piece->getColour())) {
            grid[from.getRow() - 1][3] = std::move(grid[from.getRow() - 1][0]);  // Move rook
            grid[from.getRow() - 1][0] = nullptr;
        }
    }
    
    // Update special rule flags
    if (piece->getType() == "King") {
        if (piece->getColour() == Colour::WHITE) whiteKingMoved = true;
        else blackKingMoved = true;
    }
    if (piece->getType() == "Rook") {
        if (piece->getColour() == Colour::WHITE) {
            if (from.getCol() == 1) whiteRookQueenMoved = true;
            if (from.getCol() == 8) whiteRookKingMoved = true;
        } else {
            if (from.getCol() == 1) blackRookQueenMoved = true;
            if (from.getCol() == 8) blackRookKingMoved = true;
        }
    }
    
    // Move the piece
    grid[to.getRow() - 1][to.getCol() - 1] = std::move(grid[from.getRow() - 1][from.getCol() - 1]); // Place the piece at the new grid position (to)
    grid[from.getRow() - 1][from.getCol() - 1] = nullptr; // Where we moved from is nullptr
    
    // Mark piece as moved
    grid[to.getRow() - 1][to.getCol() - 1]->setHasMoved(true);
    
    // Handle pawn promotion
    if (piece->getType() == "Pawn") {
        auto* pawn = dynamic_cast<Pawn*>(grid[to.getRow() - 1][to.getCol() - 1].get());
        if (pawn && pawn->isPromotionRank(to) && promotion != '\0') {
            Colour colour = pawn->getColour();
            switch (promotion) {
              // Overwrites pawn with Queen, Rook, Bishop, or Knight
                case 'Q': case 'q':
                    grid[to.getRow() - 1][to.getCol() - 1] = std::make_unique<Queen>(colour);
                    break;
                case 'R': case 'r':
                    grid[to.getRow() - 1][to.getCol() - 1] = std::make_unique<Rook>(colour);
                    break;
                case 'B': case 'b':
                    grid[to.getRow() - 1][to.getCol() - 1] = std::make_unique<Bishop>(colour);
                    break;
                case 'N': case 'n':
                    grid[to.getRow() - 1][to.getCol() - 1] = std::make_unique<Knight>(colour);
                    break;
            }
        }
    }
    
    // Update last move for en passant
    lastMoveFrom = from;
    lastMoveTo = to;
    
    // Notify observers of board change
    notifyObservers();
}

bool Board::isInCheckmate(Colour colour) const {
    if (!isInCheck(colour)) return false;
    
    // Try all possible moves
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (grid[row][col] && grid[row][col]->getColour() == colour) {
                Position from(row + 1, col + 1);
                auto moves = grid[row][col]->getPossibleMoves(from, *this); // vector<Positions>
                
                for (const auto& to : moves) { // Check if King's colours can help us out of check , as well as if King can help itself.
                    if (isValidMove(from, to, colour) && !wouldBeInCheck(from, to, colour)) {
                        return false;  // Found a legal move
                    }
                }
            }
        }
    }
    
    return true;  // No legal moves found
}

bool Board::isInStalemate(Colour colour) const {
    if (isInCheck(colour)) return false;  // Can't be stalemate if in check
    
    // Try all possible moves
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (grid[row][col] && grid[row][col]->getColour() == colour) {
                Position from(row + 1, col + 1);
                auto moves = grid[row][col]->getPossibleMoves(from, *this);
                
                for (const auto& to : moves) {
                    if (isValidMove(from, to, colour) && !wouldBeInCheck(from, to, colour)) {
                        return false;  // Found a legal move
                    }
                }
            }
        }
    }
    
    return true;  // No legal moves found and not in check = stalemate
}

void Board::addPiece(char pieceChar, const Position& pos) {
    if (!pos.isValid()) return;
    
    Colour colour = (pieceChar >= 'A' && pieceChar <= 'Z') ? Colour::WHITE : Colour::BLACK;
    char lowerChar = tolower(pieceChar); // Standardize the switch cases
    
    switch (lowerChar) {
        case 'k':
            grid[pos.getRow() - 1][pos.getCol() - 1] = std::make_unique<King>(colour);
            break;
        case 'q':
            grid[pos.getRow() - 1][pos.getCol() - 1] = std::make_unique<Queen>(colour);
            break;
        case 'r':
            grid[pos.getRow() - 1][pos.getCol() - 1] = std::make_unique<Rook>(colour);
            break;
        case 'b':
            grid[pos.getRow() - 1][pos.getCol() - 1] = std::make_unique<Bishop>(colour);
            break;
        case 'n':
            grid[pos.getRow() - 1][pos.getCol() - 1] = std::make_unique<Knight>(colour);
            break;
        case 'p':
            grid[pos.getRow() - 1][pos.getCol() - 1] = std::make_unique<Pawn>(colour);
            break;
    }
    
    // Notify observers of piece addition
    notifyObservers();
}

void Board::removePiece(const Position& pos) {
    if (!pos.isValid()) return;
    grid[pos.getRow() - 1][pos.getCol() - 1] = nullptr;
    
    // Notify observers of piece removal
    notifyObservers();
}

int Board::countPieces(char piece) const {
    int count = 0;
    char lowerPiece = tolower(piece); 
    bool isWhite = (piece >= 'A' && piece <= 'Z'); 
    
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (grid[row][col]) {
                char pieceSymbol = grid[row][col]->getSymbol(); // Find random piece
                if (tolower(pieceSymbol) == lowerPiece) {
                    bool pieceIsWhite = (pieceSymbol >= 'A' && pieceSymbol <= 'Z');
                    if (pieceIsWhite == isWhite) {
                        count++;
                    }
                }
            }
        }
    }
    
    return count;
}

bool Board::hasPawnsOnEndRanks() {
    // Check first row (row 1)
    for (int col = 0; col < 8; col++) {
        if (grid[0][col] && grid[0][col]->getType() == "Pawn") return true;
    }
    
    // Check last row (row 8)
    for (int col = 0; col < 8; col++) {
        if (grid[7][col] && grid[7][col]->getType() == "Pawn") return true;
    }
    
    return false;
}

bool Board::isValidSetup() const {
    // Check for exactly one king of each color
    if (countPieces('K') != 1 || countPieces('k') != 1) return false;
    
    // Check for no pawns on first or last row
    if (const_cast<Board*>(this)->hasPawnsOnEndRanks()) return false;
    
    // Check that neither king is in check
    if (isInCheck(Colour::WHITE) || isInCheck(Colour::BLACK)) return false;
    
    return true;
}

void Board::clear() {
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            grid[row][col] = nullptr;
        }
    }
}

void Board::init() {
    setupStartingPosition();
}

// Observer pattern implementation
void Board::addObserver(ChessDisplay* observer) {
    if (observer) {
        observers.push_back(observer);
    }
}

void Board::removeObserver(ChessDisplay* observer) {
    observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
}


