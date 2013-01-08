#pragma once

#include "resource.h"

int ping(int cnt, const char* ipadr, int* ret);
BOOL SetConnectionOptions(LPSTR conn_name,LPSTR proxy_full_addr);
BOOL DisableConnectionProxy(LPSTR conn_name);

