#ifndef _WEB3_CLIENT_H
#define _WEB3_CLIENT_H

#include <string>

namespace blockchain
{
std::string callContract(const std::string &contractAddress, std::string &data);
} // end namespace blockchain

#endif // _WEB3_CLIENT_H