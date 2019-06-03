#define _WINSOCK_DEPCRECATED 

#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <list>
#include <iostream>

#define SERVERPORT 9000
#define BUFSIZE    512

#define CHATROOM_SIZE 2
#define ENTERANCE_USER 20

using namespace std;

int nTotalSockets = 0;

list<string> userList[CHATROOM_SIZE];

// 소켓 정보 저장을 위한 구조체와 변수
struct SOCKETINFO
{
	SOCKET sock;
	char buf[BUFSIZE + 1];
	int recvbytes;
	int sendbytes;
};

SOCKETINFO *SocketInfoArray[FD_SETSIZE];

// 소켓 관리 함수
BOOL AddSocketInfo(SOCKET sock);
void RemoveSocketInfo(int nIndex);

// 오류 출력 함수
void err_quit(const char *msg);
void err_display(const char *msg);

//데이터 구분해주는 함수
string DataDivision(string input, int value);

//이미 존재하는 사용자인지 검사하는 함수
bool IsExistName(list<string> userList, string name);

//사용자 목록 반환하는 함수
string ReturnChatUser();

int main(int argc, char *argv[])
{
	int retval;
	
	userList[0].push_back("세영이");
	userList[1].push_back("돼지뚱");
	userList[0].push_back("seyoung");
	userList[0].push_back("dain");

	printf("\n====================== 서버 구동 시작 ======================\n");

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 넌블로킹 소켓으로 전환
	u_long on = 1;
	retval = ioctlsocket(listen_sock, FIONBIO, &on);
	if (retval == SOCKET_ERROR) err_display("ioctlsocket()");

	// 데이터 통신에 사용할 변수
	FD_SET rset, wset;
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen, i, j;
	string data, num, name, message;
	int roomNumber;

	while (1) {
		
		// 소켓 셋 초기화
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		FD_SET(listen_sock, &rset);
		for (i = 0; i < nTotalSockets; i++) {
			if (SocketInfoArray[i]->recvbytes > SocketInfoArray[i]->sendbytes)
				FD_SET(SocketInfoArray[i]->sock, &wset);
			else
				FD_SET(SocketInfoArray[i]->sock, &rset);
		}

		// select()
		retval = select(0, &rset, &wset, NULL, NULL);
		if (retval == SOCKET_ERROR) err_quit("select()");

		// 소켓 셋 검사(1): 클라이언트 접속 수용
		if (FD_ISSET(listen_sock, &rset)) {
			addrlen = sizeof(clientaddr);
			client_sock = accept(listen_sock, (SOCKADDR *)&clientaddr, &addrlen);
			if (client_sock == INVALID_SOCKET) {
				err_display("accept()");
			}
			else {

				printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
				// 소켓 정보 추가
				AddSocketInfo(client_sock);
			}
		}

		// 소켓 셋 검사(2): 데이터 통신
		for (i = 0; i < nTotalSockets; i++) {
			SOCKETINFO *ptr = SocketInfoArray[i];
			if (FD_ISSET(ptr->sock, &rset)) {
				// 데이터 받기
				retval = recv(ptr->sock, ptr->buf, BUFSIZE, 0);
				if (retval == SOCKET_ERROR) {
					err_display("recv()");
					RemoveSocketInfo(i);
					continue;
				}
				else if (retval == 0) {
					RemoveSocketInfo(i);
					continue;
				}
				ptr->recvbytes = retval;
				// 받은 데이터 출력
				addrlen = sizeof(clientaddr);
				getpeername(ptr->sock, (SOCKADDR *)&clientaddr, &addrlen);
				ptr->buf[retval] = '\0';
				printf("[TCP/%s:%d] %s\n", inet_ntoa(clientaddr.sin_addr),
					ntohs(clientaddr.sin_port), ptr->buf);
			}

			if (FD_ISSET(ptr->sock, &wset)) {
				data = (string)(ptr->buf);
				num = DataDivision(data, 1);
				printf("data : %s , num : %s \n", data.c_str(), num.c_str());
				if (num == "0") {
					roomNumber = stoi(DataDivision(data, 2)) - 1;
					bool check = IsExistName(userList[roomNumber], DataDivision(data, 3));
					if (check) {
						strcpy(ptr->buf, "Exist");
					}
					else {
						strcpy(ptr->buf, "Not Exist");
					}
					retval = send(ptr->sock, ptr->buf + ptr->sendbytes,
						ptr->recvbytes - ptr->sendbytes, 0);

					if (retval == SOCKET_ERROR) {
						err_display("send()");
						RemoveSocketInfo(i);
						continue;
					}
				}
				else if (num == "1" || num == "2") {
					name = DataDivision(data, 2);
					message = DataDivision(data, 3);
					string send_message = num + "," + "[" + name + "]" + " : " + message;

					strcpy(ptr->buf, send_message.c_str());

					for (j = 0; j < nTotalSockets; j++) {  // 여러 접속자에게 발송
						SOCKETINFO *sptr = SocketInfoArray[j];
						retval = send(sptr->sock, ptr->buf + ptr->sendbytes,
							ptr->recvbytes - ptr->sendbytes, 0);

						if (retval == SOCKET_ERROR) {
							err_display("send()");
							RemoveSocketInfo(i);
							continue;
						}
					}
				}
				else if (num == "3") {
					strcpy(ptr->buf, ReturnChatUser().c_str());
					printf("%s\n", ReturnChatUser().c_str());
					retval = send(ptr->sock, ptr->buf + ptr->sendbytes,
						ptr->recvbytes - ptr->sendbytes, 0);

					if (retval == SOCKET_ERROR) {
						err_display("send()");
						RemoveSocketInfo(i);
						continue;
					}
				}

				
				ptr->sendbytes += retval;
				if (ptr->recvbytes == ptr->sendbytes) {
					ptr->recvbytes = ptr->sendbytes = 0;
				}
			}
		}
	}

	// 윈속 종료
	WSACleanup();
	return 0;
}
	


// 소켓 정보 추가
BOOL AddSocketInfo(SOCKET sock)
{
	if (nTotalSockets >= FD_SETSIZE) {
		printf("[오류] 소켓 정보를 추가할 수 없습니다!\n");
		return FALSE;
	}

	SOCKETINFO *ptr = new SOCKETINFO;
	if (ptr == NULL) {
		printf("[오류] 메모리가 부족합니다!\n");
		return FALSE;
	}

	ptr->sock = sock;
	ptr->recvbytes = 0;
	ptr->sendbytes = 0;
	SocketInfoArray[nTotalSockets++] = ptr;

	return TRUE;
}

// 소켓 정보 삭제
void RemoveSocketInfo(int nIndex)
{
	SOCKETINFO *ptr = SocketInfoArray[nIndex];

	// 클라이언트 정보 얻기
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(ptr->sock, (SOCKADDR *)&clientaddr, &addrlen);
	printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
		inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

	closesocket(ptr->sock);
	delete ptr;

	if (nIndex != (nTotalSockets - 1))
		SocketInfoArray[nIndex] = SocketInfoArray[nTotalSockets - 1];

	--nTotalSockets;
}

// 소켓 함수 오류 출력 후 종료
void err_quit(const char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// 소켓 함수 오류 출력
void err_display(const char *msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	printf("[%s] %s", msg, (char *)lpMsgBuf);
	LocalFree(lpMsgBuf);
}


string DataDivision(string input, int value) {
	size_t pos = 0;
	string inputValue, returnValue;
	inputValue = input;
	for (int i = 0 ; i < value; i++) {
		pos = inputValue.find(",");
		returnValue = inputValue.substr(0, pos);
		inputValue = inputValue.erase(0, pos + 1);
	}
	return returnValue;
}

//이미 존재하는 사용자인지 검사하는 함수
bool IsExistName(list<string> user_list,string name) {
	list<string>::iterator iter;
	for (iter = user_list.begin(); iter != user_list.end(); iter++) {
		if ((string)(*iter) == name) {
			return true;
		}
	}
	return false;
}

//사용자 목록 반환하는 함수
string ReturnChatUser() {
	string userlist = "3,";
	int count = 0;
	list<string>::iterator iter;
	for (int i = 0; i < CHATROOM_SIZE; i++) {
		for (iter = userList[i].begin(); iter != userList[i].end(); ++iter) {
			count++;
			userlist += to_string(count) + "." + (string)(*iter) + ",";
		}
	}
	return userlist;
}
