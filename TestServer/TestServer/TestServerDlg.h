
// TestServerDlg.h: 헤더 파일
//

#pragma once

#define MAX_CLIENT_COUNT 10

// CTestServerDlg 대화 상자
class CTestServerDlg : public CDialogEx
{
private:
	SOCKET mh_listen_socket;
	SOCKET mh_client_list[MAX_CLIENT_COUNT];
	int m_client_count;
// 생성입니다.
public:
	CTestServerDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TESTSERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
};
