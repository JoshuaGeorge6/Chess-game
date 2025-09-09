#ifndef CMDINT_H
#define CMDINT_H

#include <string>
#include <memory>
#include "game.h"
#include "position.h"
#include "textDisplay.h"
#include "graphicalDisplay.h"



class CommandInterpreter {
    std::unique_ptr<Game> game;
    std::unique_ptr<TextDisplay> textDisplay;
    std::unique_ptr<GraphicalDisplay> graphicalDisplay;
    // bool setupMode = false;

public:
    void run();

private:
    void handleCommand(const std::string& command);
    Position converter(const std::string& str);
    void cleanup();
    void initializeDisplays();
};

#endif // CMDINT_H




