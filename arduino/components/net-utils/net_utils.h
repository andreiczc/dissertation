#ifndef _NET_UTILS_H
#define _NET_UTILS_H

#include <ESPAsyncWebServer.h>

class NetUtils
{
public:
  static AsyncWebServer createWebServer(int port);
  static void           startWifi();
};

#endif // _NET_UTILS_H