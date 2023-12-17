#include "NetworkManager.h"

#include <memory>

#include <iostream>

#define SERVER_PORT 8412
#define SERVER_IP "127.0.0.1"

namespace net
{
	// Move this to a structure for each client
	sockaddr_in addr;
	int addrLen;

	class Buffer
	{
	public:
		Buffer() { }
		~Buffer() { }

		std::vector<uint8_t> data;
	};

	NetworkManager::NetworkManager()
	{
	}

	NetworkManager::~NetworkManager()
	{

	}

	void NetworkManager::Initialize()
	{
		// Initialize WinSock
		WSADATA wsaData;
		int result;

		m_LastTime = std::chrono::high_resolution_clock::now();

		// Set version 2.2 with MAKEWORD(2,2)
		result = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (result != 0) {
			printf("WSAStartup failed with error %d\n", result);
			return;
		}
		printf("WSAStartup successfully!\n");


		// Socket
		m_ListenSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (m_ListenSocket == INVALID_SOCKET) {
			printf("socket failed with error %d\n", WSAGetLastError());
			WSACleanup();
			return;
		}
		printf("socket created successfully!\n");

		// using sockaddr_in
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(SERVER_PORT);
		addr.sin_addr.s_addr = htonl(INADDR_ANY);

		// Bind 
		result = bind(m_ListenSocket, (SOCKADDR*)&addr, sizeof(addr));
		if (result == SOCKET_ERROR) {
			printf("bind failed with error %d\n", WSAGetLastError());
			closesocket(m_ListenSocket);
			WSACleanup();
			return;
		}
		printf("bind was successful!\n");

		unsigned long nonblock = 1;
		result = ioctlsocket(m_ListenSocket, FIONBIO, &nonblock);
		if (result == SOCKET_ERROR) {
			printf("set socket to nonblocking failed with error %d\n", WSAGetLastError());
			closesocket(m_ListenSocket);
			WSACleanup();
			return;
		}
		printf("set socket to nonblocking was successful!\n");

		printf("NetworkManager running...\n");

		m_NextBroadcastTime = std::chrono::high_resolution_clock::now();

		m_Initialized = true;
	}

	bool hitDetection(float pX, float pZ, float bX, float bZ)
	{
		const float hitDistance = 1.5f;
		if (!(abs(pX - bX) < hitDistance))
		{
			return false;
		}
		if (!(abs(pZ - bZ) < hitDistance))
		{
			return false;
		}
		return true;
	}


	void NetworkManager::Destroy()
	{
		if (!m_Initialized)
		{
			return;
		}

		closesocket(m_ListenSocket);
		WSACleanup();

		m_Initialized = false;
	}

	void NetworkManager::Update()
	{
		if (!m_Initialized)
		{
			return;
		}

		const float shootSpeed = 15.0f;

		std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> dt = currentTime - m_LastTime;
		m_LastTime = currentTime;

		// Handle all recv data
		HandleRECV();

		// Process everything

		

		for (int i = 0; i < m_ConnectedClients.size(); i++)
		{
			ClientInfo& client = m_ConnectedClients[i];
			
			//std::cout << "X: " << client.bX << " Z: " << client.bZ << std::endl;
			//std::cout << "X: " << client.bDirX << " Z: " << client.bDirZ << std::endl;

			// Reduce bullet lifetime
			if (client.timeAlive > 0)
			{
				client.timeAlive -= dt.count();
				if (client.timeAlive <= 0)
				{
					client.bX = 50.0f;
					client.bZ = 50.0f;
					client.bDirX = 0.0f;
					client.bDirZ = 0.0f;
					client.canShoot = true;
				}
			}
			// Reduce time until next shot
			//client.timeToNextShot -= dt.count();

			if ((client.wantsToShoot != 0) && (client.canShoot)) // If player wants to shoot
			{
				client.bX = client.x;
				client.bZ = client.z;
				client.canShoot = false;
				client.timeAlive = 2.0f;

				switch (client.wantsToShoot)
				{
				case 1: // Shoot down
				{
					client.bDirX = -1.0f * shootSpeed;
					break;
				}
				case 2: // Shoot left
				{
					client.bDirZ = -1.0f * shootSpeed;
					break;
				}
				case 3: // Shoot up
				{
					client.bDirX = 1.0f * shootSpeed;
					break;
				}
				case 4: // Shoot Right
				{
					client.bDirZ = 1.0f * shootSpeed;
					break;
				}
				}
			}
		}

		// Check collision in a separate for loop, lessens host advantage
		for (int i = 0; i < m_ConnectedClients.size(); i++)
		{
			ClientInfo& client1 = m_ConnectedClients[i];
			for (int e = 0; e < m_ConnectedClients.size(); e++)
			{
				if (e == i) continue;
				ClientInfo& client2 = m_ConnectedClients[e];
				if (hitDetection(client1.x, client1.z, client2.bX, client2.bZ))
				{
					std::cout << "HIT!!!!!!" << std::endl;
					// Set bullet to off screen with 0 velocity
					client2.bX = 50.0f;
					client2.bZ = 50.0f;
					client2.bDirX = 0.0f;
					client2.bDirZ = 0.0f;
					client2.timeAlive = 0.0f;
					client2.canShoot = true;

					// Kill client1
					client1.isDead = true;
				}

			}
			
		}

		// Update bullet positions last

		for (int i = 0; i < m_ConnectedClients.size(); i++)
		{
			ClientInfo& client = m_ConnectedClients[i];
			if (client.canShoot) continue; // Don't need to update what doesn't exist

			client.bX += client.bDirX * dt.count();
			client.bZ += client.bDirZ * dt.count();
		}



		// Send information/data back to clients
		BroadcastUpdatesToClients();
	}

	void NetworkManager::HandleRECV()
	{
		// Read
		sockaddr_in addr;
		int addrLen = sizeof(addr);

		const int bufLen = 16;	// recving 2 floats only and an int + one more int for the packet number
		char buffer[bufLen];
		int result = recvfrom(m_ListenSocket, buffer, bufLen, 0, (SOCKADDR*)&addr, &addrLen);
		if (result == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				// Not a real error, we expect this.
				// -1 is an error, 0 is disconnected, >0 is a message
				// WSA uses this as a flag to check if it is a real error
				return;
			}
			else
			{
				// TODO: We want to handle this differently.
				printf("recvfrom failed with error %d\n", WSAGetLastError());
				Destroy();
				//closesocket(m_ListenSocket);
				//WSACleanup();
				return;
			}
		}

		// Compare to see if the addr is already registered
		// If it is not registered, we add it
		// If it is registered, we can set the data
		int clientId = -1;
		for (int i = 0; i < m_ConnectedClients.size(); i++)
		{
			ClientInfo& client = m_ConnectedClients[i];
			if (client.addr.sin_addr.s_addr == addr.sin_addr.s_addr
				&& client.addr.sin_port == addr.sin_port)
			{
				clientId = i;
				break;
			}
		}

		if (clientId == -1)
		{
			// Add the client
			ClientInfo newClient;
			newClient.addr = addr;
			newClient.addrLen = sizeof(addr);
			m_ConnectedClients.push_back(newClient);
			clientId = m_ConnectedClients.size() - 1;
		}

		ClientInfo& client = m_ConnectedClients[clientId];
		
		unsigned int thisPacketNum;
		memcpy(&thisPacketNum, (const void*)&(buffer[0]), sizeof(unsigned int));

		if (thisPacketNum < client.lastPacketRecieved) return;

		client.lastPacketRecieved = thisPacketNum;


		memcpy(&client.x, (const void*)&(buffer[4]), sizeof(float));
		memcpy(&client.z, (const void*)&(buffer[8]), sizeof(float));
		memcpy(&client.wantsToShoot, (const void*)&(buffer[12]), sizeof(int)); // Copy over the players shooting state

		//std::cout << "Shoot: " << client.wantsToShoot << std::endl;

		//printf("From: %s:%d: {%.2f, %.2f}\n", inet_ntoa(client.addr.sin_addr), client.addr.sin_port, client.x, client.z);
	}

	void NetworkManager::BroadcastUpdatesToClients()
	{
		std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();
		if (m_NextBroadcastTime > currentTime)
		{
			return;
		}

		//printf("broadcast!\n");

		m_NextBroadcastTime = currentTime + std::chrono::milliseconds(100);

		// Add 20 ms to the next broadcast time from now()
		//m_NextBroadcastTime 

		//const int length = sizeof(PlayerPosition) * 4;
		const int length = 8 * 8 + 8; // extra 8 for the kill bool and the server reconciliation number
		char data[length];


		PlayerPosition positions[4];

		// Packet number for reconciliation
		memcpy(&data[0], &m_PacketNum, sizeof(unsigned int));
		m_PacketNum++;

		for (int i = 0; i < m_ConnectedClients.size(); i++)
		{
			//memcpy(&data[i * sizeof(PlayerPosition)], &m_ConnectedClients[i].x, sizeof(float));
			//memcpy(&data[i * sizeof(PlayerPosition) + sizeof(float)], &m_ConnectedClients[i].z, sizeof(float));
			memcpy(&data[i * 8 + 4], &m_ConnectedClients[i].x, sizeof(float));
			memcpy(&data[i * 8 + sizeof(float) + 4], &m_ConnectedClients[i].z, sizeof(float));
		}
		for (int i = 4; i < 4 + m_ConnectedClients.size(); i++)
		{
			memcpy(&data[i * 8 + 4], &m_ConnectedClients[i - 4].bX, sizeof(float)); //
			memcpy(&data[i * 8 + sizeof(float) + 4], &m_ConnectedClients[i - 4].bZ, sizeof(float)); // Send bullet locational data
		}

		// Write
		for (int i = 0; i < m_ConnectedClients.size(); i++)
		{
			memcpy(&data[68], &m_ConnectedClients[i].isDead, sizeof(bool)); // Send unique bool to all clients informing them if they're dead
			ClientInfo& client = m_ConnectedClients[i];
			int result = sendto(m_ListenSocket, &data[0], length, 0, (SOCKADDR*)&client.addr, client.addrLen);
			if (result == SOCKET_ERROR) {
				// TODO: We want to handle this differently.
				printf("send failed with error %d\n", WSAGetLastError());
				closesocket(m_ListenSocket);
				WSACleanup();
				return;
			}
			m_ConnectedClients[i].isDead = false;
		}
	}


} // namespace net