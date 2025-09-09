#include "game.h"
#include "position.h"
#include "playerFactory.h"
#include "display.h"
#include "board.h"
#include <iostream>
#include <stdexcept>

Game::Game() 
    : currentTurn(Colour::WHITE)
    , gameInProgress(false)
    , isSetupBoard(false)
    , whiteScore(0)
    , blackScore(0) {
    // Initialize board and command interpreter when we have those classes
    // For now, just initialize the basic state
}


Game::~Game() = default;


void Game::initializePlayers(const std::string& whitePlayerType, const std::string& blackPlayerType) {
    try {
        // Create players using the factory
        whitePlayer = PlayerFactory::createPlayer(whitePlayerType, Colour::WHITE);
        blackPlayer = PlayerFactory::createPlayer(blackPlayerType, Colour::BLACK);
        
        std::cout << "Initialized players:" << std::endl;
        std::cout << "White: " << whitePlayer->getType() << std::endl;
        std::cout << "Black: " << blackPlayer->getType() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error initializing players: " << e.what() << std::endl;
        throw;
    }
}


void Game::resetGame() {
    // Reset board to starting chess position
    if (board) {
        board->setupStartingPosition();  // Set up standard chess starting position
        board->clearGameHistory();       // Clear move history, captured pieces, etc.
        board->resetSpecialRules();      // Reset castling rights, en passant, etc.
    } else {
        // Create new board if it doesn't exist
        board = std::make_unique<Board>();
        board->setupStartingPosition(); // Make clean chess board.
    }
    
    // Reset game state variables
    currentTurn = Colour::WHITE;        // White always starts
    gameInProgress = false;             // Will be set to true after reset completes
    isSetupBoard = false;               // This is now a regular game board
    
    // Reset any game-specific counters or flags
    // (These would be added when implementing draw rules, etc.)
    
    std::cout << "Board reset to starting position." << std::endl;
}




void Game::startGame(const std::string& white_p1, const std::string& black_p2) {
    try {
        initializePlayers(white_p1, black_p2); // Makes players valid, human / computer * Probably in Players
        
        // Only reset if we don't have a board (fresh start)
        if (!board) {
            resetGame(); // Set up standard starting position
        } else {
            // We have a custom setup - just reset game state but preserve board and current turn
            gameInProgress = false;             // Will be set to true after setup completes
            board->clearGameHistory();          // Clear move history, captured pieces, etc.
            board->resetSpecialRules();         // Reset castling rights, en passant, etc.
            // Keep currentTurn as set in setup mode (don't force it to WHITE)
        }
        
        gameInProgress = true;
        std::cout << "New game started. " << (currentTurn == Colour::WHITE ? "White" : "Black") << " goes first." << std::endl;
        announceCurrentPlayer();
    } catch (const std::exception& e) {
        std::cerr << "Error starting game: " << e.what() << std::endl;
    }
}


void Game::updateScore(Colour winner) {
    if (winner == Colour::WHITE) {
        whiteScore++;
    } else {
        blackScore++;
    }
}

void Game::setScores(int white, int black) {
    whiteScore = white;
    blackScore = black;
}

void Game::announceCurrentPlayer() {
    if (!gameInProgress) return;
    
    Player* currentPlayer = getCurrentPlayer();
    if (!currentPlayer) return;
    
    std::string playerColor = (currentTurn == Colour::WHITE) ? "White" : "Black";
    std::string playerType = currentPlayer->getType();
    
    std::cout << playerColor << " player's turn (" << playerType << ")" << std::endl;
} 


void Game::resign() {
    if (!gameInProgress) {
        std::cout << "No game in progress." << std::endl;
        return;
    }
    
    Colour winner = (currentTurn == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;
    updateScore(winner);
    
    std::string winnerStr = (winner == Colour::WHITE) ? "White" : "Black";
    std::cout << winnerStr << " wins!" << std::endl;
    
    gameInProgress = false; // Game just ended, so yes.
}


void Game::switchTurn() {
    currentTurn = (currentTurn == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;
}


void Game::makePlayerMove(const Position& curr, const Position& dest, char promotion) {
    if (!gameInProgress) {
        std::cout << "No game in progress." << std::endl;
        return;
    }
    
    // Validate that it's the current player's turn and they have a piece at the source
    if (!board->isValidMove(curr, dest, currentTurn)) { // Probably gonna call canCastleKingSide, canCastleQueenSide, canEnPassant
        std::cout << "Invalid move!" << std::endl;
        return;
    }
    
    // Check if the move puts own king in check
    if (board->wouldBeInCheck(curr, dest, currentTurn)) {
        std::cout << "Move would put your king in check!" << std::endl;
        return;
    }

    // Up until this point we were validating the move.
    
    // Execute the move and does promotion if valid
    board->makeMove(curr, dest, promotion); // Will make use of removePiece internally
    
    
    if (promotion != '\0') {
        std::cout << " There is a promotion to " << promotion;
    }
    std::cout << std::endl;
    
    // Check for game ending conditions
    switchTurn();
    
    if (board->isInCheckmate(currentTurn)) { // Will check based of CurrentTurn's King
        Colour winner = (currentTurn == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;
        std::cout << "Checkmate! " << (winner == Colour::WHITE ? "White" : "Black") << " wins!" << std::endl;
        updateScore(winner);
        gameInProgress = false;
    } else if (board->isInStalemate(currentTurn)) {
        std::cout << "Stalemate! The game is a draw." << std::endl;
        gameInProgress = false;
    } else if (board->isInCheck(currentTurn)) {
        std::cout << (currentTurn == Colour::WHITE ? "White" : "Black") << " is in check!" << std::endl;
        announceCurrentPlayer();
    } else {
        announceCurrentPlayer();
    }
}


Player* Game::getCurrentPlayer() const {
    if (!whitePlayer || !blackPlayer) {
        return nullptr;
    }
    return (currentTurn == Colour::WHITE) ? whitePlayer.get() : blackPlayer.get();
}


void Game::makeComputerMove() {
    if (!gameInProgress) {
        std::cout << "No game in progress." << std::endl;
        return;
    }
    
    Player* currentPlayer = getCurrentPlayer();
    if (!currentPlayer) {
        std::cout << "No current player available." << std::endl;
        return;
    }

    // Get the computer's move choice
    std::string moveStr = currentPlayer->getMove(*board);
    
    Position from(moveStr[1] - '0', moveStr[0] - 'a' + 1);
    Position to(moveStr[3] - '0', moveStr[2] - 'a' + 1);
    char promotion = (moveStr.length() == 5) ? moveStr[4] : '\0';
    
    // Validate and execute the move
    if (!board->isValidMove(from, to, currentTurn)) {
        std::cout << "Computer attempted invalid move!" << std::endl;
        return;
    }
    
    // the move was already constructed by player functions.
    board->makeMove(from, to, promotion);
    std::cout << "Computer makes move: " << moveStr << std::endl;
    
    // Check for game ending conditions
    switchTurn();
    
    if (board->isInCheckmate(currentTurn)) {
        Colour winner = (currentTurn == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;
        std::cout << "Checkmate! " << (winner == Colour::WHITE ? "White" : "Black") << " wins!" << std::endl;
        updateScore(winner);
        gameInProgress = false;
    } else if (board->isInStalemate(currentTurn)) {
        std::cout << "Stalemate! The game is a draw." << std::endl;
        gameInProgress = false;
    } else if (board->isInCheck(currentTurn)) {
        std::cout << (currentTurn == Colour::WHITE ? "White" : "Black") << " is in check!" << std::endl;
        announceCurrentPlayer();
    } else {
        announceCurrentPlayer();
    }
}


void Game::enterSetupMode() {
    if (gameInProgress) {
        std::cout << "Cannot enter setup mode while game is in progress." << std::endl;
        return;
    }
    
    // Create board if it doesn't exist
    if (!board) {
        board = std::make_unique<Board>();
        board->clear();  // Start with empty board in setup mode
    }
    
    isSetupBoard = true;  // Mark this board as coming from setup mode
    std::cout << "Setup mode activated." << std::endl;
    
    // Display the initial board when entering setup mode
    if (board) {
        board->notifyObservers();
    }
}


void Game::setupAddPiece(char piece, const Position& pos) {
    if (!board) {
        board = std::make_unique<Board>();
        
    }
    
    if (!pos.isValid()) {
        std::cout << "Invalid position for piece placement." << std::endl;
        return;
    }
    
    board->addPiece(piece, pos); // adds a piece in setup mode (already notifies observers)
}


void Game::setupRemovePiece(const Position& pos) {
    if (!board) {
        std::cout << "No board available for piece removal." << std::endl;
        return;
    }
    
    if (!pos.isValid()) {
        std::cout << "Invalid position for piece removal." << std::endl;
        return;
    }
    
    board->removePiece(pos); // removes a piece in setup mode (already notifies observers)
}


void Game::setupSetTurn(Colour colour) {
    currentTurn = colour;
    std::cout << "Set turn to " << (colour == Colour::WHITE ? "White" : "Black") << std::endl;
}


bool Game::isValidSetup() {
    if (!board) {
        std::cout << "No board available for validation." << std::endl;
        return false;
    }
    
    std::cout << "Validating setup..." << std::endl;
    
    // Check for exactly one king of each color
    if (board->countPieces('K') != 1 || board->countPieces('k') != 1) {
        std::cout << "Setup invalid: Must have exactly one white and one black king." << std::endl;
        return false;
    }
    
    // Check for no pawns on first or last row
    if (board->hasPawnsOnEndRanks()) {
        std::cout << "Setup invalid: Pawns cannot be on first or last row." << std::endl;
        return false;
    }
    
    // Check that neither king is in check in the starting position
    if (board->isInCheck(Colour::WHITE) || board->isInCheck(Colour::BLACK)) {
        std::cout << "Setup invalid: Kings cannot be in check in starting position." << std::endl;
        return false;
    }
    
    std::cout << "Setup is valid." << std::endl;
    return true;
}


bool Game::isGameOver() const {
    if (!gameInProgress) {
        return true;
    }
    
    if (!board) { // error-handling
        return false;
    }
    
    // Game is over if current player is in checkmate or stalemate
    return board->isInCheckmate(currentTurn) || board->isInStalemate(currentTurn);
}


void Game::displayScore() const {
    std::cout << "Final Score:" << std::endl;
    std::cout << "White: " << whiteScore << std::endl;
    std::cout << "Black: " << blackScore << std::endl;
}


void Game::clearBoard() {
    if (!board) {
        board = std::make_unique<Board>();
    } else {
        board->clear();  // Remove all pieces from the board
    }
    std::cout << "Board cleared." << std::endl;
}


Board* Game::getBoard() const {
    return board.get();
}


Player* Game::getPlayer(Colour colour) const {
    if (!whitePlayer || !blackPlayer) {
        return nullptr;
    }
    return (colour == Colour::WHITE) ? whitePlayer.get() : blackPlayer.get();
}

// Display management
void Game::addDisplay(ChessDisplay* display) {
    if (board && display) {
        board->addObserver(display);
    }
}

void Game::removeDisplay(ChessDisplay* display) {
    if (board && display) {
        board->removeObserver(display);
    }
}





