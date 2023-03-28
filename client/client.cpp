#include <iostream>
#include <string>
#include <thread>
#include <sstream>
#include <WS2tcpip.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <stdlib.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mysqlcppconn.lib")

#define MAX_SIZE 1024

using std::cout;
using std::cin;
using std::endl;
using std::string;

SOCKET client_sock;
string input = "";
string user_id = "���̵� : ";
string pw = "��й�ȣ : ";

bool login(string user_id, string pw) {
    try {
        sql::Driver* driver;
        sql::Connection* con;
        sql::PreparedStatement* pstmt;
        sql::ResultSet* res;

        driver = get_driver_instance();
        con = driver->connect("tcp://localhost:3306", "user", "1234"); // DB ���� ���� �Է�
        
        con->setSchema("cpp_db"); // ����� DB �̸� �Է�

        pstmt = con->prepareStatement("SELECT * FROM inventory1 WHERE user_id=\"J\" AND pw = \"123\""); // inventory1 ���̺��� id�� pw�� ��ġ�ϴ� �� �˻�

        res = pstmt->executeQuery();


        string input = "";
        while (res->next())
            printf("Reading from table=(%d, %s, %d)\n",
                res->getInt(1), res->getString(2).c_str(), res->getInt(3));


        if (res) { // ��ġ�ϴ� ���� ������ �α��� ����
            delete res;
            delete pstmt;
            delete con;
            return true;
        }
        else { // ��ġ�ϴ� ���� ������ �α��� ����
            delete res;
            delete pstmt;
            delete con;
            return false;
        }
    }
    catch (sql::SQLException& e) {
        cout << e.what() << endl;
        return false;
    }
}


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
            if (user != user_id) cout << buf << endl;
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
        while (1) {
            cout << " ���̵� �Է� >> ";
            cin >> input;
            user_id += input;
            cout << " ��й�ȣ �Է� >> ";
            cin >> input;
            pw += input;
            if (login(input, pw)) break; // DB���� id�� pw ����
            else {
                user_id = "���̵� : ";
                pw = "��й�ȣ : ";
                cout << "�α��� ����. �ٽ� �Է����ּ���."
                    << endl;
            }
        }
        struct sockaddr_in server_addr;
        client_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (client_sock == INVALID_SOCKET) {
            cout << "socket creation failed." << endl;
            return -1;
        }
        ZeroMemory(&server_addr, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(8000);
        inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

        if (connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            cout << "connection failed." << endl;
            return -1;
        }

        std::thread recv_thread(chat_recv);
        recv_thread.detach();
        string send_msg;
        while (1) {
            getline(cin, send_msg);
            string full_msg = user_id + send_msg;
            send(client_sock, full_msg.c_str(), full_msg.size(), 0);
            if (send_msg == "/quit") break;
        }
        closesocket(client_sock);
        WSACleanup();
        return 0;
    }
    else {
        cout << "WSAStartup failed with error code " << code << endl;
        return -1;
    }
}