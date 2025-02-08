#include "binance.hpp"


Binance::Binance(Auth key) : auth_key(key){}

bool Binance::ping() {
  Request request{host, port};
  RequestResult r_result = request.request(RequestType::GET,"/api/v3/ping", BaseHeader(), urlparams());
  return error_check(r_result);
}

bool Binance::diff_time(int &ms) {
  Request request{host, port};
  uint64_t srv_dttm_ms;
  uint64_t current_time = current_ms_epoch();
  if (timestamp_ms(srv_dttm_ms)) {
    ms = srv_dttm_ms - current_time;
    return true;
  }
  return false;
}

bool Binance::timestamp_ms(uint64_t &timestamp) {
  Request request{host, port};
  RequestResult r_result = request.request(RequestType::GET, "/api/v3/time", BaseHeader(), urlparams());
  if (error_check(r_result)) {
    json js = json::parse(r_result.body);
    timestamp = js.value("serverTime", std::uint64_t(0));
    return true;
  }
  return false;
}

bool Binance::data_time(std::string &dttm) {
  uint64_t srv_dttm_ms;
  if (timestamp_ms(srv_dttm_ms)) {
    dttm = since_epoch_dttm(srv_dttm_ms, dttm_format);
    return true;
  }
  return false;
}

bool Binance::symbol_price(std::string symbol, dec::decimal<8> &price) {
  Request request{host, port};
  urlparams params;
  params.add("symbol", symbol);
  RequestResult r_result = request.request(RequestType::GET, "/api/v3/ticker/price", BaseHeader(), params);
  if (error_check(r_result)) {
    json js = json::parse(r_result.body);
    price = dec::decimal<8>(js.value("price", std::string{}));
    return true;
  }
  return false;
}

bool Binance::balance(Balance &balance) {
  Request request{host, port};
  BaseHeader header;
  urlparams params;
  params.add("omitZeroBalances", true);
  sign(header, params);
  RequestResult r_result = request.request(RequestType::GET, "/api/v3/account", header, params);
  if (error_check(r_result)) {
    json js = json::parse(r_result.body);
    for(auto &array : js["balances"]) {
      balance.set(array.value("asset", std::string{}), array.value("free", std::string{}), array.value("locked", std::string{}));
    }
    return true;
  }
  return false;
}

bool Binance::create_order(Order &order) {
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
  if (error_check(r_result)) {
    json js_order = json::parse(r_result.body);
    order = js_to_order(js_order);
    return true;
  }
  return false;
}

bool Binance::open_orders(std::string symbol, std::vector<Order> &orders) {
  Request request{host, port};
  BaseHeader header;
  urlparams params;
  params.add("symbol", "VETUSDT");
  sign(header, params);
  RequestResult r_result = request.request(RequestType::GET, "/api/v3/openOrders", header, params);
  if (error_check(r_result)) {
    json js = json::parse(r_result.body);
    for(auto &js_order : js) {
      Order order = js_to_order(js_order);
      orders.push_back(order);
    }
    return true;
  }
  return false;
}

bool Binance::order_info(std::string symbol, uint64_t order_id, Order &order) {
  Request request{host, port};
  BaseHeader header;
  urlparams params;
  params.add("symbol", symbol);
  params.add("orderId", order_id);
  sign(header, params);
  RequestResult r_result = request.request(RequestType::GET, "/api/v3/order", header, params);
  if (error_check(r_result)) {
    json js_order = json::parse(r_result.body);
    order = js_to_order(js_order);
    if (OrderStatus::NEW != order.status) {
      Commission cms{};
      if (order_commission(symbol, order_id, order.commission)) {
        return true;
      }
      else {
        return false;
      }
    }
    return true;
  }
  return false;
}

bool Binance::order_commission(std::string symbol, uint64_t order_id, Commission &cms) {
  Request request{host, port};
  BaseHeader header;
  urlparams params;
  params.add("symbol", symbol);
  params.add("orderId", order_id);
  sign(header, params);
  RequestResult r_result = request.request(RequestType::GET, "/api/v3/myTrades", header, params);
  if (error_check(r_result)) {
    json js = json::parse(r_result.body);
    for(auto &js_cms : js) {
      if (js_cms.contains("commission") && (js_cms.contains("commissionAsset"))) {
        cms.set(js_cms.value("commissionAsset", std::string{}), js_cms.value("commission", std::string{}));
      }
    }
    return true;
  }
  return false;
}

bool Binance::cancel_order(std::string symbol, uint64_t order_id, Order &order) {
  Request request{host, port};
  BaseHeader header;
  urlparams params;
  params.add("symbol", symbol);
  params.add("orderId", order_id);
  sign(header, params);
  RequestResult r_result = request.request(RequestType::DELETE, "/api/v3/order", header, params);
  if (error_check(r_result)) {
    json js_order = json::parse(r_result.body);
    order = js_to_order(js_order);
    return true;
  }
  return false;
}

bool Binance::all_orders(std::string symbol, std::vector<Order> &orders) {
  Request request{host, port};
  headerparams header;
  urlparams params;
  params.add("symbol", symbol);
  sign(header, params);
  RequestResult r_result = request.request(RequestType::GET, "/api/v3/allOrders", header, params);
  if (error_check(r_result)) {
    json js = json::parse(r_result.body);
      for (auto &js_order : js) {
        Order order = js_to_order(js_order);
        orders.push_back(order);
      }
      return true;
  }
  return false;
}

Order Binance::js_to_order(json js_order) {
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

void Binance::sign(headerparams &h_params, urlparams &u_params)
{
    h_params.add("X-MBX-APIKEY", auth_key.api_key);
    u_params.add("recvWindow", 5000);
    u_params.add("timestamp", current_ms_epoch());
    u_params.add("signature", hmac_sha256(auth_key.user_key.c_str(), u_params.url_params.c_str()));
}

bool Binance::error_check(RequestResult &r_result) {
  if (0 != r_result.transport.code) {
    error = BinanceError{BinanceErrorType::Transport, r_result.transport.code, r_result.transport.msg};
    return false;
  }
  else if (200 != r_result.header.code) {
    try {
      json js = json::parse(r_result.body);
      if (js.contains("code") && (js.contains("msg"))) {
        error = BinanceError{BinanceErrorType::Binance, js["code"], js["msg"]};
        return false;
      }
      else {
        error = BinanceError{BinanceErrorType::Server, r_result.transport.code, r_result.transport.msg};
        return false;
      }
    }
    catch (json::parse_error& ex) {
      error = BinanceError{BinanceErrorType::Transport, r_result.transport.code, r_result.transport.msg};
      return false;
    }
  }
  return true;
}

BinanceError Binance::last_error() {
  return error;
}

Binance::~Binance() {}
