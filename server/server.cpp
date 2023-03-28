#pragma comment (lib, "ws2_32.lib")	// ���̺귯�� ȣ��

#include <string>
#include <vector>
#include <thread>
#include <WinSock2.h>	// ������ ���� ���̺귯��

#include <stdlib.h>
#include <iostream>

#include "mysql_connection.h"
#include <cppconn/driver.h>	// �̰� �� ������ ����.
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/statement.h> // MySQL ������ �ѱ� ���� ����

using std::cout;
using std::cin;
using std::string;
using std::endl;
using std::vector;
using std::thread;

//for demonstration only. never save your password in the code!
const string server = "tcp://127.0.0.1:3306"; // �����ͺ��̽� �ּ�
const string username = "user"; // �����ͺ��̽� �����
const string password = "1234"; // �����ͺ��̽� ���� ��й�ȣ
const string port = "3306";	// mysql ��Ʈ��ȣ

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
	// winsock version 2.2 ���
	// winsock �ʱ�ȭ �ϴ� �Լ�
	// ���� ������ 0 ��ȯ, �����ϸ� 0 �̿��� �� ��ȯ

	if (!code) {	// 0 = false. �����ߴٸ� 0�� ���Ƿ� !0 �� true
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

// ���� �Լ� ������

// 1. ���� �ʱ�ȭ
// socket(), bind(), listen();
// ������ ����, �ּҸ� ����, Ȱ��ȭ -> ������
void server_init() {
	server_sock.sck = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// socket(int AF, int type, int PROTOCOL)
	// - �ּ� ü�� ����
	// - ��� Ÿ�� ����
	// - �������� ����
	SOCKADDR_IN server_addr = {};
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(7777);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	
	bind(server_sock.sck, (sockaddr*)&server_addr, sizeof(server_addr));
	// bind(SOCKET s, const sockaddr *name, int namelen)
	// - socket()���� ������� ����
	// - ���ϰ� ������ �ּ� ������ ��� �ִ� ����ü
	//	(�ڵ忡���� server_addr)
	// - �ι�° �Ű������� ũ��

	listen(server_sock.sck, SOMAXCONN);
	// listen(SOCKET s, int backlog)

	server_sock.user = "server";

	// ���� ������ ���� ����
	cout << "server ON!" << endl;
}

// 2. Ŭ���̾�Ʈ ���� & �߰�
// accept() recv()
// ���� ����, Ŭ���̾�Ʈ�� ������ �г����� ����
void add_client() {
	SOCKADDR_IN addr = {};
	int addrsize = sizeof(addr);
	char buf[MAX_SIZE] = {};

	ZeroMemory(&addr, addrsize); // addr 0 ���� �ʱ�ȭ

	SOCKET_INFO new_client = {};

	new_client.sck = accept(server_sock.sck, (sockaddr*)&addr, &addrsize);
	
	// accept(SOCKET s, sockaddr* addr, &addrsize);
	// - socket() ���� ������� ���� = �޾ƿ��� ����
	// - client�� �ּ� ������ ������ ����ü
	// - 2�� �Ű������� ũ��

	recv(new_client.sck, buf, MAX_SIZE, 0);
	
	// recv(SOCKET s, const char *buf, int len, int flags)
	// - accept() ���� ������� ����
	// - �����͸� ���� ����
	// - �ι�° �Ű������� ����
	// - flag, �Լ��� ���ۿ� ����. �ΰ� �ɼ�
	buf;

	new_client.user = string(buf);
	// buf�� string���� ��ȯ�ؼ� ����

	string msg = "[����]" + new_client.user + "���� �����߽��ϴ�!";
	cout << msg << endl;

	sck_list.push_back(new_client);
	
	std::thread th(recv_msg, client_count);
	
	client_count++;
	cout << "[����] ���� ������ �� : " << client_count << "��" << endl;
	send_msg(msg.c_str());	// c_str() -> string�� const char�� �������ִ� �Լ�
	th.join();
}

// 3. Ŭ���̾�Ʈ���� msg ������
// send()
void send_msg(const char* msg) {
	for (int i = 0; i < client_count; i++) {
		send(sck_list[i].sck, msg, MAX_SIZE, 0);
	}
}

// 4. Ŭ���̾�Ʈ���� ä�� ������ ���� + ���� ������ ������ ���̽��� �����Ŵ.
// �����ߴٸ� �����߽��ϴ� ���� ���
void recv_msg(int idx) {
	char buf[MAX_SIZE] = {};
	string msg = "";
	while (1) {
		ZeroMemory(&buf, MAX_SIZE);
		if (recv(sck_list[idx].sck, buf, MAX_SIZE, 0) > 0) {
			// ���������� ������ �޾������� ó��
			msg = sck_list[idx].user + " : " + buf;
			cout << msg << endl;
			send_msg(msg.c_str());

			// ������ ���̽� �����ϴ� ��ġ
			if (1)
			{
				sql::Driver* driver;
				sql::Connection* con;
				sql::PreparedStatement* pstmt;
				sql::Statement* stmt;	// MySQL ������ �ѱ� ���� ����

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

				// connector���� �ѱ� ����� ���� ����
				stmt = con->createStatement();
				stmt->execute("set names euckr");	//euc-kr ���ڵ�
				if (stmt) { delete stmt; stmt = nullptr; }

				stmt = con->createStatement();


				pstmt = con->prepareStatement("INSERT INTO chat_data (date, id, nickname, content) VALUES(? ,? ,? ,?)");

				pstmt->setString(1, "1991-01-02"/*ä�� �Է��� ��¥*/);
				pstmt->setInt(2, 2 /* ȸ�� ��ȣ */);
				pstmt->setString(3, sck_list[idx].user);
				pstmt->setString(4, msg.c_str());
				pstmt->execute();
				cout << "ä�� ���� ����" << endl;

				delete pstmt;
				delete con;
			}
			
		}
		else {
			// ���� ������ ������ ����
			msg = "[����]" + sck_list[idx].user + "���� �����߽��ϴ�.";
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