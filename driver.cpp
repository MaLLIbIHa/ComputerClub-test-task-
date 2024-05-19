#include <iostream>
#include "parser.hpp"
#include "computerClub.hpp"

int main(int argc, char** argv) {
  if (argc < 2) {
    return 1;
  }
  std::ifstream input(argv[1]);
  std::vector<std::string> configLines(3);
  for (int i = 0; i < 3; i++) {
    std::getline(input, configLines[i]);
  }
  
  Parser parser;
  unsigned tablesCount;
  WorkingTime workingTime;
  unsigned costPerHour;
  try {
    tablesCount = parser.parsePositiveNumber(configLines[0]);
    workingTime = parser.parseWorkingTime(configLines[1]);
    costPerHour = parser.parsePositiveNumber(configLines[2]);
  } catch (Parser::ParserError &err) {
    std::cout << err.getLineNumber() << std::endl;
    return 1;
  }
  
  parser.setTablesCount(tablesCount);

  std::vector<Event> events;
  std::string line;
  while(std::getline(input, line)) {
    Event event;
    try {
      event = parser.parseEvent(line);
    } catch (Parser::ParserError &err) {
      std::cout << err.getLineNumber() << std::endl;
      return 1;
    }
    events.push_back(event);
  }

  std::cout << workingTime.begin.toStr() << '\n';

  ComputerClub computerClub(workingTime, tablesCount, costPerHour);
  bool createdLeave = false;
  for (auto&& event : events) {
    computerClub.handleEvent(event);
  }
  computerClub.handleEvent({"", Time(), Events::EndOfEvents, 0});

  std::cout << workingTime.end.toStr() << '\n';
  computerClub.printTablesInfo();

  return 0;
}