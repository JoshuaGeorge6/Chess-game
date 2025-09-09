#include <string>
#include <memory>
#include "player.h"

class PlayerFactory {
public:
    static std::unique_ptr<Player> createPlayer(const std::string& playerType, Colour colour);
    
private:
    static int extractComputerLevel(const std::string& playerType);
};


