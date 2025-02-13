#pragma once

#include <string>
#include <iostream>
#include <map>

#include "./binance_type.hpp"
#include "../request/request.hpp"
#include "../utils/utils.hpp"
#include "../utils/json.hpp"


using json = nlohmann::json;
using namespace nlohmann::literals;

const std::string host{"https://api.binance.com"};
const int port{443};
const std::string dttm_format = std::string{"%d.%m.%Y %H:%M:%S"};

struct BaseHeader : public headerparams {
public:
  BaseHeader() {
    this->add("ContentType", "application/x-www-form-urlencoded");
    this->add("Accept", "application/json");
    this->add("User-Agent", "binance/core/cpp/api");
  }
};

/// @brief Класс Binance
class Binance {
private:
  Auth auth_key;
  Order json_to_order(const json &js_order);
  void sign(headerparams& h_params, urlparams& u_params);
  void check_error(const RequestResult &r_result);
public:
  /// @brief Конструктор класса Binance
  /// @param key - Ключи доступа
  Binance(Auth key);

  /// @brief Пинг сервера Binance
  /// @return - True успех
  /// @exception BinanceException
  bool ping();

  /// @brief Время на сервере Binance минус cистемное время
  /// @return - diff(ms) в мс.
  /// @exception BinanceException
  int diff_time();

  /// @brief timestamp c сервера Binance
  /// @param dttm_ms - Дата и время(timestamp ms)
  /// @exception BinanceException
  uint64_t timestamp_ms();

  /// @brief Дата и время в текстовом формате
  /// @return - Дата и время
  /// @exception BinanceException
  std::string data_time();

  /// @brief Возврат цены за пару
  /// @param symbol Торговая пара
  /// @return - Прайс
  /// @exception BinanceException
  dec::decimal<8> symbol_price(const std::string &symbol);
                                /* Запросы с авторизацие */
  /// @brief Получение баланса пользователя
  /// @return - Баланс
  /// @exception BinanceException
  Balance balance();

  /// @brief Создать лимитный ордер
  /// @param order Ордер для создания
  /// @return - Новый ордер
  /// @exception BinanceException
  Order create_order(Order &order);

  /// @brief Открытые ордера
  /// @param symbol Торговая пара
  /// @return - Вектор ордеров
  /// @exception BinanceException
  std::vector<Order> open_orders(const std::string &symbol);

  /// @brief Отмена лимитного ордера
  /// @param symbol Торговая пара
  /// @param order_id Id ордера
  /// @return - Отмененный Ордер
  /// @exception BinanceException
  Order cancel_order(const std::string &symbol, const uint64_t &order_id);

  /// @brief Информация по ордеру
  /// @param symbol Торговая пара
  /// @param order_id Id ордера
  /// @return - Ордер
  /// @exception BinanceException
  Order order_info(const std::string &symbol, const uint64_t &order_id);

  /// @brief Коммисия за ордер
  /// @param symbol Торговая пара
  /// @param order_id Id ордера
  /// @return - Коммисия(контейнер)
  /// @exception BinanceException
  Commission order_commission(const std::string &symbol, const uint64_t &order_id);

  /// @brief Все ордера
  /// @param symbol Торговая пара
  /// @return - Вектор ордеров
  /// @exception BinanceException
  std::vector<Order> all_orders(const std::string &symbol);
  /// @brief Деструктор класса Binance
  ~Binance();
};


