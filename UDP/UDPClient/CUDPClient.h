#pragma once

#include <WinSock2.h>
#pragma comment (lib, "ws2_32.lib")
#include <WS2tcpip.h>
#include <iostream>
#include <thread>

class CUDPClient
{
public:
	CUDPClient() { StartUp(); CreateSocket(); }
	~CUDPClient() { CleanUp(); }

private:
	bool StartUp()
	{
		WSADATA wsa_data{};
		//라이브러리를 로드하는 함수.
		int error{ WSAStartup(MAKEWORD(2,2), &wsa_data) };

		if (error != 0)
		{
			//placeholder = 어떤 자료형을 출력할지 정함.
			//가변인수함수 = variadic function 인수의 갯수가 변할 수 있는 함수.
			//__cdecl = c언어 기본 함수 호출 규약. caller가 stack을 정리함.
			printf("에러코드: %d", error);

			return false;
		}

		return true;
	}

	bool CleanUp()
	{
		int close_sock_error{ closesocket(m_client_socket) };

		if (close_sock_error == SOCKET_ERROR)
		{
			printf("에러코드: %d", WSAGetLastError());
		}

		//사실 직접 호출을 안 해도 프로세스가 종료되면 윈도우즈가 알아서 호출해줌.
		//cleanup을 안 하고 이후 startup을 호출하면 안됨.
		int error{ WSACleanup() };

		if (error != 0)
		{
			printf("에러코드: %d", error);

			return false;
		}

		return true;
	}

	bool CreateSocket()
	{
		m_client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		//소켓의 에러코드를 확인해주는 매크로
		if (m_client_socket == INVALID_SOCKET)
		{
			//가장 마지막에 발생한 에러코드를 리턴해주는 함수
			printf("에러코드: %d", WSAGetLastError());

			return false;
		}

		return true;
	}

public:
	void SetServerAddr(const char* ip, u_short port)
	{
		sockaddr_in sock_addr{};
		sock_addr.sin_family = AF_INET;
		//0~1024번 포트까지는 예약되어있음. 우리가 임의로 못 씀.
		//윈도우즈는 little endian, 네트워크는 big endian. 바꿔줘야 함.
		sock_addr.sin_port = htons(port);
		//b = byte, w = word

		//sz = string zero-terminated null문자로 끝나는 문자열.
		int error{ inet_pton(AF_INET, ip, &sock_addr.sin_addr) };

		//1을 리턴하면 성공
		if (error != 1)
		{
			printf("에러코드: %d", WSAGetLastError());
		}

		m_server_addr = sock_addr;
	}

	bool Receive()
	{
		sockaddr_in from_addr{};

		int from_addr_size{ sizeof(from_addr) };

		//몇 바이트를 받았는지 리턴함.
		int received_byte_count{ recvfrom(m_client_socket, m_buff, KBuffSize, 0,
			(sockaddr*)&from_addr, &from_addr_size) };
		
		if (received_byte_count > 0)
		{
			OutputDebugStringA("Client 에서 Server의 Data를 receive 하는데 성공함\n");

			m_buff[received_byte_count - 1] = 0;

			return true;
		}

		return false;
	}

	const char* GetBuff() const
	{
		return m_buff;
	}

	bool Send(const char* message)
	{
		int message_size{ (int)strlen(message) };

		int sent_byte_count{ sendto(m_client_socket, message, message_size, 0,
			(sockaddr*)&m_server_addr, sizeof(m_server_addr)) };

		if (sent_byte_count > 0)
		{
			OutputDebugStringA("Server에 Data를 보내는데 성공함\n");

			return true;
		}

		return false;
	}

private:
	static const int KBuffSize{ 1024 };

private:
	SOCKET m_client_socket{};
	char m_buff[KBuffSize]{};

private:
	sockaddr_in m_server_addr{};
};
