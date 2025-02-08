#include "request.hpp"

Request::Request(std::string host, int port) : _host(host), _port(port) {
  CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
  assertm(CURLcode::CURLE_OK == res, "CURLE_FAILED_INIT");
}

RequestResult Request::request(RequestType r_type, std::string path, headerparams h_params, urlparams u_params) {
  RequestResult result;
  CURL *session{nullptr};
  session = curl_easy_init();
  if (session) {
    struct curl_slist *header{nullptr};
    CURLcode res;
    std::string header_buffer{};
    std::string body_buffer{};
    std::string url_prm{};
    curl_easy_setopt(session, CURLOPT_CUSTOMREQUEST, req_type_to_str(r_type).c_str());
    if (RequestType::GET == r_type) { //GET
      url_prm = u_params.empty() ? "" : std::format("?{}", u_params.url_params);
      curl_easy_setopt(session, CURLOPT_URL, std::string(_host + path + url_prm).c_str());
    }
    else { //POST or DELETE
      url_prm = u_params.empty() ? "" : std::format("{}", u_params.url_params);
      curl_easy_setopt(session, CURLOPT_URL, std::string(_host+path).c_str());
      curl_easy_setopt(session, CURLOPT_POSTFIELDS, url_prm.c_str());
    }
      header = header_generate(h_params, header);
      curl_easy_setopt(session, CURLOPT_TIMEOUT_MS, def_timeout_ms);
      curl_easy_setopt(session, CURLOPT_PORT, _port);
      curl_easy_setopt(session, CURLOPT_HTTPHEADER, header);
      curl_easy_setopt(session, CURLOPT_HEADERFUNCTION, Request::data_callback);
      curl_easy_setopt(session, CURLOPT_WRITEFUNCTION, Request::data_callback);
      curl_easy_setopt(session, CURLOPT_HEADERDATA, &header_buffer);
      curl_easy_setopt(session, CURLOPT_WRITEDATA, &body_buffer);
/*
    #ifdef SKIP_PEER_VERIFICATION
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    #endif

    #ifdef SKIP_HOSTNAME_VERIFICATION
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    #endif
*/
      curl_easy_setopt(session, CURLOPT_CA_CACHE_TIMEOUT, 604800L);
      res = curl_easy_perform(session);
      if (res == CURLE_OK) {
        result.header = parse_header(header_buffer);
        result.transport = Status(static_cast<int>(res), std::string(curl_easy_strerror(res)));
        result.body = body_buffer;
      }
      else {
        result.transport = Status(static_cast<int>(res), std::string(curl_easy_strerror(res)));
      }
      curl_slist_free_all(header);
  }
  else {
    result.transport = Status(2, std::string("CURL INIT FAILED"));
  }
  curl_easy_cleanup(session);
  return result;
}

size_t Request::data_callback(char *contents, size_t size, size_t nmemb, void *userp) {
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}

curl_slist *Request::header_generate(const headerparams &r_params, curl_slist *h_struct) {
  for (auto &data : r_params.header_params) {
    h_struct = curl_slist_append(h_struct, data.c_str());
  }
  return h_struct;
}

Status Request::parse_header(const std::string header_raw) {
  if (header_raw.empty()) {
    return Status(-1, std::string("No header data"));
  } 
  std::regex status_regex("^(HTTP/\\d.\\d) (\\d{3}) (.+)");
  std::smatch status_match;
  std::istringstream stream(header_raw);
  std::string line;
  while (std::getline(stream, line, '\n')) {
    if (std::regex_search(line, status_match, status_regex)) {
      if (4 == status_match.size()) {
        return Status(std::stoi(status_match[2]), std::string(status_match[3]));
      }
    }
  }
  return Status(-1, std::string("Header data is not valid"));
}

  std::string Request::req_type_to_str(RequestType r_type) {
    std::map<RequestType, std::string> req_map {
      {RequestType::NONE, std::string{"NONE"}},
      {RequestType::GET, std::string{"GET"}},
      {RequestType::POST, std::string{"POST"}},
      {RequestType::DELETE, std::string{"DELETE"}}
    };
    if (req_map.find(r_type) != req_map.end()) {
      return req_map[r_type];
    }
    else {
      return std::string{"NONE"};
    }
  }

  RequestType Request::str_to_req_type(std::string r_type) {
    std::map<std::string, RequestType> req_map {
      {std::string{"NONE"}, RequestType::NONE},
      {std::string{"GET"}, RequestType::GET},
      {std::string{"POST"}, RequestType::POST},
      {std::string{"DELETE"}, RequestType::DELETE}
    };
    if (req_map.find(r_type) != req_map.end()) {
      return req_map[r_type];
    }
    else {
      return RequestType::NONE;
    }
  };

Request::~Request() {
  curl_global_cleanup();
}