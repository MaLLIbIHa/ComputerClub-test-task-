#pragma once
#include <fstream>
#include <regex>
#include <stdexcept>
#include "time.hpp"
#include "event.hpp"

class Parser {
public:

  class ParserError final : public std::logic_error {
    unsigned lineNumber_;
  public:
    explicit ParserError(const std::string &msg, unsigned lineNumber) 
                        : std::logic_error(msg), lineNumber_(lineNumber) {}
    
    unsigned getLineNumber() const { return lineNumber_; }
  };

private:
  // Parser utility functions

  bool isCorrectTime(std::string timeString) {
    static const std::regex reg(R"(^([01]?[0-9]|2[0-3]):([0-5]\d)$)");
    return std::regex_match(timeString, reg);
  }

  bool isCorrectName(std::string name) {
    static const std::regex reg(R"(^([a-z]|[0-9]|_|-)*$)");
    return std::regex_match(name, reg);
  }

  Time stringToTime(std::string time) {
    std::stringstream timeStr(time);
    unsigned hr, min;
    char delim;
    timeStr >> hr >> delim >> min;
    return Time(hr, min);
  }

public:
  //Main parser methods

  unsigned parsePositiveNumber(std::string line) {
    std::stringstream lineStream(line);
    int number = 0;
    lineStream >> number;
    if (number < 1 || !lineStream || lineStream.peek() != -1) {
      throw ParserError("Invalid format of the specified line", currentLineNumber_);
    }
    currentLineNumber_++;
    return number;
  }

  WorkingTime parseWorkingTime(std::string line) {
    std::stringstream lineStream(line);
    std::string beginTime, endTime;
    lineStream >> beginTime >> endTime;
    if (!lineStream ||
        !isCorrectTime(beginTime) ||
        !isCorrectTime(endTime) ||
        lineStream.peek() != -1) {
      throw ParserError("Invalid format of the specified line", currentLineNumber_);
    }
    currentLineNumber_++;
    return {stringToTime(beginTime), stringToTime(endTime)};
  }

  Event parseEvent(std::string line) {
    std::stringstream lineStream(line);
    std::string time;
    std::string name;
    int id;
    int tableId = 0;
    lineStream >> time >> id >> name;
    if (!lineStream ||
        !isCorrectName(name) ||
        !isCorrectTime(time) ||
        (id > 4 && id < 11) ||
        (id < 1 || id > 13) ||
        stringToTime(time) < lastTime) {
      throw ParserError("Invalid format of the specified line", currentLineNumber_);
    }
    lastTime = stringToTime(time);

    if (id == 2 || id == 12) {
      lineStream >> tableId;
    } else if (lineStream.peek() == -1) {
      currentLineNumber_++;
      return Event{name, stringToTime(time), Events(id)};
    } else {
      throw ParserError("Invalid format of the specified line", currentLineNumber_);
    }

    if (!lineStream || 
        tableId < 1 || 
        tableId > tablesCount_ || 
        lineStream.peek() != -1) {
      throw ParserError("Invalid format of the specified line", currentLineNumber_);
    }
    currentLineNumber_++;
    return Event{name, stringToTime(time), Events(id), static_cast<unsigned>(tableId)};
  }

  void setTablesCount(unsigned tablesCount) { tablesCount_ = tablesCount; }

private:
  Time lastTime;
  unsigned tablesCount_ = 0;
  unsigned currentLineNumber_ = 1;
};