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

using namespace std;

//프로그램에 필요한 전역변수 선언
 HINSTANCE g_hInst; // hinstantce 객체

// 채팅방 목록 Dialog 처리하는 함수
BOOL CALLBACK ChattingListProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// 채팅방 입장 시 필요한 정보 입력하는 Dialog 처리하는 함수
BOOL CALLBACK InputInformationProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// 채팅 Dialog 처리하는 함수
BOOL CALLBACK ChattingProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	g_hInst = hInstance;
	
	// 채팅방 목록 Dialog창 띄우기
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, ChattingListProc);

	return 0;
}

BOOL CALLBACK ChattingListProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
	
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BUTTON2:

			// 사용자 정보 입력 Dialog 창 생성
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG2), NULL, InputInformationProc);
			return TRUE;
		case IDC_BUTTON3:

			// 사용자 정보 입력 Dialog 창 생성
			DialogBox(g_hInst, MAKEINTRESOURCE(IDD_DIALOG2), NULL, InputInformationProc);
			return TRUE;
			
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

BOOL CALLBACK InputInformationProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:

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

		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:

		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}