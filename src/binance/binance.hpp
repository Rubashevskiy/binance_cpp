#pragma once

#include <string>
#include <iostream>
#include <map>

#include "binance_type.hpp"
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
  BinanceError error;
  void sign(headerparams& h_params, urlparams& u_params);
  bool error_check(RequestResult &r_result);
public:
  /// @brief Конструктор класса Binance
  /// @param api_key - API KEY
  /// @param user_key - USER KEY
  Binance(Auth key);
  /// @brief Пинг сервера Binance
  /// @return - Ошибка/Без ошибок
  bool ping();
  /// @brief Время на сервере Binance минус cистемное время
  /// @param ms - diff в мс.
  /// @return - Ошибка/Без ошибок
  bool ping_time(int &ms);
  /// @brief timestamp c сервера Binance
  /// @param dttm_ms - Дата и время(timestamp ms)
  /// @return - Ошибка/Без ошибок
  bool timestamp_ms(uint64_t &dttm_ms);
  /// @brief Дата и время в текстовом формате
  /// @return - Ошибка/Без ошибок
  bool data_time(std::string &dttm);
  /// @brief Возврат цены за пару
  /// @param symbol Торговая пара
  /// @param price Прайс
  /// @return - Ошибка/Без ошибок
  bool symbol_price(std::string symbol, dec::decimal<8> &price);
                                /* Запросы с авторизацие */
  /// @brief Получение баланса пользователя
  /// @param balance Баланс пользователя
  /// @return - Ошибка/Без ошибок
  bool balance(Balance &balance);
  /// @brief Создать лимитный ордер
  /// @param order Данные ордера(IN/OUT)
  /// @return - Ошибка/Без ошибок
  bool create_order(Order &order);
  /// @brief Открытые ордера
  /// @param symbol Торговая пара
  /// @param orders Вектор ордеров(контейнер)
  /// @return - Ошибка/Без ошибок
  bool open_orders(std::string symbol, std::vector<Order> &orders);
  /// @brief Информация по ордеру
  /// @param symbol Торговая пара
  /// @param order_id Id ордера
  /// @param order Ордер(контейнер)
  /// @return - Ошибка/Без ошибок
  bool order_info(std::string symbol, uint64_t order_id, Order &order);
  /// @brief Коммисия за ордер
  /// @param symbol Торговая пара
  /// @param order_id Id ордера
  /// @param cms Коммисия(контейнер)
  /// @return - Ошибка/Без ошибок
  bool order_commission(std::string symbol, uint64_t order_id, Commission &cms);
  /// @brief Отмена лимитного ордера
  /// @param symbol Торговая пара
  /// @param order_id Id ордера
  /// @param order Ордер(контейнер)
  /// @return - Ошибка/Без ошибок
  bool cancel_order(std::string symbol, uint64_t order_id, Order &order);
  /// @brief Все ордера
  /// @param symbol Торговая пара
  /// @param orders Вектор ордеров
  /// @return - Ошибка/Без ошибок
  bool all_orders(std::string symbol, std::vector<Order> &orders);
  /// @brief Возврат последней ошибки
  /// @return - Ошибка в формате BinanceError
  BinanceError last_error();
  /// @brief Деструктор класса Binance
  ~Binance();
};


