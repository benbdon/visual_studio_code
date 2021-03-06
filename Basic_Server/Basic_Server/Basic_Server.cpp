// Basic_Server.cpp : Defines the entry point for the console application.
//

#include <winsock2.h> // most of the Winsock functions, structures, and definitions. 
#include <ws2tcpip.h> // definitions introduced in the WinSock 2 Protocol-Specific Annex document for TCP/IP that 
//includes newer functions and structures used to retrieve IP addresses.
#include <stdio.h> // standard input and output, specifically the printf() function.

#pragma comment(lib, "Ws2_32.lib") //Applications that use Winsock must be linked with the Ws2_32.lib library file. The #pragma comment indicates to the linker that the Ws2_32.lib file is needed.

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015" //27015 is the port number associated with the server that the client will connect to.

int main() {
	WSADATA wsaData; //Initialize Winsock. Create a WSADATA object called wsaData. contains information about the Windows Sockets implementation
	int iResult; //Call WSAStartup and return its value as an integer and check for errors.

	SOCKET ListenSocket = INVALID_SOCKET; //Create a temporary SOCKET object called ListenSocket for the server to listen for client connections
	SOCKET ClientSocket = INVALID_SOCKET; //Create a temporary SOCKET object called ClientSocket for accepting connections from clients.

	struct addrinfo *result = NULL, *ptr = NULL, hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData); //Call WSAStartup and return its value as an integer and check for errors.
	//initiate use of WS2_32.dll & makes a request for version 2.2 of Winsock on the system, and sets the passed version as the highest version of 
	//Windows Sockets support that the caller can use. 
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints)); //Fills a vlock of memory with zeros - (Destination and Length)
	hints.ai_family = AF_INET; // used to specify the IPv4 address family
	hints.ai_socktype = SOCK_STREAM; // specify a stream socket
	hints.ai_protocol = IPPROTO_TCP; // use to specify the TCP protocol
	hints.ai_flags = AI_PASSIVE; // the IP address portion of the socket address structure is set to INADDR_ANY for IPv4 
	// addresses or IN6ADDR_ANY_INIT for IPv6 addresses.

	// Resolve the local server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Define SOCKET for the server to listen for client connections
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol); // Create a SOCKET for the server
	// to listen for client connections
	if (ListenSocket == INVALID_SOCKET) { //Check for errors to ensure that the socket is a valid socket.
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Bind a socket & setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen); //Call the bind function, passing the 
	//created socket and sockaddr structure returned from the getaddrinfo function as parameters. 
	if (iResult == SOCKET_ERROR) { //Check for general errors.
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result); //Once the bind function is called, the address information returned by the getaddrinfo
	// function is no longer needed. The freeaddrinfo function is called to free the memory allocated by the getaddrinfo
	//function for this address information.

	//After the socket is bound to an IP address and port on the system, the server must then listen on that IP address
	// and port for incoming connection requests.

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) { //passing as parameters the created socket and a value for
		// the backlog, maximum length of the queue of pending connections to accept. In this example, the backlog
		// parameter was set to SOMAXCONN. This value is a special constant that instructs the Winsock provider for 
		// this socket to allow a maximum reasonable number of pending connections in the queue. 
		printf("Listen failed with error: %ld\n", WSAGetLastError()); //Check the return value for general errors.
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// Receive until the peer shuts down the connection
	do {

		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0); //returns int value of the num of bytes received or an 
		// error. Parameters are (active socket, char buffer, num of bytes to receive, any flags)
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);

			// Echo the buffer back to the sender
			iSendResult = send(ClientSocket, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
			printf("Bytes sent: %d\n", iSendResult);
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

	} while (iResult > 0);

	//Once the server is completed receiving data from the client and sending data back to the client, the server
	// disconnects from the client and shutdowns the socket.


	// shutdown the send half of the connection since no more data will be sent
	iResult = shutdown(ClientSocket, SD_SEND); //When the server is done sending data to the client, the shutdown 
	// function can be called specifying SD_SEND to shutdown the sending side of the socket. This allows the client to 
	// release some of the resources for this socket. The server application can still receive data on the socket.
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(ClientSocket);
	WSACleanup(); //When the client application is completed using the Windows Sockets DLL, the WSACleanup function
	// is called to release resources.

	return 0;
}