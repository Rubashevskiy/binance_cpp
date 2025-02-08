#pragma once

#include <string>
#include <vector>
#include <map>

#include "../utils/decimal.hpp"

struct Auth{
  std::string api_key{};
  std::string user_key{};
};

enum class BinanceErrorType {
  None = 0,
  Transport = 1,
  Server = 2,
  Binance = 3,
  Logical = 4
};

enum class Side {
  NONE = 0,
  BUY = 1,
  SELL = 2
};

enum class OrderStatus {
  NONE = 0,
  NEW = 1,
  FILLED = 2,
  CANCELED = 3
};

struct BinanceError {
  BinanceErrorType e_type = BinanceErrorType::None;
  int e_code = 0;
  std::string e_msg;
};

struct BalanceData {
  dec::decimal<8> free = dec::decimal<8>("0.00000000");
  dec::decimal<8> locked = dec::decimal<8>("0.00000000");
};

struct Balance {
  std::map<std::string, BalanceData> balance;

  BalanceData operator [] (std::string asset) {
    return this->get(asset);
  };

  void set(std::string asset, std::string free, std::string locked) {
    this->balance[asset] = BalanceData{dec::decimal<8>(free), dec::decimal<8>(locked)};
  }

  BalanceData get(std::string asset) {
    if (this->balance.find(asset) != this->balance.end()) {
      return this->balance[asset];
    }
    else {
      return BalanceData();
    }
  }

  std::vector<std::string> assets() {
    std::vector<std::string> result;
    for(auto const& asset: this->balance) {
      result.push_back(asset.first);
    }
    return result;
  }
};

struct Commission {
  std::map<std::string, dec::decimal<8>> commission;

  dec::decimal<8> operator [] (std::string asset) {
    return this->get(asset);
  };

  void set(std::string asset, std::string cms) {
    if (this->commission.find(asset) != this->commission.end()) {
      this->commission[asset] += dec::decimal<8>(cms);
    }
    else {
      this->commission[asset] = dec::decimal<8>(cms);
    }
  }

  dec::decimal<8> get(std::string asset) {
    if (this->commission.find(asset) != this->commission.end()) {
      return this->commission[asset];
    }
    else {
      return dec::decimal<8>("0.00000000");
    }
  }

  std::vector<std::string> assets() {
    std::vector<std::string> result;
    for(auto const& asset: this->commission) {
      result.push_back(asset.first);
    }
    return result;
  }
};

struct Order {
  std::string symbol{};
  uint64_t orderId{0};
  dec::decimal<8> price{"0.00000000"};
  dec::decimal<8> origQty{"0.00000000"};
  Commission commission{};
  Side side{Side::NONE};
  OrderStatus status{OrderStatus::NONE};
  uint64_t time{0};
};


static std::string binance_error_type_str(BinanceErrorType err_type) {
  std::map<BinanceErrorType, std::string> err_m {
    {BinanceErrorType::None, std::string{"None"}},
    {BinanceErrorType::Transport, std::string{"Transport"}},
    {BinanceErrorType::Server, std::string{"Server"}},
    {BinanceErrorType::Binance, std::string{"Binance"}},
    {BinanceErrorType::Logical, std::string{"Logical"}}
  };
  if (err_m.find(err_type) != err_m.end()) {
    return err_m[err_type];
  }
  else {
    return std::string{"NONE"};
  }
}

static std::string side_to_str(Side side) {
  std::map<Side, std::string> side_m {
      {Side::NONE, std::string{"NONE"}},
      {Side::BUY, std::string{"BUY"}},
      {Side::SELL, std::string{"SELL"}}
  };
  if (side_m.find(side) != side_m.end()) {
    return side_m[side];
  }
  else {
    return std::string{"NONE"};
  }
}

static Side str_to_side(std::string side) {
  std::map<std::string, Side> side_m {
    {std::string{"NONE"}, Side::NONE},
    {std::string{"BUY"}, Side::BUY},
    {std::string{"SELL"}, Side::SELL}
  };
  if (side_m.find(side) != side_m.end()) {
    return side_m[side];
  }
  else {
    return Side::NONE;
  }
}

static std::string order_status_to_str(OrderStatus status) {
  std::map<OrderStatus, std::string> status_m {
    {OrderStatus::NONE, std::string{"NONE"}},
    {OrderStatus::NEW, std::string{"NEW"}},
    {OrderStatus::FILLED, std::string{"FILLED"}},
    {OrderStatus::CANCELED, std::string{"CANCELED"}}
  };
  if (status_m.find(status) != status_m.end()) {
    return status_m[status];
  }
  else {
    return std::string{"NONE"};
  }
}

static OrderStatus str_to_order_status(std::string status) {
  std::map<std::string, OrderStatus> status_m {
    {std::string{"NONE"}, OrderStatus::NONE},
    {std::string{"NEW"}, OrderStatus::NEW},
    {std::string{"FILLED"}, OrderStatus::FILLED},
    {std::string{"CANCELED"}, OrderStatus::CANCELED}
  };
  if (status_m.find(status) != status_m.end()) {
    return status_m[status];
  }
  else {
    return OrderStatus::NONE;
  }
}