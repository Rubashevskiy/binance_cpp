#include "binance.hpp"


Binance::Binance(Auth key) : auth_key(key){}

bool Binance::ping() {
  Request request{host, port};
  RequestResult r_result = request.request(RequestType::GET,"/api/v3/ping", BaseHeader(), urlparams());
  check_error(r_result);
  return true;
}

int Binance::diff_time() {
  uint64_t current_time{current_ms_epoch()};
  uint64_t server_timestamp{timestamp_ms()};
  return server_timestamp - current_time;
}

uint64_t Binance::timestamp_ms() {
  Request request{host, port};
  RequestResult r_result = request.request(RequestType::GET, "/api/v3/time", BaseHeader(), urlparams());
  check_error(r_result);
  json js = json::parse(r_result.body);
  return js.value("serverTime", std::uint64_t(0));
}

std::string Binance::data_time() {
  std::string dttm = since_epoch_dttm(timestamp_ms(), dttm_format);
  return dttm;
}

dec::decimal<8> Binance::symbol_price(const std::string &symbol) {
  Request request{host, port};
  urlparams params;
  params.add("symbol", symbol);
  RequestResult r_result = request.request(RequestType::GET, "/api/v3/ticker/price", BaseHeader(), params);
  check_error(r_result);
  json js = json::parse(r_result.body);
  return dec::decimal<8>(js.value("price", std::string{}));
}

Balance Binance::balance() {
  Request request{host, port};
  BaseHeader header{};
  urlparams params;
  params.add("omitZeroBalances", true);
  sign(header, params);
  RequestResult r_result = request.request(RequestType::GET, "/api/v3/account", header, params);
  check_error(r_result);
  json js = json::parse(r_result.body);
  Balance balance{};
  for(auto &array : js["balances"]) {
    balance.set(array.value("asset", std::string{}), array.value("free", std::string{}), array.value("locked", std::string{}));
  }
  return balance;
}

Order Binance::create_order(Order &order) {
  Request request{host, port};
  BaseHeader header;
  urlparams params;
  params.add("symbol", order.symbol);
  params.add("side", side_to_str(order.side));
  params.add("type", std::string{"LIMIT"});
  params.add("timeInForce", std::string{"GTC"});
  params.add("quantity", order.origQty);
  params.add("price", order.price);
  params.add("newOrderRespType", std::string{"RESULT"});
  sign(header, params);
  RequestResult r_result = request.request(RequestType::POST, "/api/v3/order", header, params);
  check_error(r_result);
  return json_to_order(json::parse(r_result.body));
}

std::vector<Order> Binance::open_orders(const std::string &symbol) {
  Request request{host, port};
  BaseHeader header;
  urlparams params;
  params.add("symbol", "VETUSDT");
  sign(header, params);
  RequestResult r_result = request.request(RequestType::GET, "/api/v3/openOrders", header, params);
  check_error(r_result);
  std::vector<Order> orders{};
  json js = json::parse(r_result.body);
  for(auto &js_order : js) {
    Order order = json_to_order(js_order);
    orders.push_back(order);
  }
  return orders;
}

Order Binance::cancel_order(const std::string &symbol, const uint64_t &order_id) {
  Request request{host, port};
  BaseHeader header;
  urlparams params;
  params.add("symbol", symbol);
  params.add("orderId", order_id);
  sign(header, params);
  RequestResult r_result = request.request(RequestType::DELETE, "/api/v3/order", header, params);
  check_error(r_result);
  return json_to_order(json::parse(r_result.body));
}

Order Binance::order_info(const std::string &symbol, const uint64_t &order_id) {
  Request request{host, port};
  headerparams header;
  urlparams params;
  params.add("symbol", symbol);
  params.add("orderId", order_id);
  sign(header, params);
  RequestResult r_result = request.request(RequestType::GET, "/api/v3/order", header, params);
  check_error(r_result);
  return json_to_order(json::parse(r_result.body));
}

Commission Binance::order_commission(const std::string &symbol, const uint64_t &order_id) {
  Request request{host, port};
  headerparams header;
  urlparams params;
  params.add("symbol", symbol);
  params.add("orderId", order_id);
  sign(header, params);
  RequestResult r_result = request.request(RequestType::GET, "/api/v3/myTrades", header, params);
  check_error(r_result);
  json js = json::parse(r_result.body);
  Commission cms{};
  for (auto &js_cms : js) {
    if (js_cms.contains("commission") && (js_cms.contains("commissionAsset"))) {
      cms.set(js_cms.value("commissionAsset", std::string{}), js_cms.value("commission", std::string{}));
    }
  }
  return cms;
}

std::vector<Order> Binance::all_orders(const std::string &symbol) {
  Request request{host, port};
  headerparams header;
  urlparams params;
  params.add("symbol", symbol);
  sign(header, params);
  RequestResult r_result = request.request(RequestType::GET, "/api/v3/allOrders", header, params);
  check_error(r_result);
  json js = json::parse(r_result.body);
  std::vector<Order> orders{};
  for (auto &js_order : js) {
    orders.push_back(json_to_order(js_order));
  }
  return orders;
}

Order Binance::json_to_order(const json &js_order) {
  Order order;
  order.symbol = js_order.value("symbol", std::string{});
  order.orderId = js_order.value("orderId", uint64_t{});
  order.price = dec::decimal<8>(js_order.value("price", std::string{}));
  order.origQty = dec::decimal<8>(js_order.value("origQty", std::string{}));
  order.side = str_to_side(js_order.value("side", std::string{}));
  order.status = str_to_order_status(js_order.value("status", std::string{}));
  if (js_order.contains("time")) {
    order.time = js_order.value("time", uint64_t{});
  }
  else if (js_order.contains("transactTime")) {
    order.time = js_order.value("transactTime", uint64_t{});
  }
  else {
    order.time = 0;
  }
  return order;
}

void Binance::sign(headerparams &h_params, urlparams &u_params) {
  h_params.add("X-MBX-APIKEY", auth_key.api_key);
  u_params.add("recvWindow", 5000);
  u_params.add("timestamp", current_ms_epoch());
  u_params.add("signature", hmac_sha256(auth_key.user_key.c_str(), u_params.url_params.c_str()));
}

void Binance::check_error(const RequestResult &r_result) {
  if (0 != r_result.transport.code) {
    throw BinanceException{ExceptionType::Transport, r_result.transport.code, r_result.transport.msg};
  }
  else if (200 != r_result.header.code) {
    try {
      json js = json::parse(r_result.body);
      if (js.contains("code") && (js.contains("msg"))) {
        throw BinanceException{ExceptionType::Binance, js["code"], js["msg"]};
      }
      else {
        throw BinanceException{ExceptionType::Server, r_result.header.code, r_result.header.msg};
      }
    }
    catch (json::parse_error& ex) {
      throw BinanceException{ExceptionType::Transport, r_result.header.code, r_result.header.msg};
    }
  }
}

Binance::~Binance() {}
