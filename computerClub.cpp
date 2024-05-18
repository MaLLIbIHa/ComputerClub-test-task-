#include <iostream>
#include "eventHandler.hpp"
#include "parser.hpp"

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

  std::cout << workingTime.begin.to_str() << '\n';

  parser.setTablesCount(tablesCount);
  EventHandler eventHandler(workingTime, costPerHour, tablesCount);
  bool createdLeave = false;
  std::string line;
  while (std::getline(input, line)) {
    Event event;
    try {
      event = parser.parseEvent(line);
    } catch (Parser::ParserError &err) {
      std::cout << err.getLineNumber() << std::endl;
      return 1;
    }
    if (!createdLeave && event.time > workingTime.end) {
      eventHandler.createLeaveEventForAllClients();
      createdLeave = true;
    }
    eventHandler.handleEvent(event);
  }

  if (!createdLeave) {
    eventHandler.createLeaveEventForAllClients();
  }
  std::cout << workingTime.end.to_str() << '\n';
  eventHandler.printTablesInfo();

  return 0;
}