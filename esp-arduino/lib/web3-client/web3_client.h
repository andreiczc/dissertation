#ifndef _WEB3_CLIENT_H
#define _WEB3_CLIENT_H

#include "Web3.h"
#include <string>

class Web3Client
{
public:
  explicit Web3Client(const std::string &host, const std::string &path);

  std::string callContract(const std::string &contractAddress,
                           std::string       &data);

private:
  Web3     web3;
  uint32_t nonce;
};

#endif // _WEB3_CLIENT_H