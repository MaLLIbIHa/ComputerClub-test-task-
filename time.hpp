#pragma once
#include <string>
#include <stdexcept>
#include <sstream>
#include <iomanip>

class Time final {
public:
  Time() {}

  Time(unsigned hour, unsigned minute) : hour_(hour), minute_(minute) {
    if (hour_ > 23 || minute_ > 59) {
      throw std::domain_error("Time value out of range");
    }
  }

  std::string to_str() const {
    std::stringstream str;
    str.fill('0');
    str << std::setw(2) << hour_ << ':' << std::setw(2) << minute_;
    return str.str();
  }

  auto operator<=>(const Time &rhs) const = default;

  unsigned getHour() const { return hour_; }

  unsigned getMinute() const { return minute_; }

private:
  unsigned hour_ = 0;
  unsigned minute_ = 0;
};

struct WorkingTime final {
  Time begin;
  Time end;
};

Time timeDifference(Time first, Time second) {
  Time max = std::max(first, second);
  Time min = std::min(first, second);
  int minuteSub = max.getMinute() - min.getMinute();
  unsigned resultHour = max.getHour() - min.getHour() - (minuteSub < 0);
  unsigned resultMin = (minuteSub + 60) % 60;
  return Time(resultHour, resultMin);
}