#include <iostream>
#include <fstream>
#include <string>
#include <format>
#include "../src/binance/binance_type.hpp"
#include "../src/utils/utils.hpp"
#include "../src/binance/binance.hpp"

std::string config_path{".//config//auth.json"};

const std::string test_symbol{"VETUSDT"};
Order test_order{test_symbol, 0, dec::decimal<8>("0.02700000"), dec::decimal<8>("423.00000000"), Commission(), Side::BUY, OrderStatus::NEW, 0};



Auth read_config() {
  Auth auth{};
  std::ifstream ifs(config_path.c_str());
  std::stringstream buffer;
  buffer << ifs.rdbuf();
  if (json::accept(buffer.str())) {
    json js = json::parse(buffer.str());
    if (js.contains("api_key") && (js.contains("user_key"))) {
      auth.api_key=js.value("api_key", std::string{});
      auth.user_key=js.value("user_key", std::string{});
      return auth;
    }
    else {
      std::cout << "Error: <api_key> or <user_key> not found in " << config_path << ". " << std::endl;
      exit(-1);
    }
  }
  else {
    std::cout << "Error: Auth file not valid in " << config_path << ". " << std::endl;
    exit(-1);
  }
  return auth;
}

void printError(const std::string func, BinanceError error) {
  std::cout
    << std::format("ERROR <{}> FN <{}>: Code <{}> Msg <{}>", binance_error_type_str(error.e_type), func, error.e_code, error.e_msg)
  << std::endl;
  exit(-2);
}

void printOrder(const std::string func, Order order) {
  std::cout << "Order <" << func << ">:"
    << " " << order.symbol
    << " " << order.orderId
    << " " << type_to_str(order.origQty)
    << " " << type_to_str(order.price)
    << " " << type_to_str(order.origQty * order.price)
    << " " << side_to_str(order.side)
    << " " << order_status_to_str(order.status)
    << " " << since_epoch_dttm(order.time, dttm_format)
  << std::endl;
  for (std::string &asset : order.commission.assets()) {
    std::cout << "Commission:" 
      << " " << asset
      << " " << order.commission[asset]
    << std::endl;
  }
}

void test_ping(Binance &binance) {
    if (binance.ping()) {std::cout << "Server ping - <OK>" << std::endl;}
    else {printError("Ping", binance.last_error());};
}

void test_date_time(Binance &binance) {
  std::string dttm{};
  if (binance.data_time(dttm)) {std::cout << "Server DateTime - " << dttm << std::endl;}
  else {printError("Date_Time", binance.last_error());};
}

void test_timestamp_ms(Binance &binance) {
  uint64_t srv_dttm_ms;
  if (binance.timestamp_ms(srv_dttm_ms)) {std::cout << "Server Timestamp(ms) - " << srv_dttm_ms << std::endl;}
  else {printError("Timestamp_ms", binance.last_error());};
}

void test_ping_time(Binance &binance) {
  int ms{0};
  if (binance.ping_time(ms)) {std::cout << "Server PingTime(ms) - " << ms << std::endl;}
  else {printError("Ping_Time", binance.last_error());};
}

void test_price(Binance &binance) {
  dec::decimal<8> price;
  if (binance.symbol_price(test_symbol, price)) {
    std::cout << std::format("Price <{}> - <{}>", test_symbol, val_to_str(price)) << std::endl;
  }
  else {printError("Price", binance.last_error());};
}

void test_asset_balance(Binance &binance) {
  Balance balance;
  if (binance.balance(balance)) {
    for(auto &asset : balance.assets()) {
      std::cout<< "Balance <" << asset << ">: free: <" << balance[asset].free << "> locked: <" << balance[asset].locked << std::endl;
    }
  }
  else {printError("Balance", binance.last_error());};
}

void test_create_open_cancel_orders(Binance &binance) {
  if (binance.create_order(test_order)) {
    printOrder(std::string{"Create"}, test_order);
  }
  else {printError("Create order", binance.last_error());};
  std::vector<Order> open_orders;
  if (binance.open_orders(test_symbol, open_orders)) {
    for (Order &o_order : open_orders) {
      printOrder(std::string{"Open"}, o_order);
      Order cancel_order;
      if (binance.cancel_order(o_order.symbol, o_order.orderId, cancel_order)) {
        printOrder(std::string{"Cancel"}, cancel_order);
      }
      else {printError("Cancel order", binance.last_error());}
    }
  }
  else {printError("Open order", binance.last_error());};
}

void test_all_orders_info(Binance &binance) {
  std::vector<Order> all_orders;
  if (binance.all_orders(test_symbol, all_orders)) {
    for(Order &order : all_orders) {
      printOrder(std::string{"List"}, order);
      if (OrderStatus::FILLED == order.status) {
        Order order_info;
        if (binance.order_info(order.symbol, order.orderId, order_info)) {
          printOrder(std::string{"Order INFO"}, order_info);
          break;
        }
        else {printError("order_info", binance.last_error());}
      }
    }
  }
  else {printError("all_orders", binance.last_error());};
}

int main() {
  Binance binance{read_config()};
  test_ping(binance);
  test_date_time(binance);
  test_timestamp_ms(binance);
  test_ping_time(binance);
  test_price(binance);
  test_asset_balance(binance);
  test_all_orders_info(binance);
  test_create_open_cancel_orders(binance);
  return 0;
}