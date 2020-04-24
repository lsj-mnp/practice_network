#pragma once

#include <WinSock2.h>
#pragma comment (lib, "ws2_32.lib")
#include <WS2tcpip.h>
#include <iostream>
#include <vector>
#include <string>
#include <mutex>
#include <queue>

class CUDPServer
{
public:
	CUDPServer()
	{
		StartUp();
		CreateSocket(); 
		BindSock();
		SetHostAddr();
	}
	~CUDPServer() { CleanUp(); }

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

	bool CreateSocket()
	{
		m_server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		//소켓의 에러코드를 확인해주는 매크로
		if (m_server_socket == INVALID_SOCKET)
		{
			//가장 마지막에 발생한 에러코드를 리턴해주는 함수
			printf("에러코드: %d", WSAGetLastError());

			return false;
		}

		return true;
	}

	void SetHostAddr()
	{
		char host_name[256]{};

		//이 함수에서 원하는 것은 배열의 크기임. 바이트크기가 아님.
		//따라서 sizeof를 쓸 경우 자료형의 바이트크기로 나눠줘야 함.
		if (gethostname(host_name, 256) == SOCKET_ERROR)
		{
			printf("에러코드: %d", WSAGetLastError());

			return;
		}

		ADDRINFOA hint{};
		hint.ai_family = PF_INET;

		ADDRINFOA* result{};

		//node = 네트워크에서 각각의 단말기를 의미.
		//pp = 동적할당 한다는 의미.
		//동적할당 = 미리 만들어둔 공간(stack)에 new로
		//새로 만들어진 공간(heap = 동적할당하는 공간)의 주소를 할당함.
		int error{ getaddrinfo(host_name, nullptr, &hint, &result) };

		if (error)
		{
			printf("에러코드: %d", error);

			return;
		}

		memcpy(&m_host_addr, result->ai_addr, sizeof(m_host_addr));

		char ip[16]{};

		inet_ntop(AF_INET, &m_host_addr.sin_addr, ip, 16);

		m_host_ip = ip;

		//상기 함수(getaddrinfo)에서 result를 동적할당 하므로, 해제(delete 같은 것) 해줘야 함.
		//delete = 동적할당으로 접근권한을 받고
		//사용중이던 메모리 공간의 특정 부분에 대한 접근권한을 해제함.
		freeaddrinfo(result);
	}

	bool BindSock()
	{
		sockaddr_in sock_addr{};
		sock_addr.sin_family = AF_INET;
		//0~1024번 포트까지는 예약되어있음. 우리가 임의로 못 씀.
		//윈도우즈는 little endian, 네트워크는 big endian. 바꿔줘야 함.
		sock_addr.sin_port = htons(KServicePort);
		//b = byte, w = word
		//특정 ip주소에만 국한하지 않음.
		/*sock_addr.sin_addr.S_un.S_un_b.s_b1 = 192;
		sock_addr.sin_addr.S_un.S_un_b.s_b2 = 168;
		sock_addr.sin_addr.S_un.S_un_b.s_b3 = 35;
		sock_addr.sin_addr.S_un.S_un_b.s_b4 = 229;*/
		//그냥 0임. 의미부여를 위해 매크로로 지정해둠.
		//만약 ip주소가 들어가야되면 htonl을 써줘야됨.
		sock_addr.sin_addr.S_un.S_addr = INADDR_ANY;

		//강제 형변환. & = 참조. 주소를 받아옴. * = 포인터. 주소를 저장함.
		int error{ bind(m_server_socket, (sockaddr*)&sock_addr, sizeof(sock_addr)) };

		if (error == SOCKET_ERROR)
		{
			printf("에러코드: %d", error);

			return false;
		}
		
		return true;
	}

	bool CleanUp()
	{
		int close_sock_error{ closesocket(m_server_socket) };

		if (close_sock_error == SOCKET_ERROR)
		{
			printf("에러코드: %d", WSAGetLastError());
		}

		//직접 호출을 안 해도 프로세스가 종료되면 윈도우즈가 알아서 호출해줌.
		//cleanup을 안 하고 이후 startup을 호출하면 안됨.
		int error{ WSACleanup() };

		if (error != 0)
		{
			printf("에러코드: %d", error);

			return false;
		}

		return true;
	}

public:
	const std::string& GetHostIP() const
	{
		return m_host_ip;
	}

	const char* GetBuff() const
	{
		return m_buff;
	}

	u_short GetServicePort() const
	{
		return KServicePort;
	}

	bool Receive()
	{
		sockaddr_in client_addr{};
		int client_addr_size{ sizeof(client_addr) };

		//몇 바이트를 받았는지 리턴함.
		int received_byte_count{ recvfrom(m_server_socket, m_buff,
			KBuffSize, 0, (sockaddr*)&client_addr, &client_addr_size) };

		if (received_byte_count > 0)
		{
			for (const auto& client : m_v_clients)
			{
				//중복 ip 주소 확인.
				if (client.sin_addr.S_un.S_addr == client_addr.sin_addr.S_un.S_addr)
				{
					//중복이다.
					return true;
				}
			}

			//std::mutex::lock() = lock이 가능할 때까지 계속 대기함.
			//dead lock = 대기하는 lock이 중복 발생할 경우 무한 대기상태가 됨.
			
			//m_mtx.lock();

			//m_v_clients.emplace_back(client_addr);
			m_q_joinningClientsAddr.push(client_addr);

			//m_mtx.unlock();

			return true;
		}
		
		return false;
	}

	//함수가 const일 때는 const가 아닌 함수를 호출을 못 함.
	bool SendToAll(const char* message)
	{
		int message_size{ (int)strlen(message) };

		/*strlen =
		while (message[message_size] != '/0')
		{
			++message_size;
		}*/

		bool failed{};

		while (m_q_joinningClientsAddr.size() > 0)
		{
			//참조형은 원래 참조를 못 바꿈. 
			const sockaddr_in& joined_client_addr = m_q_joinningClientsAddr.front();

			m_v_clients.emplace_back(joined_client_addr);
			
			m_q_joinningClientsAddr.pop();
		}

		//여러 쓰레드가 1. 공통된 공간에 2. 동시에 3. 한 번이라도 쓰기연산을 할 경우에만 race condition
		//m_mtx.lock();

		for (const auto& client : m_v_clients)
		{
			int sent_byte_count{ sendto(m_server_socket, message,
				message_size, 0, (sockaddr*)&client, sizeof(client)) };

			if (sent_byte_count > 0)
			{
				failed = true;
			}
		}

		//m_mtx.unlock();

		return failed;
	}

private:
	static constexpr int KBuffSize{ 1024 };
	//sevice = 기능
	static const u_short KServicePort{ 15000 };

private:
	SOCKET m_server_socket{};
	char m_buff[KBuffSize]{};
	sockaddr_in m_host_addr{};
	std::string m_host_ip{};

private:
	//shift+f12 == 사용된 곳을 띄워줌.
	std::mutex m_mtx{};

	//임계구역
private:
	std::vector<sockaddr_in> m_v_clients{};
	std::queue<sockaddr_in> m_q_joinningClientsAddr{};
};
