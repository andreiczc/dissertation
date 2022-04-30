#include "web3_client.h"

static constexpr auto *ETH_ADDRESS =
    "000000000000000000000000ea8964911E02471f519300eE6D1eEc11236E3165";
static constexpr auto *PRIVATE_KEY =
    "afe65d226f1d8f3706cdf02d509166e43ccd17c8a08aba9d486b4f10c1192960";
static constexpr auto *ETHERSCAN_TX = "https://ropsten.etherscan.io/tx/";
static constexpr auto *TAG          = "ETH";

Web3Client::Web3Client(const std::string &host, const std::string &path)
    : web3(host.c_str(), path.c_str())
{
  nonce = 17;
}

std::string Web3Client::callContract(const std::string &contractAddress,
                                     std::string       &data)
{
  using namespace std;

  ESP_LOGI(TAG, "Calling contract");

  Contract contract(&this->web3, "");
  contract.SetPrivateKey(PRIVATE_KEY);
  unsigned long long gasPriceVal = 100000000000ULL;
  uint32_t           gasLimitVal = 600000;
  string             destination = contractAddress;
  uint256_t          weiValue    = Util::ConvertToWei(0, 18);

  ESP_LOGI(TAG, "Sending transaction");
  string result = contract.SendTransaction(
      this->nonce++, gasPriceVal, gasLimitVal, &destination, &weiValue, &data);
  ESP_LOGI(TAG, "Transaction result: %s", result);

  string transactionHash = web3.getString(&result);
  ESP_LOGI(TAG, "Etherscan: %s%s", ETHERSCAN_TX, transactionHash.c_str());

  return transactionHash;
}