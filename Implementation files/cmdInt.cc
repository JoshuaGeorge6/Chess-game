#include "cmdInt.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include "game.h"
#include "position.h"
#include "textDisplay.h"
#include "graphicalDisplay.h"
#include "board.h"

using namespace std;

// ==========================
// Cleanup Game Memory
// ==========================
void CommandInterpreter::cleanup() {
    // RAII: unique_ptr automatically handles cleanup
    game.reset();
    textDisplay.reset();
    graphicalDisplay.reset();
}

void CommandInterpreter::initializeDisplays() {
    if (!game) return;
    
    // Create displays with board pointer using RAII
    textDisplay = std::make_unique<TextDisplay>(game->getBoard());
    graphicalDisplay = std::make_unique<GraphicalDisplay>(game->getBoard());
    
    // Add displays to game (get raw pointers for observer pattern)
    game->addDisplay(textDisplay.get());
    game->addDisplay(graphicalDisplay.get());
    
    cout << "Displays initialized: Text and Graphical" << endl;
    
    // Trigger initial display of the board
    if (game->getBoard()) {
        game->getBoard()->notifyObservers();
    }
}

Position CommandInterpreter::converter(const string& str) {
    if (str.length() != 2 || str[0] < 'a' || str[0] > 'h' || str[1] < '1' || str[1] > '8') {   // checks for any edgecase when coordinate is entered ex. "e5"
        throw invalid_argument("Invalid position format.");
    }
    int col = str[0] - 'a' + 1;          // function converts a position like "e5" to "5,5"
    int row = str[1] - '0';              // where e5 has row as 5 -> and column as e -> 5
    return Position(row, col);           // Assumes Position(row, col) constructor
}


// Run Loop ===================================================================

void CommandInterpreter::run() {
    string command;
    cout << "Welcome to CS246 Chess. Type your command:\n";

    while (getline(cin, command)) {
        try {
            handleCommand(command);
        } catch (const exception& e) {
            cerr << "Error: " << e.what() << endl;          // catches error depending on which command throws
        }
    }

    if (game) {
        game->displayScore();
        
    }

    cout << "Exiting. Goodbye!\n";
}

// 
// Handle Individual Commands ===================================================================
// 
void CommandInterpreter::handleCommand(const string& cmd) {
    istringstream iss(cmd);       // takes in the whole command in a line, ex. game white-player black-player
    string keyword;               // keyword stores the first primary command          
    iss >> keyword;

    if (keyword == "game") {    
        string white_p1;    // the format for the commands is  human, computer1, computer2 ... computer4
        string black_p2;
        iss >> white_p1 >> black_p2;

        // Preserve scores from previous games
        int prevWhiteScore = 0, prevBlackScore = 0;
        if (game) {
            // Get current scores before cleanup
            prevWhiteScore = game->getWhiteScore();
            prevBlackScore = game->getBlackScore();
        }

        // If we have an existing game with a board from setup mode, preserve it
        // But if it's a finished regular game, reset it to starting position
        bool hasCustomSetup = (game && game->getBoard() && game->isFromSetup());
        
        if (!hasCustomSetup) {
            cleanup();
            game = std::make_unique<Game>();
        }
        
        // Restore previous scores
        game->setScores(prevWhiteScore, prevBlackScore);
        
        game->startGame(white_p1, black_p2);
        
        // Only initialize displays if we created a new game (not preserving setup)
        if (!hasCustomSetup) {
            initializeDisplays();
        } else {
            // For custom setup, just display the board (displays already exist)
            if (game->getBoard()) {
                game->getBoard()->notifyObservers();
            }
        }   

    } else if (keyword == "move") {    // move e5 e6 - player syntax ,   // move - computer syntax
        if (!game) throw runtime_error("No game in progress.");

        vector<string> grid_pos_vec;   
        string grid_pos;
        while (iss >> grid_pos) {
            grid_pos_vec.emplace_back(grid_pos);   // pushes the string into the vector
            
        }

        if (grid_pos_vec.empty()) {
            game->makeComputerMove();
        } else if (grid_pos_vec.size() == 2 || grid_pos_vec.size() == 3) {
            Position from = converter(grid_pos_vec[0]);
            Position to = converter(grid_pos_vec[1]);
            // Added [0] (for now)
            char promotion = (grid_pos_vec.size() == 3) ? grid_pos_vec[2][0] : '\0';
            game->makePlayerMove(from, to, promotion);
        } else {
            throw runtime_error("Invalid move syntax.");
        }

    } else if (keyword == "resign") {
        if (!game) throw runtime_error("No game in progress.");
        game->resign();


    } else if (keyword == "setup") {
        if (game) throw runtime_error("Cannot enter setup during a game.");
        
        game = std::make_unique<Game>(); 
        game->enterSetupMode();
        initializeDisplays();
        //setupMode = true;
        
        cout << "Entered setup mode. Type '+', '-', '=', or 'done'.\n";

        string subcmd;
        while (getline(cin, subcmd)) {
            try {
                istringstream sub(subcmd);
                string token;
                sub >> token;

                if (token == "+") {
                    char piece;
                    string posStr;
                    sub >> piece >> posStr;
                    Position pos = converter(posStr);
                    game->setupAddPiece(piece, pos);
                } else if (token == "-") {
                    string posStr;
                    sub >> posStr;
                    Position pos = converter(posStr);
                    game->setupRemovePiece(pos);
                } else if (token == "=") {
                                    string colour;
                sub >> colour;
                // Convert to lowercase for case-insensitive comparison
                for (char& c : colour) {
                    c = tolower(c);
                }
                Colour turnColour = (colour == "white") ? Colour::WHITE : Colour::BLACK;
                    game->setupSetTurn(turnColour);
                } else if (token == "done") {
                    if (!game->isValidSetup()) {
                        cout << "Setup invalid. Must have exactly one white and one black king, no pawns on first or last row, and no check.\n";
                        continue;
                    }
                    //setupMode = false;
                    break;
                } else {
                    cout << "Invalid setup command.\n";
                }
            } catch (const exception& e) {
                cerr << "Error: " << e.what() << endl;
                // Continue in setup mode despite the error
            }
        }

    } else {
        cout << "Unknown command: " << keyword << endl;
    }
}



