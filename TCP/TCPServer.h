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
	//����(�����ڵ�)�� ������.
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
	//socket(Address Family, Socket Type(�� ��� TCP��), Protocol)
	m_Server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//���� ���� ���н�
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
	//��Ʈ ��ȣ ����.
	addr.sin_port = htons(20000);
	addr.sin_addr.S_un.S_un_b.s_b1 = 192;
	addr.sin_addr.S_un.S_un_b.s_b2 = 168;
	addr.sin_addr.S_un.S_un_b.s_b3 = 35;
	addr.sin_addr.S_un.S_un_b.s_b4 = 229;

	//bind�� ������ ��. ������ Ư�� �ּҿ� ��Ʈ�� ���� �Լ�. ������ ��Ʈ �ϳ��� Ȯ���ϰ� �־�� ��.
	if (bind(m_Server_socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) // (error)�� �ᵵ ��.
	{
		std::cerr << KErrorMessage << WSAGetLastError();
		return false;
	}
	return true;
}

//listen = ��Ŷ�� ���Ͽ� ������ ������ ��ٸ�.
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

//client�� ����(connect) ��û�� �޾��ִ� �Լ�
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
	//��� �迭�� ��� ũ�⸦ �˾Ƽ� ����� ��
	const char* KErrorMessage{ "error code: " };
	SOCKET m_Server_socket{};
	SOCKET m_client{};
};
