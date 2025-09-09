
#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <vector>
#include <memory>
#include "colour.h"

// Forward declarations
class Board;
class Position;

class Player {
protected:
    Colour colour;
    // Remove: std::string name;

public:
    Player(Colour colour);  // Simplified constructor
    virtual ~Player() = default;
    
    // Pure virtual method for making moves
    virtual std::string getMove(const Board& board) = 0;
    
    // Getters
    Colour getColour() const { return colour; }
    
    // Virtual method for player type identification
    virtual std::string getType() const = 0;

protected:
    // Helper method for AI players to get all legal moves
    std::vector<std::string> getAllLegalMoves(const Board& board) const;
    std::string positionToString(const Position& pos) const;
    
    // Helper methods for AI decision making
    bool putsEnemyInCheck(const std::string& move, const Board& board) const;
    bool avoidsCapture(const std::string& move, const Board& board) const;
    int getPieceValue(const std::string& pieceType) const;
};

// Subclasses

class HumanPlayer : public Player {
    
public:
    HumanPlayer(Colour colour);
    std::string getMove(const Board& board) override;
    std::string getType() const override { return "Human"; }
};

class ComputerPlayer1 : public Player {
public:
    ComputerPlayer1(Colour colour);
    std::string getMove(const Board& board) override;
    std::string getType() const override { return "Computer Level 1"; }
    int getLevel() const { return 1; }
};

class ComputerPlayer2 : public Player {
public:
    ComputerPlayer2(Colour colour);
    std::string getMove(const Board& board) override;
    std::string getType() const override { return "Computer Level 2"; }
    int getLevel() const { return 2; }
};

class ComputerPlayer3 : public Player {
public:
    ComputerPlayer3(Colour colour);
    std::string getMove(const Board& board) override;
    std::string getType() const override { return "Computer Level 3"; }
    int getLevel() const { return 3; }
};

class ComputerPlayer4 : public Player {
public:
    ComputerPlayer4(Colour colour);
    std::string getMove(const Board& board) override;
    std::string getType() const override { return "Computer Level 4"; }
    int getLevel() const { return 4; }
};

#endif // PLAYER_H


