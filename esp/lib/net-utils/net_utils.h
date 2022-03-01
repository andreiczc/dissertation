#ifndef _NET_UTILS_H
#define _NET_UTILS_H

#include <PubSubClient.h>

class NetUtils
{
public:
  static void startWifi();

  static PubSubClient initClient();
};

#endif // _NET_UTILS_H