//2019년 1학기 네트워크프로그래밍 숙제 3번
//성명: 전세영 학번: 14011024
//플랫폼: V

#define _CRT_SECURE_NO_WARNINGS         // 최신 VC++ 컴파일 시 경고 방지
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib, "ws2_32")

#ifndef _WINSOCK2API_
#include <WINSOCK2.H>
#include <windows.h>
#endif

#include <stdio.h>
#include "resource.h"
#include <regex>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <ctime>
#include <string.h>

#define IPADDRESS_SIZE 15
#define CHATNAME_SIZE 10
#define SENDMESSAGE_SIZE 50
#define PORT_SIZE 5
#define ROOMNAME_SIZE 10
#define MESSAGE_SIZE 50
#define BUFFER_SIZE 512

// 임시 주소 포트
#define SERVERIP   "127.0.0.1"
#define SERVERPORT 9000

using namespace std;

//프로그램에 필요한 전역변수 선언
HINSTANCE g_hInst; // hinstantce 객체
HWND ipAddressEdit, chatNameEdit, portEdit, roomNumberEdit1, roomNumberEdit2, messageEdit,
ReadOnlyRoomNameEdit, ReadOnlyChatNameEdit, chattingMessageEdit;
char ipAddress[IPADDRESS_SIZE], chatName[CHATNAME_SIZE], port[PORT_SIZE], roomName[ROOMNAME_SIZE];

SOCKET sock;

// 채팅방 입장 시 필요한 정보 입력하는 Dialog 처리하는 함수
BOOL CALLBACK InputInformationProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// 채팅 Dialog 처리하는 함수
BOOL CALLBACK ChattingProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// 채팅방 목록 Dialog 처리하는 함수
BOOL CALLBACK CurrentUserListProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Dialog 창 가운데로 설정하는 함수
void MoveCenterDialog(HWND hDlg);

// EditText에 출력해주는 함수
void DisplayText(const char *fmt, ...);

// IP 주소 예외처리 함수 (Class A,B,C 주소)
bool IsAvailableIP(string ipAddress);

// 대화명 예외처리 함수
bool IsAvailableChatName(string chatName);

// Port 예외처리 함수
bool IsAvailablePort(string port);

// 소켓 함수 오류시 처리하는 함수
void err_quit(const char *msg);
void err_display(const char *msg);

// 실시간 데이터 받는 함수
DWORD WINAPI ProcessReciveData(LPVOID arg);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	g_hInst = hInstance;

	// 채팅방 목록 Dialog창 띄우기
	DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG2), NULL, InputInformationProc);

	return 0;
}

BOOL CALLBACK InputInformationProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	bool checkException = false;
	string warnningMessage = "";
	bool IsCheckRoom1, IsCheckRoom2;

	switch (uMsg) {

	case WM_INITDIALOG:
		MoveCenterDialog(hDlg);

		ipAddressEdit = GetDlgItem(hDlg, IDC_IPADDRESS1);
		portEdit = GetDlgItem(hDlg, Port_EDIT);
		chatNameEdit = GetDlgItem(hDlg, ChatName_EDIT);
		roomNumberEdit1 = GetDlgItem(hDlg, IDC_RADIO1);
		roomNumberEdit2 = GetDlgItem(hDlg, IDC_RADIO2);

		// EditText 글자 수 제한
		SendMessage(portEdit, EM_SETLIMITTEXT, PORT_SIZE, 0);
		SendMessage(chatNameEdit, EM_SETLIMITTEXT, CHATNAME_SIZE, 0);

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:

			// EditText Text 내용 가져오기
			GetWindowText(ipAddressEdit, ipAddress, IPADDRESS_SIZE + 1);
			GetWindowText(portEdit, port, PORT_SIZE + 1);
			GetWindowText(chatNameEdit, chatName, CHATNAME_SIZE + 1);
			IsCheckRoom1 = IsDlgButtonChecked(hDlg, IDC_RADIO1);
			IsCheckRoom2 = IsDlgButtonChecked(hDlg, IDC_RADIO2);

			if (!IsAvailableIP(ipAddress)) {
				warnningMessage += "Class A,B,C에 해당하는 주소를 입력하세요.\n";
				checkException = true;
			}
			if (!IsAvailablePort(port)) {
				warnningMessage += "Port 번호는 0 ~ 65535 사이에 수만 가능합니다.\n";
				checkException = true;
			}
			if (!IsAvailableChatName(chatName)) {
				warnningMessage += "대화명은 영어(소문자)와 숫자를 포함한 10글자 이내만 가능합니다.\n";
				checkException = true;
			}
			if (!IsCheckRoom1 && !IsCheckRoom2) {
				warnningMessage += "대화방을 선택하세요.\n";
				checkException = true;
			}

			if (checkException) {
				MessageBox(nullptr, TEXT(warnningMessage.c_str()), TEXT("Meesage"), MB_OK);
			}
			else {
				if (IsCheckRoom1) {
					strcpy(roomName, "1번 대화방");
				}
				else if (IsCheckRoom2) {
					strcpy(roomName, "2번 대화방");
				}
				EndDialog(hDlg, IDC_BUTTON2);
				// 채팅창 Dialog 창 생성
				DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG3), NULL, ChattingProc);
			}

			return TRUE;
		case IDC_BUTTON2:

			// 현재 접속한 사용자 Dialog 창 생성
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, ChattingProc);

			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

BOOL CALLBACK ChattingProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int retval, len;
	WSADATA wsa;
	char buf[BUFFER_SIZE + 11];

	switch (uMsg) {
	case WM_INITDIALOG:
		MoveCenterDialog(hDlg);

		ReadOnlyChatNameEdit = GetDlgItem(hDlg, IDC_EDIT5);
		ReadOnlyRoomNameEdit = GetDlgItem(hDlg, IDC_EDIT10);

		chattingMessageEdit = GetDlgItem(hDlg, IDC_EDIT1);
		messageEdit = GetDlgItem(hDlg, IDC_EDIT2);

		SendMessage(messageEdit, EM_SETLIMITTEXT, MESSAGE_SIZE, 0);

		SetWindowText(ReadOnlyChatNameEdit, chatName);
		SetWindowText(ReadOnlyRoomNameEdit, roomName);

		// 윈속 초기화
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
			return 1;

		HANDLE hThread; // 스레드

		// socket()
		sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == INVALID_SOCKET) err_quit("socket()");

		// connect()
		SOCKADDR_IN serveraddr;
		ZeroMemory(&serveraddr, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
		serveraddr.sin_port = htons(SERVERPORT);
		retval = connect(sock, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
		if (retval == SOCKET_ERROR) err_quit("connect()");

		// 서버와 데이터 통신
		// 스레드 생성
		hThread = CreateThread(NULL, 0, ProcessReciveData, NULL, 0, NULL);
		if (hThread == NULL) {
			printf("fail make thread\n");
		}
		else {
			CloseHandle(hThread);
		}

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {

		case IDCANCEL:
			closesocket(sock);
			WSACleanup();

			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		case IDCANCEL2:
			closesocket(sock);
			WSACleanup();

			EndDialog(hDlg, IDCANCEL);
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG2), NULL, InputInformationProc);

			return TRUE;
		case IDC_BUTTON:
			// 현재 접속한 사용자 Dialog 창 생성
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, ChattingProc);
			return TRUE;
		case IDC_BUTTON1:
			char sendMessage[MESSAGE_SIZE];

			GetWindowText(messageEdit, sendMessage, MESSAGE_SIZE + 1);
			
			// '\n' 문자 제거
			len = strlen(sendMessage);
			if (sendMessage[len - 1] == '\n')
				sendMessage[len - 1] = '\0';
			if (strlen(sendMessage) == 0)
				break;

			string message_buf = "[" + (string)chatName + "]" + " : " + (string)sendMessage;

			// 데이터 보내기
			retval = send(sock, const_cast<char *>(message_buf.c_str()), strlen(message_buf.c_str()), 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				return 0;
			}
			
			SetFocus(messageEdit);
			SendMessage(messageEdit, EM_SETSEL, 0, -1);
			SetWindowText(messageEdit, "");
		}
		return TRUE;
	}
	return FALSE;
}

BOOL CALLBACK CurrentUserListProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		MoveCenterDialog(hDlg);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

DWORD WINAPI ProcessReciveData(LPVOID arg)
{

	int retval;		// 데이터 입력
	char buf[BUFFER_SIZE + 1];

	while (1) {
		// 데이터 받기
		retval = recv(sock, buf, BUFFER_SIZE + 1, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		// 받은 데이터 출력
		buf[retval] = '\0';

		DisplayText("%s \r\n", buf);
	}
}

void MoveCenterDialog(HWND hDlg) {
	int x, y, width, height;
	RECT rtDesk, rtWindow;
	GetWindowRect(GetDesktopWindow(), &rtDesk);
	GetWindowRect(hDlg, &rtWindow);

	width = rtWindow.right - rtWindow.left;
	height = rtWindow.bottom - rtWindow.top;

	x = (rtDesk.right - width) / 2;
	y = (rtDesk.bottom - height) / 2;

	MoveWindow(hDlg, x, y, width, height, TRUE);
}

bool IsAvailablePort(string port) {
	if (port == "") {
		return false;
	}

	int num = stoi(port);

	if (num >= 0 && num <= 65535) {
		return true;
	}
	else {
		return false;
	}
}

bool IsAvailableIP(string ipAddress)
{
	int pos = ipAddress.find(".");
	string token = ipAddress.substr(0, pos);
	if (token == "")
		return false;

	int num = stoi(ipAddress);
	if (num >= 224)
		return false;

	return true;
}

bool IsAvailableChatName(string chatName) {
	regex name("^[A-Za-z0-9]+$");

	if (!regex_match(chatName, name)) {
		return false;
	}

	return true;
}

void DisplayText(const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);

	char cbuf[512];
	vsprintf(cbuf, fmt, arg);

	int nLength = GetWindowTextLength(chattingMessageEdit);
	SendMessage(chattingMessageEdit, EM_SETSEL, nLength, nLength);
	SendMessage(chattingMessageEdit, EM_REPLACESEL, FALSE, (LPARAM)cbuf);

	va_end(arg);
}

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