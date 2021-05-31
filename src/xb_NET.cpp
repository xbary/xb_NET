#include <xb_NET.h>

#ifdef XB_ETH
#include <xb_ETH.h>
#include <Ethernet.h>
#include <Dns.h>
#endif

#ifdef XB_WIFI
#include <xb_WIFI.h>
#include <WiFi.h>

#endif

#include <xb_board.h>
#ifdef XB_GUI
#include <xb_GUI.h>
#include <xb_GUI_Gadget.h>
TWindowClass *NET_winHandle0;
TGADGETMenu *NET_menuHandle0;
#endif

typedef enum {ifsIDLE, ifsHandleSocketStart, ifsHandleSocket, ifsHandleSocketStop, ifsHandleSocketDisconnectAllClient
} TNET_FunctionStep;
TNET_FunctionStep NET_FunctionStep = ifsIDLE;

TINTERNETStatus INTERNETStatus;

DEFLIST_VAR(TSocket, SocketList)

#pragma region KONFIGURACJE
bool NET_Start = true;
bool NET_ShowDebug = false;
bool NET_ShowRaport = true;
uint32_t NET_TickAfterWhichToCheckInternetAvailability = 5000;
uint32_t NET_SocketRXinitbufsize = 1472;
uint32_t NET_SocketTXinitbufsize = 1472;
uint32_t NET_BufGranulation = 512;
uint32_t NET_TickRememberBuffer = 2000;
IPAddress NET_InternetIPCheck(172, 217, 20, 164);
uint16_t NET_InternetPortCheck = 80;

bool NET_LoadConfig()
{
#ifdef XB_PREFERENCES
	if (board.PREFERENCES_BeginSection("NET"))
	{
		NET_Start = board.PREFERENCES_GetBool("START", NET_Start);
		NET_ShowDebug = board.PREFERENCES_GetBool("SD", NET_ShowDebug);
		NET_ShowRaport = board.PREFERENCES_GetBool("SR", NET_ShowRaport);
		NET_TickAfterWhichToCheckInternetAvailability = board.PREFERENCES_GetUINT32("TAWTCIA", NET_TickAfterWhichToCheckInternetAvailability);
		NET_SocketRXinitbufsize = board.PREFERENCES_GetUINT32("TSRIBS", NET_SocketRXinitbufsize);
		NET_SocketTXinitbufsize = board.PREFERENCES_GetUINT32("TSTIBS", NET_SocketTXinitbufsize);
		NET_TickRememberBuffer = board.PREFERENCES_GetUINT32("TRB", NET_TickRememberBuffer);
		NET_InternetIPCheck = (uint32_t)board.PREFERENCES_GetUINT32("IIC", (uint32_t)NET_InternetIPCheck);
		NET_InternetPortCheck = board.PREFERENCES_GetUINT16("IPC", NET_InternetPortCheck);
		board.PREFERENCES_EndSection();
	}
	else
	{
		return false;
	}
	return true;
#else
	return false
#endif

}

bool NET_SaveConfig()
{
#ifdef XB_PREFERENCES
	if (board.PREFERENCES_BeginSection("NET"))
	{
		board.PREFERENCES_PutBool("START", NET_Start);
		board.PREFERENCES_PutBool("SD", NET_ShowDebug);
		board.PREFERENCES_PutBool("SR", NET_ShowRaport);
		board.PREFERENCES_PutUINT32("TAWTCIA", NET_TickAfterWhichToCheckInternetAvailability);
		board.PREFERENCES_PutUINT32("TSRIBS", NET_SocketRXinitbufsize);
		board.PREFERENCES_PutUINT32("TSTIBS", NET_SocketTXinitbufsize);
		board.PREFERENCES_PutUINT32("TRB", NET_TickRememberBuffer);
		board.PREFERENCES_PutUINT32("IIC", (uint32_t)NET_InternetIPCheck);
		board.PREFERENCES_PutUINT16("IPC", NET_InternetPortCheck);
		board.PREFERENCES_EndSection();
	}
	else
	{
		return false;
	}
	return true;
#else
	return false
#endif
}

bool NET_ResetConfig()
{
#ifdef XB_PREFERENCES
	if (board.PREFERENCES_BeginSection("NET"))
	{
		board.PREFERENCES_CLEAR();
		board.PREFERENCES_EndSection();
	}
	else
	{
		return false;
	}
	return true;
#else
	return false
#endif
}

#pragma endregion
//-------------------------------------------------------------------------------------------------------------
String NET_GetString_TypeSocket(TTypeSocket Ats)
{
	switch (Ats)
	{
		GET_ENUMSTRING(tsTCPClient, 2);
		GET_ENUMSTRING(tsTCPServer, 2);
		GET_ENUMSTRING(tsUDPClient, 2);
		GET_ENUMSTRING(tsUDPServer, 2);
		GET_ENUMSTRING(tsTCPClientSecure, 2);
	}
	return "No ident Enum";
}
//-------------------------------------------------------------------------------------------------------------
String NET_GetString_StatusSocket(TSocketStatus Ass)
{
	switch (Ass)
	{
		GET_ENUMSTRING(sConnect, 1);
		GET_ENUMSTRING(sConnecting, 1);
		GET_ENUMSTRING(sErrorConnect, 1);
		GET_ENUMSTRING(sDisconnect, 1);
		GET_ENUMSTRING(sStartingServer, 1);
		GET_ENUMSTRING(sStartedServer, 1);
		GET_ENUMSTRING(sStopedServer, 1);
		GET_ENUMSTRING(sDestroy, 1);
	}
	return "No ident Enum";
}
//-------------------------------------------------------------------------------------------------------------
String NET_GetString_SocketResult(TSocketResult Asr)
{
	switch (Asr)
	{
		GET_ENUMSTRING(srOK, 2);
		GET_ENUMSTRING(srERROR,2);
		GET_ENUMSTRING(srErrorCreateSocket, 2);
		GET_ENUMSTRING(srErrorConnect, 2);
		GET_ENUMSTRING(srErrorNotConnect, 2);
		GET_ENUMSTRING(srNoHTTPResponse, 2);
		GET_ENUMSTRING(srErrorHTTPStructure, 2);
	}
	return "No ident Enum";
}
//-------------------------------------------------------------------------------------------------------------
void NET_RepaintSocket(TSocket* Asocket)
{
#ifdef XB_GUI
	if (NET_winHandle0 != NULL)
	{
		if (NET_winHandle0->Visible)
		{
			if (Asocket != NULL)
			{
				Asocket->Repaint++;
				NET_winHandle0->RepaintData();
			}
		}
	}
#endif
}
//-------------------------------------------------------------------------------------------------------------
void NET_ResizeWindowRaportSocket()
{
#ifdef XB_GUI
	if (NET_winHandle0 != NULL)
	{
		if ((SocketList_count + 10) > NET_winHandle0->Height)
		{
			NET_winHandle0->SetWindowSize(140, SocketList_count + 10);
			NET_winHandle0->Repaint();
		}
		else
		{
			NET_winHandle0->Repaint();
		}
	}
#endif
}
//-------------------------------------------------------------------------------------------------------------
TSocket* NET_CreateSocket()
{
	TSocket* socket = NULL;
	CREATE_STR_ADD_TO_LIST(SocketList, TSocket, socket);
	if (socket == NULL)
	{
		board.Log("Error create TSocket. Out of memory.", true, true, tlError);
		return NULL;
	}
	NET_ResizeWindowRaportSocket();

	socket->OwnerTask = board.CurrentTask;

	socket->RX_Buf.SectorSize = NET_BufGranulation;
	socket->TX_Buf.SectorSize = NET_BufGranulation;

	socket->RX_ReceiveBytes = 0;
	socket->RX_ReceiveBytesLast = 0;
	socket->TX_SendBytes = 0;
	socket->TX_SendBytesLast = 0;
	socket->CountNilRXData = 0;

	socket->TickCreate = SysTickCount;
	socket->Client = NULL;
	socket->ClientSecure = NULL;
	socket->Root_CA = NULL;
	socket->Server = NULL;
	socket->ServerUDP = NULL;
	socket->OutCreate = false;
	socket->AutoConnect = false;

	socket->Status = sDisconnect;

	return socket;
}
//-------------------------------------------------------------------------------------------------------------
TSocketResult NET_DestroySocket(TSocket** SocketHandle)
{
	if ((*SocketHandle) != NULL)
	{
		if ((*SocketHandle)->Client != NULL)
		{
			if ((*SocketHandle)->OutCreate == false)
			{
				(*SocketHandle)->Client->stop();
				delete((*SocketHandle)->Client);
			}
			(*SocketHandle)->Client = NULL;
		}

		if ((*SocketHandle)->ClientSecure != NULL)
		{
			if ((*SocketHandle)->OutCreate == false)
			{
				(*SocketHandle)->ClientSecure->stop();
				delete((*SocketHandle)->ClientSecure);
			}
			(*SocketHandle)->ClientSecure = NULL;
		}

		if ((*SocketHandle)->Server != NULL)
		{
			if ((*SocketHandle)->OutCreate == false)
			{
				//(*SocketHandle)->Server->stop();
				delete((*SocketHandle)->Server);
			}
			(*SocketHandle)->Server = NULL;
		}

		if ((*SocketHandle)->ServerUDP != NULL)
		{
			if ((*SocketHandle)->OutCreate == false)
			{
				TSocket* s = SocketList_last;
				while (s != NULL)
				{
					if (s->Type == tsUDPClient)
					{
						if (s != (*SocketHandle))
						{
							if (s->ServerUDP == (*SocketHandle)->ServerUDP)
							{
								if (s->OutCreate == true)
								{
									NET_SendMessage_Event(s, tsaDisconnect);
									NET_DestroySocket(&s);
									s = SocketList_last;
									continue;
								}
							}
						}
					}
					s = s->Prev;
				}
				NET_SendMessage_Event(*SocketHandle, tsaServerStop);
				delete((*SocketHandle)->ServerUDP);
			}
			(*SocketHandle)->ServerUDP = NULL;
		}

		BUFFER_Reset(&(*SocketHandle)->RX_Buf);
		BUFFER_Reset(&(*SocketHandle)->TX_Buf);

		(*SocketHandle)->RemoteHostName.~String();
		DESTROY_STR_DEL_FROM_LIST(SocketList, (*SocketHandle));
		NET_ResizeWindowRaportSocket();
	}
	return srOK;
}
//-------------------------------------------------------------------------------------------------------------
TSocketResult NET_ResetBuffersInSocket(TSocket** SocketHandle)
{
	if (SocketHandle != NULL)
	{
		if (*SocketHandle != NULL)
		{
			BUFFER_Reset(&(*SocketHandle)->RX_Buf);
			BUFFER_Reset(&(*SocketHandle)->TX_Buf);
			return srOK;
		}
	}
	return srERROR;
}
//-------------------------------------------------------------------------------------------------------------
TSocketResult NET_CreateTCPClientSecure(TSocket** SocketHandle, CLIENT_SSL_TCP* Aclient_ssl, String Aremotehostname, uint16_t Aremoteport, const char* Aroot_ca)
{
	TSocket* socket = NULL;
	if (Aclient_ssl != NULL)
	{
		socket = SocketList;
		while (socket != NULL)
		{
			if (socket->ClientSecure == Aclient_ssl) break;
			socket = socket->Next;
		}
	}
	else if (*SocketHandle != NULL)
	{
		socket = *SocketHandle;
	}
	else
	{
		socket = NULL;
	}


	if (socket == NULL)
	{
		if (*SocketHandle == NULL)
		{
			socket = NET_CreateSocket();
		}
		else
		{
			socket = *SocketHandle;
		}

	}


	if (socket == NULL)
	{
		*SocketHandle = NULL;
		return srErrorCreateSocket;
	}

	socket->Type = tsTCPClientSecure;
	if (Aclient_ssl != NULL)
	{
		socket->OutCreate = true;
		socket->ClientSecure = Aclient_ssl;
		socket->RemoteIP = socket->ClientSecure->remoteIP();
		socket->RemotePort = socket->ClientSecure->remotePort();
	}
	else
	{
		socket->OutCreate = false;

		if (socket->ClientSecure != NULL)
		{
			socket->ClientSecure->stop();
		}
		else
		{
			socket->ClientSecure = new CLIENT_SSL_TCP();
		}

		socket->RemoteHostName = Aremotehostname;
		socket->RemoteIP = (uint32_t)0;
		socket->RemotePort = Aremoteport;
#ifdef XB_ETH
		socket->Client->setConnectionTimeout(500);
#endif
#ifdef XB_WIFI
		socket->ClientSecure->setTimeout(1);
#endif
	}
	socket->Root_CA = Aroot_ca;

	*SocketHandle = socket;
	socket->Status = sConnecting;
	NET_RepaintSocket(socket);
	return srOK;

}
//-------------------------------------------------------------------------------------------------------------
TSocketResult NET_CreateTCPClient(TSocket** SocketHandle,CLIENT_TCP* Aclient, IPAddress Aremoteip, uint16_t Aremoteport)
{
	TSocket* socket = NULL;
	if (Aclient != NULL)
	{
		socket = SocketList;
		while (socket != NULL)
		{
			if (socket->Client == Aclient) break;
			socket = socket->Next;
		}
	}
	else
	{
		socket = NULL;
	}

	if (socket == NULL)
	{
		if (*SocketHandle == NULL)
		{
			socket = NET_CreateSocket();
		}
		else
		{
			socket = *SocketHandle;
		}

	}
	if (socket == NULL)
	{
		*SocketHandle = NULL;
		return srErrorCreateSocket;
	}

	socket->Type = tsTCPClient;
	if (Aclient != NULL)
	{
		socket->OutCreate = true;
		socket->Client = Aclient;
		socket->RemoteIP = socket->Client->remoteIP();
		socket->RemotePort = socket->Client->remotePort();
	}
	else
	{
		socket->OutCreate = false;
		if (socket->Client != NULL)
		{
			socket->Client->stop();
		}
		else
		{
			socket->Client = new CLIENT_TCP();
		}
		socket->RemoteHostName = "";
		socket->RemoteIP = Aremoteip;
		socket->RemotePort = Aremoteport;
#ifdef XB_ETH
		socket->Client->setConnectionTimeout(500);
#endif
#ifdef XB_WIFI
		socket->Client->setTimeout(1);
#endif
	}
	*SocketHandle = socket;
	socket->Status = sConnecting;
	NET_RepaintSocket(socket);
	return srOK;

}
//-------------------------------------------------------------------------------------------------------------
TSocketResult NET_CreateTCPClient(TSocket** SocketHandle, CLIENT_TCP* Aclient, String Aremotehostname, uint16_t Aremoteport)
{
	TSocket* socket = NULL;
	if (Aclient != NULL)
	{
		socket = SocketList;
		while (socket != NULL)
		{
			if (socket->Client == Aclient) break;
			socket = socket->Next;
		}
	}
	else
	{
		socket = NULL;
	}

	if (socket == NULL)
	{
		if (*SocketHandle == NULL)
		{
			socket = NET_CreateSocket();
		}
		else
		{
			socket = *SocketHandle;
		}

	}
	if (socket == NULL)
	{
		*SocketHandle = NULL;
		return srErrorCreateSocket;
	}

	socket->Type = tsTCPClient;
	if (Aclient != NULL)
	{
		socket->OutCreate = true;
		socket->Client = Aclient;
		socket->RemoteIP = socket->Client->remoteIP();
		socket->RemotePort = socket->Client->remotePort();
	}
	else
	{
		socket->OutCreate = false;
		if (socket->Client != NULL)
		{
			socket->Client->stop();
		}
		else
		{
			socket->Client = new CLIENT_TCP();
		}
		socket->RemoteHostName = Aremotehostname;
		socket->RemoteIP = (uint32_t)0;
		socket->RemotePort = Aremoteport;
#ifdef XB_ETH
		socket->Client->setConnectionTimeout(500);
#endif
#ifdef XB_WIFI
		socket->Client->setTimeout(1);
#endif
	}
	*SocketHandle = socket;
	socket->Status = sConnecting;
	NET_RepaintSocket(socket);
	return srOK;

}
//-------------------------------------------------------------------------------------------------------------
TSocketResult NET_CreateTCPServer(TSocket** SocketHandle, SERVER_TCP* Aserver, uint16_t Aport)
{
	TSocket* socket = NULL;
	if (Aserver != NULL)
	{
		socket = SocketList;
		while (socket != NULL)
		{
			if (socket->Server == Aserver) break;
			socket = socket->Next;
		}
	}
	else
	{
		socket = NULL;
	}

	if (socket == NULL)
	{
		if (*SocketHandle == NULL)
		{
			socket = NET_CreateSocket();
		}
		else
		{
			socket = *SocketHandle;
		}

	}
	if (socket == NULL)
	{
		*SocketHandle = NULL;
		return srErrorCreateSocket;
	}

	socket->Type = tsTCPServer;
	if (Aserver != NULL)
	{
		socket->OutCreate = true;
		socket->Server = Aserver;
		socket->ServerPort = Aport;
	}
	else
	{
		socket->OutCreate = false;
		if (socket->Server != NULL)
		{
#ifdef XB_WIFI
			socket->Server->end();
#endif
		}
		else
		{
			socket->Server = new SERVER_TCP(Aport);
			//
		}
		socket->ServerPort = Aport;
	}
	*SocketHandle = socket;
	NET_StartTCPServer(socket);
	NET_RepaintSocket(socket);
	return srOK;
}
//-------------------------------------------------------------------------------------------------------------
TSocketResult NET_StartTCPServer(TSocket* SocketHandle)
{
	if (SocketHandle != NULL)
	{
		if (SocketHandle->Type == tsTCPServer)
		{
			SocketHandle->Status = sStartingServer;
			return srOK;
		}
	}
	return srERROR;
}
//-------------------------------------------------------------------------------------------------------------
TSocketResult NET_CreateUDPClient(TSocket** SocketHandle, SERVER_UDP* Aserver, uint16_t Alocalport, IPAddress Aremoteip, uint16_t Aremoteport)
{
	TSocket* socketserverudp = NULL;
	bool socketserverudpcreate = false;
	// Czy podano klase bazow¹ obs³ugi UDP
	if (Aserver != NULL)
	{
		// Jeœli tak to wyszukanie gniazda servera UDP
		socketserverudp = SocketList;
		while (socketserverudp != NULL)
		{
			// Zapamiêtanie gniazda jeœli znaleziono pasuj¹ce
			if (socketserverudp->ServerUDP == Aserver) break;
			socketserverudp = socketserverudp->Next;
		}
	}
	else
	{
		socketserverudp = NULL;
	}

	// Sprawdzenie czy znaleziono server UDP pasuj¹cy do podanej klasy obs³ugi UDP
	if (socketserverudp == NULL)
	{
		// Utworzenie gniazda servera UDP
		socketserverudpcreate = true;
		TSocketResult sr = NET_CreateUDPServer(&socketserverudp, NULL, Alocalport);

		if (sr != srOK)
		{
			board.Log("Error create server UDP.", true, true, tlError);
			return srErrorCreateSocket;
		}
	}

	// Sprawdzenie czy jest ju¿ utworzone ju¿ gniazdo takiego klienta
	TSocket* socket = NULL;
	socket = SocketList;
	while (socket != NULL)
	{
		if (
			(socket->Type == tsUDPClient) &&
			(socket->RemoteIP == Aremoteip) && 
			(socket->RemotePort == Aremoteport) && 
			(socket->ServerUDP == socketserverudp->ServerUDP))
		{
			break;
		}
		socket = socket->Next;
	}

	// Jeœli jest takie gniazdo to nietwórz kolejnego
	if (socket != NULL)
	{
		*SocketHandle = socket;
		NET_RepaintSocket(socket);
		return srOK;
	}

	// Utworzenie gniazda klienta UDP
	socket = NET_CreateSocket();
	if (socket == NULL)
	{
		if (socketserverudpcreate)
		{
			NET_DestroySocket(&socketserverudp);
		}
		return srErrorCreateSocket;
	}
	
	socket->OwnerTask = socketserverudp->OwnerTask;
	socket->OutCreate = true;
	socket->Type = tsUDPClient;
	if (socketserverudp->Status == sStartedServer)
	{
		socket->Status = sConnect;
	}
	else
	{
		socket->Status = sConnecting;
	}
	
	socket->RemoteIP = Aremoteip;
	socket->RemotePort = Aremoteport;
	socket->ServerUDP = socketserverudp->ServerUDP;
	socket->ServerUDPIP = socketserverudp->ServerUDPIP;
	socket->ServerUDPPort = socketserverudp->ServerUDPPort;
	NET_SendMessage_Event(socketserverudp, tsaNewClientSocket, socket);
	*SocketHandle = socket;
	NET_RepaintSocket(socket);
	return srOK;
}
//-------------------------------------------------------------------------------------------------------------
TSocketResult NET_CreateUDPServer(TSocket** SocketHandle, SERVER_UDP* Aserver, uint16_t Aport)
{
	TSocket* socket = NULL;
	if (Aserver != NULL)
	{
		socket = SocketList;
		while (socket != NULL)
		{
			if (socket->ServerUDP == Aserver) break;
			socket = socket->Next;
		}
	}
	else
	{
		socket = NULL;
	}



	if (socket == NULL)
	{

		socket = SocketList;
		while (socket != NULL)
		{
			if (socket->Type == tsUDPServer)
			{
				if (socket->ServerUDP != NULL)
				{
					if (socket->ServerUDPPort == Aport)
					{
						*SocketHandle = socket;
						NET_StartUDPServer(socket);
						NET_RepaintSocket(socket);
						return srOK;
					}
				}
			}
			socket = socket->Next;
		}



		if (*SocketHandle == NULL)
		{
			socket = NET_CreateSocket();
		}
		else
		{
			socket = *SocketHandle;
		}

	}
	if (socket == NULL)
	{
		*SocketHandle = NULL;
		return srErrorCreateSocket;
	}

	socket->Type = tsUDPServer;
	if (Aserver != NULL)
	{
		socket->OutCreate = true;
		socket->ServerUDP = Aserver;
		socket->ServerUDPPort = Aport;
#ifdef XB_WIFI
		socket->ServerUDPIP = CFG_WIFI_StaticIP_IP;
#endif
#ifdef XB_ETH
		socket->ServerUDPIP = ETH_StaticIP;
#endif

	}
	else
	{
		socket->OutCreate = false;

		if (socket->ServerUDP != NULL)
		{
			socket->ServerUDP->stop();
		}
		else
		{
			socket->ServerUDP = new SERVER_UDP();
		}
		
		socket->ServerUDPPort = Aport;
#ifdef XB_WIFI
		socket->ServerUDPIP = CFG_WIFI_StaticIP_IP;
#endif
#ifdef XB_ETH
		socket->ServerUDPIP = ETH_StaticIP;
#endif
	}
	*SocketHandle = socket;
	NET_StartUDPServer(socket);
	NET_RepaintSocket(socket);
	return srOK;
}
//-------------------------------------------------------------------------------------------------------------
TSocketResult NET_StartUDPServer(TSocket* SocketHandle)
{
	if (SocketHandle != NULL)
	{
		if (SocketHandle->Type == tsUDPServer)
		{
			if (SocketHandle->Status != sStartedServer)
			{
				SocketHandle->Status = sStartingServer;
			}
			return srOK;
		}
	}
	return srERROR;
}
//-------------------------------------------------------------------------------------------------------------
int NET_Write(TSocket* SocketHandle, uint8_t Avalue)
{
	if (SocketHandle != NULL)
	{
		if (SocketHandle->Status == sDisconnect)
		{
			if (SocketHandle->AutoConnect == false)
			{
				SocketHandle->Status = sConnecting;
			}
			else return -1;
		}

		if (BUFFER_GetSizeData(&SocketHandle->TX_Buf) > (NET_SocketTXinitbufsize))
		{
			int res = NET_flushTX(SocketHandle);
			if (res < 0) return -1;
		}


		if ((SocketHandle->Status == sConnect) || (SocketHandle->Status == sConnecting))
		{
			if (BUFFER_Write_UINT8(&SocketHandle->TX_Buf, Avalue))
			{
				return 1;
			}
		}
		return 0;
	}
	return -1;
}
//-------------------------------------------------------------------------------------------------------------
int NET_Write(TSocket* SocketHandle, uint8_t* Avaluebuf, int Asizebuf,char Aendch)
{
	if (SocketHandle != NULL)
	{
		if (SocketHandle->Status == sDisconnect)
		{
			if (SocketHandle->AutoConnect == false)
			{
				SocketHandle->Status = sConnecting;
			}
			else return -1;

		}

		if (BUFFER_GetSizeData(&SocketHandle->TX_Buf) > (NET_SocketTXinitbufsize))
		{
			int res=NET_flushTX(SocketHandle);
			if (res < 0) return -1;
		}

		if ((SocketHandle->Status == sConnect) || (SocketHandle->Status == sConnecting))
		{
			if (Asizebuf == -1)
			{
				Asizebuf = 0;
				while (Avaluebuf[Asizebuf] != Aendch)
				{
					if (!BUFFER_Write_UINT8(&SocketHandle->TX_Buf, Avaluebuf[Asizebuf]))
					{
						return Asizebuf;
					}
					Asizebuf++;
				}
			}
			else
			{
				for (int i = 0; i < Asizebuf; i++)
				{
					if (!BUFFER_Write_UINT8(&SocketHandle->TX_Buf, Avaluebuf[i]))
					{
						return i;
					}
				}
			}
			return Asizebuf;
		}
		return 0;
	}
	return -1;
}
//-------------------------------------------------------------------------------------------------------------
int NET_Read(TSocket* SocketHandle, uint8_t* Avalue)
{
	if (SocketHandle != NULL)
	{
		if (BUFFER_Read_UINT8(&SocketHandle->RX_Buf, Avalue))
		{
			return 1;
		}
	}
	return 0;
}
//-------------------------------------------------------------------------------------------------------------
int NET_Read(TSocket* SocketHandle, uint8_t* Avalues, int Amaxreadsize)
{
	if (SocketHandle != NULL)
	{
		for (int i = 0; i < Amaxreadsize; i++)
		{
			if (!BUFFER_Read_UINT8(&SocketHandle->RX_Buf, &Avalues[i]))
			{
				return i;
			}
		}
		return Amaxreadsize;
	}
	return 0;
}
//-------------------------------------------------------------------------------------------------------------
bool NET_SendMessage_Event(TSocket* Asocket, TTypeSocketAction Atypesocketaction,TSocket *ANewClientSocket)
{
	TMessageBoard message; xb_memoryfill(&message, sizeof(TMessageBoard), 0);
	message.IDMessage = IM_SOCKET;
	message.fromTask = XB_NET_DefTask.Task;
	message.Data.SocketData.Socket = Asocket;
	message.Data.SocketData.NewClientSocket = ANewClientSocket;
	message.Data.SocketData.TypeSocketAction = Atypesocketaction;
	message.Data.SocketData.DestroySocket = false;
	message.Data.SocketData.AcceptSocket = true;

	Asocket->SocketDataAction = message.Data.SocketData;

	bool result = board.DoMessage(&message, true, XB_NET_DefTask.Task, Asocket->OwnerTask->TaskDef);
	if (result)
	{
		if (message.Data.SocketData.DestroySocket)
		{
			Asocket->Status = sDestroy;
		}
		return message.Data.SocketData.AcceptSocket;
	}
	return false;
}
//-------------------------------------------------------------------------------------------------------------
bool NET_SendMessage_Event_tsaReceived(TSocket* Asocket, uint32_t Areceivedlength)
{
	TMessageBoard message; xb_memoryfill(&message, sizeof(TMessageBoard), 0);
	message.IDMessage = IM_SOCKET;
	message.fromTask = XB_NET_DefTask.Task;
	message.Data.SocketData.Socket = Asocket;
	message.Data.SocketData.TypeSocketAction = tsaReceived;
	message.Data.SocketData.ReceivedLength = Areceivedlength;
	message.Data.SocketData.DestroySocket = false;
	message.Data.SocketData.AcceptSocket = true;

	Asocket->SocketDataAction = message.Data.SocketData;

	bool result = board.DoMessage(&message, true, XB_NET_DefTask.Task, Asocket->OwnerTask->TaskDef);
	if (result)
	{
		if (message.Data.SocketData.DestroySocket)
		{
			Asocket->Status = sDestroy;
		}
		return message.Data.SocketData.AcceptSocket;
	}
	return false;
}
//-------------------------------------------------------------------------------------------------------------
TSocketResult NET_HTTP_GET(TSocket* SocketHandle, String Apath, String Aparams,char *Acookies)
{
	if (SocketHandle == NULL) return srERROR;
	if (SocketHandle->Status != sConnect) return srErrorNotConnect;

	NET_Write(SocketHandle, (uint8_t*)"GET ", 4);
	NET_Write(SocketHandle, (uint8_t*)Apath.c_str(), Apath.length());
	if (Aparams.length() > 0)
	{
		NET_Write(SocketHandle, (uint8_t)'?');
		NET_Write(SocketHandle, (uint8_t*)Aparams.c_str(), Aparams.length());
	}
	NET_Write(SocketHandle, (uint8_t*)" HTTP/1.1\r\n", -1);
	

	NET_Write(SocketHandle, (uint8_t*)"Host: ", -1);	NET_Write(SocketHandle, (uint8_t*)SocketHandle->RemoteHostName.c_str(), -1);	NET_Write(SocketHandle, (uint8_t*)"\r\n", -1);
	NET_Write(SocketHandle, (uint8_t*)"User-Agent: Mozilla/5.0/XBARYOS\r\n", -1);
	NET_Write(SocketHandle, (uint8_t*)"Content-Type: application/x-www-form-urlencoded\r\n", -1);
	NET_Write(SocketHandle, (uint8_t*)"cache-control: no-cache\r\n", -1);

	if (Acookies != NULL)
	{
		NET_Write(SocketHandle, (uint8_t*)"cookie: ", -1);
		NET_Write(SocketHandle, (uint8_t*)Acookies, -1);
		NET_Write(SocketHandle, (uint8_t*)"\r\n", -1);
	}
	
	NET_Write(SocketHandle, (uint8_t*)"\r\n", -1);

	if (NET_ShowDebug)
	{

		board.Log("----- BEGIN GET -----", true, true);
		board.Log("NET_HTTP_GET", true, true);
		board.Log('\n');
		uint8_t* dataptr = BUFFER_GetReadPtr(&SocketHandle->TX_Buf);
		uint32_t datalen = BUFFER_GetSizeData(&SocketHandle->TX_Buf);
		for (uint32_t i = 0; i < datalen; i++)
		{
			if (dataptr[i] != (uint8_t)'\r')
			{
				board.Log((char)dataptr[i]);
			}
		}
		board.Log("----- END GET -----", true, true);
	}


	return srOK;
}
//-------------------------------------------------------------------------------------------------------------
TSocketResult NET_HTTP_POST(TSocket* SocketHandle, String Apath, String Aparams, char* Acookies)
{
	if (SocketHandle == NULL) return srERROR;
	if (SocketHandle->Status != sConnect) return srErrorNotConnect;

	NET_Write(SocketHandle, (uint8_t*)"POST ", -1);
	NET_Write(SocketHandle, (uint8_t*)Apath.c_str(), Apath.length());
	NET_Write(SocketHandle, (uint8_t*)" HTTP/1.1\r\n", -1);

	NET_Write(SocketHandle, (uint8_t*)"Host: ", -1);	NET_Write(SocketHandle, (uint8_t*)SocketHandle->RemoteHostName.c_str(), -1);	NET_Write(SocketHandle, (uint8_t*)"\r\n", -1);
	NET_Write(SocketHandle, (uint8_t*)"User-Agent: XBARYOS\r\n", -1);
	NET_Write(SocketHandle, (uint8_t*)"Content-Type: application/x-www-form-urlencoded\r\n", -1);
	NET_Write(SocketHandle, (uint8_t*)"cache-control: no-cache\r\n", -1);

	if (Acookies != NULL)
	{
		NET_Write(SocketHandle, (uint8_t*)"cookie: ", -1);
		NET_Write(SocketHandle, (uint8_t*)Acookies, -1);
		NET_Write(SocketHandle, (uint8_t*)"\r\n", -1);
	}

	NET_Write(SocketHandle, (uint8_t*)String("Content-Length: " + String(Aparams.length()) + "\r\n").c_str(), -1);

	NET_Write(SocketHandle, (uint8_t*)"\r\n", -1);

	NET_Write(SocketHandle, (uint8_t*)Aparams.c_str(), Aparams.length());

	if (NET_ShowDebug)
	{

		board.Log("----- BEGIN POST -----", true, true);
		board.Log("NET_HTTP_POST", true, true);
		board.Log('\n');
		uint8_t *dataptr=BUFFER_GetReadPtr(&SocketHandle->TX_Buf);
		uint32_t datalen = BUFFER_GetSizeData(&SocketHandle->TX_Buf);
		for (uint32_t i = 0; i < datalen; i++)
		{
			if (dataptr[i] != (uint8_t)'\r')
			{
				board.Log((char)dataptr[i]);
			}
		}
		board.Log("----- END POST -----", true, true);
	}

	return srOK;
}
//-------------------------------------------------------------------------------------------------------------
TSocketResult NET_HTTP_PARSE_RESPONSE(TSocket* Ash, TNET_HTTP_RESPONSE **Anhr,bool Areadheaders,bool Areadbody,bool ARxBufferflush)
{
	TSocketResult result = srOK;

	if (Ash == NULL)
	{
		result = srERROR;
		if (NET_ShowDebug)
		{
			String s = "NET_HTTP_PARSE_RESPONSE result = " + NET_GetString_SocketResult(result);
			board.Log(s.c_str(), true, true, tlError);
		}
		return result;
	}

	uint32_t length_response = BUFFER_GetSizeData(&Ash->RX_Buf);
	if (length_response<9) return srNoHTTPResponse;
	char *ptr_response = (char* )BUFFER_GetReadPtr(&Ash->RX_Buf);


	if (Anhr==NULL) 
	{
		result = srERROR;
		if (NET_ShowDebug)
		{
			String s = "NET_HTTP_PARSE_RESPONSE result = " + NET_GetString_SocketResult(result);
			board.Log(s.c_str(), true, true, tlError);
		}
		return result;
	}

	if (*Anhr == NULL)
	{
		*Anhr = (TNET_HTTP_RESPONSE*)board._malloc_psram(sizeof(TNET_HTTP_RESPONSE));
		if (*Anhr == NULL)
		{
			result = srERROR;
			if (NET_ShowDebug)
			{
				String s = "NET_HTTP_PARSE_RESPONSE result = " + NET_GetString_SocketResult(result);
				board.Log(s.c_str(), true, true, tlError);
			}
			return result;
		}

		(*Anhr)->Cookies = "";
	}
	(*Anhr)->Body = "";
	(*Anhr)->Headers = "";
	(*Anhr)->HttpCode = 0;
	(*Anhr)->ContentLength = 0;

	uint32_t indx_response = 0;
	uint32_t c = 0;

	// Sprawdzenie czy HTTP odpowiedŸ
	if (indx_response < length_response)
	{
		c = xb_memorycompare(&ptr_response[indx_response], (void *)"HTTP/1.1 ");
		if (c == 0)
		{
			result = srNoHTTPResponse;
			if (NET_ShowDebug)
			{
				String s = "NET_HTTP_PARSE_RESPONSE result = " + NET_GetString_SocketResult(result);
				board.Log(s.c_str(), true, true, tlError);
			}
			return result;
		}
		indx_response += c;
		c = 0;
	}
	else
	{
		if (ARxBufferflush) BUFFER_Readed(&Ash->RX_Buf, length_response);

		{
			result = srErrorHTTPStructure;
			if (NET_ShowDebug)
			{
				String s = "NET_HTTP_PARSE_RESPONSE result = " + NET_GetString_SocketResult(result);
				board.Log(s.c_str(), true, true, tlError);
			}
			return result;
		}
	}

		
	// Odczyt kodu HTTP odpowiedzi.
	if (indx_response < length_response)
	{
		uint32_t v = 0;
		c = StringToUINT(&ptr_response[indx_response], &v);
		if (c == 0) { if (ARxBufferflush) BUFFER_Readed(&Ash->RX_Buf, length_response); return srNoHTTPResponse; }
		(*Anhr)->HttpCode = v;
		indx_response += c;
	}
	else
	{
		if (ARxBufferflush) BUFFER_Readed(&Ash->RX_Buf, length_response);

		{
			result = srErrorHTTPStructure;
			if (NET_ShowDebug)
			{
				String s = "NET_HTTP_PARSE_RESPONSE result = " + NET_GetString_SocketResult(result);
				board.Log(s.c_str(), true, true, tlError);
			}
			return result;
		}
	}

	
	// Nastêpna linia odpowiedzi
	if (indx_response < length_response)
	{
		uint32_t p = 0;
		c = StringPos(&ptr_response[indx_response],"\r\n", &p);
		if (c == 0) { if (ARxBufferflush) BUFFER_Readed(&Ash->RX_Buf, length_response); return srNoHTTPResponse; }

		indx_response += c;
	}
	else
	{
		result = srErrorHTTPStructure;
		if (NET_ShowDebug)
		{
			String s = "NET_HTTP_PARSE_RESPONSE result = " + NET_GetString_SocketResult(result);
			board.Log(s.c_str(), true, true, tlError);
		}
		return result;
	}

	// Wyciêcie Headers
	if (indx_response < length_response)
	{
		uint32_t p = 0;
		c = StringPos(&ptr_response[indx_response], "\r\n\r\n", &p);
		if (c == 0) { if (ARxBufferflush) BUFFER_Readed(&Ash->RX_Buf, length_response); return srNoHTTPResponse; }

		(*Anhr)->Headers.reserve(p);
		for (uint32_t i = 0; i < p; i++)
		{
			(*Anhr)->Headers += (char)ptr_response[indx_response+i];
		}		

		indx_response += c;
	}
	else
	{
		if (ARxBufferflush) BUFFER_Readed(&Ash->RX_Buf, length_response);

		{
			result = srErrorHTTPStructure;
			if (NET_ShowDebug)
			{
				String s = "NET_HTTP_PARSE_RESPONSE result = " + NET_GetString_SocketResult(result);
				board.Log(s.c_str(), true, true, tlError);
			}
			return result;
		}
	}

	// Wyszukanie w nag³ówku wielkoœci body
	int p = (*Anhr)->Headers.indexOf("Content-Length: ");
	if (p != -1)
	{
		p += sizeof("Content-Length: ");
		StringToUINT(&(*Anhr)->Headers.c_str()[p-1], &(*Anhr)->ContentLength);

		// Wyciêcie Body
		if (indx_response < length_response)
		{
			(*Anhr)->Body.reserve((*Anhr)->ContentLength);
			for (uint32_t i = 0; i < (*Anhr)->ContentLength; i++)
			{
				(*Anhr)->Body+= (char)ptr_response[indx_response + i];
			}
			indx_response += (*Anhr)->ContentLength;
		}
		else
		{
			if (ARxBufferflush) BUFFER_Readed(&Ash->RX_Buf, length_response);
			
			{
				result = srErrorHTTPStructure;
				if (NET_ShowDebug)
				{
					String s = "NET_HTTP_PARSE_RESPONSE result = " + NET_GetString_SocketResult(result);
					board.Log(s.c_str(), true, true, tlError);
				}
				return result;
			}
		}
	}
	else
	{
		// Wyszukanie w nag³ówku czy jest podzia³ body na chunki
		int p = (*Anhr)->Headers.indexOf("Transfer-Encoding: chunked");
		if (p != -1)
		{
			uint32_t l = 0;
			uint32_t siz = 0;
			uint32_t c = 0;

			c = StringHEXToUINT(&ptr_response[indx_response], &l);
			while (l > 0)
			{
				if (c == 0)
				{
					if (ARxBufferflush) BUFFER_Readed(&Ash->RX_Buf, length_response);

					{
						result = srErrorHTTPStructure;
						if (NET_ShowDebug)
						{
							String s = "NET_HTTP_PARSE_RESPONSE result = " + NET_GetString_SocketResult(result);
							board.Log(s.c_str(), true, true, tlError);
						}
						return result;
					}
				}
				indx_response += c+2;

				for (uint32_t i = 0; i < l;i++)
				{
					if (indx_response+i < length_response)
					{
						(*Anhr)->Body += (char)ptr_response[indx_response + i];
					}
					else
					{
						if (ARxBufferflush) BUFFER_Readed(&Ash->RX_Buf, length_response);

						{
							result = srErrorHTTPStructure;
							if (NET_ShowDebug)
							{
								String s = "NET_HTTP_PARSE_RESPONSE result = " + NET_GetString_SocketResult(result);
								board.Log(s.c_str(), true, true, tlError);
							}
							return result;
						}
					}
				}
				indx_response += l;
				c = StringHEXToUINT(&ptr_response[indx_response], &l);
			}
			(*Anhr)->ContentLength = (*Anhr)->Body.length();

		}
		else
		{
			if (ARxBufferflush) BUFFER_Readed(&Ash->RX_Buf, length_response);

			{
				result = srErrorHTTPStructure;
				if (NET_ShowDebug)
				{
					String s = "NET_HTTP_PARSE_RESPONSE result = " + NET_GetString_SocketResult(result);
					board.Log(s.c_str(), true, true, tlError);
				}
				return result;
			}
		}
	}

	// Zwolnienie bufora RX
	if (ARxBufferflush) BUFFER_Readed(&Ash->RX_Buf, length_response);


	if (NET_ShowDebug)
	{
		String s = "NET_HTTP_PARSE_RESPONSE result = " + NET_GetString_SocketResult(result);
		board.Log(s.c_str(), true, true, tlInfo);


		board.Log("HTTP RESPONSE CODE: ", true, true, tlInfo);
		board.Log(String((*Anhr)->HttpCode).c_str(), false, false, tlWarn);
		board.Log("HTTP HEADERS:\n", true, true, tlInfo);
		s = (*Anhr)->Headers;
		s.replace("\r\n", "\n");
		board.Log(s.c_str(), false, false, tlWarn);

		board.Log("HTTP RESPONSE ContentLength: ", true, true, tlInfo);
		board.Log(String((*Anhr)->ContentLength).c_str(), false, false, tlWarn);

		board.Log("HTTP BODY:\n", true, true, tlInfo);
		s = (*Anhr)->Body;
		s.replace("\r\n", "\n");
		board.Log(s.c_str(), false, false, tlWarn);
	}

	return result;
}
//-------------------------------------------------------------------------------------------------------------
void NET_HTTP_RESPONSE_Free(TNET_HTTP_RESPONSE** Anhr)
{
	if (Anhr != NULL)
	{
		if (*Anhr != NULL)
		{
			(*Anhr)->Body.~String();
			(*Anhr)->Cookies.~String();
			(*Anhr)->Headers.~String();

			board.free(*Anhr);
			*Anhr = NULL;
		}
	}
}
//-------------------------------------------------------------------------------------------------------------

TSocket* NET_CurrentSocket; // Aktualne obs³ugiwane gniazdo
uint32_t LastRWtoNetSocket; // Tick ostatniego prawid³owego odczytu/zapisu gniazda w sieci lokalnej
uint32_t LastRWtoInternetSocket; // Tick ostatniego prawid³owego odczytu/zapisu gniazda w sieci internet

//==========================================================================================
bool NET_CheckIPisLAN(IPAddress Aip)
{
#ifdef XB_WIFI
	if ((Aip & CFG_WIFI_MASK_IP) == (CFG_WIFI_StaticIP_IP & CFG_WIFI_MASK_IP))
#endif
#ifdef XB_ETH
		if ((Aip & ETH_Mask) == (ETH_StaticIP & ETH_Mask))
#endif
		{
		return true;
	}
#ifdef XB_WIFI
	if ((Aip & CFG_WIFI_MASK_IP) == IPAddress(192, 168, 4, 0))
	{
		return true;
	}
#endif
	return false;
}
//==========================================================================================
void INTERNET_Connect()
{
	if (INTERNETStatus == isConnect) return;
	INTERNETStatus = isConnect;
	TMessageBoard mb; xb_memoryfill(&mb, sizeof(TMessageBoard), 0);
	mb.IDMessage = IM_INTERNET_CONNECT;
	board.DoMessageOnAllTask(&mb, true, doFORWARD);
}
//==========================================================================================
void INTERNET_Disconnect()
{
	if (INTERNETStatus == isDisconnect) return;
	INTERNETStatus = isDisconnect;
	TMessageBoard mb; xb_memoryfill(&mb, sizeof(TMessageBoard), 0);
	mb.IDMessage = IM_INTERNET_DISCONNECT;
	board.DoMessageOnAllTask(&mb, true, doBACKWARD);
}
//==========================================================================================
bool lastNetAvaliable;
bool CheckNetAvaliable()
{
	bool result = false;
	if (SysTickCount - LastRWtoNetSocket > 5000)
	{
#ifdef XB_WIFI
		if (WiFi.status() == WL_CONNECTED)
#endif
#ifdef XB_ETH
		if ((Ethernet.hardwareStatus() != EthernetNoHardware) && (Ethernet.linkStatus() == LinkON))
#endif
		{
			CLIENT_TCP client;
#ifdef XB_WIFI
			if (true)//(client.connect(CFG_WIFI_GATEWAY_IP, 80, 1200))
#endif
#ifdef XB_ETH
			client.setConnectionTimeout(500);
			int v = client.connect(ETH_Gateway, 80);
			if (v == 0)
			{
				delay(500);
				v = client.connect(ETH_Gateway, 80);
			}
			if (NET_ShowDebug) board.Log(String("."+String(v)).c_str());
			if (v!=0)
#endif
			{
				client.stop();
				if (lastNetAvaliable == false)
				{
					lastNetAvaliable = true;
					if (NET_ShowDebug) board.Log("Check Net avaliable..", true, true);
					if (NET_ShowDebug) board.Log(".Connected");
				}
				NET_Connect();
				result = true;
			}
			else
			{
				if (lastNetAvaliable == true)
				{
					lastNetAvaliable = false;
					if (NET_ShowDebug) board.Log("Check Net avaliable..", true, true);
					if (NET_ShowDebug) board.Log(".Disconnected");
				}

				
				NET_Disconnect();
				INTERNET_Disconnect();
				result = false;
			}
		} 
		else
		{
			NET_Disconnect();
			INTERNET_Disconnect();
		}

		LastRWtoNetSocket = SysTickCount;
	}
	else result = (NETStatus==nsConnect);

	return result;
}
//==========================================================================================
bool lastInternetAvaliable;
bool CheckInternetAvaliable()
{
	bool result = false;
	if (SysTickCount - LastRWtoInternetSocket > NET_TickAfterWhichToCheckInternetAvailability)
	{
#ifdef XB_WIFI
		if (WiFi.status() == WL_CONNECTED)
#endif
#ifdef XB_ETH
			if (NETStatus == nsConnect)
#endif
			{
			CLIENT_TCP client;
			
#ifdef XB_WIFI
			bool res=false;
			if (INTERNETStatus == isConnect)
			{
				res = client.connect(NET_InternetIPCheck, NET_InternetPortCheck, 1500);
			}
			else
			{
					res = client.connect(NET_InternetIPCheck, NET_InternetPortCheck, 250);
			}
			if (res)
#endif
#ifdef XB_ETH
			if (client.connect(NET_InternetIPCheck, NET_InternetPortCheck))
#endif
			{
				client.stop();

				if (NET_ShowDebug)
				{
					if (!lastInternetAvaliable)
					{
						board.Log("Check internet avaliable...Connected", true, true);
						lastInternetAvaliable = true;
					}
				}
				INTERNET_Connect();
				result = true;
			}
			else
			{
				if (NET_ShowDebug)
				{
					if (lastInternetAvaliable)
					{
						board.Log("Check internet avaliable...Disconnected", true, true);
						lastInternetAvaliable = false;
					}
				}
				INTERNET_Disconnect();
			}
		}
		else
		{
			INTERNET_Disconnect();

		}
		LastRWtoInternetSocket = SysTickCount;
	}
	else result = (INTERNETStatus==isConnect);

	return result;
}
//==========================================================================================
void SetLastTickRWNet(TSocket* SocketHandle)
{
	if (SocketHandle != NULL)
	{
		if (SocketHandle->Type == tsTCPClient)
		{
			if (NET_CheckIPisLAN(SocketHandle->RemoteIP))
			{
				LastRWtoNetSocket = SysTickCount;
			}
			else
			{
				LastRWtoInternetSocket = SysTickCount;
			}
		}
		else if (SocketHandle->Type == tsTCPClientSecure)
		{
			LastRWtoInternetSocket = SysTickCount;
		}
	}
	return;
}
//==========================================================================================
bool SetCurrentSocket_Disconnect()
{
	if (NET_CurrentSocket != NULL)
	{
		BUFFER_Reset(&NET_CurrentSocket->TX_Buf);
		BUFFER_Reset(&NET_CurrentSocket->RX_Buf);
		if (NET_CurrentSocket->Type == tsTCPClientSecure)
		{
			NET_CurrentSocket->Status = sDisconnect;
			NET_SendMessage_Event(NET_CurrentSocket, tsaDisconnect);
		}
		else if (NET_CurrentSocket->Type == tsTCPClient)
		{
			NET_CurrentSocket->Status = sDisconnect;
			NET_SendMessage_Event(NET_CurrentSocket, tsaDisconnect);
		}
		else if (NET_CurrentSocket->Type == tsTCPServer)
		{
			NET_CurrentSocket->Status = sStopedServer;
			NET_SendMessage_Event(NET_CurrentSocket, tsaServerStop);
		}
		else if (NET_CurrentSocket->Type == tsUDPServer)
		{
			NET_CurrentSocket->Status = sStopedServer;
			NET_SendMessage_Event(NET_CurrentSocket, tsaServerStop);
		}
		else if (NET_CurrentSocket->Type == tsUDPClient)
		{
			NET_CurrentSocket->Status = sDisconnect;
			NET_SendMessage_Event(NET_CurrentSocket, tsaDisconnect);
		}

		
		if (NET_CurrentSocket == NULL) return true;
		if (NET_CurrentSocket->Client != NULL)
		{
			NET_CurrentSocket->Client->stop();
		}
		if (NET_CurrentSocket->ClientSecure != NULL)
		{
			NET_CurrentSocket->ClientSecure->stop();
		}
		NET_RepaintSocket(NET_CurrentSocket);
	}
	return false;
}
//-------------------------------------------------------------------------------------------------------------
int NET_flushTX(TSocket* SocketHandle)
{
	if ((SocketHandle->Type == tsTCPClient) || (SocketHandle->Type == tsTCPClientSecure))
	{

		uint32_t TXSize = BUFFER_GetSizeData(&SocketHandle->TX_Buf);
		int32_t TXSended = 0;

		if (TXSize > 0)
		{
#ifdef XB_ETH
			int availableForWriteCount = SocketHandle->Client->availableForWrite();
#endif
#ifdef XB_WIFI
			int availableForWriteCount = 1024; // SocketHandle->Client->availableForWrite();
#endif
			if (availableForWriteCount > 0)
			{
				uint8_t* buf = NULL;

				if (availableForWriteCount >= TXSize)
				{
					buf = BUFFER_GetReadPtr(&SocketHandle->TX_Buf);
					if (SocketHandle->Type == tsTCPClient) TXSended = SocketHandle->Client->write(buf, TXSize);
					if (SocketHandle->Type == tsTCPClientSecure) TXSended = SocketHandle->ClientSecure->write(buf, TXSize);
					delay(10);
					if (TXSended < 0)
					{
						if (SetCurrentSocket_Disconnect()) return -1;
					}
					else
					{
						if (SocketHandle->CountZeroWrite >= 3)
						{
							if (SetCurrentSocket_Disconnect()) return -1;
						}

						SetLastTickRWNet(SocketHandle);
						if (TXSended == 0)
						{
							SocketHandle->CountZeroWrite++;
						}
						else
						{
							SocketHandle->CountZeroWrite = 0;
							board.Blink_TX();
						}

						BUFFER_Readed(&SocketHandle->TX_Buf, TXSended);
						SocketHandle->TX_SendBytes += TXSended;
					}
				}
				else
				{
					buf = BUFFER_GetReadPtr(&SocketHandle->TX_Buf);
					if (SocketHandle->Type == tsTCPClient) TXSended = SocketHandle->Client->write(buf, availableForWriteCount);
					if (SocketHandle->Type == tsTCPClientSecure) TXSended = SocketHandle->ClientSecure->write(buf, availableForWriteCount);
					delay(10);
					if (TXSended < 0)
					{
						if (SetCurrentSocket_Disconnect()) return -1;
					}
					else
					{
						if (SocketHandle->CountZeroWrite >= 3)
						{
							if (SetCurrentSocket_Disconnect()) return -1;
						}

						if (TXSended == 0)
						{
							SocketHandle->CountZeroWrite++;
						}
						else
						{
							SocketHandle->CountZeroWrite = 0;
							board.Blink_TX();
						}

						BUFFER_Readed(&SocketHandle->TX_Buf, TXSended);
						SocketHandle->TX_SendBytes += TXSended;
						SetLastTickRWNet(SocketHandle);
					}
				}
				NET_RepaintSocket(SocketHandle);
			}
		}
		return TXSended;
	}
	else
	{
		return -1;
	}
}
//-------------------------------------------------------------------------------------------------------------
IPAddress NET_GetLocalIPAddress()
{
#ifdef XB_WIFI
	return WiFi.localIP();
#endif
#ifdef XB_ETH
#endif
	return IPAddress(0,0,0,0);
}


void XB_NET_Setup()
{
	board.Log("Init.", true, true);
	board.LoadConfiguration();
	INTERNETStatus = isDisconnect;
	NETStatus = nsDisconnect;
	board.Log("OK");
	if (NET_ShowRaport)
	{
		NET_ShowNetworkRaportWindow();
	}
}

uint32_t XB_NET_DoLoop()
{
	switch (NET_FunctionStep)
	{
	case ifsIDLE:
	{
		if (NET_Start)
		{
			NET_FunctionStep = ifsHandleSocketStart;
			NET_CurrentSocket = NULL;
		}
		break;
	}
	case ifsHandleSocketStart:
	{
		if (!NET_Start)
		{
			NET_FunctionStep = ifsIDLE;
			break;
		}

		if (NETStatus == nsConnect)
		{
			if (NET_ShowDebug) board.Log("Engine socket start.", true, true, tlInfo);
			NET_FunctionStep = ifsHandleSocket;
			LastRWtoInternetSocket = SysTickCount;
			LastRWtoNetSocket = SysTickCount;
#ifdef XB_WIFI
#ifdef XB__OTA
			board.Log("OTA Init", true, true);
			board.Log('.');
			ArduinoOTA.setPort(CFG_WIFI_PORTOTA);
			board.Log('.');
			ArduinoOTA.setHostname(board.DeviceName.c_str());
			board.Log('.');
			if (CFG_WIFI_PSWOTA != "")
			{
				ArduinoOTA.setPassword(CFG_WIFI_PSWOTA.c_str());
			}
			board.Log('.');


			//#if !defined(_VMICRO_INTELLISENSE)
			ArduinoOTA.onStart([](void) {
				board.SendMessage_OTAUpdateStarted();
				String type;
				if (ArduinoOTA.getCommand() == U_FLASH)
					type = "sketch";
				else
					type = "filesystem";

				board.Log(String("\n\n[WIFI] Start updating " + type).c_str());
				});

			board.Log('.');
			ArduinoOTA.onEnd([](void) {
				board.Log("\n\rEnd");
				});

			board.Log('.');
			ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
				board.Blink_RX();
				board.Blink_TX();
				board.Blink_Life();
				board.Log('.');
				});

			board.Log('.');
			ArduinoOTA.onError([](ota_error_t error) {
				Serial.printf(("Error[%u]: "), error);
				if (error == OTA_AUTH_ERROR) Serial.println(("Auth Failed"));
				else if (error == OTA_BEGIN_ERROR) Serial.println(("Begin Failed"));
				else if (error == OTA_CONNECT_ERROR) Serial.println(("Connect Failed"));
				else if (error == OTA_RECEIVE_ERROR) Serial.println(("Receive Failed"));
				else if (error == OTA_END_ERROR) Serial.println(("End Failed"));
				});
			//#endif
			board.Log('.');
			ArduinoOTA.begin();
			board.Log('.');
			board.Log(("OK"));
#endif
#endif
		}
		
		CheckNetAvaliable();
		break;
	}
	case ifsHandleSocket:
	{
		// Jeœli wy³aczono algorytm to roz³¹cz wszystkie gniazda
		if (!NET_Start)
		{
			NET_FunctionStep = ifsHandleSocketDisconnectAllClient;
			break;
		}

		// Jeœli utracono sieæ to roz³¹cz wszystkie gniazda
		if (NETStatus == nsDisconnect)
		{
			NET_FunctionStep = ifsHandleSocketDisconnectAllClient;
			break;
		}

		// --- Sprawdzenie dostêpnoœci sieci
		CheckNetAvaliable();
		//if (!CheckNetAvaliable()) break;
		CheckInternetAvaliable();


		// Jeœli aktualne gniazdo jest NULL to za³aduj poczatek listy
		if (NET_CurrentSocket == NULL)
		{
			NET_CurrentSocket = SocketList;
			// Jeœli niema gniazd to opuszczenie pêtli
			if (NET_CurrentSocket == NULL) break;
		}
		else
		{
			// Za³adowanie kolejnego gniazda
			NET_CurrentSocket = NET_CurrentSocket->Next;
			// Jeœli niema nastêpnego gniazda to za³adowanie pocz¹tku listy
			if (NET_CurrentSocket == NULL) break;
		}

		// Sprawdze czy nie anulowaæ buforów starszych jak defaultowo 5 sekund w danym gniazdku
		if (
			(BUFFER_Handle(&NET_CurrentSocket->RX_Buf, NET_TickRememberBuffer)) ||
			(BUFFER_Handle(&NET_CurrentSocket->TX_Buf, NET_TickRememberBuffer))
			)
		{
			NET_RepaintSocket(NET_CurrentSocket);
		}

		// Sprawdzenie czy nie usuwaæ aktualnego gniazda jeœli ma status do usuniêcia
		if (NET_CurrentSocket->Status == sDestroy)
		{
			NET_DestroySocket(&NET_CurrentSocket);
			break;
		}

		// Sprawdzenie czy gniazdo jest klientem
		if ((NET_CurrentSocket->Type == tsTCPClient) || (NET_CurrentSocket->Type == tsTCPClientSecure) || (NET_CurrentSocket->Type == tsUDPClient))
		{
			// Sprawdzenie czy aktuane gniazdo odnosi siê do sieci internet		
			if (!NET_CheckIPisLAN(NET_CurrentSocket->RemoteIP))
			{
				// Jeœli nie ma po³¹czenia internetowego to pomiñ obs³uge gniazda 
				if (!CheckInternetAvaliable()) break;
			}
		}
		// ---


		switch (NET_CurrentSocket->Type)
		{
		case tsTCPClientSecure:
		{
			if (NET_CurrentSocket->ClientSecure != NULL)
			{
				if (NET_CurrentSocket->Status == sConnecting)
				{
					if (NET_CurrentSocket->ClientSecure->connected())
					{
						NET_CurrentSocket->Status = sConnect;
						NET_RepaintSocket(NET_CurrentSocket);
						NET_SendMessage_Event(NET_CurrentSocket, tsaConnect);
						if (NET_CurrentSocket == NULL) break;
					}
					else
					{
						//BUFFER_Reset(&NET_CurrentSocket->RX_Buf);
						//BUFFER_Reset(&NET_CurrentSocket->TX_Buf);

						if (NET_CurrentSocket->RemoteIP != (uint32_t)0)
						{
#ifdef XB_WIFI
							NET_CurrentSocket->ClientSecure->setCACert(NET_CurrentSocket->Root_CA);
							int result = NET_CurrentSocket->ClientSecure->connect(NET_CurrentSocket->RemoteIP, NET_CurrentSocket->RemotePort, 2000);
							if (!result)
#endif
#ifdef XB_ETH
							//NET_CurrentSocket->ClientSecure->setCACert(NET_CurrentSocket->Root_CA);
							int result = NET_CurrentSocket->ClientSecure->connect(NET_CurrentSocket->RemoteIP, NET_CurrentSocket->RemotePort);
							if (result == 0)
#endif
							{
								NET_CurrentSocket->Status = sErrorConnect;
								NET_RepaintSocket(NET_CurrentSocket);
								NET_SendMessage_Event(NET_CurrentSocket, tsaConnectError);
								if (NET_CurrentSocket == NULL) break;
							}
							else
							{
								NET_CurrentSocket->Status = sConnect;
								NET_RepaintSocket(NET_CurrentSocket);
								NET_SendMessage_Event(NET_CurrentSocket, tsaConnect);
								if (NET_CurrentSocket == NULL) break;
							}
						}
						else
						{
#ifdef XB_WIFI
							NET_CurrentSocket->ClientSecure->setCACert(NET_CurrentSocket->Root_CA);
							int result = NET_CurrentSocket->ClientSecure->connect(NET_CurrentSocket->RemoteHostName.c_str(), NET_CurrentSocket->RemotePort, 2000);
							if (!result)
#endif
#ifdef XB_ETH
						//	NET_CurrentSocket->ClientSecure->setCACert(NET_CurrentSocket->Root_CA);
							int result = NET_CurrentSocket->ClientSecure->connect(NET_CurrentSocket->RemoteIP, NET_CurrentSocket->RemotePort);
							if (result == 0)
#endif
							{
								NET_CurrentSocket->Status = sErrorConnect;
								NET_RepaintSocket(NET_CurrentSocket);
								NET_SendMessage_Event(NET_CurrentSocket, tsaConnectError);
								if (NET_CurrentSocket == NULL) break;
							}
							else
							{
								NET_CurrentSocket->Status = sConnect;
								NET_RepaintSocket(NET_CurrentSocket);
								NET_SendMessage_Event(NET_CurrentSocket, tsaConnect);
								if (NET_CurrentSocket == NULL) break;
							}


						}
					}
					break;
				}

				if (NET_CurrentSocket->Status == sConnect)
				{
					if (NET_CurrentSocket->ClientSecure->connected())
					{
						uint32_t TXSize = BUFFER_GetSizeData(&NET_CurrentSocket->TX_Buf);
						int32_t TXSended = 0;

						if (TXSize > 0)
						{
#ifdef XB_ETH
							int availableForWriteCount = NET_CurrentSocket->ClientSecure->availableForWrite();
#endif
#ifdef XB_WIFI
							int availableForWriteCount = 1024; // NET_CurrentSocket->ClientSecure->availableForWrite();
#endif
							if (availableForWriteCount > 0)
							{
								uint8_t* buf = NULL;

								if (availableForWriteCount >= TXSize)
								{
									buf = BUFFER_GetReadPtr(&NET_CurrentSocket->TX_Buf);
									//board.Log((const char*)buf);
									TXSended = NET_CurrentSocket->ClientSecure->write(buf, TXSize);
									if (TXSended < 0)
									{
										if (SetCurrentSocket_Disconnect()) break;
									}
									else
									{
										if (NET_CurrentSocket->CountZeroWrite >= 3)
										{
											board.Log("Error after third attempt to write to socket.", true, true, tlError);
											if (SetCurrentSocket_Disconnect()) break;
										}

										SetLastTickRWNet(NET_CurrentSocket);
										if (TXSended == 0)
										{
											NET_CurrentSocket->CountZeroWrite++;
										}
										else
										{
											NET_CurrentSocket->CountZeroWrite = 0;
											board.Blink_TX();
										}

										BUFFER_Readed(&NET_CurrentSocket->TX_Buf, TXSended);
										NET_CurrentSocket->TX_SendBytes += TXSended;
									}
								}
								else
								{
									buf = BUFFER_GetReadPtr(&NET_CurrentSocket->TX_Buf);
									TXSended = NET_CurrentSocket->ClientSecure->write(buf, availableForWriteCount);
									if (TXSended < 0)
									{
										if (SetCurrentSocket_Disconnect()) break;
									}
									else
									{
										if (NET_CurrentSocket->CountZeroWrite >= 3)
										{
											board.Log("Error after third attempt to write to socket.", true, true, tlError);
											if (SetCurrentSocket_Disconnect()) break;
										}

										if (TXSended == 0)
										{
											NET_CurrentSocket->CountZeroWrite++;
										}
										else
										{
											NET_CurrentSocket->CountZeroWrite = 0;
											board.Blink_TX();
										}

										BUFFER_Readed(&NET_CurrentSocket->TX_Buf, TXSended);
										NET_CurrentSocket->TX_SendBytes += TXSended;
										SetLastTickRWNet(NET_CurrentSocket);
									}
								}
								NET_RepaintSocket(NET_CurrentSocket);
							}
						}
						else
						{
							if (NET_CurrentSocket->TX_Buf.IndxR == NET_CurrentSocket->TX_Buf.IndxW)
							{
								if (NET_CurrentSocket->TX_SendBytes != NET_CurrentSocket->TX_SendBytesLast)
								{
									NET_CurrentSocket->TX_SendBytesLast = NET_CurrentSocket->TX_SendBytes;
									NET_SendMessage_Event(NET_CurrentSocket, tsaSended);
									if (NET_CurrentSocket == NULL) break;
									NET_RepaintSocket(NET_CurrentSocket);
								}
							}
						}

						// -------------------------------------------------------------------------------------------------
						int availableForReadCount = NET_CurrentSocket->ClientSecure->available();
						if (availableForReadCount > 0)
						{
							board.Blink_RX();
							int v = 0;
							for (uint32_t i = 0; i < availableForReadCount; i++)
							{

								v = NET_CurrentSocket->ClientSecure->read();
								if (v > -1)
								{
									uint8_t v8 = v;
									BUFFER_Write_UINT8(&NET_CurrentSocket->RX_Buf, v8);
									NET_CurrentSocket->RX_ReceiveBytes++;
									NET_RepaintSocket(NET_CurrentSocket);
									SetLastTickRWNet(NET_CurrentSocket);
								}
								else
								{
									break;
								}
							}
							NET_CurrentSocket->CountNilRXData = 0;

						}
						else
						{
							if ((NET_CurrentSocket->RX_ReceiveBytes != NET_CurrentSocket->RX_ReceiveBytesLast) && (NET_CurrentSocket->CountNilRXData > SocketList_count))
							{
								NET_CurrentSocket->CountNilRXData = 0;
								SetLastTickRWNet(NET_CurrentSocket);
								NET_CurrentSocket->RX_ReceiveBytesLast = NET_CurrentSocket->RX_ReceiveBytes;
								NET_SendMessage_Event(NET_CurrentSocket, tsaReceived);
								if (NET_CurrentSocket == NULL) break;
								NET_RepaintSocket(NET_CurrentSocket);
							}
							else
							{
								if (NET_CurrentSocket->RX_ReceiveBytes != NET_CurrentSocket->RX_ReceiveBytesLast)
								{
									NET_CurrentSocket->CountNilRXData++;
								}
								else
								{
									NET_CurrentSocket->CountNilRXData = 0;
								}
							}



							// -------------------------------------------------------------------------------------------------
						}
						// <>



						// ------------------------------------------------------------------------------------------------
					}
					else
					{
						if ((NET_CurrentSocket->RX_ReceiveBytes != NET_CurrentSocket->RX_ReceiveBytesLast))
						{
							NET_CurrentSocket->CountNilRXData = 0;
							SetLastTickRWNet(NET_CurrentSocket);
							NET_CurrentSocket->RX_ReceiveBytesLast = NET_CurrentSocket->RX_ReceiveBytes;
							NET_SendMessage_Event(NET_CurrentSocket, tsaReceived);
							if (NET_CurrentSocket == NULL) break;
							NET_RepaintSocket(NET_CurrentSocket);
							break;
						}

						if (NET_CurrentSocket->Status != sDisconnect)
						{
							NET_CurrentSocket->Status = sDisconnect;
							BUFFER_Reset(&NET_CurrentSocket->TX_Buf);
							BUFFER_Reset(&NET_CurrentSocket->RX_Buf);
							NET_SendMessage_Event(NET_CurrentSocket, tsaDisconnect);
							if (NET_CurrentSocket == NULL) break;
							if (NET_CurrentSocket->ClientSecure!=NULL ) NET_CurrentSocket->ClientSecure->stop();
							NET_RepaintSocket(NET_CurrentSocket);
						}
					}
				}
				else if (NET_CurrentSocket->Status == sDisconnect)
				{
					if (NET_CurrentSocket->ClientSecure->connected())
					{
						NET_CurrentSocket->Status = sConnect;
						NET_SendMessage_Event(NET_CurrentSocket, tsaConnect);
						if (NET_CurrentSocket == NULL) break;
						NET_RepaintSocket(NET_CurrentSocket);
					}
					else
					{
						if (NET_CurrentSocket->AutoConnect)
						{
							NET_CurrentSocket->Status = sConnecting;
							NET_RepaintSocket(NET_CurrentSocket);
						}
					}
				}
			}
			break;
		}
		case tsTCPServer:
		{
			if (NET_CurrentSocket->Server != NULL)
			{
				if (NET_CurrentSocket->Status == sStartingServer)
				{
					NET_CurrentSocket->Server->begin();
					NET_CurrentSocket->Status = sStartedServer;
					NET_RepaintSocket(NET_CurrentSocket);
					NET_SendMessage_Event(NET_CurrentSocket, tsaServerStart);
					if (NET_CurrentSocket == NULL) break;
				}

				if (NET_CurrentSocket->Status == sStartedServer)
				{

#ifdef XB_WIFI
					if (NET_CurrentSocket->Server->hasClient())
					{
						CLIENT_TCP client = NET_CurrentSocket->Server->accept();

						if (NET_ShowDebug) board.Log("Request new client from server...", true, true, tlInfo);

						TSocket* socketclient = NET_CreateSocket();
						if (socketclient != NULL)
						{
							socketclient->OwnerTask = NET_CurrentSocket->OwnerTask;
							socketclient->OutCreate = false;
							socketclient->Type = tsTCPClient;
							socketclient->Client = new CLIENT_TCP(client);
							socketclient->Client->setTimeout(2);
							socketclient->Client->setNoDelay(true);

							socketclient->Status = sConnecting;
							socketclient->RemoteIP = socketclient->Client->remoteIP();
							socketclient->RemotePort = socketclient->Client->remotePort();

							NET_SendMessage_Event(NET_CurrentSocket, tsaNewClientSocket, socketclient);
							if (NET_CurrentSocket == NULL) break;
						}
				}
#endif
#ifdef XB_ETH
					CLIENT_TCP client = NET_CurrentSocket->Server->accept();
					if (client)
					{
							
						if (NET_ShowDebug) board.Log("Request new client from server...", true, true, tlInfo);

						TSocket* socketclient = NET_CreateSocket();
						if (socketclient != NULL)
						{
							socketclient->OwnerTask = NET_CurrentSocket->OwnerTask;
							socketclient->OutCreate = false;
							socketclient->Type = tsTCPClient;
							socketclient->Client = new CLIENT_TCP(client.getSocketNumber());
							socketclient->Client->setConnectionTimeout(1280);
							socketclient->Client->setTimeout(1280);
							socketclient->Status = sConnecting;
							socketclient->RemoteIP = socketclient->Client->remoteIP();
							socketclient->RemotePort = socketclient->Client->remotePort();

							NET_SendMessage_Event(NET_CurrentSocket, tsaNewClientSocket, socketclient);
							if (NET_CurrentSocket == NULL) break;
						}
					}
#endif
				}
				else if (NET_CurrentSocket->Status == sStopedServer)
				{
					NET_CurrentSocket->Status = sStartingServer;
				}
			}
			break;
		}
		case tsTCPClient:
		{
			if (NET_CurrentSocket->Client != NULL)
			{
				if (NET_CurrentSocket->Status == sConnecting)
				{
					if (NET_CurrentSocket->Client->connected())
					{
						NET_CurrentSocket->Status = sConnect;
						NET_RepaintSocket(NET_CurrentSocket);
						NET_SendMessage_Event(NET_CurrentSocket, tsaConnect);
						if (NET_CurrentSocket == NULL) break;
					}
					else
					{
						//BUFFER_Reset(&NET_CurrentSocket->RX_Buf);
						//BUFFER_Reset(&NET_CurrentSocket->TX_Buf);
		
						if (NET_CurrentSocket->RemoteIP != (uint32_t)0)
						{
#ifdef XB_WIFI
							int result = NET_CurrentSocket->Client->connect(NET_CurrentSocket->RemoteIP, NET_CurrentSocket->RemotePort, 2000);
							if (result != 1)
#endif
#ifdef XB_ETH
							int result = NET_CurrentSocket->Client->connect(NET_CurrentSocket->RemoteIP, NET_CurrentSocket->RemotePort);
							if (result == 0)
#endif
							{
								NET_CurrentSocket->Status = sErrorConnect;
								NET_RepaintSocket(NET_CurrentSocket);
								NET_SendMessage_Event(NET_CurrentSocket, tsaConnectError);
								if (NET_CurrentSocket == NULL) break;
							}
							else
							{
								NET_CurrentSocket->Status = sConnect;
								NET_RepaintSocket(NET_CurrentSocket);
								NET_SendMessage_Event(NET_CurrentSocket, tsaConnect);
								if (NET_CurrentSocket == NULL) break;
							}
						}
						else 
						{
#ifdef XB_ETH
							DNSClient dns;
							dns.begin(Ethernet.dnsServerIP());
							int res = dns.getHostByName(NET_CurrentSocket->RemoteHostName.c_str(), NET_CurrentSocket->RemoteIP);
							if (res < 1)
#endif
#ifdef XB_WIFI
							int res = WiFi.hostByName(NET_CurrentSocket->RemoteHostName.c_str(), NET_CurrentSocket->RemoteIP);
							if (res != 1)
#endif
							{
								board.Log("Error get IP by HostName...", true, true, tlError);
								NET_CurrentSocket->Status = sErrorConnect;
								NET_RepaintSocket(NET_CurrentSocket);
								NET_SendMessage_Event(NET_CurrentSocket, tsaConnectError);
								if (NET_CurrentSocket == NULL) break;
							}
						}
					}
				}
				
				if (NET_CurrentSocket->Status == sConnect)
				{
					if (NET_CurrentSocket->Client->connected())
					{
						uint32_t TXSize = BUFFER_GetSizeData(&NET_CurrentSocket->TX_Buf);
						int32_t TXSended = 0;

						if (TXSize > 0)
						{
#ifdef XB_ETH
							int availableForWriteCount = NET_CurrentSocket->Client->availableForWrite();
#endif
#ifdef XB_WIFI
							int availableForWriteCount = 1024; // NET_CurrentSocket->Client->availableForWrite();
#endif
							if (availableForWriteCount > 0)
							{
								uint8_t* buf = NULL;

								if (availableForWriteCount >= TXSize)
								{
									buf = BUFFER_GetReadPtr(&NET_CurrentSocket->TX_Buf);
									TXSended = NET_CurrentSocket->Client->write(buf, TXSize);
									if (TXSended < 0)
									{
										if (SetCurrentSocket_Disconnect()) break;
									}
									else
									{
										if (NET_CurrentSocket->CountZeroWrite >= 3)
										{
											board.Log("Error after third attempt to write to socket.", true, true, tlError);
											if (SetCurrentSocket_Disconnect()) break;
										}

										SetLastTickRWNet(NET_CurrentSocket);
										if (TXSended == 0)
										{
											NET_CurrentSocket->CountZeroWrite++;
										}
										else
										{
											NET_CurrentSocket->CountZeroWrite = 0;
											board.Blink_TX();
										}

										BUFFER_Readed(&NET_CurrentSocket->TX_Buf, TXSended);
										NET_CurrentSocket->TX_SendBytes += TXSended;
									}
								}
								else
								{
									buf = BUFFER_GetReadPtr(&NET_CurrentSocket->TX_Buf);
									TXSended = NET_CurrentSocket->Client->write(buf, availableForWriteCount);
									if (TXSended < 0)
									{
										if (SetCurrentSocket_Disconnect()) break;
									}
									else
									{
										if (NET_CurrentSocket->CountZeroWrite >= 3)
										{
											board.Log("Error after third attempt to write to socket.", true, true, tlError);
											if (SetCurrentSocket_Disconnect()) break;
										}

										if (TXSended == 0)
										{
											NET_CurrentSocket->CountZeroWrite++;
										}
										else
										{
											NET_CurrentSocket->CountZeroWrite = 0;
											board.Blink_TX();
										}

										BUFFER_Readed(&NET_CurrentSocket->TX_Buf, TXSended);
										NET_CurrentSocket->TX_SendBytes += TXSended;
										SetLastTickRWNet(NET_CurrentSocket);
									}
								}
								NET_RepaintSocket(NET_CurrentSocket);
							}
						}
						else
						{
							if (NET_CurrentSocket->TX_Buf.IndxR == NET_CurrentSocket->TX_Buf.IndxW)
							{
								if (NET_CurrentSocket->TX_SendBytes != NET_CurrentSocket->TX_SendBytesLast)
								{
									NET_CurrentSocket->TX_SendBytesLast = NET_CurrentSocket->TX_SendBytes;
									NET_SendMessage_Event(NET_CurrentSocket, tsaSended);
									if (NET_CurrentSocket == NULL) break;
									NET_RepaintSocket(NET_CurrentSocket);
								}
							}
						}

						// -------------------------------------------------------------------------------------------------
						int availableForReadCount = NET_CurrentSocket->Client->available();
						if (availableForReadCount > 0)
						{
							board.Blink_RX();
							int v = 0;
							for (uint32_t i = 0; i < availableForReadCount; i++)
							{

								v = NET_CurrentSocket->Client->read();
								if (v > -1)
								{
									uint8_t v8 = v;
									BUFFER_Write_UINT8(&NET_CurrentSocket->RX_Buf, v8);
									NET_CurrentSocket->RX_ReceiveBytes++;
									NET_RepaintSocket(NET_CurrentSocket);
									SetLastTickRWNet(NET_CurrentSocket);
								}
								else
								{
									break;
								}
							}
							NET_CurrentSocket->CountNilRXData = 0;
							
						}
						else
						{
							if ((NET_CurrentSocket->RX_ReceiveBytes != NET_CurrentSocket->RX_ReceiveBytesLast) && (NET_CurrentSocket->CountNilRXData > SocketList_count))
							{
								NET_CurrentSocket->CountNilRXData = 0;
								SetLastTickRWNet(NET_CurrentSocket);
								NET_CurrentSocket->RX_ReceiveBytesLast = NET_CurrentSocket->RX_ReceiveBytes;
								NET_SendMessage_Event(NET_CurrentSocket, tsaReceived);
								if (NET_CurrentSocket == NULL) break;
								NET_RepaintSocket(NET_CurrentSocket);
							}
							else
							{
								if (NET_CurrentSocket->RX_ReceiveBytes != NET_CurrentSocket->RX_ReceiveBytesLast)
								{
									NET_CurrentSocket->CountNilRXData++;
								}
								else
								{
									NET_CurrentSocket->CountNilRXData = 0;
								}
							}



							// -------------------------------------------------------------------------------------------------
						}
						// <>



						// ------------------------------------------------------------------------------------------------
					}
					else
					{
						if ((NET_CurrentSocket->RX_ReceiveBytes != NET_CurrentSocket->RX_ReceiveBytesLast))
						{
							NET_CurrentSocket->CountNilRXData = 0;
							SetLastTickRWNet(NET_CurrentSocket);
							NET_CurrentSocket->RX_ReceiveBytesLast = NET_CurrentSocket->RX_ReceiveBytes;
							NET_SendMessage_Event(NET_CurrentSocket, tsaReceived);
							if (NET_CurrentSocket == NULL) break;
							NET_RepaintSocket(NET_CurrentSocket);
							break;
						}

						if (NET_CurrentSocket->Status != sDisconnect)
						{
							NET_CurrentSocket->Status = sDisconnect;
							BUFFER_Reset(&NET_CurrentSocket->TX_Buf);
							BUFFER_Reset(&NET_CurrentSocket->RX_Buf);
							NET_SendMessage_Event(NET_CurrentSocket, tsaDisconnect);
							if (NET_CurrentSocket == NULL) break;
							if (NET_CurrentSocket->Client!=NULL) NET_CurrentSocket->Client->stop();
							NET_RepaintSocket(NET_CurrentSocket);
						}
					}
				}
				else if (NET_CurrentSocket->Status == sDisconnect)
				{
					if (NET_CurrentSocket->Client->connected())
					{
						NET_CurrentSocket->Status = sConnect;
						NET_SendMessage_Event(NET_CurrentSocket, tsaConnect);
						if (NET_CurrentSocket == NULL) break;
						NET_RepaintSocket(NET_CurrentSocket);
					}
					else
					{
						if (NET_CurrentSocket->AutoConnect)
						{
							NET_CurrentSocket->Status = sConnecting;
							NET_RepaintSocket(NET_CurrentSocket);
						}
					}
				}
			}
			break;
		}
		case tsUDPServer:
		{
			if (NET_CurrentSocket->ServerUDP != NULL)
			{
				if (NET_CurrentSocket->Status == sStartingServer)
				{
#ifdef XB_WIFI
					NET_CurrentSocket->ServerUDP->begin(NET_CurrentSocket->ServerUDPIP,NET_CurrentSocket->ServerUDPPort);
#endif
#ifdef XB_ETH
					NET_CurrentSocket->ServerUDP->begin(NET_CurrentSocket->ServerUDPPort);
#endif
					NET_CurrentSocket->Status = sStartedServer;
					NET_RepaintSocket(NET_CurrentSocket);
					NET_SendMessage_Event(NET_CurrentSocket, tsaServerStart);
					if (NET_CurrentSocket == NULL) break;

					TSocket* s = SocketList;
					while (s != NULL)
					{
						if (s->Type == tsUDPClient)
						{
							if (s->ServerUDP == NET_CurrentSocket->ServerUDP)
							{
								if (s->Status == sConnecting)
								{
									s->Status = sConnect;
									NET_SendMessage_Event(s, tsaConnect);
									if (NET_CurrentSocket == NULL) break;
								}
							}
						}
						s = s->Next;
					}
					if (NET_CurrentSocket == NULL) break;
				}

				if (NET_CurrentSocket->Status == sStartedServer)
				{
					// czy nadchodzi pakiet
					int sizenewpacket = NET_CurrentSocket->ServerUDP->parsePacket();
					if (sizenewpacket > 0)
					{
						// zapamiêtanie  Ÿród³a z któego pochodzi pakiet
						IPAddress remoteIP = NET_CurrentSocket->ServerUDP->remoteIP();
						uint16_t remotePort = NET_CurrentSocket->ServerUDP->remotePort();

						// Sprawdzenie czy jest ju¿ gniazdo do tego Ÿród³a
						TSocket* cs = SocketList;
						TSocket* s = NULL;
						while (cs != NULL)
						{
							if (cs->Type == tsUDPClient)
							{
								if ((remoteIP == cs->RemoteIP) && (remotePort == cs->RemotePort))
								{
									s = cs;
									break;
								}
							}
							cs = cs->Next;
						}

						// Jeœli nie znaleziono gniazda to tworzenie nowego
						if (s == NULL)
						{
							if (NET_ShowDebug) board.Log("Request new client udp from server udp...", true, true, tlInfo);
							TSocketResult sr = NET_CreateUDPClient(&s, NET_CurrentSocket->ServerUDP, NET_CurrentSocket->ServerUDPPort, remoteIP, remotePort);
							if (sr == srOK)
							{

								if (!NET_SendMessage_Event(NET_CurrentSocket, tsaNewClientSocket,s))
								{
									board.Log("Ignore new client udp...", true, true, tlWarn);

									int availableForReadCount = NET_CurrentSocket->ServerUDP->available();
									for (uint32_t i = 0; i < availableForReadCount; i++)
									{
										int v = NET_CurrentSocket->ServerUDP->read();
										if (v < 0)
										{
											break;
										}
									}
									NET_DestroySocket(&s);
									break;
								}

								s->Status = sConnect;
								NET_SendMessage_Event(s, tsaConnect);
								if (NET_CurrentSocket == NULL) break;
							}
							else
							{
								board.Log("Error add new client udp...", true, true, tlError);
								break;
							}
						}

						// Jeœli jest gniazdo klienta podejmowanie odczytu pakietu
						if (s!=NULL)
						{
							int v;
							int availableForReadCount = NET_CurrentSocket->ServerUDP->available();
							// Sprawdzenie czy wielkoœæ jest dopuszczalna przez system
							if ((availableForReadCount > 0) && (availableForReadCount < 1024))
							{
								for (uint32_t i = 0; i < availableForReadCount; i++)
								{

									v = NET_CurrentSocket->ServerUDP->read();
									if (v > -1)
									{
										uint8_t v8 = v;
										BUFFER_Write_UINT8(&s->RX_Buf, v8);
										s->RX_ReceiveBytes++;
										NET_RepaintSocket(s);
										SetLastTickRWNet(s);
									}
									else
									{
										break;
									}
								}
								s->RX_ReceiveBytesLast = s->RX_ReceiveBytes;

								NET_SendMessage_Event_tsaReceived(s, s->RX_Buf.IndxW- s->RX_Buf.IndxR);
								if (s == NULL) break;
								NET_RepaintSocket(s);
							}
							else
							{
								// Ignorowanie nieprawid³owej wielkoœci pakietów
								for (uint32_t i = 0; i < availableForReadCount; i++)
								{
									int v = NET_CurrentSocket->ServerUDP->read();
									if (v < 0)
									{
										break;
									}
								}
								board.Log("Error packet is size over 1024 bytes...", true, true, tlError);
							}
						}
						else
						{
							// Jeœli nie zosta³o utworzone gniazdo z powodu b³êdu
							// Ignorowanie przychodz¹cego pakietu
							int availableForReadCount = NET_CurrentSocket->ServerUDP->available();
							if (availableForReadCount > 0)
							{
								for (uint32_t i = 0; i < availableForReadCount; i++)
								{
									int v = NET_CurrentSocket->ServerUDP->read();
									if (v < 0)
									{
										break;
									}
								}
							}
						}
					}
				}
				else if (NET_CurrentSocket->Status == sStopedServer)
				{
					NET_CurrentSocket->Status = sStartingServer;
				}
			}
			break;
		}
		case tsUDPClient:
		{
			if (NET_CurrentSocket->Status == sConnecting)
			{
				if (NET_CurrentSocket->ServerUDP != NULL)
				{
					NET_RepaintSocket(NET_CurrentSocket);
					NET_SendMessage_Event(NET_CurrentSocket, tsaConnect);
					if (NET_CurrentSocket == NULL) break;
				}
				else
				{
					NET_CurrentSocket->Status = sDisconnect;
				}
			}
			else if (NET_CurrentSocket->Status == sConnect)
			{
				if (NET_CurrentSocket->ServerUDP != NULL)
				{
					uint32_t sizetosend = BUFFER_GetSizeData(&NET_CurrentSocket->TX_Buf);
					if (sizetosend > 0)
					{
						uint8_t* buf = BUFFER_GetReadPtr(&NET_CurrentSocket->TX_Buf);
						size_t sizesend = 0;
						if (NET_CurrentSocket->ServerUDP->beginPacket(NET_CurrentSocket->RemoteIP, NET_CurrentSocket->RemotePort) != 0)
						{
							NET_CurrentSocket->ServerUDP->setTimeout(10);
							sizesend = NET_CurrentSocket->ServerUDP->write(buf, sizetosend);
							NET_CurrentSocket->ServerUDP->endPacket();
						} 
						else
						{
							board.Log("UDP Begin packet error...", true, true, tlError);
						}

						if (sizesend > 0)
						{
							NET_CurrentSocket->TX_SendBytes += sizesend;
							NET_SendMessage_Event(NET_CurrentSocket, tsaSended);
							if (NET_CurrentSocket == NULL) break;
							BUFFER_Readed(&NET_CurrentSocket->TX_Buf, sizesend);
						}
					}
				}
			}
			break;
		}
		default: break;
		}

		break;
	}
	case ifsHandleSocketDisconnectAllClient:
	{
		TSocket* s = SocketList;
		while (s != NULL)
		{
			if ((s->Type == tsTCPClient) || (s->Type == tsTCPClientSecure))
			{
				if (s->Status == sConnect)
				{
					if (s->Client != NULL)
					{
						s->Client->stop();
					}
					if (s->ClientSecure != NULL)
					{
						s->ClientSecure->stop();
					}
					s->Status = sDisconnect;
					NET_SendMessage_Event(s, tsaDisconnect);
					s = SocketList;
				}
			}
			if (s!=NULL) s = s->Next;
		}

		NET_FunctionStep = ifsHandleSocketStop;
		break;
	}

	case ifsHandleSocketStop:
	{
		if (NET_ShowDebug) board.Log("Engine socket stop.", true, true, tlInfo);
		NET_FunctionStep = ifsIDLE;
		break;
	}
	default:
	{
		board.Log("Internal error in loop()...", true, true, tlError);
		NET_FunctionStep = ifsIDLE; 
		return 10000;
	}
	}
	return 0;
}

#ifdef XB_GUI

void NET_ShowNetworkRaportWindow()
{
	NET_winHandle0 = GUI_WindowCreate(&XB_NET_DefTask, 0);
}

void NET_CloseNetworkRaportWindow()
{
	if (NET_winHandle0 != NULL)
	{
		NET_winHandle0->Close();
	}
}

void NET_SocketClientsSecureDrawCaption(TWindowClass* Awh, Ty& Ay)
{
	Awh->GoToXY(0, Ay);
	String str = NET_GetString_TypeSocket(tsTCPClientSecure); Awh->PutStr(str.c_str(), 140, '-', taLeft);
	Ay++;
	Awh->GoToXY(0, Ay);
	Awh->PutStr("OWNER TASK ", 16, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("STATUS", 10, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("ALL RX", 10, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("ALL TX", 10, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("Remote HOST", 24, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("Remote Port", 11, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("Local Port", 10, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("MLTX", 8, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("MLRX", 8, ' ', taLeft); Awh->PutChar('|');
	Ay++;
}

void NET_SocketClientSecureDraw(TWindowClass* Awh, TSocket* As, Ty& Ay)
{

	String str; str.reserve(64);
	Awh->GoToXY(0, Ay);
	if (As != NULL)
	{
		if (As->ClientSecure != NULL)
		{
			str = ""; board.SendMessage_GetTaskNameString(As->OwnerTask, &str); Awh->PutStr(str.c_str(), 16, ' ', taLeft); Awh->PutChar('|');
			str = NET_GetString_StatusSocket(As->Status); Awh->PutStr(str.c_str(), 10, ' ', taLeft); Awh->PutChar('|');
			Awh->PutStr(String(As->RX_ReceiveBytes).c_str(), 10, ' ', taRight); Awh->PutChar('|');
			Awh->PutStr(String(As->TX_SendBytes).c_str(), 10, ' ', taRight); Awh->PutChar('|');
			Awh->PutStr(As->RemoteHostName.c_str(), 24, ' ', taCentre); Awh->PutChar('|');
			Awh->PutStr(String(As->RemotePort).c_str(), 11, ' ', taCentre); Awh->PutChar('>');
			if (As->ClientSecure != NULL)
			{
				Awh->PutStr(String(As->ClientSecure->localPort()).c_str(), 10, ' ', taCentre); Awh->PutChar('|');
			}
			else
			{
				Awh->PutStr("x", 10, ' ', taCentre); Awh->PutChar('|');
			}
			Awh->PutStr(String(As->TX_Buf.MaxLength).c_str(), 8, ' ', taCentre); Awh->PutChar('|');
			Awh->PutStr(String(As->RX_Buf.MaxLength).c_str(), 8, ' ', taCentre); Awh->PutChar('|');
		}
		As->Repaint = 0;
	}
	Ay++;

}

void NET_SocketClientsDrawCaption(TWindowClass *Awh,Ty &Ay)
{
	Awh->GoToXY(0, Ay);
	String str = NET_GetString_TypeSocket(tsTCPClient); Awh->PutStr(str.c_str(), 140, '-', taLeft);
	Ay++;
	Awh->GoToXY(0, Ay);
	Awh->PutStr("OWNER TASK ", 16, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("STATUS", 10, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("ALL RX", 10, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("ALL TX", 10, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("Remote IP", 16, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("Remote Port", 11, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("Local Port", 10, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("MLTX", 8, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("MLRX", 8, ' ', taLeft); Awh->PutChar('|');
	Ay++;
}

void NET_SocketClientDraw(TWindowClass* Awh, TSocket* As, Ty &Ay)
{
	String str; str.reserve(64);
	Awh->GoToXY(0, Ay);
	if (As != NULL)
	{
		if (As->Client != NULL)
		{
			str = ""; board.SendMessage_GetTaskNameString(As->OwnerTask, &str); Awh->PutStr(str.c_str(), 16, ' ', taLeft); Awh->PutChar('|');
			str = NET_GetString_StatusSocket(As->Status); Awh->PutStr(str.c_str(), 10, ' ', taLeft); Awh->PutChar('|');
			Awh->PutStr(String(As->RX_ReceiveBytes).c_str(), 10, ' ', taRight); Awh->PutChar('|');
			Awh->PutStr(String(As->TX_SendBytes).c_str(), 10, ' ', taRight); Awh->PutChar('|');
			Awh->PutStr(As->RemoteIP.toString().c_str(), 16, ' ', taCentre); Awh->PutChar('|');
			Awh->PutStr(String(As->RemotePort).c_str(), 11, ' ', taCentre); Awh->PutChar('>');
			if (As->Client != NULL)
			{
				Awh->PutStr(String(As->Client->localPort()).c_str(), 10, ' ', taCentre); Awh->PutChar('|');
			}
			else
			{
				Awh->PutStr("x", 10, ' ', taCentre); Awh->PutChar('|');
			}
			Awh->PutStr(String(As->TX_Buf.MaxLength).c_str(), 8, ' ', taCentre); Awh->PutChar('|');
			Awh->PutStr(String(As->RX_Buf.MaxLength).c_str(), 8, ' ', taCentre); Awh->PutChar('|');
		}
		As->Repaint = 0;
	}
	Ay++;
}

void NET_SocketServersDrawCaption(TWindowClass* Awh, Ty& Ay)
{
	Awh->GoToXY(0, Ay);
	String str = NET_GetString_TypeSocket(tsTCPServer); Awh->PutStr(str.c_str(), 140, '-', taLeft);
	Ay++;
	Awh->GoToXY(0, Ay);
	Awh->PutStr("OWNER TASK ", 16, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("STATUS", 16, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("Server Port", 11, ' ', taLeft); Awh->PutChar('|');
	Ay++;
}

void NET_SocketServerDraw(TWindowClass* Awh, TSocket* As, Ty& Ay)
{
	String str; str.reserve(64);
	Awh->GoToXY(0, Ay);
	str = ""; board.SendMessage_GetTaskNameString(As->OwnerTask, &str); Awh->PutStr(str.c_str(), 16, ' ', taLeft); Awh->PutChar('|');
	str = NET_GetString_StatusSocket(As->Status); Awh->PutStr(str.c_str(), 16, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr(String(As->ServerPort).c_str(), 11, ' ', taCentre); Awh->PutChar('|');
	Ay++;
	As->Repaint = 0;
}

void NET_SocketServersUDPDrawCaption(TWindowClass* Awh, Ty& Ay)
{
	Awh->GoToXY(0, Ay);
	String str = NET_GetString_TypeSocket(tsUDPServer); Awh->PutStr(str.c_str(), 140, '-', taLeft);
	Ay++;
	Awh->GoToXY(0, Ay);
	Awh->PutStr("OWNER TASK ", 16, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("STATUS", 16, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("Server Port", 11, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("Server IP", 16, ' ', taLeft); Awh->PutChar('|');
	Ay++;
}

void NET_SocketServerUDPDraw(TWindowClass* Awh, TSocket* As, Ty& Ay)
{
	String str; str.reserve(64);
	Awh->GoToXY(0, Ay);
	str = ""; board.SendMessage_GetTaskNameString(As->OwnerTask, &str); Awh->PutStr(str.c_str(), 16, ' ', taLeft); Awh->PutChar('|');
	str = NET_GetString_StatusSocket(As->Status); Awh->PutStr(str.c_str(), 16, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr(String(As->ServerUDPPort).c_str(), 11, ' ', taCentre); Awh->PutChar('|');
	Awh->PutStr(As->ServerUDPIP.toString().c_str(), 16, ' ', taCentre); Awh->PutChar('|');
	Ay++;
	As->Repaint = 0;
}

void NET_SocketClientsUDPDrawCaption(TWindowClass* Awh, Ty& Ay)
{
	Awh->GoToXY(0, Ay);
	String str = NET_GetString_TypeSocket(tsUDPClient); Awh->PutStr(str.c_str(), 140, '-', taLeft);
	Ay++;
	Awh->GoToXY(0, Ay);
	Awh->PutStr("OWNER TASK ", 16, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("STATUS", 10, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("ALL RX", 10, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("ALL TX", 10, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("Remote IP", 16, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("Remote Port", 11, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("Local Port", 10, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("Local IP", 16, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("MLTX", 8, ' ', taLeft); Awh->PutChar('|');
	Awh->PutStr("MLRX", 8, ' ', taLeft); Awh->PutChar('|');

	Ay++;
}

void NET_SocketClientUDPDraw(TWindowClass* Awh, TSocket* As, Ty& Ay)
{
	String str; str.reserve(64);
	Awh->GoToXY(0, Ay);
	if (As != NULL)
	{
		if (As->ServerUDP != NULL)
		{
			str = ""; board.SendMessage_GetTaskNameString(As->OwnerTask, &str); Awh->PutStr(str.c_str(), 16, ' ', taLeft); Awh->PutChar('|');
			str = NET_GetString_StatusSocket(As->Status); Awh->PutStr(str.c_str(), 10, ' ', taLeft); Awh->PutChar('|');
			Awh->PutStr(String(As->RX_ReceiveBytes).c_str(), 10, ' ', taRight); Awh->PutChar('|');
			Awh->PutStr(String(As->TX_SendBytes).c_str(), 10, ' ', taRight); Awh->PutChar('|');
			Awh->PutStr(As->RemoteIP.toString().c_str(), 16, ' ', taCentre); Awh->PutChar('|');
			Awh->PutStr(String(As->RemotePort).c_str(), 11, ' ', taCentre); Awh->PutChar('>');
			Awh->PutStr(String(As->ServerUDPPort).c_str(), 10, ' ', taCentre); Awh->PutChar('|');
			Awh->PutStr(As->ServerUDPIP.toString().c_str(), 16, ' ', taCentre); Awh->PutChar('|');
			Awh->PutStr(String(As->TX_Buf.MaxLength).c_str(), 8, ' ', taCentre); Awh->PutChar('|');
			Awh->PutStr(String(As->RX_Buf.MaxLength).c_str(), 8, ' ', taCentre); Awh->PutChar('|');

		}
		As->Repaint = 0;
	}
	Ay++;
}

#endif


TSocket* testudpserver;
TSocket* testudpclient;
uint32_t testSendFrameID;
TSocket* testtcpclientsecure;

const char* test_root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFFjCCAv6gAwIBAgIRAJErCErPDBinU/bWLiWnX1owDQYJKoZIhvcNAQELBQAw\n" \
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n" \
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjAwOTA0MDAwMDAw\n" \
"WhcNMjUwOTE1MTYwMDAwWjAyMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg\n" \
"RW5jcnlwdDELMAkGA1UEAxMCUjMwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\n" \
"AoIBAQC7AhUozPaglNMPEuyNVZLD+ILxmaZ6QoinXSaqtSu5xUyxr45r+XXIo9cP\n" \
"R5QUVTVXjJ6oojkZ9YI8QqlObvU7wy7bjcCwXPNZOOftz2nwWgsbvsCUJCWH+jdx\n" \
"sxPnHKzhm+/b5DtFUkWWqcFTzjTIUu61ru2P3mBw4qVUq7ZtDpelQDRrK9O8Zutm\n" \
"NHz6a4uPVymZ+DAXXbpyb/uBxa3Shlg9F8fnCbvxK/eG3MHacV3URuPMrSXBiLxg\n" \
"Z3Vms/EY96Jc5lP/Ooi2R6X/ExjqmAl3P51T+c8B5fWmcBcUr2Ok/5mzk53cU6cG\n" \
"/kiFHaFpriV1uxPMUgP17VGhi9sVAgMBAAGjggEIMIIBBDAOBgNVHQ8BAf8EBAMC\n" \
"AYYwHQYDVR0lBBYwFAYIKwYBBQUHAwIGCCsGAQUFBwMBMBIGA1UdEwEB/wQIMAYB\n" \
"Af8CAQAwHQYDVR0OBBYEFBQusxe3WFbLrlAJQOYfr52LFMLGMB8GA1UdIwQYMBaA\n" \
"FHm0WeZ7tuXkAXOACIjIGlj26ZtuMDIGCCsGAQUFBwEBBCYwJDAiBggrBgEFBQcw\n" \
"AoYWaHR0cDovL3gxLmkubGVuY3Iub3JnLzAnBgNVHR8EIDAeMBygGqAYhhZodHRw\n" \
"Oi8veDEuYy5sZW5jci5vcmcvMCIGA1UdIAQbMBkwCAYGZ4EMAQIBMA0GCysGAQQB\n" \
"gt8TAQEBMA0GCSqGSIb3DQEBCwUAA4ICAQCFyk5HPqP3hUSFvNVneLKYY611TR6W\n" \
"PTNlclQtgaDqw+34IL9fzLdwALduO/ZelN7kIJ+m74uyA+eitRY8kc607TkC53wl\n" \
"ikfmZW4/RvTZ8M6UK+5UzhK8jCdLuMGYL6KvzXGRSgi3yLgjewQtCPkIVz6D2QQz\n" \
"CkcheAmCJ8MqyJu5zlzyZMjAvnnAT45tRAxekrsu94sQ4egdRCnbWSDtY7kh+BIm\n" \
"lJNXoB1lBMEKIq4QDUOXoRgffuDghje1WrG9ML+Hbisq/yFOGwXD9RiX8F6sw6W4\n" \
"avAuvDszue5L3sz85K+EC4Y/wFVDNvZo4TYXao6Z0f+lQKc0t8DQYzk1OXVu8rp2\n" \
"yJMC6alLbBfODALZvYH7n7do1AZls4I9d1P4jnkDrQoxB3UqQ9hVl3LEKQ73xF1O\n" \
"yK5GhDDX8oVfGKF5u+decIsH4YaTw7mP3GFxJSqv3+0lUFJoi5Lc5da149p90Ids\n" \
"hCExroL1+7mryIkXPeFM5TgO9r0rvZaBFOvV2z0gp35Z0+L4WPlbuEjN/lxPFin+\n" \
"HlUjr8gRsI3qfJOQFy/9rKIJR0Y/8Omwt/8oTWgy1mdeHmmjk7j1nYsvC9JSQ6Zv\n" \
"MldlTTKB3zhThV1+XWYp6rjd5JW1zbVWEkLNxE7GJThEUG3szgBVGP7pSWTUTsqX\n" \
"nLRbwHOoq7hHwg==\n" \
"-----END CERTIFICATE-----\n";

/*
const char* test_root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIEkjCCA3qgAwIBAgIQCgFBQgAAAVOFc2oLheynCDANBgkqhkiG9w0BAQsFADA/\n" \
"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n" \
"DkRTVCBSb290IENBIFgzMB4XDTE2MDMxNzE2NDA0NloXDTIxMDMxNzE2NDA0Nlow\n" \
"SjELMAkGA1UEBhMCVVMxFjAUBgNVBAoTDUxldCdzIEVuY3J5cHQxIzAhBgNVBAMT\n" \
"GkxldCdzIEVuY3J5cHQgQXV0aG9yaXR5IFgzMIIBIjANBgkqhkiG9w0BAQEFAAOC\n" \
"AQ8AMIIBCgKCAQEAnNMM8FrlLke3cl03g7NoYzDq1zUmGSXhvb418XCSL7e4S0EF\n" \
"q6meNQhY7LEqxGiHC6PjdeTm86dicbp5gWAf15Gan/PQeGdxyGkOlZHP/uaZ6WA8\n" \
"SMx+yk13EiSdRxta67nsHjcAHJyse6cF6s5K671B5TaYucv9bTyWaN8jKkKQDIZ0\n" \
"Z8h/pZq4UmEUEz9l6YKHy9v6Dlb2honzhT+Xhq+w3Brvaw2VFn3EK6BlspkENnWA\n" \
"a6xK8xuQSXgvopZPKiAlKQTGdMDQMc2PMTiVFrqoM7hD8bEfwzB/onkxEz0tNvjj\n" \
"/PIzark5McWvxI0NHWQWM6r6hCm21AvA2H3DkwIDAQABo4IBfTCCAXkwEgYDVR0T\n" \
"AQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAYYwfwYIKwYBBQUHAQEEczBxMDIG\n" \
"CCsGAQUFBzABhiZodHRwOi8vaXNyZy50cnVzdGlkLm9jc3AuaWRlbnRydXN0LmNv\n" \
"bTA7BggrBgEFBQcwAoYvaHR0cDovL2FwcHMuaWRlbnRydXN0LmNvbS9yb290cy9k\n" \
"c3Ryb290Y2F4My5wN2MwHwYDVR0jBBgwFoAUxKexpHsscfrb4UuQdf/EFWCFiRAw\n" \
"VAYDVR0gBE0wSzAIBgZngQwBAgEwPwYLKwYBBAGC3xMBAQEwMDAuBggrBgEFBQcC\n" \
"ARYiaHR0cDovL2Nwcy5yb290LXgxLmxldHNlbmNyeXB0Lm9yZzA8BgNVHR8ENTAz\n" \
"MDGgL6AthitodHRwOi8vY3JsLmlkZW50cnVzdC5jb20vRFNUUk9PVENBWDNDUkwu\n" \
"Y3JsMB0GA1UdDgQWBBSoSmpjBH3duubRObemRWXv86jsoTANBgkqhkiG9w0BAQsF\n" \
"AAOCAQEA3TPXEfNjWDjdGBX7CVW+dla5cEilaUcne8IkCJLxWh9KEik3JHRRHGJo\n" \
"uM2VcGfl96S8TihRzZvoroed6ti6WqEBmtzw3Wodatg+VyOeph4EYpr/1wXKtx8/\n" \
"wApIvJSwtmVi4MFU5aMqrSDE6ea73Mj2tcMyo5jMd6jmeWUHK8so/joWUoHOUgwu\n" \
"X4Po1QYz+3dszkDqMp4fklxBwXRsW10KXzPMTZ+sOPAveyxindmjkW8lGy+QsRlG\n" \
"PfZ+G6Z6h7mjem0Y+iWlkYcV4PIWL1iwBi8saCbGS5jN2p8M+X+Q7UNKEkROb3N6\n" \
"KOqkqm57TH2H3eDJAkSnh6/DNFu0Qg==\n" \
"-----END CERTIFICATE-----\n";
*/
TNET_HTTP_RESPONSE *HTTP_RESPONSE=NULL;

bool XB_NET_DoMessage(TMessageBoard* Am)
{
	bool Result = false;
	switch (Am->IDMessage)
	{
	case IM_FRAME_RESPONSE:
	{
		if (Am->Data.FrameResponseData.FrameID == testSendFrameID)
		{
			board.Log("Destination task response...OK",true,true);
		}
		else
		{

		}
		Result = true;
		break;
	}
	case IM_FRAME_RECEIVE:
	{
		board.Log("IM_FRAME_RECEIVE:", true, true);
		board.Log(String("  from address: "+IPAddress(Am->Data.FrameReceiveData.SourceAddress).toString()).c_str(), true, true);
		board.Log(String("  from task: '" + String(Am->Data.FrameReceiveData.SourceTaskName)+"'").c_str(), true, true);
		board.Log(String("  to address: " + IPAddress(Am->Data.FrameReceiveData.DestAddress).toString()).c_str(), true, true);
		board.Log(String("  size frame: " + String(Am->Data.FrameReceiveData.SizeFrame)).c_str(), true, true);
		board.Log(String("  data frame: " + String((char *)Am->Data.FrameReceiveData.DataFrame)).c_str(), true, true);
		Am->Data.FrameReceiveData.FrameReceiveResult = frrOK;
		Result = true;
		break;
	}
	case IM_GET_TASKNAME_STRING:
	{
		GET_TASKNAME("NET");
		Result = true;
		break;

	}
	case IM_GET_TASKSTATUS_STRING:
	{
		switch (NET_FunctionStep)
		{
			GET_TASKSTATUS(ifsIDLE, 3);
			GET_TASKSTATUS(ifsHandleSocketStart, 3);
			GET_TASKSTATUS(ifsHandleSocket, 3);
			GET_TASKSTATUS(ifsHandleSocketStop, 3);
			GET_TASKSTATUS(ifsHandleSocketDisconnectAllClient, 3);
		}
		GET_TASKSTATUS_ADDSTR(" sc:" + String(SocketList_count));

		if (NETStatus==nsConnect) GET_TASKSTATUS_ADDSTR(" N:1")
		else GET_TASKSTATUS_ADDSTR(" N:0")

		if (INTERNETStatus == isConnect) GET_TASKSTATUS_ADDSTR(" I:1")
		else GET_TASKSTATUS_ADDSTR(" I:0")


		Result = true;
		break;
	}
	case IM_LOAD_CONFIGURATION:
	{
		if (!NET_LoadConfig())
		{
			board.Log("Error");
		}
		Result = true;
		break;
	}
	case IM_SAVE_CONFIGURATION:
	{
		if (!NET_SaveConfig())
		{
			board.Log("Error");
		}
		Result = true;
		break;
	}
	case IM_RESET_CONFIGURATION:
	{
		if (!NET_ResetConfig())
		{
			board.Log("Error");
		}
		Result = true;
		break;
	}
	case IM_HANDLEPTR:
	{
		HANDLEPTR(testudpclient);
		HANDLEPTR(testudpserver);
		HANDLEPTR(testtcpclientsecure);
		HANDLEPTR(NET_CurrentSocket);
#ifdef XB_GUI
		HANDLEPTR(NET_winHandle0);
		HANDLEPTR(NET_menuHandle0);
#endif
		Result = true;
		break;
	}
	case IM_NET_CONNECT:
	{
		return true;
	}
	case IM_NET_DISCONNECT:
	{
		if ((NET_FunctionStep == ifsHandleSocket) || (NET_FunctionStep == ifsHandleSocketStart))
		{
			NET_FunctionStep = ifsHandleSocketDisconnectAllClient;
		}
		return true;
	}
	case IM_SOCKET:
	{
		if (testudpserver == Am->Data.SocketData.Socket)
		{
			switch (Am->Data.SocketData.TypeSocketAction)
			{
			case tsaNewClientSocket:
			{
				if (Am->Data.SocketData.NewClientSocket == testudpclient)
				{
					break;
				}
				else
				{

					if (testudpclient != NULL)
					{
						board.Log("IM_SOCKET: Destroy previus UDP Client.", true, true);
						NET_DestroySocket(&testudpclient);
					}
					board.Log("IM_SOCKET: Create new UDP Client.", true, true);
					testudpclient = Am->Data.SocketData.NewClientSocket;

				}
				break;
			}
			case tsaServerStart:
			{
				board.Log("IM_SOCKET: Server UDP Start.", true, true);
				break;
			}
			case tsaServerStartingError:
			{
				board.Log("IM_SOCKET: Server UDP Starting error.", true, true,tlError);
				break;
			}
			case tsaServerStop:
			{
				board.Log("IM_SOCKET: Server UDP Stop.", true, true);
				break;
			}
			default:break;
			}
			Result = true;
		}

		if (testudpclient == Am->Data.SocketData.Socket)
		{
			switch (Am->Data.SocketData.TypeSocketAction)
			{
			case tsaConnect:
			{
				board.Log("IM_SOCKET: Connect UDP Client", true, true);
				break;
			}
			case tsaDisconnect:
			{
				board.Log("IM_SOCKET: Disconnect UDP Client", true, true);
				break;
			}
			case tsaReceived:
			{
				char data[1024]; xb_memoryfill(data, 1024, 0);

				NET_Read(testudpclient,(uint8_t *) data, 1024);

				board.Log(String("IM_SOCKET: Receive data UDP Client ["+String(data)+"]").c_str(), true, true);
				break;
			}
			case tsaSended:
			{
				board.Log(String("IM_SOCKET: Sended UDP Client ['dupa']").c_str(), true, true);
				break;
			}
			default:break;
			}
			Result = true;
		}

		if (testtcpclientsecure == Am->Data.SocketData.Socket)
		{
			switch (Am->Data.SocketData.TypeSocketAction)
			{
			case tsaConnect:
			{
				board.Log("IM_SOCKET: Connect TCP Client Secure", true, true);
				break;
			}
			case tsaDisconnect:
			{
				board.Log("IM_SOCKET: Disconnect TCP Client Secure", true, true);
				break;
			}
			case tsaReceived:
			{
				if (NET_ShowDebug)
				{
					String s = "IM_SOCKET: NET_HTTP_PARSE_RESPONSE result = " + NET_GetString_SocketResult(NET_HTTP_PARSE_RESPONSE(testtcpclientsecure, &HTTP_RESPONSE, true, true, true));
					board.Log(s.c_str(), true, true, tlInfo);

					if (HTTP_RESPONSE != NULL)
					{
						board.Log("HTTP RESPONSE CODE: ", true, true, tlInfo);
						board.Log(String(HTTP_RESPONSE->HttpCode).c_str(),false,false,tlWarn);
						board.Log("HTTP HEADERS:\n", true, true, tlInfo);
						String s = HTTP_RESPONSE->Headers;
						s.replace("\r\n", "\n");
						board.Log(s.c_str(), false, false, tlWarn);

						board.Log("HTTP RESPONSE ContentLength: ", true, true, tlInfo);
						board.Log(String(HTTP_RESPONSE->ContentLength).c_str(), false, false, tlWarn);

						board.Log("HTTP BODY:\n", true, true, tlInfo);
						s = HTTP_RESPONSE->Body;
						s.replace("\r\n", "\n");
						board.Log(s.c_str(), false, false, tlWarn);
					}

				}
				break;
			}
			case tsaSended:
			{
				board.Log(String("IM_SOCKET: Sended TCP Client Secure ... OK").c_str(), true, true);
				break;
			}
			default:break;
			}
			Result = true;
		}
		break;
	}
#ifdef XB_GUI
	case IM_WINDOW:
	{
		BEGIN_WINDOW_DEF(0, "Network raport", 20, 0, 140, SocketList_count + 10, NET_winHandle0)
		{
			static Ty TopRowTCPClientsSecure = 0;
			static Ty TopRowTCPClients = 0;
			static Ty TopRowTCPServers = 0;
			static Ty TopRowUDPServers = 0;
			static Ty TopRowUDPClients = 0;

			DESTROY_WINDOW()
			{
				NET_ShowRaport = false;
			}

			REPAINT_WINDOW()
			{
				
				WH->BeginDraw();
				{
					Ty Row = 0;

					// ----

					WH->SetNormalChar();
					WH->SetTextColor(tfcMagenta);
					NET_SocketClientsSecureDrawCaption(WH, Row);
					TopRowTCPClientsSecure = Row;
					WH->SetTextColor(tfcWhite);
					TSocket* s = SocketList;
					while (s != NULL)
					{
						if (s->Type == tsTCPClientSecure)
						{
							NET_SocketClientSecureDraw(WH, s, Row);
						}
						s = s->Next;
					}

					// ----

					WH->SetNormalChar();
					WH->SetTextColor(tfcMagenta);
					NET_SocketClientsDrawCaption(WH,Row);
					TopRowTCPClients = Row;
					WH->SetTextColor(tfcWhite);
					s = SocketList;
					while (s != NULL)
					{
						if (s->Type == tsTCPClient)
						{
							NET_SocketClientDraw(WH, s, Row);
						}
						s = s->Next;
					}

					// ----

					WH->SetNormalChar();
					WH->SetTextColor(tfcMagenta);
					NET_SocketServersDrawCaption(WH, Row);
					TopRowTCPServers = Row;
					WH->SetTextColor(tfcWhite);
					s = SocketList;
					while (s != NULL)
					{
						if (s->Type == tsTCPServer)
						{
							NET_SocketServerDraw(WH, s, Row);
						}
						s = s->Next;
					}

					// ----

					WH->SetNormalChar();
					WH->SetTextColor(tfcMagenta);
					NET_SocketServersUDPDrawCaption(WH, Row);
					TopRowUDPServers = Row;
					WH->SetTextColor(tfcWhite);
					s = SocketList;
					while (s != NULL)
					{
						if (s->Type == tsUDPServer)
						{
							NET_SocketServerUDPDraw(WH, s, Row);
						}
						s = s->Next;
					}

					// ----

					WH->SetNormalChar();
					WH->SetTextColor(tfcMagenta);
					NET_SocketClientsUDPDrawCaption(WH, Row);
					TopRowUDPClients = Row;
					WH->SetTextColor(tfcWhite);
					s = SocketList;
					while (s != NULL)
					{
						if (s->Type == tsUDPClient)
						{
							NET_SocketClientUDPDraw(WH, s, Row);
						}
						s = s->Next;
					}

				}
				WH->EndDraw();
			}
			REPAINTDATA_WINDOW()
			{
				WH->BeginDraw();
				{
					WH->SetNormalChar();
					Ty Row = TopRowTCPClientsSecure;
					WH->SetTextColor(tfcWhite);
					TSocket* s = SocketList;
					while (s != NULL)
					{
						if (s->Type == tsTCPClientSecure)
						{
							if (s->Repaint != 0)
							{
								NET_SocketClientSecureDraw(WH, s, Row);
							}
							else
							{
								Row++;
							}
						}
						s = s->Next;
					}

					Row = TopRowTCPClients;
					WH->SetTextColor(tfcWhite);
					s = SocketList;
					while (s != NULL)
					{
						if (s->Type == tsTCPClient)
						{
							if (s->Repaint != 0)
							{
								NET_SocketClientDraw(WH, s, Row);
							}
							else
							{
								Row++;
							}
						}
						s = s->Next;
					}

					Row = TopRowTCPServers;
					s = SocketList;
					while (s != NULL)
					{
						if (s->Type == tsTCPServer)
						{
							if (s->Repaint != 0)
							{
								NET_SocketServerDraw(WH, s, Row);
							}
							else
							{
								Row++;
							}
						}
						s = s->Next;
					}

					Row = TopRowUDPClients;
					s = SocketList;
					while (s != NULL)
					{
						if (s->Type == tsUDPClient)
						{
							if (s->Repaint != 0)
							{
								NET_SocketClientUDPDraw(WH, s, Row);
							}
							else
							{
								Row++;
							}
						}
						s = s->Next;
					}

					Row = TopRowUDPServers;
					s = SocketList;
					while (s != NULL)
					{
						if (s->Type == tsUDPServer)
						{
							if (s->Repaint != 0)
							{
								NET_SocketServerUDPDraw(WH, s, Row);
							}
							else
							{
								Row++;
							}
						}
						s = s->Next;
					}


				}
				WH->EndDraw();
			}
		}
		END_WINDOW_DEF()

		Result = true;
		break;
	}
	case IM_MENU:
	{
		OPEN_MAINMENU()
		{
			NET_menuHandle0 = GUIGADGET_CreateMenu(&XB_NET_DefTask, 0, false, X, Y);
		}

		BEGIN_MENU(0, "NET MAIN MENU", WINDOW_POS_X_DEF, WINDOW_POS_Y_DEF, 64, MENU_AUTOCOUNT, 0, true)
		{
			BEGIN_MENUITEM_CHECKED("Start engine", taLeft,NET_Start)
			{
				CLICK_MENUITEM()
				{
					NET_Start = !NET_Start;
				}
			}
			END_MENUITEM()
			BEGIN_MENUITEM_CHECKED("Show debug info", taLeft, NET_ShowDebug)
			{
				CLICK_MENUITEM()
				{
					NET_ShowDebug = !NET_ShowDebug;
				}
			}
			END_MENUITEM()
			BEGIN_MENUITEM_CHECKED("Show network raport", taLeft, NET_ShowRaport)
			{
				CLICK_MENUITEM()
				{
					NET_ShowRaport = !NET_ShowRaport;
					if (NET_ShowRaport)
					{
						NET_ShowNetworkRaportWindow();
					}
					else
					{
						NET_CloseNetworkRaportWindow();
					}
				}
			}
			END_MENUITEM()
			SEPARATOR_MENUITEM()
			BEGIN_MENUITEM("Create TEST TCP Client Secure, connect https://msys.hostingasp.pl:443", taLeft)
			{
				CLICK_MENUITEM()
				{
					String s="NET_CreateTCPClientSecure result = " + NET_GetString_SocketResult(NET_CreateTCPClientSecure(&testtcpclientsecure,NULL,"msys.hostingasp.pl",443,test_root_ca));
					testtcpclientsecure->AutoConnect = true;
					board.Log(s.c_str(), true, true, tlInfo);
				}
			}
			END_MENUITEM()
			BEGIN_MENUITEM("Destroy TEST TCP Client Secure...", taLeft)
			{
				CLICK_MENUITEM()
				{
					String s = "NET_DestroySocket result = " + NET_GetString_SocketResult(NET_DestroySocket(&testtcpclientsecure));
					board.Log(s.c_str(), true, true, tlInfo);
				}
			}
			END_MENUITEM()
			BEGIN_MENUITEM("NET_HTTP_GET msys.hostingasp.pl/api", taLeft)
			{
				CLICK_MENUITEM()
				{
					String s = "NET_HTTP_GET result = " + NET_GetString_SocketResult(NET_HTTP_GET(testtcpclientsecure,"/api",""));
					board.Log(s.c_str(), true, true, tlInfo);
				}
			}
			END_MENUITEM()
			BEGIN_MENUITEM("NET_HTTP_POST msys.hostingasp.pl/api/ConnectDevice", taLeft)
			{
				CLICK_MENUITEM()
				{
					String s = "NET_HTTP_POST result = " + NET_GetString_SocketResult(NET_HTTP_POST(testtcpclientsecure, "/api/connectdevice", "h="+String(SysTickCount)+"&deviceidrom="+ board.DeviceIDtoString(board.DeviceID) +"&devicename=DUPA"));
					board.Log(s.c_str(), true, true, tlInfo);
				}
			}
			END_MENUITEM()
			SEPARATOR_MENUITEM()
			BEGIN_MENUITEM("Create test UDP Server", taLeft)
			{
				CLICK_MENUITEM()
				{
					NET_CreateUDPServer(&testudpserver, NULL, 3623);
				}
			}
			END_MENUITEM()
			BEGIN_MENUITEM("Destroy test UDP Server", taLeft)
			{
				CLICK_MENUITEM()
				{
					NET_DestroySocket(&testudpserver);
				}
			}
			END_MENUITEM()
			BEGIN_MENUITEM("Send  'dupa' to current test UDP client", taLeft)
			{
				CLICK_MENUITEM()
				{
					if (testudpclient != NULL)
					{
						NET_Write(testudpclient, (uint8_t*)"DUPA", 4);
					}

				}
			}
			END_MENUITEM()
			SEPARATOR_MENUITEM()
			BEGIN_MENUITEM("Send frame 'dupa' to 192.168.1.18 on UDPStream", taLeft)
			{
				CLICK_MENUITEM()
				{
					char dupa[] = "dupa\0";
					board.Log("Send frame 'dupa' to 192.168.1.18 on UDPStream...", true, true);
					
					if (board.SendFrameToDeviceTask("NET", IPAddress(192, 168, 1, 19), "UDPSTREAM", IPAddress(192, 168, 1, 18), "NET", dupa, 5, &testSendFrameID))
					{
						board.Log("OK");
					}
					else
					{
						board.Log("ERROR");
					}
				}
			}
			END_MENUITEM()
			BEGIN_MENUITEM("Send frame 'dupy' to 192.168.1.19 on UDPStream", taLeft)
			{
				CLICK_MENUITEM()
				{
					char dupa[] = "dupy\0";
					board.Log("Send frame 'dupy' to 192.168.1.19 on UDPStream...", true, true);

					if (board.SendFrameToDeviceTask("NET", IPAddress(192, 168, 1, 18), "UDPSTREAM", IPAddress(192, 168, 1, 19), "NET", dupa, 5, &testSendFrameID))
					{
						board.Log("OK");
					}
					else
					{
						board.Log("ERROR");
					}
				}
			}
			END_MENUITEM()
			SEPARATOR_MENUITEM()

			CONFIGURATION_MENUITEMS()
		}
		END_MENU()

		Result = true;
		break;
	}
#endif

	default: break;
	}
	return Result;
}

TTaskDef XB_NET_DefTask = {0,&XB_NET_Setup,&XB_NET_DoLoop,&XB_NET_DoMessage};