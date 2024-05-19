#include <unordered_map>
#include <vector>
#include <list>
#include <memory>
#include <iostream>
#include "time.hpp"
#include "event.hpp"
#include "clubInfo.hpp"

class ComputerClub final {
private:
  struct TableInfo {
    Time beginTime;
    Time allUsageTime;
    unsigned income = 0;
    bool isBusy = false;
  };

  struct ClientInfo {
    std::list<std::string>::iterator queueIt;
    unsigned tableNum;
  };

public:
  ComputerClub(WorkingTime workingTime,
               unsigned tablesCount,
               unsigned costPerHour) 
      : clubInfo_(workingTime, tablesCount, costPerHour) {}

  Time getBeginTime() const {
    return clubInfo_.getBeginTime();
  }

  Time getEndTime() const {
    return clubInfo_.getEndTime();
  }

  unsigned getEmptyTables() const {
    return clubInfo_.getEmptyTables();
  }
  
  unsigned getTableIncome(unsigned tableNum) const {
    return clubInfo_.getTableIncome(tableNum);
  }

  Time getTableUsageTime(unsigned tableNum) const {
    return clubInfo_.getTableUsageTime(tableNum);
  }

  unsigned getClientsTable(std::string clientName) const {
    return clubInfo_.getClientsTable(clientName);
  }

  void printTablesInfo() const {
    clubInfo_.printTablesInfo();
  }

  void handleEvent(Event e) {
    if (!isWorkingDayOver &&
        (e.time > clubInfo_.getEndTime() || e.eventId == Events::EndOfEvents)) {
      createLeaveEventForAllClients();
      isWorkingDayOver = true;
    }

    switch(e.eventId) {
      case Events::InClientArrived:
        incomingClientArrivedHandler(e);
        break;
      case Events::InClientIsWaiting:
        incomingClientWaitingHandler(e);
        break;
      case Events::InClientLeft:
        incomingClientLeftHandler(e);
        break;
      case Events::InClientSatDown:
        incomingClientSatDownHandler(e);
        break;
    }
  }

private:
  void printEventWithoutBody(Event e) {
    std::cout << e.time.toStr() << ' '
              << static_cast<unsigned>(e.eventId) << ' '
              << e.val << '\n';
  }

  void printEventWithBody(Event e) {
    std::cout << e.time.toStr() << ' '
              << static_cast<unsigned>(e.eventId) << ' ' 
              << e.val << ' '
              << e.tableNum << '\n';
  }

  // Main event handlers

  void incomingClientArrivedHandler(Event e) {
    printEventWithoutBody(e);
    if (clubInfo_.getClientsTable(e.val) != -1) {
      outgoingErrorHandler(
          Event{"YouShallNotPass", e.time, Events::OutError, 0});
      return;
    } else if (e.time < clubInfo_.getBeginTime() || e.time > clubInfo_.getEndTime()) {
      outgoingErrorHandler(
          Event{"NotOpenYet", e.time, Events::OutError, 0});
      return;
    }
    clubInfo_.registerNewClient(e.val);
  }

  void incomingClientSatDownHandler(Event e) {
    printEventWithBody(e);
    if (clubInfo_.getClientsTable(e.val) == -1) {
      outgoingErrorHandler(
          Event{"ClientUnknown", e.time, Events::OutError, 0});
      return;
    } else if (clubInfo_.isTableBusy(e.tableNum)) {
      outgoingErrorHandler(
          Event{"PlaceIsBusy", e.time, Events::OutError, 0});
      return;
    }

    clubInfo_.moveClientToTable(e.val, e.time, e.tableNum);
  }

  void incomingClientWaitingHandler(Event e) {
    printEventWithoutBody(e);
    if (clubInfo_.getEmptyTables() > 0) {
      outgoingErrorHandler(
          Event{"ICanWaitNoLonger!", e.time, Events::OutError, 0});
      return;
    }
    if (clubInfo_.getQueueSize() > clubInfo_.getTablesCount()) {
      outgoingClientLeftHandler(
          Event{e.val, e.time, Events::OutClientLeft, 0});
      return;
    }
  }

  void incomingClientLeftHandler(Event e) {
    printEventWithoutBody(e);
    int clientsTable = clubInfo_.getClientsTable(e.val);
    if (clientsTable == -1) {
      outgoingErrorHandler(
          Event{"ClientUnknown", e.time, Events::OutError, 0});
      return;
    }

    if (clientsTable != 0) {
      clubInfo_.clientLeaveTable(e.val, e.time);
    }
    clubInfo_.clientLeaveClub(e.val);

    if (!clubInfo_.isQueueEmpty()) {
      std::string firstInQueue = clubInfo_.getFirstInQueue();
      outgoingClientSatDownHandler(
          Event{firstInQueue, 
                e.time,
                Events::OutClientSatDown,
                static_cast<unsigned>(clientsTable)});
    }
  }

  void outgoingClientLeftHandler(Event e) {
    printEventWithoutBody(e);
    int clientsTable = clubInfo_.getClientsTable(e.val);
    if (clientsTable != 0) {
      clubInfo_.clientLeaveTable(e.val, e.time);
    }
    clubInfo_.clientLeaveClub(e.val);
  }

  void outgoingClientSatDownHandler(Event e) {
    printEventWithBody(e);
    clubInfo_.moveClientToTable(e.val, e.time, e.tableNum);
  }

  void outgoingErrorHandler(Event e) {
    printEventWithoutBody(e);
  }

  // Create ID 11 event for all clients after end of working day 
  void createLeaveEventForAllClients() {
	  std::vector<std::string> clientNames = clubInfo_.getClientsNames();
	  std::sort(clientNames.begin(), clientNames.end());
	  for (auto&& clientName : clientNames) {
		  Event e = {clientName, clubInfo_.getEndTime(), Events::OutClientLeft, 0};
		  outgoingClientLeftHandler(e);
	  }
  }

private:
  ClubInfo clubInfo_;
  bool isWorkingDayOver = false;
};