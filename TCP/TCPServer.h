#pragma once
#include <WinSock2.h>
#include <iostream>

class TCPServer
{
	TCPServer() {}
	~TCPServer() {}

public:

bool startup()
{
	WSADATA wsa_data{};
	//숫자(에러코드)를 리턴함.
	int error{ WSAStartup(MAKEWORD(2,2), &wsa_data) };
	if (error)
	{
		std::cerr << KErrorMessage << error;
		return false;
	}
	return true;
}

bool create_socket()
{
	//socket(Address Family, Socket Type(이 경우 TCP용), Protocol)
	m_Server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//소켓 생성 실패시
	if (m_Server_socket == INVALID_SOCKET)
	{
		std::cerr << KErrorMessage << WSAGetLastError();
		m_Server_socket = 0;
		return false;
	}
	return true;
}

bool bind_socket()
{
	if (!m_Server_socket) { return false; }

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	//포트 번호 지정.
	addr.sin_port = htons(20000);
	addr.sin_addr.S_un.S_un_b.s_b1 = 192;
	addr.sin_addr.S_un.S_un_b.s_b2 = 168;
	addr.sin_addr.S_un.S_un_b.s_b3 = 35;
	addr.sin_addr.S_un.S_un_b.s_b4 = 229;

	//bind는 서버만 씀. 소켓을 특정 주소와 포트에 묶는 함수. 서버는 포트 하나가 확실하게 있어야 함.
	if (bind(m_Server_socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) // (error)만 써도 됨.
	{
		std::cerr << KErrorMessage << WSAGetLastError();
		return false;
	}
	return true;
}

//listen = 패킷이 소켓에 도착할 때까지 기다림.
bool listen_client()
{
	if (!m_Server_socket) { return false; }

	if (listen(m_Server_socket, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cerr << KErrorMessage << WSAGetLastError();
		return false;
	}
	return true;
}

//client의 연결(connect) 요청을 받아주는 함수
bool accept_client()
{
	if (!m_Server_socket) { return false; }

	sockaddr_in client_addr{};
	int client_addr_len{ sizeof(sockaddr_in) };

	m_client = accept(m_Server_socket, (sockaddr*)&client_addr, &client_addr_len);
	if (m_client == INVALID_SOCKET)
	{
		std::cerr << KErrorMessage << WSAGetLastError();
		m_client = 0;
		return false;
	}
	return true;
}

bool Cleanup()
{
	if (m_client) { closesocket(m_client); }
	if (m_Server_socket) { closesocket(m_Server_socket); }

	int error{ WSACleanup() };
	if (error)
	{
		std::cerr << KErrorMessage << error;
		return false;
	}
	return true;
}

private:
	//상수 배열일 경우 크기를 알아서 계산해 줌
	const char* KErrorMessage{ "error code: " };
	SOCKET m_Server_socket{};
	SOCKET m_client{};
};
