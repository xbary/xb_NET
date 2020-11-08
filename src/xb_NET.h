#ifndef __XB_NET_H
#define __XB_NET_H

#include <xb_board.h>

#ifdef XB_ETH
#include <xb_ETH.h>
#include <Ethernet.h>

#define CLIENT_TCP EthernetClient
#define SERVER_TCP EthernetServer
#define SERVER_UDP EthernetUDP
#endif

#ifdef XB_WIFI
#include <xb_WIFI.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ssl_client.h>

#define CLIENT_TCP WiFiClient
#define SERVER_TCP WiFiServer
#define SERVER_UDP WiFiUDP
#define CLIENT_SSL_TCP WiFiClientSecure

#endif

typedef enum { isDisconnect, isConnect } TINTERNETStatus;


enum TTypeSocket { tsTCPClient, tsTCPServer, tsUDPServer,tsUDPClient, tsTCPClientSecure};
enum TSocketStatus { 
	sConnect, 
	sDisconnect, 
	sConnecting, 
	sErrorConnect, 
	sStartingServer, 
	sStartedServer, 
	sStopedServer, 
	sDestroy };

struct TSocket {
	TSocket* Next;
	TSocket* Prev;
	TTask* OwnerTask;
	uint32_t TickCreate;
	TBuf RX_Buf;
	TBuf TX_Buf;
	uint32_t TX_SendBytes;
	uint32_t TX_SendBytesLast;
	uint8_t CountZeroWrite;
	uint32_t RX_ReceiveBytes;
	uint32_t RX_ReceiveBytesLast;
	uint32_t CountNilRXData;
	CLIENT_TCP* Client;
	String RemoteHostName;
	IPAddress RemoteIP;
	uint16_t RemotePort;
	bool AutoConnect;
	SERVER_TCP* Server;
	uint16_t ServerPort;
	SERVER_UDP* ServerUDP;
	IPAddress ServerUDPIP;
	uint16_t ServerUDPPort;
	CLIENT_SSL_TCP* ClientSecure;
	const char* Root_CA;
	bool OutCreate;
	TTypeSocket Type;
	TSocketStatus Status;
	TSocketData SocketDataAction;
#ifdef XB_GUI
	int8_t Repaint;
#endif
};

enum TSocketResult {srOK,srERROR, srErrorCreateSocket,srErrorConnect, srErrorNotConnect,srNoHTTPResponse,srErrorHTTPStructure};


extern TINTERNETStatus INTERNETStatus;

TSocketResult NET_DestroySocket(TSocket** SocketHandle);
TSocketResult NET_CreateTCPClientSecure(TSocket** SocketHandle, CLIENT_SSL_TCP* Aclient_ssl, String Aremotehostname, uint16_t Aremoteport, const char* Aroot_ca);
TSocketResult NET_CreateTCPClient(TSocket** SocketHandle, CLIENT_TCP* Aclient = NULL, IPAddress Aremoteip = { 0,0,0,0 }, uint16_t Aremoteport = 0);
TSocketResult NET_CreateTCPClient(TSocket** SocketHandle, CLIENT_TCP* Aclient = NULL, String Aremotehostname=String(""), uint16_t Aremoteport = 0);
TSocketResult NET_CreateTCPServer(TSocket** SocketHandle, SERVER_TCP* Aserver, uint16_t Aport);
TSocketResult NET_StartTCPServer(TSocket* SocketHandle);
TSocketResult NET_CreateUDPClient(TSocket** SocketHandle, SERVER_UDP* Aserver, uint16_t Alocalport, IPAddress Aremoteip, uint16_t Aremoteport);
TSocketResult NET_CreateUDPServer(TSocket** SocketHandle, SERVER_UDP* Aserver, uint16_t Aport);
TSocketResult NET_StartUDPServer(TSocket* SocketHandle);
int NET_Write(TSocket* SocketHandle, uint8_t Avalue);
int NET_Write(TSocket* SocketHandle, uint8_t* Avaluebuf, int Asizebuf, char Aendch = 0);
int NET_Read(TSocket* SocketHandle, uint8_t* Avalue);
int NET_Read(TSocket* SocketHandle, uint8_t* Avalues, int Amaxreadsize);
bool NET_SendMessage_Event(TSocket* Asocket, TTypeSocketAction Atypesocketaction, TSocket* ANewClientSocket = NULL);

struct TNET_HTTP_RESPONSE
{
	uint16_t HttpCode;
	uint32_t ContentLength;
	String Cookies;
	String Headers;
	String Body;
};

TSocketResult NET_HTTP_GET(TSocket* SocketHandle, String Apath, String Aparams, char* Acookies = NULL);
TSocketResult NET_HTTP_POST(TSocket* SocketHandle, String Apath, String Aparams, char* Acookies = NULL);

int NET_flushTX(TSocket* SocketHandle);
IPAddress NET_GetLocalIPAddress();

#ifdef XB_GUI
void NET_CloseNetworkRaportWindow();
void NET_ShowNetworkRaportWindow();
#endif

extern TTaskDef XB_NET_DefTask;

#endif
