#include "stdafx.h"
#include "proxyclt.h"
#include <Wininet.h>

BOOL SetConnectionOptions(LPSTR conn_name,LPSTR proxy_full_addr)
{
    //conn_name: active connection name. 
    //proxy_full_addr : eg "210.78.22.87:8000"
    INTERNET_PER_CONN_OPTION_LIST list;
    BOOL    bReturn;
    DWORD   dwBufSize = sizeof(list);
    // Fill out list struct.
    list.dwSize = sizeof(list);
    // NULL == LAN, otherwise connectoid name.
    list.pszConnection = conn_name;
    // Set three options.
    list.dwOptionCount = 3;
    list.pOptions = new INTERNET_PER_CONN_OPTION[3];
    // Make sure the memory was allocated.
    if(NULL == list.pOptions)
    {
        // Return FALSE if the memory wasn't allocated.
        OutputDebugString("failed to allocat memory in SetConnectionOptions()");
        return FALSE;
    }
    // Set flags.
    list.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
    list.pOptions[0].Value.dwValue = PROXY_TYPE_DIRECT |
        PROXY_TYPE_PROXY;

    // Set proxy name.
    list.pOptions[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
    list.pOptions[1].Value.pszValue = proxy_full_addr;//"http://proxy:80";

    // Set proxy override.
    list.pOptions[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
    list.pOptions[2].Value.pszValue = "local";

    // Set the options on the connection.
    bReturn = InternetSetOption(NULL,
        INTERNET_OPTION_PER_CONNECTION_OPTION, &list, dwBufSize);

    // Free the allocated memory.
    delete [] list.pOptions;
    InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
    InternetSetOption(NULL, INTERNET_OPTION_REFRESH , NULL, 0);
    return bReturn;
}
BOOL DisableConnectionProxy(LPSTR conn_name)
{
    //conn_name: active connection name. 
    INTERNET_PER_CONN_OPTION_LIST list;
    BOOL    bReturn;
    DWORD   dwBufSize = sizeof(list);
    // Fill out list struct.
    list.dwSize = sizeof(list);
    // NULL == LAN, otherwise connectoid name.
    list.pszConnection = conn_name;
    // Set three options.
    list.dwOptionCount = 1;
    list.pOptions = new INTERNET_PER_CONN_OPTION[list.dwOptionCount];
    // Make sure the memory was allocated.
    if(NULL == list.pOptions)
    {
        // Return FALSE if the memory wasn't allocated.
        OutputDebugString("failed to allocat memory in DisableConnectionProxy()");
        return FALSE;
    }
    // Set flags.
    list.pOptions[0].dwOption = INTERNET_PER_CONN_FLAGS;
    list.pOptions[0].Value.dwValue = PROXY_TYPE_DIRECT  ;
    // Set the options on the connection.
    bReturn = InternetSetOption(NULL,
        INTERNET_OPTION_PER_CONNECTION_OPTION, &list, dwBufSize);
    // Free the allocated memory.
    delete [] list.pOptions;
    InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
    InternetSetOption(NULL, INTERNET_OPTION_REFRESH , NULL, 0);
    return bReturn;
}

/*
The usage is very straightforward:

//set proxy
    const char* connection_name="Connection to adsl3";
    SetConnectionOptions(connection_name,"62.81.236.23:80");
//disable proxy 
    DisableConnectionProxy(connection_name);
    */