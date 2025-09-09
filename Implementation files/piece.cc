#include "piece.h"
#include "board.h"
#include <algorithm>

// Base Piece class implementation
Piece::Piece(Colour colour, char symbol) : colour(colour), symbol(symbol), hasMoved(false) {}

// King implementation
King::King(Colour colour) : Piece(colour, colour == Colour::WHITE ? 'K' : 'k') {}

bool King::isValidMove(const Position& from, const Position& to, const Board& board) const {
    int rowDiff = abs(to.getRow() - from.getRow());
    int colDiff = abs(to.getCol() - from.getCol());
    
    // Normal king moves (one square in any direction)
    if (rowDiff <= 1 && colDiff <= 1 && !(rowDiff == 0 && colDiff == 0)) {
        return true;
    }
    
    // Castling moves (2 squares horizontally)
    if (rowDiff == 0 && colDiff == 2) {
        // King-side castling (move right)
        if (to.getCol() > from.getCol()) {
            return board.canCastleKingSide(colour);
        }
        // Queen-side castling (move left)
        else {
            return board.canCastleQueenSide(colour);
        }
    }
    
    return false;
}

std::vector<Position> King::getPossibleMoves(const Position& from, const Board& board) const {
    std::vector<Position> moves;
    
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (dr == 0 && dc == 0) continue;
            
            Position to(from.getRow() + dr, from.getCol() + dc); // construct possible move
            if (to.isValid() && isValidMove(from, to, board)) { // Check if valid 
                Piece* targetPiece = board.getPiece(to); // Can capture, or is empty. 
                if (!targetPiece || targetPiece->getColour() != colour) {
                    moves.push_back(to);
                }
            }
        }
    }
    
    return moves;
}

std::unique_ptr<Piece> King::clone() const {
    auto newKing = std::make_unique<King>(colour);
    newKing->hasMoved = this->hasMoved;
    return newKing;
}

// Queen implementation
Queen::Queen(Colour colour) : Piece(colour, colour == Colour::WHITE ? 'Q' : 'q') {}

bool Queen::isValidMove(const Position& from, const Position& to, const Board& board) const {
    int rowDiff = abs(to.getRow() - from.getRow());
    int colDiff = abs(to.getCol() - from.getCol());
    
    // Queen moves like rook (straight) or bishop (diagonal)
    bool isStraight = (rowDiff == 0 || colDiff == 0);
    bool isDiagonal = (rowDiff == colDiff);
    
    if (!isStraight && !isDiagonal) return false;
    
    // Check path is clear
    int rowStep = (to.getRow() > from.getRow()) ? 1 : (to.getRow() < from.getRow()) ? -1 : 0;
    int colStep = (to.getCol() > from.getCol()) ? 1 : (to.getCol() < from.getCol()) ? -1 : 0;
    
    Position current(from.getRow() + rowStep, from.getCol() + colStep);
    while (current != to) {
        if (board.getPiece(current) != nullptr) return false;
        current = Position(current.getRow() + rowStep, current.getCol() + colStep);
    }
    
    return true;
}

std::vector<Position> Queen::getPossibleMoves(const Position& from, const Board& board) const {
    std::vector<Position> moves;
    
    // All 8 directions (rook + bishop)
    int directions[8][2] = {{-1,-1}, {-1,0}, {-1,1}, {0,-1}, {0,1}, {1,-1}, {1,0}, {1,1}};
    
    for (auto& dir : directions) {
        for (int i = 1; i < 8; i++) {
            Position to(from.getRow() + i * dir[0], from.getCol() + i * dir[1]);
            if (!to.isValid()) break;
            
            Piece* targetPiece = board.getPiece(to);
            if (targetPiece) { // skip if empty
                if (targetPiece->getColour() != colour) {
                    moves.push_back(to);
                }
                break;
            }
            moves.push_back(to);
        }
    }
    
    return moves;
}

std::unique_ptr<Piece> Queen::clone() const {
    auto newQueen = std::make_unique<Queen>(colour);
    newQueen->hasMoved = this->hasMoved;
    return newQueen;
}

// Rook implementation
Rook::Rook(Colour colour) : Piece(colour, colour == Colour::WHITE ? 'R' : 'r') {}

bool Rook::isValidMove(const Position& from, const Position& to, const Board& board) const {
    // Rook moves in straight lines (either same row OR same column, but not both different)
    if (from.getRow() != to.getRow() && from.getCol() != to.getCol()) return false;
    
    // Check path is clear
    int rowStep = (to.getRow() > from.getRow()) ? 1 : (to.getRow() < from.getRow()) ? -1 : 0;
    int colStep = (to.getCol() > from.getCol()) ? 1 : (to.getCol() < from.getCol()) ? -1 : 0; // checks horizontal and vertical direction
    
    Position current(from.getRow() + rowStep, from.getCol() + colStep);
    while (current != to) {
        if (board.getPiece(current) != nullptr) return false;
        current = Position(current.getRow() + rowStep, current.getCol() + colStep);
    }
    
    return true;
}

std::vector<Position> Rook::getPossibleMoves(const Position& from, const Board& board) const {
    std::vector<Position> moves;
    
    // 4 directions (up, down, left, right)
    int directions[4][2] = {{-1,0}, {1,0}, {0,-1}, {0,1}};
    
    for (auto& dir : directions) {
        for (int i = 1; i < 8; i++) {
            Position to(from.getRow() + i * dir[0], from.getCol() + i * dir[1]);
            if (!to.isValid()) break;
            
            Piece* targetPiece = board.getPiece(to);
            if (targetPiece) {
                if (targetPiece->getColour() != colour) {
                    moves.push_back(to);
                }
                break;
            }
            moves.push_back(to);
        }
    }
    
    return moves;
}

std::unique_ptr<Piece> Rook::clone() const {
    auto newRook = std::make_unique<Rook>(colour);
    newRook->hasMoved = this->hasMoved;
    return newRook;
}

// Bishop implementation
Bishop::Bishop(Colour colour) : Piece(colour, colour == Colour::WHITE ? 'B' : 'b') {}

bool Bishop::isValidMove(const Position& from, const Position& to, const Board& board) const {
    int rowDiff = abs(to.getRow() - from.getRow());
    int colDiff = abs(to.getCol() - from.getCol());
    
    // Bishop moves diagonally
    if (rowDiff != colDiff) return false;
    
    // Check path is clear
    int rowStep = (to.getRow() > from.getRow()) ? 1 : -1;
    int colStep = (to.getCol() > from.getCol()) ? 1 : -1;
    
    Position current(from.getRow() + rowStep, from.getCol() + colStep);
    while (current != to) {
        if (board.getPiece(current) != nullptr) return false;
        current = Position(current.getRow() + rowStep, current.getCol() + colStep);
    }
    
    return true;
}

std::vector<Position> Bishop::getPossibleMoves(const Position& from, const Board& board) const {
    std::vector<Position> moves;
    
    // 4 diagonal directions
    int directions[4][2] = {{-1,-1}, {-1,1}, {1,-1}, {1,1}};
    
    for (auto& dir : directions) {
        for (int i = 1; i < 8; i++) {
            Position to(from.getRow() + i * dir[0], from.getCol() + i * dir[1]);
            if (!to.isValid()) break;
            
            Piece* targetPiece = board.getPiece(to);
            if (targetPiece) {
                if (targetPiece->getColour() != colour) {
                    moves.push_back(to);
                }
                break;
            }
            moves.push_back(to);
        }
    }
    
    return moves;
}

std::unique_ptr<Piece> Bishop::clone() const {
    auto newBishop = std::make_unique<Bishop>(colour);
    newBishop->hasMoved = this->hasMoved;
    return newBishop;
}

// Knight implementation
Knight::Knight(Colour colour) : Piece(colour, colour == Colour::WHITE ? 'N' : 'n') {}

bool Knight::isValidMove(const Position& from, const Position& to, const Board& board) const {
    int rowDiff = abs(to.getRow() - from.getRow());
    int colDiff = abs(to.getCol() - from.getCol());
    
    // Knight moves in L-shape: 2+1 or 1+2
    return (rowDiff == 2 && colDiff == 1) || (rowDiff == 1 && colDiff == 2);
}

std::vector<Position> Knight::getPossibleMoves(const Position& from, const Board& board) const {
    std::vector<Position> moves;
    
    // All 8 possible knight moves
    int knightMoves[8][2] = {{-2,-1}, {-2,1}, {-1,-2}, {-1,2}, {1,-2}, {1,2}, {2,-1}, {2,1}};
    
    for (auto& move : knightMoves) {
        Position to(from.getRow() + move[0], from.getCol() + move[1]);
        if (to.isValid()) {
            Piece* targetPiece = board.getPiece(to);
            if (!targetPiece || targetPiece->getColour() != colour) {
                moves.push_back(to);
            }
        }
    }
    
    return moves;
}

std::unique_ptr<Piece> Knight::clone() const {
    auto newKnight = std::make_unique<Knight>(colour);
    newKnight->hasMoved = this->hasMoved;
    return newKnight;
}

// Pawn implementation
Pawn::Pawn(Colour colour) : Piece(colour, colour == Colour::WHITE ? 'P' : 'p'), canEnPassant(false) {}

bool Pawn::isValidMove(const Position& from, const Position& to, const Board& board) const {
    int direction = (colour == Colour::WHITE) ? 1 : -1;
    int rowDiff = to.getRow() - from.getRow();
    int colDiff = abs(to.getCol() - from.getCol());
    
    // Forward move
    if (colDiff == 0) {
        if (rowDiff == direction && !board.getPiece(to)) return true;
        if (rowDiff == 2 * direction && !hasMoved && !board.getPiece(to) && !board.getPiece(Position(from.getRow() + direction, from.getCol()))) return true;
    }
    // Diagonal capture
    else if (colDiff == 1 && rowDiff == direction) {
        Piece* targetPiece = board.getPiece(to);
        if (targetPiece && targetPiece->getColour() != colour) return true;
        
        // En passant
        if (board.isEnPassant(from, to, colour)) return true;
    }
    
    return false;
}

std::vector<Position> Pawn::getPossibleMoves(const Position& from, const Board& board) const {
    std::vector<Position> moves;
    int direction = (colour == Colour::WHITE) ? 1 : -1;
    
    // Forward move
    Position oneStep(from.getRow() + direction, from.getCol());
    if (oneStep.isValid() && !board.getPiece(oneStep)) {
        moves.push_back(oneStep);
        
        // Two steps from starting position
        if (!hasMoved) {
            Position twoStep(from.getRow() + 2 * direction, from.getCol());
            if (twoStep.isValid() && !board.getPiece(twoStep)) {
                moves.push_back(twoStep);
            }
        }
    }
    
    // Diagonal captures
    for (int dc = -1; dc <= 1; dc += 2) {
        Position capture(from.getRow() + direction, from.getCol() + dc);
        if (capture.isValid()) {
            Piece* targetPiece = board.getPiece(capture);
            if (targetPiece && targetPiece->getColour() != colour) {
                moves.push_back(capture);
            }
            // En passant would be checked in isValidMove
        }
    }
    
    return moves;
}

std::unique_ptr<Piece> Pawn::clone() const {
    auto newPawn = std::make_unique<Pawn>(colour);
    newPawn->hasMoved = this->hasMoved;
    newPawn->canEnPassant = this->canEnPassant;
    return newPawn;
}

bool Pawn::isPromotionRank(const Position& pos) const {
    return (colour == Colour::WHITE && pos.getRow() == 8) || 
           (colour == Colour::BLACK && pos.getRow() == 1);
} 



