#include "web3_client.h"

#include "Web3.h"
#include "esp_log.h"

static constexpr auto *TAG = "ETH";

static constexpr auto *INFURA_HOST = "ropsten.infura.io";
static constexpr auto *INFURA_PATH = "/v3/8297ac153d3948b78c03d2aff759abef";

static constexpr auto *ETH_ADDRESS =
    "000000000000000000000000ea8964911E02471f519300eE6D1eEc11236E3165";

static constexpr auto *PRIVATE_KEY =
    "afe65d226f1d8f3706cdf02d509166e43ccd17c8a08aba9d486b4f10c1192960";
static constexpr auto *ETHERSCAN_TX = "https://ropsten.etherscan.io/tx/";

static uint32_t nonce = 18;

static Web3 web3(INFURA_HOST, INFURA_PATH);

namespace blockchain {
std::string callContract(const std::string &contractAddress,
                         std::string &data) {
  using namespace std;

  ESP_LOGI(TAG, "Calling contract");

  Contract contract(&web3, "");
  contract.SetPrivateKey(PRIVATE_KEY);
  unsigned long long gasPriceVal = 100000000000ULL;
  uint32_t gasLimitVal = 600000;
  string destination = contractAddress;
  uint256_t weiValue = Util::ConvertToWei(0, 18);

  ESP_LOGI(TAG, "Sending transaction");
  string result = contract.SendTransaction(nonce++, gasPriceVal, gasLimitVal,
                                           &destination, &weiValue, &data);
  ESP_LOGI(TAG, "Transaction result: %s", result.c_str());

  string transactionHash = web3.getString(&result);
  ESP_LOGI(TAG, "Etherscan: %s%s", ETHERSCAN_TX, transactionHash.c_str());

  return transactionHash;
}
} // end namespace blockchain