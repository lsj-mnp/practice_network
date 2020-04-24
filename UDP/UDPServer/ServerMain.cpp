#include "CUDPServer.h"

int main()
{
	CUDPServer server{};

	//ip주소 첫 자리 = 0~ 클래스A, 128~ 클래스B, 192~ 클래스C, 224~ 클래스D, 240~ 클래스E
	//ip주소 끝 자리 = 기기별 주소
	//255.255.255.255 = 방송용 주소.
	//마스크 = 필터 같은 것. 0인 부분은 막음. 1인 부분만 출력.
	//서브넷 마스크 = 네트워크 구분용
	//기본 게이트웨이 = 엑세스포인터 (ap. 보통 공유기의 주소)
	//도메인 네임 = www.blabla.etc = 그냥 특정 ip주소에 대응되는 문자열일 뿐임.(DNS)

	//printf("이름: %s 나이: %d ", "문자열", 9);
	printf("Server Address: %s:%d\n", server.GetHostIP().c_str(), server.GetServicePort());

	std::thread thrReceive
	{
		[&]
		{
			while (true)
			{
				if (server.Receive())
				{
					printf("%s\n", server.GetBuff());
				}
			}
		}
	};

	char sendMessage[5000]{};

	while (true)
	{
		fgets(sendMessage, 5000, stdin);

		server.SendToAll(sendMessage);
	}

	thrReceive.join();

	return 0;
}