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
#define ROOMNUMBER_SIZE 1
#define BUFFER_SIZE 1024

using namespace std;

//프로그램에 필요한 전역변수 선언
 HINSTANCE g_hInst; // hinstantce 객체
 HWND ipAddressEdit, chatNameEdit, portEdit, roomNuberEdit; // Control과 연결할 변수 선언
 char ipAddress[IPADDRESS_SIZE], chatName[CHATNAME_SIZE], port[PORT_SIZE], roomNumber[ROOMNUMBER_SIZE];



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

// 방 번호 예외처리하는 함수
bool IsAvailableRoomNumber(string roomNumber);

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
	
	switch (uMsg) {
		
	case WM_INITDIALOG:
		MoveCenterDialog(hDlg);

		ipAddressEdit = GetDlgItem(hDlg, IDC_IPADDRESS1);
		portEdit = GetDlgItem(hDlg, IDC_EDIT2);
		roomNuberEdit = GetDlgItem(hDlg, IDC_EDIT3);
		chatNameEdit = GetDlgItem(hDlg, IDC_EDIT4);

		// EditText 글자 수 제한
		SendMessage(portEdit, EM_SETLIMITTEXT, PORT_SIZE, 0);
		SendMessage(chatNameEdit, EM_SETLIMITTEXT, CHATNAME_SIZE, 0);
		SendMessage(roomNuberEdit, EM_SETLIMITTEXT, ROOMNUMBER_SIZE, 0);

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			
			// EditText Text 내용 가져오기
			GetDlgItemText(hDlg, IDC_IPADDRESS1, ipAddress, IPADDRESS_SIZE + 1);
			GetDlgItemText(hDlg, IDC_EDIT2, port, PORT_SIZE + 1);
			GetDlgItemText(hDlg, IDC_EDIT3, roomNumber, ROOMNUMBER_SIZE + 1);
			GetDlgItemText(hDlg, IDC_EDIT4, chatName, CHATNAME_SIZE + 1);

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
			if (!IsAvailableRoomNumber(roomNumber)) {
				warnningMessage += "방 번호는 1번 혹은 2번만 가능합니다.\n";
				checkException = true;
			}

			if (checkException) {
				MessageBox(nullptr, TEXT(warnningMessage.c_str()), TEXT("Meesage"), MB_OK);
			}
			else {
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
	switch (uMsg) {
	case WM_INITDIALOG:
		MoveCenterDialog(hDlg);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
	
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		case IDCANCEL2:
			EndDialog(hDlg, IDCANCEL);
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG2), NULL, InputInformationProc);

			return TRUE;
		case IDC_BUTTON:
			// 현재 접속한 사용자 Dialog 창 생성
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, ChattingProc);
			return TRUE;
		case IDC_BUTTON1:
			return TRUE;

		}
		return FALSE;
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

bool IsAvailableRoomNumber(string roomNumber) {
	if (roomNumber == "")
		return false;

	int num = stoi(roomNumber);

	if (num == 1 || num == 2) {
		return true;
	}
	else {
		return false;
	}
}

void DisplayText(const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);

	char cbuf[512];
	vsprintf(cbuf, fmt, arg);

	//int nLength = GetWindowTextLength(chattingMessageEdit);
	//SendMessage(chattingMessageEdit, EM_SETSEL, nLength, nLength);
	//SendMessage(chattingMessageEdit, EM_REPLACESEL, FALSE, (LPARAM)cbuf);

	va_end(arg);
}