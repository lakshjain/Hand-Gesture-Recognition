#include "Socket.h"
#include <sstream>
#include <windows.h>
#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 3110
#define DEFAULT_HOST "127.0.0.1"

#define MAX_PACKET_SIZE 100

Socket Socket::sSocket;

Socket* Socket::GetInstance()
{
	return &sSocket;
}

Socket::Socket()
{	
	isInitialized = false;
}

int Socket::Init() 
{	
    WSADATA wsaData;
   
    ListenSocket = INVALID_SOCKET;
    ClientSocket = INVALID_SOCKET;

	int iResult;
     
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return -1;
    }
	
    // Create a SOCKET for connecting to server
    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        //freeaddrinfo(result);
        WSACleanup();
        return -1;
    }
	
	// Set the mode of the socket to be nonblocking
    u_long iMode = 1;
    iResult = ioctlsocket(ListenSocket, FIONBIO, &iMode);

    if (iResult == SOCKET_ERROR) {
        printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return -1;
    }
		
	sockaddr_in service;
    service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr(DEFAULT_HOST);
	service.sin_port = htons((u_short)DEFAULT_PORT);
	
    // Setup the TCP listening socket
    iResult = bind( ListenSocket, (SOCKADDR *) & service, (int)sizeof (service));
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        //freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return -1;
    }

   iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return -1;
    }
	  
    isInitialized = true;
	mLastClientId = 0;

    return 0;
}

void Socket::Run()
{
	if(!isInitialized)
		return;
	
	if(AcceptNewClient(mLastClientId))
	{
		cout<<"\nNew Client Connected : "<<mLastClientId;
		mLastClientId++;
		std::string data = "Hey";
		Broadcast(data.c_str(),data.length());
	}

	std::map<unsigned int, SOCKET>::iterator iter;

	for(iter = mClientSessions.begin(); iter != mClientSessions.end(); iter++)
    {
		char data[MAX_PACKET_SIZE];
		int dLength = ReceiveData(iter->first, data);
        if (dLength < 0) 
               continue;
		else if (dLength == 0)
		
        {
            printf("Connection closed\n");
			closesocket(iter->second);
			iter = mClientSessions.erase (iter);
			if(iter == mClientSessions.end())
				break;

			continue;
        }
	
		string dataString(data, dLength);
		cout<<"\n Got message "<<dataString.c_str();
	}
	
}

void Socket::Dispose()
{
	if(!isInitialized)
		return;
	isInitialized = false;
	closesocket(ClientSocket);
    WSACleanup();
}

void Socket::Shutdown()
{
	if(isInitialized)
	{// shutdown the connection since we're done
		int iResult = shutdown(ClientSocket, SD_SEND);
		if (iResult == SOCKET_ERROR) {
			 printf("shutdown failed with error: %d\n", WSAGetLastError());
		} 
		Dispose();
	}
}

bool Socket::AcceptNewClient(unsigned int & id)
{
    // if client waiting, accept the connection and save the socket
    ClientSocket = accept(ListenSocket,NULL,NULL);

    if (ClientSocket != INVALID_SOCKET) 
    {
        //disable nagle on the client's socket
        char value = 1;
        setsockopt( ClientSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof( value ) );

        // insert new client into session id table
        mClientSessions.insert( pair<unsigned int, SOCKET>(id, ClientSocket) );

        return true;
    }

    return false;
}

int Socket::ReceiveData(unsigned int client_id, char * recvbuf)
{
	if(!isInitialized)
		return -1;
    int iResult = -1;

	if( mClientSessions.find(client_id) != mClientSessions.end() )
    {
        SOCKET currentSocket = mClientSessions[client_id];
        iResult = recv(currentSocket, recvbuf, MAX_PACKET_SIZE,0);
    }

    return iResult;
}

// send data to all clients
void Socket::Broadcast(const char* msg, int totalSize)
{
	if(!isInitialized)
		return;
    SOCKET currentSocket;
    std::map<unsigned int, SOCKET>::iterator iter;
    int iSendResult;

	for (iter = mClientSessions.begin(); iter != mClientSessions.end(); iter++)
    {
		int packetSize = sizeof(int)+totalSize;
		char * buf = new char[packetSize];
		char* offset = buf;
		memcpy(offset, (const char*)&totalSize, sizeof(int));
		offset += sizeof(int);
		memcpy(offset,msg,totalSize);

		currentSocket = iter->second;
		
		iSendResult = send(currentSocket, buf, packetSize ,0);

        if (iSendResult == SOCKET_ERROR) 
        {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(currentSocket);
        }
    }
}
