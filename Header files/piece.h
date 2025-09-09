#ifndef PIECE_H
#define PIECE_H

#include "position.h"
#include "colour.h"
#include <vector>
#include <memory>

// Forward declaration
class Board;

class Piece {
protected:
    Colour colour;
    char symbol;
    bool hasMoved;

public:
    Piece(Colour colour, char symbol);
    virtual ~Piece() = default;
    
    // Pure virtual methods that must be implemented by subclasses
    virtual bool isValidMove(const Position& from, const Position& to, const Board& board) const = 0;
    virtual std::vector<Position> getPossibleMoves(const Position& from, const Board& board) const = 0;
    virtual std::unique_ptr<Piece> clone() const = 0;
    
    // Common methods
    Colour getColour() const { return colour; }
    char getSymbol() const { return symbol; }
    bool getHasMoved() const { return hasMoved; }
    void setHasMoved(bool moved) { hasMoved = moved; }
    
    // Virtual method for piece type identification
    virtual std::string getType() const = 0;
};

class King : public Piece {
public:
    King(Colour colour);
    bool isValidMove(const Position& from, const Position& to, const Board& board) const override;
    std::vector<Position> getPossibleMoves(const Position& from, const Board& board) const override;
    std::unique_ptr<Piece> clone() const override;
    std::string getType() const override { return "King"; }
};

class Queen : public Piece {
public:
    Queen(Colour colour);
    bool isValidMove(const Position& from, const Position& to, const Board& board) const override;
    std::vector<Position> getPossibleMoves(const Position& from, const Board& board) const override;
    std::unique_ptr<Piece> clone() const override;
    std::string getType() const override { return "Queen"; }
};

class Rook : public Piece {
public:
    Rook(Colour colour);
    bool isValidMove(const Position& from, const Position& to, const Board& board) const override;
    std::vector<Position> getPossibleMoves(const Position& from, const Board& board) const override;
    std::unique_ptr<Piece> clone() const override;
    std::string getType() const override { return "Rook"; }
};

class Bishop : public Piece {
public:
    Bishop(Colour colour);
    bool isValidMove(const Position& from, const Position& to, const Board& board) const override;
    std::vector<Position> getPossibleMoves(const Position& from, const Board& board) const override;
    std::unique_ptr<Piece> clone() const override;
    std::string getType() const override { return "Bishop"; }
};

class Knight : public Piece {
public:
    Knight(Colour colour);
    bool isValidMove(const Position& from, const Position& to, const Board& board) const override;
    std::vector<Position> getPossibleMoves(const Position& from, const Board& board) const override;
    std::unique_ptr<Piece> clone() const override;
    std::string getType() const override { return "Knight"; }
};

class Pawn : public Piece {
private:
    bool canEnPassant;
    
public:
    Pawn(Colour colour);
    bool isValidMove(const Position& from, const Position& to, const Board& board) const override;
    std::vector<Position> getPossibleMoves(const Position& from, const Board& board) const override;
    std::unique_ptr<Piece> clone() const override;
    std::string getType() const override { return "Pawn"; }
    
    // Pawn-specific methods
    bool getCanEnPassant() const { return canEnPassant; }
    void setCanEnPassant(bool canEP) { canEnPassant = canEP; }
    bool isPromotionRank(const Position& pos) const;
};

#endif // PIECE_H




