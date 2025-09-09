// board.h – Basic Board interface for CS246 Chess

#include "position.h"
#include "colour.h"
#include "piece.h"
#include <vector>
#include <memory>

// Forward declarations
class ChessDisplay;
class Player;

class Board {
    friend class Player;  // Allow Player to access private members for AI logic
public:
    Board();
    ~Board();

    void setupStartingPosition();   //sets up boards starting chess position
    void clearGameHistory();        // resets game back to its normal state
    void resetSpecialRules();       // resets all flags for enpassant castling etc. back to false when starting a new game, takes care of any other special rules too

    bool isInCheck(Colour colour) const;  // checks if the current player is facing a check.
    bool canCastleKingSide(Colour colour) const;
    bool canCastleQueenSide(Colour colour) const;
    bool isEnPassant(const Position& from, const Position& to, Colour turn) const;

    bool isValidMove(const Position& from, const Position& to, Colour turn) const;    // make sure its the current players turn and they have a piece at curr
                                                                                      // make sure dest is not occupied by current player's own piece
                                                                                      // makes sure user if hes not making the pieces move agaisnt their behaviour ex. making queen move like knig
    bool wouldBeInCheck(const Position& from, const Position& to, Colour turn) const; 
    void makeMove(const Position& from, const Position& to, char promotion);    // make move will first check if the user is not retarded (hes not trying to promote the wrong thing)
                                                                               // executes the move and makes the promotion if possible 
                                                                               // make move will also use removePiece from the board function to handle any removal of pieces.

    bool isInCheckmate(Colour colour) const;   // checks whether the current colour has been checkmated
    bool isInStalemate(Colour colour) const;   // checks whether the current colour is in stalemate or not

    void addPiece(char pieceChar, const Position& pos);   // Place a piece on pos in setup mode (replace any piece currently on pos)
    void removePiece(const Position& pos);                // Remove a piece from pos in setup mode ()
    int countPieces(char piece) const; // runs a loop over the entire board to count the number of piece pieces.
    bool hasPawnsOnEndRanks();      //  checks for no pawns on first or last row 
    bool isValidSetup() const;                            // Ensure setup meets rules
    void clear();  // Clear the board (used in setup) 

    void init();   // (didnt see)                                           // Set up initial chessboard
                             

    

    Piece* getPiece(const Position& pos) const;    //  (didnt see)     // Get piece at position

    // Observer pattern methods
    void addObserver(ChessDisplay* observer);
    void removeObserver(ChessDisplay* observer);
    void notifyObservers() const;

private:
    std::vector<std::vector<std::unique_ptr<Piece>>> grid;  // 8x8 board, nullptr for empty squares
    Position lastMoveFrom;  // Tracks the source from the last move
    Position lastMoveTo; // Tracks where the previous piece just moved on the board
    bool whiteKingMoved;
    bool blackKingMoved;
    bool whiteRookKingMoved;   // King-side rook
    bool whiteRookQueenMoved;  // Queen-side rook
    bool blackRookKingMoved;
    bool blackRookQueenMoved;
    
    // Observer pattern
    std::vector<ChessDisplay*> observers;
};

