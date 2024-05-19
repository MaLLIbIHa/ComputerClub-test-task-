#include <list>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <string>
#include "time.hpp"

class ClubInfo final {
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

  Time sumTime(Time first, Time second) {
    unsigned newMin = first.getMinute() + second.getMinute();
    unsigned newHour = first.getHour() + second.getHour() + (newMin >= 60);
    return Time(newHour, newMin % 60);
  }

  unsigned calculateProfit(Time usingTime) {
    return (usingTime.getHour() + (usingTime.getMinute() > 0)) * costPerHour_;
  }

public:
  ClubInfo(WorkingTime workingTime,
           unsigned tableCount, unsigned costPerHour) 
          : workingTime_(workingTime), tablesInfo_(tableCount),
            emptyTables_(tableCount), costPerHour_(costPerHour)
          {}

  Time getBeginTime() const {
    return workingTime_.begin;
  }

  Time getEndTime() const {
    return workingTime_.end;
  }

  int getClientsTable(std::string clientName) const {
    auto clientInfoIt = clientsInfo_.find(clientName);
    if (clientInfoIt != clientsInfo_.end()) {
      return clientInfoIt->second.tableNum;
    }
    return -1;
  }

  Time getTableUsageTime(unsigned tableNum) const {
    return tablesInfo_[tableNum - 1].allUsageTime;
  }

  unsigned getTableIncome(unsigned tableNum) const {
    return tablesInfo_[tableNum - 1].income;
  }

  unsigned getEmptyTables() const { return emptyTables_; }

  bool isTableBusy(unsigned tableNum) const {
    return tablesInfo_[tableNum - 1].isBusy;
  }

  bool isQueueEmpty() const {
    return clientsQueue_.empty();
  }

  const std::string& getFirstInQueue() const {
    return clientsQueue_.front();
  }

  unsigned getTablesCount() const { return tablesInfo_.size(); }

  unsigned getQueueSize() const { return clientsQueue_.size(); }

  std::vector<std::string> getClientsNames() const {
    std::vector<std::string> clientsNames(clientsInfo_.size());
    std::transform(clientsInfo_.begin(), clientsInfo_.end(),
		  clientsNames.begin(), [](auto pair) { return pair.first; });
    return clientsNames;
  }

  void printTablesInfo() const {
    unsigned tableNum = 1;
    for (auto&& tableInfo : tablesInfo_) {
      std::cout << tableNum << ' ' 
                << tableInfo.income << ' '
                << tableInfo.allUsageTime.toStr() << '\n';
      tableNum++;
    }
  }

  void registerNewClient(std::string clientName) {
    clientsQueue_.push_back(clientName);
    auto clientQueueIt = std::prev(clientsQueue_.end());
    clientsInfo_.insert({clientName, ClientInfo{clientQueueIt, 0}});
  }

  void moveClientToTable(std::string clientName, 
                         Time currentTime, 
                         unsigned tableNum) {
    ClientInfo &clientInfo = clientsInfo_[clientName];
    if (clientInfo.tableNum == 0) {
      clientsQueue_.erase(clientInfo.queueIt);
      emptyTables_--;
    } else {
      clientLeaveTable(clientName, currentTime);
    }
    clientsInfo_[clientName] = {clientsQueue_.end(), tableNum};
    TableInfo &tableInfo = tablesInfo_[tableNum - 1];
    tableInfo.isBusy = true;
    tableInfo.beginTime = currentTime;
  }

  void clientLeaveTable(std::string clientName, Time currentTime) {
    ClientInfo &clientInfo = clientsInfo_[clientName];
    TableInfo &currentTable = tablesInfo_[clientInfo.tableNum - 1];
    Time usingTime = timeDifference(
        currentTable.beginTime, currentTime);
    unsigned payment = calculateProfit(usingTime);
    currentTable.allUsageTime = sumTime(
        currentTable.allUsageTime, usingTime);
    currentTable.income += payment;
    currentTable.isBusy = false;

    clientsInfo_[clientName] = {clientsQueue_.end(), 0};
  }

  void clientLeaveClub(std::string clientName) {
    ClientInfo &clientInfo = clientsInfo_[clientName];
    if (clientInfo.queueIt != clientsQueue_.end()) {
      clientsQueue_.erase(clientInfo.queueIt);
    }
    clientsInfo_.erase(clientName);
  }
  
private:
  std::unordered_map<std::string, ClientInfo> clientsInfo_;
  std::vector<TableInfo> tablesInfo_;
  std::list<std::string> clientsQueue_;
  WorkingTime workingTime_;
  unsigned costPerHour_;
  unsigned emptyTables_;
};