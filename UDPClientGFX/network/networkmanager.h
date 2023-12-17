#pragma once

#include <common.h>

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

gdpNamespaceBegin

namespace net
{
	const int NUM_PLAYERS = 4;

	struct PlayerPosition
	{
		unsigned int packetNumber = 0;
		float x = 0.0f;
		float z = 0.0f;
		int wantsToShoot; // 0 = no shoot  1 = down   2 = left   3 = up   4 = right
	};

	struct DeadReckoning
	{
		float oldX = 0.0f;
		float oldZ = 0.0f;
		float currX = 0.0f;
		float currZ = 0.0f;

		float xDir = 0.0f;
		float zDir = 0.0f;

		double timeSinceLastServerUpdate = 0;

		// Position will be currX + dt * direction
		// Direction is old->curr
	};

	struct ClientInfo
	{
		sockaddr_in addr;
		int addrLen;
	};

	class NetworkManager
	{
	public:
		NetworkManager();
		~NetworkManager();

		void Initialize();
		void Destroy();

		void Update();

		void SendPlayerPositionToServer(float x, float z, int shoot);

		// 
		// Positions of players
		std::vector<PlayerPosition> m_NetworkedPositions;


		std::vector<DeadReckoning> m_DeadReckoningHelpers;
		double timeSinceLastPacket = 0;


		bool isPlayerDead(void);

	private:
		void HandleRECV();
		void SendDataToServer();

		bool m_Initialized = false;
		bool m_Connected = false;
		float m_Wait = 0;

		bool m_IsDead = false;

		unsigned int m_LastPacketNum = 0;
		

		SOCKET m_ServerSocket;
		sockaddr_in m_ServerAddr;
		int m_ServerAddrLen = 0;


		PlayerPosition m_PlayerPosition;

		// 
		// Time
		std::chrono::high_resolution_clock::time_point m_NextSendTime;

		//
		// ConnectedClients
		std::vector<ClientInfo> m_ConnectedClients;

	};

	const float RETRY_TIME_MS = 10000;
}

gdpNamespaceEnd