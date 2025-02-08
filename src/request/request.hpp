#pragma once

#include <string>
#include <format>
#include <vector>
#include <regex>
#include <curl/curl.h>
#include <cassert>

#define assertm(exp, msg) assert(((void)msg, exp))

/// @brief Ожидание ответа от сервера
const int def_timeout_ms = 5000;


enum class RequestType {
    NONE = 0,
    GET = 1,
    POST = 2,
    DELETE = 3
};

template<typename v>
static std::string type_to_str(v val) {
  if constexpr (std::is_same_v<v, bool>) {
    return std::format("{}", val);
  }
  std::ostringstream strm_v;
  strm_v << val;
  return strm_v.str();
};

/// @brief Структура(IN) для добавление параметров Header
struct headerparams {
  std::vector<std::string> header_params;
  /// @brief Функция (шаблон) для добавления параметров Header
  /// @param key Ключ, переменная шаблона(конвертируется в std::string)
  /// @param val Значение, переменная шаблона(конвертируется в std::string)
  template<typename k, typename v>
  void add(k key, v val) {
    header_params.push_back(std::string(std::format("{}: {}", type_to_str(key), type_to_str(val))));
  };
};
/// @brief Структура(IN) для добавление параметров параметров URL
struct urlparams {
  std::string url_params{""};
  /// @brief Функция (шаблон) для добавления параметров URL
  /// @tparam k  Ключ, переменная шаблона(конвертируется в std::string)
  /// @tparam v Значение, переменная шаблона(конвертируется в std::string)
  /// @param key Ключ
  /// @param val Значение
  template<typename K, typename v>
  void add(K key, v val) {
    if (!url_params.empty()) {
      url_params += "&";
    }
    url_params += std::string(std::format("{}={}", type_to_str(key), type_to_str(val)));
  }
  /// @brief Проверка на наличие параметров URL
  /// @return bool True - данных нет; False - Данные есть
  bool empty() {
    return url_params.empty();
  }
};

struct Status {
  Status() {};
  Status(int code, std::string msg) : code(code), msg(msg) {};
  int code{-1}; // Код(результат)
  std::string msg{"null"}; // Сообщение
};

/// @brief Структура для хранения результатов запроса
struct RequestResult {
  Status header{}; // Код и расшифровка статуса запроса HTTPS запроса(Берется из Header)
  Status transport{}; // Код и расшифровка статуса транспорта(CURL)
  std::string body{""}; // "Тело ответа" сервера
};


/// @brief Класс-обёртка(CURL) реализующие HTTPS запросы GET, POST, DELETE
class Request {
private:
  std::string _host;
  int _port;
  static size_t data_callback(char *contents, size_t size, size_t nmemb, void *userp);
  curl_slist* header_generate(const headerparams& r_params, curl_slist *h_struct);
  Status parse_header(const std::string header_raw);
  std::string req_type_to_str(RequestType r_type);
  RequestType str_to_req_type(std::string r_type);
public:
  /// @brief Конструктор класса Request
  /// @param host Адресс ресурса
  Request(std::string host, int port);
  /// @brief Реализация запроса
  /// @param r_type Тип запроса
  /// @param path Путь к ресурсу
  /// @param h_params Параметры хедера
  /// @param u_params параметры URL
  /// @return RequestResult структура с ответом и статусами
  RequestResult request(RequestType r_type, std::string path, headerparams h_params, urlparams u_params);
  ~Request();
};
