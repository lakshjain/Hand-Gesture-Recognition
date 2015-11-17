#pragma once
#undef UNICODE

#define WIN32_LEAN_AND_MEAN


#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <map>

using namespace std;

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")





class Socket
{
	private:
		struct DataPacket
		{
			int len;
			string message;
			int GetSize()
			{
				return len + message.length();
			}

		};

		static Socket sSocket;
		Socket();
		SOCKET ClientSocket;
		SOCKET ListenSocket;
		bool isInitialized;
		unsigned int mLastClientId;
		std::map<unsigned int, SOCKET> mClientSessions; 

	public:
		static Socket* GetInstance();		
		int  Init();
		void Run();
		void Dispose();
		void Shutdown();
		bool AcceptNewClient(unsigned int & id);
		void Broadcast(const char * packets, int totalSize);
		int ReceiveData(unsigned int client_id, char * recvbuf);
		
};