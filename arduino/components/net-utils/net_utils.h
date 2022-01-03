#ifndef _NET_UTILS_H
#define _NET_UTILS_H

#include <ESPAsyncWebServer.h>

class NetUtils
{
public:
  static void createWebServer(AsyncWebServer *server);
  static void startWifi();
};

#endif // _NET_UTILS_H