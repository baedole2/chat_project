#pragma comment (lib, "ws2_32.lib")
// pragmae comment 랑 없는거랑 무슨차이

#include <iostream>
#include <string>
#include <thread>
#include <WinSock2.h>	// 윈도우 소켓 라이브러리
#include <sstream>
#include <WS2tcpip.h>

#define MAX_SIZE 1024
#define MAX_CLIENT 3

using std::cout;
using std::cin;
using std::endl;
using std::string;

SOCKET client_sock;
string my_nick;

int chat_recv() {
	char buf[MAX_SIZE] = {};
	string msg;
	while (1) {
		ZeroMemory(&buf, MAX_SIZE);
		if (recv(client_sock, buf, MAX_SIZE, 0) > 0) {
			msg = buf;
			string user;
			std::stringstream ss(msg);	// msg 에 담긴 문자를 stream(01010 로 바꾸는거) 화
			ss >> user;

			if (user != my_nick)
				cout << buf << endl;
		}
		else {
			cout << "Server OFF!" << endl;
			return -1;
		}
	}
}

int main() {
	WSADATA wsa;
	int code = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (!code) {
		cout << "사용할 닉네임 입력 >> ";
		cin >> my_nick;

		client_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

		SOCKADDR_IN client_addr = {};
		client_addr.sin_family = AF_INET;
		client_addr.sin_port = htons(7777);
		InetPton(AF_INET, TEXT("127.0.0.1"), &client_addr.sin_addr);
		//client_addr.sin_addr.s_addr = htonl(INADDR_ANY);

		while (1) {
			if (!connect(client_sock, (SOCKADDR*)&client_addr, sizeof(client_addr))) {	// 정상 연결 되면 0반환
				send(client_sock, my_nick.c_str(), my_nick.length(), 0);	// 닉네임 전송
				break;
			}
			cout << "connecting..." << endl;
		}
		std::thread th2(chat_recv);

		while (1) {
			string text;
			std::getline(cin, text);
			const char* buffer = text.c_str();
			//cout << my_nick << " : ";	// 여기 바꿈
			send(client_sock, buffer, strlen(buffer), 0);
		}

		th2.join();
		closesocket(client_sock);
	}

	WSACleanup();
	return 0;
}