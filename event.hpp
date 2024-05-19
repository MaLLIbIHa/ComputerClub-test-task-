#pragma once
#include "time.hpp"

enum class Events {
  //incoming events
  InClientArrived = 1,
  InClientSatDown,
  InClientIsWaiting,
  InClientLeft,
  //outcoming events
  OutClientLeft = 11,
  OutClientSatDown,
  OutError,
  //final event
  EndOfEvents,
};

struct Event final {
  std::string val;
  Time time;
  Events eventId;
  unsigned tableNum = 0; // 0 if event don't have body
};