#pragma once
#include <unordered_map>
#include <vector>
#include <iostream>
#include <list>
#include <algorithm>
#include "event.hpp"
#include "time.hpp"

class EventHandler final {
public:
  EventHandler(WorkingTime wTime,
               unsigned costPerHour,
               unsigned tablesCount) 
              : workingTime_(wTime), 
                costPerHour_(costPerHour),
                tablesInfo_(tablesCount + 1),
                emptyTables_(tablesCount)
              {}

  void handleEvent(Event e) {
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
      case Events::OutClientLeft:
        outgoingClientLeftHandler(e);
        break;
      case Events::OutClientSatDown:
        outgoingClientSatDownHandler(e);
        break;
      case Events::OutError:
        outgoingErrorHandler(e);
        break;
    }
  }

  void createLeaveEventForAllClients() {
	  std::vector<std::string> clientNames(clientsInfo_.size());
	  std::transform(clientsInfo_.begin(), clientsInfo_.end(),
		  clientNames.begin(), [](auto pair) { return pair.first; });
	  std::sort(clientNames.begin(), clientNames.end());
	  for (auto&& clientName : clientNames) {
		  Event e = { clientName, workingTime_.end, Events::OutClientLeft, 0 };
		  outgoingClientLeftHandler(e);
	  }
  }

  void printTablesInfo() const {
    for (int i = 1; i < tablesInfo_.size(); i++) {
      std::cout << i << ' ' 
                << tablesInfo_[i].income << ' '
                << tablesInfo_[i].allUsageTime.to_str() << '\n';
    }
  }

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

private:
  //utility functions for handle events

  void printEventWithoutBody(Event e) {
    std::cout << e.time.to_str() << ' '
              << static_cast<unsigned>(e.eventId) << ' '
              << e.val << '\n';
  }

  void printEventWithBody(Event e) {
    std::cout << e.time.to_str() << ' '
              << static_cast<unsigned>(e.eventId) << ' ' 
              << e.val << ' '
              << e.tableNum << '\n';
  }

  Time sumTime(Time first, Time second) {
    unsigned newMin = first.getMinute() + second.getMinute();
    unsigned newHour = first.getHour() + second.getHour() + (newMin >= 60);
    return Time(newHour, newMin % 60);
  }

  unsigned calculateProfit(Time begin, Time end) {
    Time usingTime = timeDifference(begin, end);
    return (usingTime.getHour() + (usingTime.getMinute() > 0)) * costPerHour_;
  }

  TableInfo processClientLeavingTable(TableInfo tableInfo, Time currentTime) {
    Time beginTime = tableInfo.beginTime;
    Time usingTime = timeDifference(beginTime, currentTime);
    unsigned payment = calculateProfit(currentTime, beginTime);
    tableInfo.allUsageTime = sumTime(tableInfo.allUsageTime, usingTime);
    tableInfo.income += payment;
    tableInfo.isBusy = false;
    return tableInfo;
  }

  TableInfo processClientSatDownAtTable(TableInfo tableInfo, Time currentTime) {
    tableInfo.beginTime = currentTime;
    tableInfo.isBusy = true;
    return tableInfo;
  }

  // Main event handlers

  void incomingClientArrivedHandler(Event e) {
    printEventWithoutBody(e);
    if (clientsInfo_.find(e.val) != clientsInfo_.end()) {
      outgoingErrorHandler(
          Event{"YouShallNotPass", e.time, Events::OutError, 0});
      return;
    } else if (e.time < workingTime_.begin || workingTime_.end < e.time) {
      outgoingErrorHandler(
          Event{"NotOpenYet", e.time, Events::OutError, 0});
      return;
    }
    clientsQueue_.push_back(e.val);
    auto clientQueueIt = std::prev(clientsQueue_.end());
    clientsInfo_.insert({e.val, ClientInfo{clientQueueIt, 0}});
  }

  void incomingClientSatDownHandler(Event e) {
    printEventWithBody(e);
    auto clientInfoIt = clientsInfo_.find(e.val);
    if (clientInfoIt == clientsInfo_.end()) {
      outgoingErrorHandler(
          Event{"ClientUnknown", e.time, Events::OutError, 0});
      return;
    } else if (tablesInfo_[e.tableNum].isBusy) {
      outgoingErrorHandler(
          Event{"PlaceIsBusy", e.time, Events::OutError, 0});
      return;
    }

    unsigned currentTable = clientInfoIt->second.tableNum;
    if (currentTable != 0) {
      TableInfo updatedTableInfo = 
        processClientLeavingTable(tablesInfo_[currentTable], e.time);
      tablesInfo_[currentTable] = updatedTableInfo;
    } else {
      clientsQueue_.erase(clientInfoIt->second.queueIt);
      emptyTables_--;
    }
    //move client to new table
    clientsInfo_[e.val] = {clientsQueue_.end(), e.tableNum};
    TableInfo newTableInfo = 
      processClientSatDownAtTable(tablesInfo_[e.tableNum], e.time);
    tablesInfo_[e.tableNum] = newTableInfo;
  }

  void incomingClientWaitingHandler(Event e) {
    printEventWithoutBody(e);
    if (emptyTables_ > 0) {
      outgoingErrorHandler(
          Event{"ICanWaitNoLonger!", e.time, Events::OutError, 0});
      return;
    }
    if (clientsQueue_.size() > tablesInfo_.size() - 1) {
      outgoingClientLeftHandler(
          Event{e.val, e.time, Events::OutClientLeft, 0});
      return;
    }
  }

  void incomingClientLeftHandler(Event e) {
    printEventWithoutBody(e);
    auto clientInfoIt = clientsInfo_.find(e.val);
    if (clientInfoIt == clientsInfo_.end()) {
      outgoingErrorHandler(
          Event{"ClientUnknown", e.time, Events::OutError, 0});
      return;
    }

    ClientInfo clientInfo = clientInfoIt->second;
    unsigned currentTable = clientInfo.tableNum;
    if (currentTable == 0) {
      clientsQueue_.erase(clientInfo.queueIt);
    } else {
      TableInfo updatedTableInfo = 
        processClientLeavingTable(tablesInfo_[currentTable], e.time);
      tablesInfo_[currentTable] = updatedTableInfo;

      if (!clientsQueue_.empty()) {
        std::string nextClient = clientsQueue_.front();
        clientsQueue_.pop_front();
        outgoingClientSatDownHandler(
            Event{nextClient, e.time, Events::OutClientSatDown, currentTable});
      } else {
        emptyTables_++;
      }
    }
    clientsInfo_.erase(e.val);
  }

  void outgoingClientLeftHandler(Event e) {
    printEventWithoutBody(e);
    auto clientInfo = clientsInfo_[e.val];
    unsigned currentTable = clientInfo.tableNum;
    if (currentTable == 0) {
      clientsQueue_.erase(clientInfo.queueIt);
    } else {
      TableInfo updatedTableInfo = 
        processClientLeavingTable(tablesInfo_[currentTable], e.time);
      tablesInfo_[currentTable] = updatedTableInfo;
      emptyTables_++;
    }
    clientsInfo_.erase(e.val);
  }

  void outgoingClientSatDownHandler(Event e) {
    printEventWithBody(e);
    std::string nextClient = e.val;
    clientsInfo_[nextClient] = {clientsQueue_.end(), e.tableNum};
    TableInfo newTableInfo =
      processClientSatDownAtTable(tablesInfo_[e.tableNum], e.time);
    tablesInfo_[e.tableNum] = newTableInfo;
  }

  void outgoingErrorHandler(Event e) {
    printEventWithoutBody(e);
  }

private:
  std::unordered_map<std::string, ClientInfo> clientsInfo_;
  std::vector<TableInfo> tablesInfo_;
  std::list<std::string> clientsQueue_;
  WorkingTime workingTime_;
  unsigned costPerHour_;
  unsigned emptyTables_;
};