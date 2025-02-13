#include <iostream>
#include <fstream>
#include <string>
#include <format>
#include "../src/binance/binance_type.hpp"
#include "../src/utils/utils.hpp"
#include "../src/binance/binance.hpp"

using namespace std;

string config_path{".//config//auth.json"};

const string test_symbol{"VETUSDT"};
Order test_order{test_symbol, 0, dec::decimal<8>("0.02700000"), dec::decimal<8>("423.00000000"), Side::BUY, OrderStatus::NEW, 0};

Auth get_auth() {
  Auth auth{};
  ifstream ifs(config_path.c_str());
  stringstream buffer;
  buffer << ifs.rdbuf();
  if (json::accept(buffer.str())) {
    json js = json::parse(buffer.str());
    if (js.contains("api_key") && (js.contains("user_key"))) {
      auth.api_key=js.value("api_key", string{});
      auth.user_key=js.value("user_key", string{});
      return auth;
    }
    else {
      cout << "Error: <api_key> or <user_key> not found in " << config_path << ". " << endl;
      exit(-1);
    }
  }
  else {
    cout << "Error: Auth file not valid in " << config_path << ". " << endl;
    exit(-1);
  }
  return auth;
}

void print_error(const string func, BinanceException error) {
  cout << "!!!!!!!!!!!!!!!!!!!ERROR!!!!!!!!!!!!!!!!!!!!" << endl;
  cout << left << setw(15) << "FUNCTION" << left << setw(65) << func << endl;
  cout << left << setw(15) << "TYPE" << left << setw(65) << error.e_type_str() << endl;
  cout << left << setw(15) << "E_CODE" << left << setw(65) << error.e_code << endl;
  cout << left << setw(15) << "E_MESSAGE" << left << setw(65) << error.e_msg << endl;
  cout << "!!!!!!!!!!!!!!!!!!!ERROR!!!!!!!!!!!!!!!!!!!!" << endl;
  exit(-2);
}


void print_balance_header() {
  cout << left << setw(10) << "ASSET"
       << left << setw(15) << "FREE"
       << left << setw(15) << "LOCKED"
  << endl;
}

void print_balance_data(const string &asset, const BalanceData &data) {
  cout << left << setw(10) << asset
       << left << setw(15) << val_to_str(data.free)
       << left << setw(15) << val_to_str(data.locked)
  << endl;
}

void print_order_header() {
  cout << left << setw(10) << "Symbol"
       << left << setw(13) << "OrderId"
       << left << setw(15) << "OrigQty"
       << left << setw(15) << "Price"
       << left << setw(15) << "Price(SUM)"
       << left << setw(10) << "Side"
       << left << setw(10) << "Status"
       << left << setw(20) << "DTTM"
  << endl;
}

void print_order(const Order &order) {
  cout << left << setw(10) << order.symbol
       << left << setw(13) << order.orderId
       << left << setw(15) << val_to_str(order.origQty)
       << left << setw(15) << val_to_str(order.price)
       << left << setw(15) << val_to_str(order.origQty*order.price)
       << left << setw(10) << side_to_str(order.side)
       << left << setw(10) << order_status_to_str(order.status)
       << left << setw(20) << since_epoch_dttm(order.time, dttm_format)
  << endl;
}

void print_commission(Commission cms) {
  if (0 < cms.assets().size()) {
    cout << left << setw(10) << "ASSET"
         << left << setw(15) << "Commision"
         << endl;
    for (string &asset : cms.assets()) {
      cout << left << setw(10) << asset
           << left << setw(15) << val_to_str(cms[asset])
      << endl;
    }
  }
}

void test_ping(Binance &binance) {
  try {
    binance.ping();
    cout << left << setw(25) << "Ping" << left << setw(25) << "OK" << endl;
  }
  catch(const BinanceException& e) {
    print_error("Ping", e);
  }
}

void test_date_time(Binance &binance) {
  try {
    cout << left << setw(25) << "Server Date and Time" << left << setw(25) << binance.data_time() << endl;
  }
  catch(const BinanceException& e) {
    print_error("Date_Time", e);
  }
}

void test_timestamp_ms(Binance &binance) {
  try {
    cout << left << setw(25) << "Server Timestamp(ms)" << left << setw(25) << binance.timestamp_ms() << endl;
  }
  catch(const BinanceException& e) {
    print_error("Server Timestamp(ms)", e);
  }
}

void test_diff_time(Binance &binance) {
  try {
    cout << left << setw(25) << "Server DiffTime(ms)" << left << setw(25) << binance.diff_time() << endl;
  }
  catch(const BinanceException& e) {
    print_error("Server DiffTime(ms)", e);
  }
}

void test_price(Binance &binance) {
  try {
    cout << left << setw(25) << "Price  " + test_symbol << left << setw(25) << val_to_str(binance.symbol_price(test_symbol)) << endl;
  }
  catch(const BinanceException& e) {
    print_error("Price", e);
  }
}

void test_asset_balance(Binance &binance) {
  try {
    Balance balance{binance.balance()};
    cout << left << setw(25) << "Balance" << left << setw(25) << "OK" << endl;
    if (0 < balance.assets().size()) {
      print_balance_header();
      for(auto &asset : balance.assets()) {
        print_balance_data(asset, balance[asset]);
      }
    }
  }
  catch(const BinanceException& e) {
    print_error("Balance", e);
  }
}

void test_create_open_cancel_orders(Binance &binance) {
  try {
    Order new_order{binance.create_order(test_order)};
    cout << left << setw(25) << "Create new order" << left << setw(25) << "OK" << endl;
    print_order_header();
    print_order(new_order);
    std::vector<Order> open_orders{binance.open_orders(new_order.symbol)};
    cout << "============================================" << endl;
    cout << left << setw(25) << "Open orders" << left << setw(25) << "OK" << endl;
    print_order_header();
    for(Order &order : open_orders) {
      print_order(order);
      Order cancel_order{binance.cancel_order(order.symbol, order.orderId)};
      cout << "============================================" << endl;
      cout << left << setw(25) << "Cancel order" << left << setw(25) << "OK" << endl;
      print_order(cancel_order);
    };
  }
  catch(const BinanceException& e) {
    print_error("Create Open Cancel", e);
  }
}

void test_filled_orders_info(Binance &binance) {
  try {
    vector<Order> all_orders{binance.all_orders(test_symbol)};
    cout << left << setw(25) << "Filled Orders" << left << setw(25) << "OK" << endl;
    for(Order &order : all_orders) {
      if (OrderStatus::FILLED == order.status) {
        print_order_header();
        print_order(order);
        cout << "============================================" << endl;
        cout << left << setw(25) << "Commission" << left << setw(25) << "OK" << endl;
        print_commission(binance.order_commission(order.symbol, order.orderId));
        return;
      }
    }
  }
  catch(const BinanceException& e) {
    print_error("Filled orders info", e);
  }
}

int main() {
  Binance binance{get_auth()};
  cout << "============================================" << endl;
  cout << "===============BASE REQUEST=================" << endl;
  cout << "============================================" << endl << endl;
  cout << "============================================" << endl;
  test_ping(binance);
  cout << "============================================" << endl;
  test_date_time(binance);
  cout << "============================================" << endl;
  test_timestamp_ms(binance);
  cout << "============================================" << endl;
  test_diff_time(binance);
  cout << "============================================" << endl;
  test_price(binance);
  cout << "============================================" << endl;
  cout << "===============AUTH REQUEST=================" << endl;
  cout << "============================================" << endl << endl;
  cout << "============================================" << endl;
  test_asset_balance(binance);
  cout << "============================================" << endl;
  test_filled_orders_info(binance);
  cout << "============================================" << endl;
  test_create_open_cancel_orders(binance);
  cout << "==================OK========================" << endl;
  return 0;
}