#pragma once

// WinSock2 Windows Sockets
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

// Need to link Ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#include <chrono>
#include <vector>

namespace net
{
	struct PlayerPosition
	{
		float x;
		float z;
		int wantsToShoot; // 0 = no  1 = down  2 = left  3 = up   4 = right
		bool canShoot = true;
		bool isDead = false;
	};

	struct BulletPosition // This is just to keep track of the internal info of the bullet, only broadcasting the position
	{
		float bX = 50.0f;
		float bZ = 50.0f; // Offscreen

		float bDirX = 0.0f;
		float bDirZ = 0.0f;
		double timeAlive;
	};

	struct ClientInfo : public PlayerPosition, public BulletPosition
	{
		sockaddr_in addr;
		int addrLen;
		unsigned int lastPacketRecieved = 0;
	};

	class NetworkManager
	{
	public:
		NetworkManager();
		~NetworkManager();

		void Initialize();
		void Destroy();

		void Update();

	private:
		void HandleRECV();
		void BroadcastUpdatesToClients();

		bool m_Initialized = false;
		bool m_Connected = false;
		float m_Wait = 0;

		unsigned int m_PacketNum = 0; //Keeps track of packet order on the client's side; for reconciliation

		SOCKET m_ListenSocket;

		// 
		// Time
		std::chrono::high_resolution_clock::time_point m_NextBroadcastTime;
		std::chrono::high_resolution_clock::time_point m_LastTime;

		//
		// ConnectedClients
		std::vector<ClientInfo> m_ConnectedClients;

	};

	const float RETRY_TIME_MS = 10000;
}