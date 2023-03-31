#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>

#include <iostream>
#include <string>
#include <thread>
#include <sstream>
#include <WS2tcpip.h>
#include <conio.h>  // getch 사용하기위해.

#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <stdlib.h>


#define MAX_SIZE 1024

using std::cout;
using std::cin;
using std::endl;
using std::string;

SOCKET client_sock;
string input = "";
string user_id = "아이디 : ";
string pw = "비밀번호 : ";
string login_info = "";

bool login(string user_id, string pw) {
    try {
        sql::Driver* driver;
        sql::Connection* con;
        sql::PreparedStatement* pstmt;
        sql::ResultSet* res;    

        // MySQL 데이터베이스 값을 담아두기 위한 용도
        int table_id = 0;   
        string table_user_id = "";
        int table_pw = 0;

        bool isLoginInfoValid = false;

        driver = get_driver_instance();
        con = driver->connect("tcp://localhost:3306", "user", "1234"); // DB 접속 정보 입력

        con->setSchema("cpp_db"); // 사용할 DB 이름 입력
        pstmt = con->prepareStatement("SELECT * FROM inventory1"); // inventory1 테이블에서 id와 pw가 일치하는 행 검색

        res = pstmt->executeQuery();

        while (res->next()) {
            table_id = res->getInt("id");
            table_user_id = res->getString("user_id");
            table_pw = res->getInt("pw");
            int int_pw = std::stoi(pw); //형변환 str -> int
            if (user_id == table_user_id && int(int_pw) == table_pw) { // 일치하는 행이 있으면 로그인 성공
                isLoginInfoValid = true;
                break;
            }
            else  // 일치하는 행이 없으면 로그인 실패
                isLoginInfoValid = false;
        }
        delete res;
        delete pstmt;
        delete con;

        return isLoginInfoValid;
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

            if (user != user_id)
                cout << buf << endl;
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
        // 아이디, 비밀번호 입력부
        while (1) {
            cout << " 아이디 입력 >> ";
            cin >> input;

            login_info += input;
            string login_id = input;

            cout << " 비밀번호 입력 >> ";

            // 키보드 입력 받기
            string password;
            char ch;
            while ((ch = _getch()) != 13) { // 아스키코드 13으로 값을 받기 전까지 작동 (Enter키)
                if (ch == 8) { // 8 = 백스페이스
                    if (!password.empty()) {
                        password.pop_back(); // 마지막 입력된 문자 삭제
                        std::cout << "\b \b"; // 화면에 * 삭제
                    }
                }
                else if (ch >= 32 && ch <= 126) { // 키입력
                    password += ch;   // password에 문자 입력
                    std::cout << "*"; // 문자대신 * 출력
                }
            }

            cout << std::endl;
            cout << " 입력한 비밀번호 : " << password << endl;
            login_info += "|" + password;
            string login_pw = password;

            if (login(login_id, login_pw)) {// DB에서 id와 pw 검증
                user_id = login_id;
                break;
            }
            else {
                cout << "\n로그인 실패. 다시 입력해주세요." << endl;
                login_info = "";
            }
        }

        // 소켓 생성
        client_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
         //client_sock = socket(AF_INET, SOCK_STREAM, 6);
        
        SOCKADDR_IN client_addr = {};
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons(7777);
        InetPton(AF_INET, TEXT("127.0.0.1"), &client_addr.sin_addr);
         //inet_pton(AF_INET, "127.0.0.1", &client_addr.sin_addr);

        if (client_sock == INVALID_SOCKET) {    // socket() 함수 실행시 오류로 소켓이 생성되지 않으면 -1 반환
            cout << " 소켓 생성 실패. " << endl;
            cout << "errorcode : " << WSAGetLastError() << endl;
            return -1;
        }
       
        //socket_error가 발생했을 때 반환하는 함수는 WSAGetLastError()
        //가장 최근의 소켓 에러 코드 값을 반환하며, 반환 값은 int 자료형


        // connect() 함수 실행시 오류 발생하면 -1 반환, 접속 하면 0 반환
        while (1) {
            if (!connect(client_sock, (SOCKADDR*)&client_addr, sizeof(client_addr))) {	// 정상 연결 되면 0반환
                send(client_sock, login_info.c_str(), login_info.length(), 0);	// 닉네임 전송
                break;
            }
            cout << " 연결 실패" << endl;
            cout << "errorcode : " << WSAGetLastError() << endl;
            return -1;
        }

        std::thread recv_thread(chat_recv);
        recv_thread.detach();
        while (1) {
            string text;
            getline(cin, text);
            const char* buffer = text.c_str();
            send(client_sock, text.c_str(), text.size(), 0);
            if (text == "/quit") break; // 클라이언트 부에서 탈출
        }

        //recv_thread.join();
        closesocket(client_sock);

        WSACleanup();   // Winsock DLL을 종료하는 함수
        return 0;
    }
    else {
        cout << "WSAStartup 오류 \nerrorcode : " << code << endl;
        return -1;
    }

}