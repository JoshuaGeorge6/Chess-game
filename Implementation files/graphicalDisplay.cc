#include "graphicalDisplay.h"
#include "board.h"
#include "piece.h"
#include <iostream>

#if defined(__linux__) || defined(__APPLE__)
#include <X11/Xlib.h>
#include <cstring>
// Helper macros to avoid repetitive casting
#define DISPLAY static_cast<::Display*>(xDisplay)
#define GC_CONTEXT static_cast<::GC>(gc)
#endif

GraphicalDisplay::GraphicalDisplay(const Board* board, int width, int height) 
    : ChessDisplay(board), windowWidth(width), windowHeight(height),
      squareSize(width / 8), initialized(false) {
    
#if defined(__linux__) || defined(__APPLE__)
    xDisplay = nullptr;
    window = 0;
    gc = 0;
    
    // Initialize piece image arrays
    for (int i = 0; i < 12; i++) {
        piecePixmaps[i] = 0;
        pieceImagesLoaded[i] = false;
        pieceWidths[i] = 0;
        pieceHeights[i] = 0;
    }
    
    if (initializeX11()) {
        initialized = true;
        // Load piece images
        if (loadPieceImages()) {
            std::cout << "Graphical display initialized with piece images." << std::endl;
        } else {
            std::cout << "Warning: Some piece images failed to load." << std::endl;
        }
    } else {
        std::cout << "Failed to initialize graphical display." << std::endl;
    }
#else
    std::cout << "Graphical display not available on this platform. Using text display only." << std::endl;
#endif
}

GraphicalDisplay::~GraphicalDisplay() {
    cleanup();
}

#if defined(__linux__) || defined(__APPLE__)
bool GraphicalDisplay::initializeX11() {
    // Open connection to X server
    ::Display* display = XOpenDisplay(nullptr);
    xDisplay = display;
    if (!xDisplay) {
        std::cerr << "Cannot open X display" << std::endl;
        return false;
    }
    
    int screen = DefaultScreen(display);
    
    // Create window with some margin for coordinates
    window = XCreateSimpleWindow(display, RootWindow(display, screen),
                                100, 100, windowWidth, windowHeight, 2,
                                BlackPixel(display, screen),
                                lightSquareColor);
    
    // Set window properties
    XStoreName(display, window, "CS246 Chess - Beautiful 2D Board");
    
    // Create graphics context
    ::GC graphicsContext = XCreateGC(display, window, 0, nullptr);
    gc = graphicsContext;
    
    // Set up colors - Classic black and white theme like the image
    whiteColor = getColor("#FFFFFF");        // Pure white for light pieces
    blackColor = getColor("#000000");        // Pure black for dark pieces
    lightSquareColor = getColor("#FFFFFF");  // Pure white squares
    darkSquareColor = getColor("#000000");   // Pure black squares
    
    // Select input events
    XSelectInput(display, window, ExposureMask | KeyPressMask);
    
    // Map window
    XMapWindow(display, window);
    XFlush(display);
    
    return true;
}

bool GraphicalDisplay::loadPieceImages() {
#if defined(__linux__) || defined(__APPLE__)
    if (!initialized || !DISPLAY) return false;
    
    // Piece file names and corresponding indices
    const char* pieceFiles[] = {
        "pieces/w_king.xbm",   // 0
        "pieces/w_queen.xbm",  // 1
        "pieces/w_rook.xbm",   // 2
        "pieces/w_bishop.xbm", // 3
        "pieces/w_knight.xbm", // 4
        "pieces/w_pawn.xbm",   // 5
        "pieces/b_king.xbm",   // 6
        "pieces/b_queen.xbm",  // 7
        "pieces/b_rook.xbm",   // 8
        "pieces/b_bishop.xbm", // 9
        "pieces/b_knight.xbm", // 10
        "pieces/b_pawn.xbm"    // 11
    };
    
    int loadedCount = 0;
    for (int i = 0; i < 12; i++) {
        if (loadPieceImage(pieceFiles[i], i)) {
            loadedCount++;
        }
    }
    
    return (loadedCount >= 6); // At least half the pieces loaded successfully
#else
    return false;
#endif
}

bool GraphicalDisplay::loadPieceImage(const std::string& filename, int pieceIndex) {
#if defined(__linux__) || defined(__APPLE__)
    if (!initialized || !DISPLAY || pieceIndex < 0 || pieceIndex >= 12) return false;
    
    unsigned int width, height;
    int x_hot, y_hot;
    
    // Try to load XBM file
    int result = XReadBitmapFile(DISPLAY, window, filename.c_str(),
                                &width, &height, &piecePixmaps[pieceIndex], 
                                &x_hot, &y_hot);
    
    if (result == 0) { // BitmapSuccess = 0
        pieceImagesLoaded[pieceIndex] = true;
        pieceWidths[pieceIndex] = width;
        pieceHeights[pieceIndex] = height;
        std::cout << "Loaded " << filename << " (" << width << "x" << height << ")" << std::endl;
        return true;
    }
    
    std::cout << "Failed to load " << filename << " (error code: " << result << ")" << std::endl;
    pieceImagesLoaded[pieceIndex] = false;
    return false;
#else
    (void)filename; (void)pieceIndex;
    return false;
#endif
}

int GraphicalDisplay::getPieceIndex(char piece) {
    switch (piece) {
        case 'K': return 0;  // White King
        case 'Q': return 1;  // White Queen
        case 'R': return 2;  // White Rook
        case 'B': return 3;  // White Bishop
        case 'N': return 4;  // White Knight
        case 'P': return 5;  // White Pawn
        case 'k': return 6;  // Black King
        case 'q': return 7;  // Black Queen
        case 'r': return 8;  // Black Rook
        case 'b': return 9;  // Black Bishop
        case 'n': return 10; // Black Knight
        case 'p': return 11; // Black Pawn
        default: return -1;  // Invalid piece
    }
}

void GraphicalDisplay::cleanup() {
    if (xDisplay) {
        // Free loaded piece pixmaps
        for (int i = 0; i < 12; i++) {
            if (pieceImagesLoaded[i] && piecePixmaps[i] != 0) {
                XFreePixmap(DISPLAY, piecePixmaps[i]);
            }
        }
        
        if (gc) XFreeGC(DISPLAY, GC_CONTEXT);
        if (window) XDestroyWindow(DISPLAY, window);
        XCloseDisplay(DISPLAY);
        xDisplay = nullptr;
    }
}

void GraphicalDisplay::notify() {
#if defined(__linux__) || defined(__APPLE__)
    if (initialized) {
        displayBoard();
    }
#endif
}

void GraphicalDisplay::displayBoard() {
    if (!initialized || !board) return;
    
    // Clear window
    XClearWindow(DISPLAY, window);
    
    // Draw chess board squares and pieces
    for (int row = 1; row <= 8; row++) {
        for (int col = 1; col <= 8; col++) {
            // Determine square color (light/dark)
            bool isLightSquare = ((row + col) % 2 == 0);
            
            // Draw square
            drawSquare(row, col, isLightSquare);
            
            // Draw piece if present
            Position pos(row, col);
            Piece* piece = board->getPiece(pos);
            if (piece) {
                drawPiece(row, col, piece->getSymbol());
            }
        }
    }
    
    // Draw board border
    int boardOffsetX = 20;
    int boardOffsetY = 20;
    XSetForeground(DISPLAY, GC_CONTEXT, blackColor);
    XDrawRectangle(DISPLAY, window, GC_CONTEXT, boardOffsetX - 2, boardOffsetY - 2, 8 * squareSize + 4, 8 * squareSize + 4);
    
    // Draw coordinate labels
    drawCoordinateLabels();
    
    XFlush(DISPLAY);
}

void GraphicalDisplay::drawSquare(int row, int col, bool isLight) {
    // Convert chess coordinates to screen coordinates with board offset
    // Row 8 is at top (y=0), Row 1 is at bottom
    int boardOffsetX = 20;
    int boardOffsetY = 20;
    int x = (col - 1) * squareSize + boardOffsetX;
    int y = (8 - row) * squareSize + boardOffsetY;
    
    // Set color
    XSetForeground(DISPLAY, GC_CONTEXT, isLight ? lightSquareColor : darkSquareColor);
    
    // Fill square
    XFillRectangle(DISPLAY, window, GC_CONTEXT, x, y, squareSize, squareSize);
}

void GraphicalDisplay::drawPiece(int row, int col, char piece) {
    // Convert chess coordinates to screen coordinates with board offset
    // This must match the drawSquare coordinate system
    int boardOffsetX = 20;
    int boardOffsetY = 20;
    int x = (col - 1) * squareSize + boardOffsetX;
    int y = (8 - row) * squareSize + boardOffsetY;
    
    // Draw using loaded XBM piece images
    int pieceIndex = getPieceIndex(piece);
    if (pieceIndex >= 0 && pieceIndex < 12 && pieceImagesLoaded[pieceIndex]) {
        // Draw using loaded pixmap with actual dimensions
        unsigned int imgWidth = pieceWidths[pieceIndex];
        unsigned int imgHeight = pieceHeights[pieceIndex];
        
        // Scale down if the image is larger than the square
        int maxSize = squareSize - 10;
        int drawWidth = (imgWidth > maxSize) ? maxSize : imgWidth;
        int drawHeight = (imgHeight > maxSize) ? maxSize : imgHeight;
        
        // Center the piece in the square
        int drawX = x + (squareSize - drawWidth) / 2;
        int drawY = y + (squareSize - drawHeight) / 2;
        
        // Set colors for bitmap rendering
        // White pieces: black outline on white background
        // Black pieces: black fill on white background
        bool isWhitePiece = (piece >= 'A' && piece <= 'Z');
        if (isWhitePiece) {
            XSetForeground(DISPLAY, GC_CONTEXT, blackColor);    // Black outline
            XSetBackground(DISPLAY, GC_CONTEXT, whiteColor);    // White fill
        } else {
            XSetForeground(DISPLAY, GC_CONTEXT, blackColor);    // Black fill
            XSetBackground(DISPLAY, GC_CONTEXT, whiteColor);    // White background
        }
        
        XCopyPlane(DISPLAY, piecePixmaps[pieceIndex], window, GC_CONTEXT,
                  0, 0, drawWidth, drawHeight,  // Source area
                  drawX, drawY, 1); // Destination + plane (1 for bitmap)
    }
    // Note: If piece image fails to load, piece simply won't be drawn (empty square)
}

void GraphicalDisplay::drawCoordinateLabels() {
    XSetForeground(DISPLAY, GC_CONTEXT, blackColor);
    
    int boardOffsetX = 20;
    int boardOffsetY = 20;
    
    // Draw file labels (a-h) at the bottom
    for (int col = 0; col < 8; col++) {
        char label = 'a' + col;
        std::string labelStr(1, label);
        int x = col * squareSize + squareSize / 2 + boardOffsetX - 3; // Center in square
        int y = 8 * squareSize + boardOffsetY + 15; // Below the board
        drawText(labelStr, x, y);
    }
    
    // Draw rank labels (1-8) on the left
    for (int row = 0; row < 8; row++) {
        char label = '8' - row; // Rank 8 at top, rank 1 at bottom
        std::string labelStr(1, label);
        int x = boardOffsetX - 15; // Left of the board
        int y = row * squareSize + squareSize / 2 + boardOffsetY + 5; // Center in square
        drawText(labelStr, x, y);
    }
}

void GraphicalDisplay::drawText(const std::string& text, int x, int y) {
    XSetForeground(DISPLAY, GC_CONTEXT, blackColor);
    XDrawString(DISPLAY, window, GC_CONTEXT, x, y, text.c_str(), text.length());
}

unsigned long GraphicalDisplay::getColor(const char* colorName) {
    Colormap colormap = DefaultColormap(DISPLAY, DefaultScreen(DISPLAY));
    XColor color;
    
    if (XParseColor(DISPLAY, colormap, colorName, &color) &&
        XAllocColor(DISPLAY, colormap, &color)) {
        return color.pixel;
    }
    
    // Fallback to white if color parsing fails
    return WhitePixel(DISPLAY, DefaultScreen(DISPLAY));
}


#else


// Stub implementations for platforms without X11
bool GraphicalDisplay::initializeX11() { return false; }
void GraphicalDisplay::cleanup() {}
void GraphicalDisplay::notify() {}
void GraphicalDisplay::displayBoard() {}
void GraphicalDisplay::drawSquare(int row, int col, bool isLight) { (void)row; (void)col; (void)isLight; }
void GraphicalDisplay::drawPiece(int row, int col, char piece) { (void)row; (void)col; (void)piece; }
void GraphicalDisplay::drawText(const std::string& text, int x, int y) { (void)text; (void)x; (void)y; }
unsigned long GraphicalDisplay::getColor(const char* colorName) { (void)colorName; return 0; }
#endif



 

