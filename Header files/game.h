#ifndef GAME_H
#define GAME_H

#include <string>
#include <memory>
#include "player.h"
#include "colour.h"


// Within this file we will make use of everything inside board

// Forward declarations
class Board;
class ChessDisplay;

class Game {
private:
    std::unique_ptr<Board> board;
    std::unique_ptr<Player> whitePlayer;
    std::unique_ptr<Player> blackPlayer;
    Colour currentTurn;
    bool gameInProgress;
    bool isSetupBoard;  // True if current board came from setup mode
    int whiteScore;
    int blackScore;
    void initializePlayers(const std::string& whitePlayer, const std::string& blackPlayer);
    void resetGame();
    void updateScore(Colour winner);
    void announceCurrentPlayer();

public:
    // Constructor and destructor
    Game();
    ~Game();

    // Core game management methods
    void startGame(const std::string& whitePlayer, const std::string& blackPlayer); // Takes in human or Computer[1-4]. 
    void resign();
    void makePlayerMove(const Position& curr, const Position& dest, char promotion);
    void makeComputerMove();
    void enterSetupMode(); // shall stop normalc game flow, and allow configuration to happen cleanly. 
    void setupAddPiece(char piece, const Position& pos);
    void setupRemovePiece(const Position& pos); // Removes piece at that position (1, 1) to (8, 8).
    void setupSetTurn(Colour colour);
    bool isValidSetup(); // Makes sure we have exactly 2 different colour Kings, no pawn about to be promoted etc.
    bool isGameOver() const;
    void displayScore() const;
    void clearBoard();
    void switchTurn();

    // Getters
    Colour getCurrentTurn() const { return currentTurn; }
    bool isGameInProgress() const { return gameInProgress; }
    bool isFromSetup() const { return isSetupBoard; }
    Board* getBoard() const;
    Player* getCurrentPlayer() const;
    Player* getPlayer(Colour colour) const;
    int getWhiteScore() const { return whiteScore; }
    int getBlackScore() const { return blackScore; }
    
    // Score management
    void setScores(int white, int black);
    
    // Display management
    void addDisplay(ChessDisplay* display);
    void removeDisplay(ChessDisplay* display);


};

#endif // GAME_H
