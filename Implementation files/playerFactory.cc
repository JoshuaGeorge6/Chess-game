#include "playerFactory.h"
#include <stdexcept>

int PlayerFactory::extractComputerLevel(const std::string& playerType) {
  // Check for "computer1", "computer2", etc. patterns manually
  // Also check case variations like "Computer1", "COMPUTER1"
  
  // Check length first - should be exactly 9 characters for "computer1"
  if (playerType.length() != 9) {
      return -1;
  }
  
  // Check if it starts with "computer" (case insensitive)
  std::string prefix = playerType.substr(0, 8);
  if (prefix != "Computer" && prefix != "computer" && prefix != "COMPUTER") {
      return -1;
  }
  
  // Extract the last character (the level)
  char levelChar = playerType[8];
  if (levelChar >= '1' && levelChar <= '4') {
      return levelChar - '0'; // Convert char to int
  }
  
  return -1; // Invalid format
} 

std::unique_ptr<Player> PlayerFactory::createPlayer(const std::string& playerType, Colour colour) {
    // Manual case-insensitive comparison for "human"
    if (playerType == "human" || playerType == "Human" || playerType == "HUMAN") {
        return std::make_unique<HumanPlayer>(colour);
    }
    
    // Check for computer player patterns
    int level = extractComputerLevel(playerType);
    if (level != -1) {
        switch (level) {
            case 1:
                return std::make_unique<ComputerPlayer1>(colour);
            case 2:
                return std::make_unique<ComputerPlayer2>(colour);
            case 3:
                return std::make_unique<ComputerPlayer3>(colour);
            case 4:
                return std::make_unique<ComputerPlayer4>(colour);
            default:
                throw std::invalid_argument("Invalid computer level");
        }
    }
    
    throw std::invalid_argument("Invalid player type: " + playerType);
}

