#pragma comment (lib, "ws2_32.lib")	// 라이브러리 호출

#include <string>
#include <vector>
#include <thread>
#include <WinSock2.h>	// 윈도우 소켓 라이브러리

#include <stdlib.h>
#include <iostream>

#include "mysql_connection.h"
#include <cppconn/driver.h>	// 이게 왜 오류가 뜨지.
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/statement.h> // MySQL 데이터 한글 깨짐 방지

using std::cout;
using std::cin;
using std::string;
using std::endl;
using std::vector;
using std::thread;

//for demonstration only. never save your password in the code!
const string server = "tcp://127.0.0.1:3306"; // 데이터베이스 주소
const string username = "user"; // 데이터베이스 사용자
const string password = "1234"; // 데이터베이스 접속 비밀번호
const string port = "3306";	// mysql 포트번호

#define MAX_SIZE 1024
#define MAX_CLIENT 3

struct SOCKET_INFO {
	SOCKET sck;
	string user;
};

vector<SOCKET_INFO> sck_list;
SOCKET_INFO server_sock;

int client_count = 0;

void server_init();
void add_client();
void send_msg(const char* msg);
void recv_msg(int idx);
void del_client(int idx);

int main() {
	WSADATA wsa;

	int code = WSAStartup(MAKEWORD(2, 2), &wsa);
	// winsock version 2.2 사용
	// winsock 초기화 하는 함수
	// 실행 성공시 0 반환, 실패하면 0 이외의 값 반환

	if (!code) {	// 0 = false. 성공했다면 0이 들어가므로 !0 은 true
		server_init();

		thread th1[MAX_CLIENT];
		for (int i = 0; i < MAX_CLIENT; i++) {
			th1[i] = std::thread(add_client);
		}

		while (1) {
			string text, msg = "";
			std::getline(cin, text);
			const char* buf = text.c_str();

			msg = server_sock.user + " : " + buf;

			send_msg(msg.c_str());
		}

		for (int i = 0; MAX_CLIENT; i++) {
			th1[i].join();
		}

		closesocket(server_sock.sck);

		WSACleanup();
		return 0;
	}
}

// 이하 함수 구현부

// 1. 소켓 초기화
// socket(), bind(), listen();
// 소켓을 생성, 주소를 묶음, 활성화 -> 대기상태
void server_init() {
	server_sock.sck = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// socket(int AF, int type, int PROTOCOL)
	// - 주소 체계 형식
	// - 통신 타입 설정
	// - 프로토콜 지정
	SOCKADDR_IN server_addr = {};
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(7777);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	
	bind(server_sock.sck, (sockaddr*)&server_addr, sizeof(server_addr));
	// bind(SOCKET s, const sockaddr *name, int namelen)
	// - socket()으로 만들어준 소켓
	// - 소켓과 연결할 주소 정보를 담고 있는 구조체
	//	(코드에서는 server_addr)
	// - 두번째 매개변수의 크기

	listen(server_sock.sck, SOMAXCONN);
	// listen(SOCKET s, int backlog)

	server_sock.user = "server";

	// 서버 켜지면 나올 문구
	cout << "server ON!" << endl;
}

// 2. 클라이언트 연결 & 추가
// accept() recv()
// 연결 설정, 클라이언트가 전송한 닉네임을 받음
void add_client() {
	SOCKADDR_IN addr = {};
	int addrsize = sizeof(addr);
	char buf[MAX_SIZE] = {};

	ZeroMemory(&addr, addrsize); // addr 0 으로 초기화

	SOCKET_INFO new_client = {};

	new_client.sck = accept(server_sock.sck, (sockaddr*)&addr, &addrsize);
	
	// accept(SOCKET s, sockaddr* addr, &addrsize);
	// - socket() 으로 만들어준 소켓 = 받아오는 소켓
	// - client의 주소 정보를 저장할 구조체
	// - 2번 매개변수의 크기

	recv(new_client.sck, buf, MAX_SIZE, 0);
	
	// recv(SOCKET s, const char *buf, int len, int flags)
	// - accept() 에서 만들어준 소켓
	// - 데이터를 받을 변수
	// - 두번째 매개변수의 길이
	// - flag, 함수의 동작에 영향. 부가 옵션
	buf;

	new_client.user = string(buf);
	// buf를 string으로 변환해서 담음

	string msg = "[공지]" + new_client.user + "님이 입장했습니다!";
	cout << msg << endl;

	sck_list.push_back(new_client);
	
	std::thread th(recv_msg, client_count);
	
	client_count++;
	cout << "[공지] 현재 접속자 수 : " << client_count << "명" << endl;
	send_msg(msg.c_str());	// c_str() -> string을 const char로 변경해주는 함수
	th.join();
}

// 3. 클라이언트에게 msg 보내기
// send()
void send_msg(const char* msg) {
	for (int i = 0; i < client_count; i++) {
		send(sck_list[i].sck, msg, MAX_SIZE, 0);
	}
}

// 4. 클라이언트에게 채팅 내용을 받음 + 받은 내용을 데이터 베이스에 저장시킴.
// 퇴장했다면 퇴장했습니다 공지 띄움
void recv_msg(int idx) {
	char buf[MAX_SIZE] = {};
	string msg = "";
	while (1) {
		ZeroMemory(&buf, MAX_SIZE);
		if (recv(sck_list[idx].sck, buf, MAX_SIZE, 0) > 0) {
			// 정상적으로 정보를 받았을때의 처리
			msg = sck_list[idx].user + " : " + buf;
			cout << msg << endl;
			send_msg(msg.c_str());

			// 데이터 베이스 저장하는 위치
			if (1)
			{
				sql::Driver* driver;
				sql::Connection* con;
				sql::PreparedStatement* pstmt;
				sql::Statement* stmt;	// MySQL 데이터 한글 깨짐 방지

				try
				{
					driver = get_driver_instance();
					con = driver->connect(server, username, password);
				}
				catch (sql::SQLException e)
				{
					cout << "Could not connect to server. Error message: " << e.what() << endl;
					system("pause");
					exit(1);
				}

				//please create database "quickstartdb" ahead of time
				con->setSchema("cpp_db_chat");

				// connector에서 한글 출력을 위한 셋팅
				stmt = con->createStatement();
				stmt->execute("set names euckr");	//euc-kr 인코딩
				if (stmt) { delete stmt; stmt = nullptr; }

				stmt = con->createStatement();


				pstmt = con->prepareStatement("INSERT INTO chat_data (date, id, nickname, content) VALUES(? ,? ,? ,?)");

				pstmt->setString(1, "1991-01-02"/*채팅 입력한 날짜*/);
				pstmt->setInt(2, 2 /* 회원 번호 */);
				pstmt->setString(3, sck_list[idx].user);
				pstmt->setString(4, msg.c_str());
				pstmt->execute();
				cout << "채팅 내역 저장" << endl;

				delete pstmt;
				delete con;
			}
			
		}
		else {
			// 무언가 오류가 생겼을 떄에
			msg = "[공지]" + sck_list[idx].user + "님이 퇴장했습니다.";
			cout << msg << endl;
			send_msg(msg.c_str());
			del_client(idx);
			return;
		}

	}
}


void del_client(int idx) {
	closesocket(sck_list[idx].sck);
	client_count--;
}