#include "bitcoinfees.h"

#include <array>

#include "log.h"
#include "tls_client.h"
#include "external/picojson.h"

using namespace std;

err_code BitcoinFees::handle(const uint8_t *req, size_t len, int *resp_data) {
  (void) req;
  (void) len;

  try {
    RecommendedFees fees;
    get_recommended_fees(&fees);
    *resp_data = fees.fastestFee;

    return NO_ERROR;
  }
  catch (const std::invalid_argument &e) {
    return INVALID_PARAMS;
  }
  catch (const std::exception &e) {
    LL_CRITICAL("https error: %s", e.what());
    return WEB_ERROR;
  }
  catch (...) {
    return INVALID_PARAMS;
  }
}

void BitcoinFees::get_recommended_fees(RecommendedFees* result) {
  HttpRequest httpRequest(HOSTNAME, URL, true);
  HttpsClient httpClient(httpRequest);

  HttpResponse http_response = httpClient.getResponse();

  LL_LOG("response is %s", http_response.getContent().c_str());

  if (http_response.getStatusCode() != 200) {
    LL_CRITICAL("error code: %d", http_response.getStatusCode());
    return;
  }

  picojson::value response_json;
  string err_msg = picojson::parse(response_json, http_response.getContent());
  if (!err_msg.empty()) {
    LL_CRITICAL("can't parse JSON result: %s", err_msg.c_str());
    return;
  }

  try {
    auto FASTEST_FEE = "fastestFee";
    auto HALF_HOUR_FEE = "halfHourFee";
    auto HOUR_FEE = "hourFee";

    if (!response_json.is<picojson::object>()) {
      throw runtime_error("received non-json object");
    }

    result->fastestFee = (int)(response_json.get(FASTEST_FEE).get<double>());
    result->halfHourFee = (int)(response_json.get(HALF_HOUR_FEE).get<double>());
    result->hourFee = (int)(response_json.get(HOUR_FEE).get<double>());
  }
  catch (const exception& e) {
    LL_CRITICAL("exception: %s", e.what());
  }
}