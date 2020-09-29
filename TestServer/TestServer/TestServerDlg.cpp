
// TestServerDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "TestServer.h"
#include "TestServerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTestServerDlg 대화 상자



CTestServerDlg::CTestServerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_TESTSERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_client_count = 0;
}

void CTestServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTestServerDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CTestServerDlg 메시지 처리기

BOOL CTestServerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	mh_listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	SOCKADDR_IN sock_addr;
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);	//inet_addr("172.30.1.23");
	sock_addr.sin_port = htons(1125);

	bind(mh_listen_socket, (SOCKADDR *)&sock_addr, sizeof(sock_addr));

	listen(mh_listen_socket, 1);

	WSAAsyncSelect(mh_listen_socket, m_hWnd, 11250, FD_ACCEPT);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CTestServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CTestServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CTestServerDlg::AcceptProcess(SOCKET param_h_socket) {
	if (MAX_CLIENT_COUNT > m_client_count) {
		SOCKADDR_IN client_addr;
		int sockaddr_in_size = sizeof(client_addr);

		mh_client_list[m_client_count] = accept(param_h_socket, (SOCKADDR*)&client_addr, &sockaddr_in_size);

		WSAAsyncSelect(mh_client_list[m_client_count], m_hWnd, 11251, FD_READ | FD_CLOSE);

		m_client_count++;

		CString ip_address;
		ip_address = inet_ntoa(client_addr.sin_addr);

		MessageBox(ip_address, L"새로운 클라이언트가 접속했습니다.", MB_OK);
	}
}

void CTestServerDlg::ClientCloseProcess(SOCKET param_h_socket, char param_force_flag) {
	if (param_force_flag == 1) {
		LINGER temp_linger = { TRUE, 0 }; 
		setsockopt(param_h_socket, SOL_SOCKET, SO_LINGER, (char*)&temp_linger, sizeof(temp_linger));
	}

	closesocket(param_h_socket);

	for (int i = 0; i < m_client_count; i++) {
		if (mh_client_list[i] == param_h_socket) {
			m_client_count--;

			if (i != m_client_count) {
				mh_client_list[i] = mh_client_list[m_client_count];
			}
		}
	}
}

LRESULT CTestServerDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (11250 == message) {
		AcceptProcess((SOCKET)wParam);
	}
	else if (11251 == message) {
		SOCKET h_socket = (SOCKET)wParam;

		if (WSAGETSELECTEVENT(lParam) == FD_READ) {
			WSAAsyncSelect(h_socket, m_hWnd, 11251, FD_CLOSE);
			char key, network_message_id;
			recv(h_socket, &key, 1, 0);

			if (key == 27) {
				int current_size, total_size = 0, retry_count = 0;
				unsigned short int body_size;
				recv(h_socket, (char*)&body_size, sizeof(body_size), 0);
				recv(h_socket, &network_message_id, 1, 0); 

				if (body_size > 0) {
					char* p_body_size = new char[body_size];

					while (total_size < body_size) {
						current_size = recv(h_socket, p_body_size + total_size, body_size - total_size, 0);

						if (current_size == SOCKET_ERROR) {
							retry_count++;
							Sleep(50);
							if (retry_count > 5) break;
						}
						else {
							retry_count = 0;
							total_size += current_size;
						}
					}

					if (network_message_id == 1) {
						//실제로 클라이언트가 보내준 데이터를 처리(p_body_size)
					}

					delete[] p_body_size;
				}

				WSAAsyncSelect(h_socket, m_hWnd, 11251, FD_CLOSE | FD_READ);
			}
			else {
				ClientCloseProcess(h_socket, 1);
			}
		}
		else {	//FD_CLOSE
			ClientCloseProcess(h_socket, 0);
		}
	}

	return CDialogEx::WindowProc(message, wParam, lParam);
}
