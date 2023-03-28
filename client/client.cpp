#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <iostream>
#include <string>
#include <thread>
#include <sstream>
#include <WS2tcpip.h>

#include "mysql_connection.h"
#include <cppconn/driver.h>	// 이게 왜 오류가 뜨지.
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/statement.h> // MySQL 데이터 한글 깨짐 방지

#define MAX_SIZE 1024

using std::cout;
using std::cin;
using std::endl;
using std::string;

SOCKET client_sock;
string input = "";
string message = "";
string my_nick = "";
string user_id = "아이디 : ";
string user_pw = "비밀번호 : ";

int chat_recv() {
    char buf[MAX_SIZE] = {};
    string msg;
    while (1) {
        ZeroMemory(&buf, MAX_SIZE);
        if (recv(client_sock, buf, MAX_SIZE, 0) > 0) {
            msg = buf;
            string user;
            std::stringstream ss(msg);
            ss >> user;
            if (user != my_nick) cout << buf << endl;
        }
        else {
            cout << "Server Off!" << endl;
            return -1;
        }
    }
}

int main() {
    WSADATA wsa;
    int code = WSAStartup(MAKEWORD(2, 2), &wsa);

    if (!code) {
        cout << " 사용할 닉네임 입력 >>";
        cin >> input;
        //my_nick = input;
        message += input + "|";

        cout << " 아이디 입력 >> ";
        cin >> input;
        //user_id += input;
        message += input + "|";
        
        cout << " 비밀번호 입력 >> ";
        cin >> input;
        //user_pw += input;
        message += input + "|";
     
        client_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        SOCKADDR_IN client_addr = {};
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons(7777);
        InetPton(AF_INET, TEXT("127.0.0.1"), &client_addr.sin_addr);
        while (1) {
            if (!connect(client_sock, (SOCKADDR*)&client_addr, sizeof(client_addr))) {
                cout << "Server Connect" << endl;
                //send(client_sock, my_nick.c_str(), my_nick.length(), 0);
                //send(client_sock, user_id.c_str(), user_id.length(), 0);
                //send(client_sock, user_pw.c_str(), user_pw.length(), 0);
                send(client_sock, message.c_str(), user_pw.length(), 0);
                break;
            }
            cout << "connecting..." << endl;
        }
        std::thread th2(chat_recv);
        while (1) {
            string text;
            std::getline(cin, text);
            const char* buffer = text.c_str();
            send(client_sock, buffer, strlen(buffer), 0);
        }
        th2.join();
        closesocket(client_sock);
    }
    WSACleanup();
    return 0;
}