#include "player.h"
#include "board.h"
#include "piece.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <stdexcept>

// Forward declaration - will be implemented later
class Board;

// Base Player class implementation
Player::Player(Colour colour) : colour(colour) {
    // Initialize random seed for AI players
    static bool seedInitialized = false;
    if (!seedInitialized) {
        srand(time(nullptr)); // Uses time to generate a random sequence of numbers which lets you pick a set random set of moves.
        seedInitialized = true;
    }
}

std::string Player::positionToString(const Position& pos) const {
    char col = 'a' + (pos.getCol() - 1); // Makem it index form, then convert to string
    char row = '0' + pos.getRow(); 
    return std::string(1, col) + std::string(1, row); // Makes the char's become strings, by constructing one char each time then adding them 
}

// Helper method to get piece values for AI decision making
int Player::getPieceValue(const std::string& pieceType) const {
    if (pieceType == "Queen") return 9;
    if (pieceType == "Rook") return 5;
    if (pieceType == "Bishop") return 3;
    if (pieceType == "Knight") return 2;  // Some consider Knight = 3, others = 2.5
    if (pieceType == "Pawn") return 1;
    if (pieceType == "King") return 0;   // King is invaluable, not captured in normal play
    return 0;  // Unknown piece type
}

std::vector<std::string> Player::getAllLegalMoves(const Board& board) const {
    std::vector<std::string> legalMoves;
    
    // Iterate through all squares to find pieces of our color
    for (int row = 1; row <= 8; row++) {
        for (int col = 1; col <= 8; col++) {
            Position from(row, col);
            Piece* piece = board.getPiece(from);
            
            if (piece && piece->getColour() == colour) {
                // Get all possible moves for the current colour
                std::vector<Position> possibleMoves = piece->getPossibleMoves(from, board); 
                // contains all the moves we can play for the current piece
                
                for (const Position& to : possibleMoves) {
                    // Check if this move is legal (doesn't put own king in check)
                    if (board.isValidMove(from, to, colour) && !board.wouldBeInCheck(from, to, colour)) {
                        // Check if this is a pawn promotion move
                        if (piece->getType() == "Pawn") {
                            // Cast to Pawn to access isPromotionRank method
                            Pawn* pawn = static_cast<Pawn*>(piece);
                            if (pawn->isPromotionRank(to)) {
                                // Generate all four promotion options
                                std::string baseMove = positionToString(from) + positionToString(to);
                                legalMoves.push_back(baseMove + "Q"); // Promote to Queen
                                legalMoves.push_back(baseMove + "R"); // Promote to Rook
                                legalMoves.push_back(baseMove + "B"); // Promote to Bishop
                                legalMoves.push_back(baseMove + "N"); // Promote to Knight
                            } else {
                                // Regular pawn move (not promotion)
                                std::string moveStr = positionToString(from) + positionToString(to);
                                legalMoves.push_back(moveStr);
                            }
                        } else {
                            // Non-pawn move
                            std::string moveStr = positionToString(from) + positionToString(to);
                            legalMoves.push_back(moveStr);
                        }
                    }
                }
            }
        }
    }
    
    return legalMoves;
}



// Helper method to check if a move puts enemy king in check
bool Player::putsEnemyInCheck(const std::string& move, const Board& board) const {
    Position from(move[1] - '0', move[0] - 'a' + 1);
    Position to(move[3] - '0', move[2] - 'a' + 1);
    
    // Get the enemy color
    Colour enemyColour = (colour == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;
    
    // Use the board's wouldBeInCheck logic but check enemy king instead
    // Since we're a friend class, we can access the private implementation
    Piece* movingPiece = board.getPiece(from);
    if (!movingPiece) return false;
    
    // Clone the captured piece BEFORE making the move
    std::unique_ptr<Piece> capturedPieceClone = nullptr;
    Piece* capturedPiece = board.getPiece(to);
    if (capturedPiece) {
        capturedPieceClone = capturedPiece->clone();
    }
    
    // Temporarily make the move using friend access
    Board& nonConstBoard = const_cast<Board&>(board);
    nonConstBoard.grid[to.getRow() - 1][to.getCol() - 1] = std::move(nonConstBoard.grid[from.getRow() - 1][from.getCol() - 1]);
    nonConstBoard.grid[from.getRow() - 1][from.getCol() - 1] = nullptr;
    
    // Check if enemy king is in check (this is what we want to know)
    bool enemyInCheck = board.isInCheck(enemyColour);
    
    // Restore the board
    nonConstBoard.grid[from.getRow() - 1][from.getCol() - 1] = std::move(nonConstBoard.grid[to.getRow() - 1][to.getCol() - 1]);
    if (capturedPieceClone) {
        nonConstBoard.grid[to.getRow() - 1][to.getCol() - 1] = std::move(capturedPieceClone);
    } else {
        nonConstBoard.grid[to.getRow() - 1][to.getCol() - 1] = nullptr;
    }
    
    return enemyInCheck;
}

// Helper method to check if a move avoids being captured
bool Player::avoidsCapture(const std::string& move, const Board& board) const {
    Position from(move[1] - '0', move[0] - 'a' + 1);
    Position to(move[3] - '0', move[2] - 'a' + 1);
    
    // Check if the piece at 'from' is currently under attack
    Piece* ourPiece = board.getPiece(from);
    if (!ourPiece) return false;
    
    // Check if any enemy piece can capture our piece at the 'from' position
    Colour enemyColour = (colour == Colour::WHITE) ? Colour::BLACK : Colour::WHITE;
    bool currentlyUnderAttack = false;
    
    // Check if any enemy piece can attack the 'from' position
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (board.grid[row][col] && board.grid[row][col]->getColour() == enemyColour) {
                // Just found an enemy piece to check if it can attack us
                Position enemyPos(row + 1, col + 1);  // Convert to 1-indexed
                if (board.grid[row][col]->isValidMove(enemyPos, from, board)) { // We are under attack
                    currentlyUnderAttack = true;
                    break;
                }
            }
        }
        if (currentlyUnderAttack) break;
    }
    
    // If not currently under attack, this move doesn't avoid capture
    if (!currentlyUnderAttack) return false;
    // We only do everything beneath if we are currently in danger. 

    // Check if moving to 'to' position puts us out of danger
    bool wouldBeUnderAttack = false;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (board.grid[row][col] && board.grid[row][col]->getColour() == enemyColour) {
                Position enemyPos(row + 1, col + 1);  // Convert to 1-indexed
                if (board.grid[row][col]->isValidMove(enemyPos, to, board)) {
                    wouldBeUnderAttack = true;
                    break;
                }
            }
        }
        if (wouldBeUnderAttack) break;
    }
    
    // Return true if we're currently under attack but wouldn't be after the move
    return currentlyUnderAttack && !wouldBeUnderAttack;
}

// HumanPlayer implementation
HumanPlayer::HumanPlayer(Colour colour) : Player(colour) {}

std::string HumanPlayer::getMove(const Board& board) {
    // Human moves are handled through the command interpreter
    // This function should never be called in normal gameplay
    std::cerr << "Warning: HumanPlayer::getMove called - human input should be handled by command interpreter" << std::endl;
    return "a1a1";  // Dummy return to satisfy linker
}

// ComputerPlayer1 implementation (Level 1 - Basic)
ComputerPlayer1::ComputerPlayer1(Colour colour) : Player(colour) {}

std::string ComputerPlayer1::getMove(const Board& board) {
    // Level 1: Random legal moves
    std::cout << "Computer Level 1 (" << (colour == Colour::WHITE ? "White" : "Black") << ") is thinking..." << std::endl;
    
    std::vector<std::string> legalMoves = getAllLegalMoves(board); // Promotions are in here already 
    // If no legal moves, let the game handle stalemate/checkmate detection
    
    // Return a random legal move
    int randomIndex = rand() % legalMoves.size(); // Will always be less than legalMoves.size() 
    return legalMoves[randomIndex]; // Randomly choose a legal move from there to play. 
}

// ComputerPlayer2 implementation (Level 2 - Intermediate)
ComputerPlayer2::ComputerPlayer2(Colour colour) : Player(colour) {}

std::string ComputerPlayer2::getMove(const Board& board) {
    // Level 2: Prefers moves that capture enemy pieces OR put enemy king in check
    std::cout << "Computer Level 2 (" << (colour == Colour::WHITE ? "White" : "Black") << ") is thinking..." << std::endl;
    
    std::vector<std::string> legalMoves = getAllLegalMoves(board);
    // If no legal moves, let the game handle stalemate/checkmate detection
    
    std::vector<std::string> filteredMoves;
    
    // Find moves that capture enemy pieces OR put enemy king in check
    for (const std::string& move : legalMoves) {
        Position from(move[1] - '0', move[0] - 'a' + 1);
        Position to(move[3] - '0', move[2] - 'a' + 1);
        
        bool isCapture = false;
        bool putsInCheck = false;
        
        // Check if it's a capturing move
        Piece* targetPiece = board.getPiece(to);
        if (targetPiece && targetPiece->getColour() != colour) {
            isCapture = true;
        }
        
        // Check if it puts enemy king in check
        if (putsEnemyInCheck(move, board)) {
            putsInCheck = true;
        }
        
        // Add to filtered moves if it captures OR puts in check
        if (isCapture || putsInCheck) {
            filteredMoves.push_back(move);
        }
    }
    
    // If filtered moves exist, pick randomly from them; otherwise pick from all legal moves
    if (!filteredMoves.empty()) {
        int randomIndex = rand() % filteredMoves.size();
        return filteredMoves[randomIndex];
    } else {
        int randomIndex = rand() % legalMoves.size();
        return legalMoves[randomIndex];
    }
}

// ComputerPlayer3 implementation (Level 3 - Advanced)
ComputerPlayer3::ComputerPlayer3(Colour colour) : Player(colour) {}

std::string ComputerPlayer3::getMove(const Board& board) {
    // Level 3: Prefers moves that capture enemy pieces OR put enemy king in check OR avoid being captured
    std::cout << "Computer Level 3 (" << (colour == Colour::WHITE ? "White" : "Black") << ") is thinking..." << std::endl;
    
    std::vector<std::string> legalMoves = getAllLegalMoves(board);
    // If no legal moves, let the game handle stalemate/checkmate detection
    
    std::vector<std::string> filteredMoves;
    
    // Find moves that capture enemy pieces OR put enemy king in check OR avoid being captured
    for (const std::string& move : legalMoves) {
        Position from(move[1] - '0', move[0] - 'a' + 1);
        Position to(move[3] - '0', move[2] - 'a' + 1);
        
        bool isCapture = false;
        bool putsInCheck = false;
        bool avoidsBeingCaptured = false;
        
        // Check if it's a capturing move
        Piece* targetPiece = board.getPiece(to);
        if (targetPiece && targetPiece->getColour() != colour) {
            isCapture = true;
        }
        
        // Check if it puts enemy king in check
        if (putsEnemyInCheck(move, board)) {
            putsInCheck = true;
        }
        
        // Check if it avoids being captured
        if (avoidsCapture(move, board)) {
            avoidsBeingCaptured = true;
        }
        
        // Add to filtered moves if it captures OR puts in check OR avoids capture
        if (isCapture || putsInCheck || avoidsBeingCaptured) {
            filteredMoves.push_back(move);
        }
    }
    
    // If filtered moves exist, pick randomly from them; otherwise pick from all legal moves
    if (!filteredMoves.empty()) {
        int randomIndex = rand() % filteredMoves.size();
        return filteredMoves[randomIndex];
    } else {
        int randomIndex = rand() % legalMoves.size();
        return legalMoves[randomIndex];
    }
}

// ComputerPlayer4 implementation (Level 4 - Expert)
ComputerPlayer4::ComputerPlayer4(Colour colour) : Player(colour) {}

std::string ComputerPlayer4::getMove(const Board& board) {
    // Level 4: Priority system - Check > Capture > Avoid Capture > Random
    std::cout << "Computer Level 4 (" << (colour == Colour::WHITE ? "White" : "Black") << ") is thinking..." << std::endl;
    
    std::vector<std::string> legalMoves = getAllLegalMoves(board);
    // If no legal moves, let the game handle stalemate/checkmate detection
    
    std::vector<std::string> checkMoves;
    std::vector<std::string> capturingMoves;
    std::vector<std::string> avoidCaptureMoves;
    
    // Tracker variables for highest value pieces
    int highestCaptureValue = 0;
    int highestAvoidValue = 0;
    
    // Categorize all moves and track highest values
    for (const std::string& move : legalMoves) {
        Position from(move[1] - '0', move[0] - 'a' + 1);
        Position to(move[3] - '0', move[2] - 'a' + 1);
        
        // Check for check moves
        if (putsEnemyInCheck(move, board)) {
            checkMoves.push_back(move);
        }
        
        // Check for capturing moves and track highest value (store only first highest)
        Piece* targetPiece = board.getPiece(to);
        if (targetPiece && targetPiece->getColour() != colour) {
            int enemyPieceValue = getPieceValue(targetPiece->getType());
            if (enemyPieceValue > highestCaptureValue) {
                highestCaptureValue = enemyPieceValue;
                capturingMoves.clear();  // Clear previous lower value moves
                capturingMoves.push_back(move);  // Store only the first highest value move
            }
            // Don't store equal value moves - only keep the first one
        }
        
        // Check for avoid capture moves and track highest value (store only first highest)
        if (avoidsCapture(move, board)) {
            Piece* ourPiece = board.getPiece(from);
            int ourPieceValue = ourPiece ? getPieceValue(ourPiece->getType()) : 0;
            if (ourPieceValue > highestAvoidValue) {
                highestAvoidValue = ourPieceValue;
                avoidCaptureMoves.clear();  // Clear previous lower value moves
                avoidCaptureMoves.push_back(move);  // Store only the first highest value move
            }
            // Don't store equal value moves - only keep the first one
        }
    }
    
    // Priority 1: Check moves (randomly pick from check moves)
    if (!checkMoves.empty()) {
        int randomIndex = rand() % checkMoves.size();
        return checkMoves[randomIndex];
    }
    
    // Priority 2: Compare capture vs avoid capture by max values
    if (!capturingMoves.empty() && !avoidCaptureMoves.empty()) {
        // Both options available - choose based on higher value
        if (highestCaptureValue >= highestAvoidValue) {
            return capturingMoves[0];  // Only one move stored
        } else {
            return avoidCaptureMoves[0];  // Only one move stored
        }
    } else if (!capturingMoves.empty()) {
        // Only capturing moves available
        return capturingMoves[0];  // Only one move stored
    } else if (!avoidCaptureMoves.empty()) {
        // Only avoid capture moves available
        return avoidCaptureMoves[0];  // Only one move stored
    }
    
    // Priority 4: Any legal move (randomly pick from all legal moves)
    int randomIndex = rand() % legalMoves.size();
    return legalMoves[randomIndex];
}



// If we wanna strengthen any of these AI's, simply strengthen the filtering criteria of the vectors they randomly chppse from


