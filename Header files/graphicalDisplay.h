#ifndef GRAPHICALDISPLAY_H
#define GRAPHICALDISPLAY_H

#include "display.h"
#include <string>

#if defined(__linux__) || defined(__APPLE__)
#include <X11/Xlib.h> // Included here
#endif

class GraphicalDisplay : public ChessDisplay { // Inherits from ChessDisplay
private:
#if defined(__linux__) || defined(__APPLE__)
    void* xDisplay; // X11 Display pointer
    unsigned long window; // X11 Window ID
    void* gc; // X11 Graphics Context
    unsigned long whiteColor;
    unsigned long blackColor;
    unsigned long lightSquareColor;
    unsigned long darkSquareColor;
    
    // Piece pixmaps for image-based rendering
    unsigned long piecePixmaps[12]; // 6 pieces * 2 colors
    bool pieceImagesLoaded[12];
    unsigned int pieceWidths[12];   // Store actual image dimensions
    unsigned int pieceHeights[12];
#endif
    int windowWidth;
    int windowHeight;
    int squareSize;
    bool initialized;

public:
    GraphicalDisplay(const Board* board, int width = 680, int height = 680); // Increased size
    ~GraphicalDisplay();
    void notify() override;

private:
    bool initializeX11();
    void cleanup();
    void displayBoard();
    void drawSquare(int row, int col, bool isLight);
    void drawPiece(int row, int col, char piece);
    
    // Image-based piece rendering
    bool loadPieceImages();
    bool loadPieceImage(const std::string& filename, int pieceIndex);
    int getPieceIndex(char piece);
    void drawCoordinateLabels();
    void drawText(const std::string& text, int x, int y);
    unsigned long getColor(const char* colorName);
};

#endif // GRAPHICALDISPLAY_H 




