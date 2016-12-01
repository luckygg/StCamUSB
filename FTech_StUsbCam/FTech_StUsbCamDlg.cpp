
// FTech_StUsbCamDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FTech_StUsbCam.h"
#include "FTech_StUsbCamDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFTech_StUsbCamDlg dialog



UINT DisplayThread1(LPVOID param);
UINT DisplayThread2(LPVOID param);
UINT DisplayThread3(LPVOID param);
UINT DisplayThread4(LPVOID param);
CFTech_StUsbCamDlg::CFTech_StUsbCamDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFTech_StUsbCamDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	for (int i=0; i<MAX_CAM; i++)
	{
		m_bThdDisplay[i] = false;
		m_pBitmapInfo[i] = NULL;
	}
	
}

void CFTech_StUsbCamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CFTech_StUsbCamDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_SCAN, &CFTech_StUsbCamDlg::OnBnClickedBtnScan)
	ON_BN_CLICKED(IDC_BTN_CONNECTION1, &CFTech_StUsbCamDlg::OnBnClickedBtnConnection1)
	ON_BN_CLICKED(IDC_BTN_ACQUISITION, &CFTech_StUsbCamDlg::OnBnClickedBtnAcquisition)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_TRGEVENT, &CFTech_StUsbCamDlg::OnBnClickedBtnTrgevent)
	ON_BN_CLICKED(IDC_RBTN_FREE, &CFTech_StUsbCamDlg::OnBnClickedRbtnFree)
	ON_BN_CLICKED(IDC_RBTN_SOFT, &CFTech_StUsbCamDlg::OnBnClickedRbtnSoft)
	ON_BN_CLICKED(IDC_RBTN_HARD, &CFTech_StUsbCamDlg::OnBnClickedRbtnHard)
	ON_BN_CLICKED(IDC_RBTN_TIMED, &CFTech_StUsbCamDlg::OnBnClickedRbtnTimed)
	ON_BN_CLICKED(IDC_RBTN_PULSE, &CFTech_StUsbCamDlg::OnBnClickedRbtnPulse)
	ON_BN_CLICKED(IDC_BTN_CONNECTION2, &CFTech_StUsbCamDlg::OnBnClickedBtnConnection2)
	ON_BN_CLICKED(IDC_BTN_CONNECTION3, &CFTech_StUsbCamDlg::OnBnClickedBtnConnection3)
	ON_BN_CLICKED(IDC_BTN_CONNECTION4, &CFTech_StUsbCamDlg::OnBnClickedBtnConnection4)
END_MESSAGE_MAP()

void CFTech_StUsbCamDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	for (int i=0; i<MAX_CAM; i++)
	{
		if (m_pBitmapInfo[i] != NULL)
		{
			delete []m_pBitmapInfo[i];
			m_pBitmapInfo[i] = NULL;
		}
	}
	
}
// CFTech_StUsbCamDlg message handlers

BOOL CFTech_StUsbCamDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CheckRadioButton(IDC_RBTN_FREE, IDC_RBTN_HARD, IDC_RBTN_FREE);
	CheckRadioButton(IDC_RBTN_TIMED, IDC_RBTN_PULSE, IDC_RBTN_TIMED);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFTech_StUsbCamDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFTech_StUsbCamDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CFTech_StUsbCamDlg::OnBnClickedBtnScan()
{
	CStUSbInterface StInterface;
	int nCnt = StInterface.GetDeviceCount();

	CListBox* pLB = (CListBox*)GetDlgItem(IDC_LIST_CAMERA);

	pLB->ResetContent();
	for (int i=0; i<nCnt; i++)
	{
		CString name=L"";
		StInterface.GetDeviceName(i,name);
		pLB->AddString(name);
	}
}

void CFTech_StUsbCamDlg::OnBnClickedBtnAcquisition()
{
	CString caption=L"";
	GetDlgItemText(IDC_BTN_ACQUISITION, caption);
	bool ret=false;
	if (caption == L"Start")
	{
		for (int i=0; i<MAX_CAM; i++)
		{
			if (m_StCam[i].IsConnected() == false)
				continue;

			ret = m_StCam[i].OnStartAcquisition();
			if (ret == true)
			{
				m_bThdDisplay[i] = true;
				switch(i)
				{
				case 0 :
					AfxBeginThread(DisplayThread1,this);
					break;
				case 1 :
					AfxBeginThread(DisplayThread2,this);
					break;
				case 2 :
					AfxBeginThread(DisplayThread3,this);
					break;
				case 3 :
					AfxBeginThread(DisplayThread4,this);
					break;
				}
				
				SetDlgItemText(IDC_BTN_ACQUISITION, L"Stop");
			}
		}
	}
	else
	{
		for (int i=0; i<MAX_CAM; i++)
		{
			if (m_StCam[i].IsConnected() == false)
				continue;

			m_bThdDisplay[i] = false;
			m_StCam[i].OnStopAcquisition();
		}

		SetDlgItemText(IDC_BTN_ACQUISITION, L"Start");
	}
}

void CFTech_StUsbCamDlg::OnCreateBmpInfo(int nIndex, int nWidth, int nHeight, int nBpp)
{
	if (m_pBitmapInfo[nIndex] != NULL)
	{
		delete []m_pBitmapInfo[nIndex];
		m_pBitmapInfo[nIndex] = NULL;
	}

	if (nBpp == 8)
		m_pBitmapInfo[nIndex] = (BITMAPINFO *) new BYTE[sizeof(BITMAPINFO) + 255*sizeof(RGBQUAD)];
	else if (nBpp == 24)
		m_pBitmapInfo[nIndex] = (BITMAPINFO *) new BYTE[sizeof(BITMAPINFO)];

	m_pBitmapInfo[nIndex]->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_pBitmapInfo[nIndex]->bmiHeader.biPlanes = 1;
	m_pBitmapInfo[nIndex]->bmiHeader.biBitCount = nBpp;
	m_pBitmapInfo[nIndex]->bmiHeader.biCompression = BI_RGB;
	m_pBitmapInfo[nIndex]->bmiHeader.biSizeImage = 0;
	m_pBitmapInfo[nIndex]->bmiHeader.biXPelsPerMeter = 0;
	m_pBitmapInfo[nIndex]->bmiHeader.biYPelsPerMeter = 0;
	m_pBitmapInfo[nIndex]->bmiHeader.biClrUsed = 0;
	m_pBitmapInfo[nIndex]->bmiHeader.biClrImportant = 0;

	if (nBpp == 8)
	{
		for (int i = 0 ; i < 256 ; i++)
		{
			m_pBitmapInfo[nIndex]->bmiColors[i].rgbBlue = (BYTE)i;
			m_pBitmapInfo[nIndex]->bmiColors[i].rgbGreen = (BYTE)i;
			m_pBitmapInfo[nIndex]->bmiColors[i].rgbRed = (BYTE)i;
			m_pBitmapInfo[nIndex]->bmiColors[i].rgbReserved = 0;
		}
	}
	
	m_pBitmapInfo[nIndex]->bmiHeader.biWidth = nWidth;
	m_pBitmapInfo[nIndex]->bmiHeader.biHeight = -nHeight;
}


UINT DisplayThread1(LPVOID param)
{
	CFTech_StUsbCamDlg* pMain = (CFTech_StUsbCamDlg*)param;

	while(pMain->m_bThdDisplay[0])
	{
		Sleep(30);

		pMain->OnDisplayCamera1();
	}
	return 0;
}

UINT DisplayThread2(LPVOID param)
{
	CFTech_StUsbCamDlg* pMain = (CFTech_StUsbCamDlg*)param;

	while(pMain->m_bThdDisplay[1])
	{
		Sleep(30);

		pMain->OnDisplayCamera2();
	}
	return 0;
}

UINT DisplayThread3(LPVOID param)
{
	CFTech_StUsbCamDlg* pMain = (CFTech_StUsbCamDlg*)param;

	while(pMain->m_bThdDisplay[2])
	{
		Sleep(30);

		pMain->OnDisplayCamera3();
	}
	return 0;
}

UINT DisplayThread4(LPVOID param)
{
	CFTech_StUsbCamDlg* pMain = (CFTech_StUsbCamDlg*)param;

	while(pMain->m_bThdDisplay[3])
	{
		Sleep(30);

		pMain->OnDisplayCamera4();
	}
	return 0;
}


void CFTech_StUsbCamDlg::OnDisplayCamera1()
{
	if (m_StCam[0].IsConnected() == false) return;

	BYTE *pBuffer = m_StCam[0].GetBufferPointer();
	if (pBuffer == NULL) return;

	CClientDC dc(GetDlgItem(IDC_PC_CAMERA1));
	CRect rect;
	GetDlgItem(IDC_PC_CAMERA1)->GetClientRect(&rect);
	
	int w=0,h=0;
	m_StCam[0].GetWidth(w);
	m_StCam[0].GetHeight(h);

	SetStretchBltMode(dc.GetSafeHdc(), COLORONCOLOR); 
	StretchDIBits(dc.GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), 0, 0, w, h, pBuffer, m_pBitmapInfo[0], DIB_RGB_COLORS, SRCCOPY);
}

void CFTech_StUsbCamDlg::OnDisplayCamera2()
{
	if (m_StCam[1].IsConnected() == false) return;

	BYTE *pBuffer = m_StCam[1].GetBufferPointer();
	if (pBuffer == NULL) return;

	CClientDC dc(GetDlgItem(IDC_PC_CAMERA2));
	CRect rect;
	GetDlgItem(IDC_PC_CAMERA2)->GetClientRect(&rect);

	int w=0,h=0;
	m_StCam[1].GetWidth(w);
	m_StCam[1].GetHeight(h);

	SetStretchBltMode(dc.GetSafeHdc(), COLORONCOLOR); 
	StretchDIBits(dc.GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), 0, 0, w, h, pBuffer, m_pBitmapInfo[1], DIB_RGB_COLORS, SRCCOPY);
}

void CFTech_StUsbCamDlg::OnDisplayCamera3()
{
	if (m_StCam[2].IsConnected() == false) return;

	BYTE *pBuffer = m_StCam[2].GetBufferPointer();
	if (pBuffer == NULL) return;

	CClientDC dc(GetDlgItem(IDC_PC_CAMERA3));
	CRect rect;
	GetDlgItem(IDC_PC_CAMERA3)->GetClientRect(&rect);

	int w=0,h=0;
	m_StCam[2].GetWidth(w);
	m_StCam[2].GetHeight(h);

	SetStretchBltMode(dc.GetSafeHdc(), COLORONCOLOR); 
	StretchDIBits(dc.GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), 0, 0, w, h, pBuffer, m_pBitmapInfo[2], DIB_RGB_COLORS, SRCCOPY);
}

void CFTech_StUsbCamDlg::OnDisplayCamera4()
{
	if (m_StCam[3].IsConnected() == false) return;

	BYTE *pBuffer = m_StCam[3].GetBufferPointer();
	if (pBuffer == NULL) return;

	CClientDC dc(GetDlgItem(IDC_PC_CAMERA4));
	CRect rect;
	GetDlgItem(IDC_PC_CAMERA4)->GetClientRect(&rect);

	int w=0,h=0;
	m_StCam[3].GetWidth(w);
	m_StCam[3].GetHeight(h);

	SetStretchBltMode(dc.GetSafeHdc(), COLORONCOLOR); 
	StretchDIBits(dc.GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), 0, 0, w, h, pBuffer, m_pBitmapInfo[3], DIB_RGB_COLORS, SRCCOPY);
}

void CFTech_StUsbCamDlg::OnBnClickedBtnTrgevent()
{
	for (int i=0; i<MAX_CAM; i++)
	{
		if (m_StCam[i].IsConnected() == false)
			continue;

		bool ret = m_StCam[i].OnTriggerEvent();
		if (ret == false)
		{
			AfxMessageBox(L"ERROR : OnTriggerEvent()");
		}
	}
}


void CFTech_StUsbCamDlg::OnBnClickedRbtnFree()
{
	for (int i=0; i<MAX_CAM; i++)
	{
		if (m_StCam[i].IsConnected() == false)
			continue;

		bool ret = m_StCam[i].SetTrgModeFreeRun();
		if (ret == false)
		{
			AfxMessageBox(L"ERROR : SetTrgModeFreeRun()");
		}
	}
}

void CFTech_StUsbCamDlg::OnBnClickedRbtnSoft()
{
	for (int i=0; i<MAX_CAM; i++)
	{
		if (m_StCam[i].IsConnected() == false)
			continue;

		bool ret = m_StCam[i].SetTrgModeSoft();
		if (ret == false)
		{
			AfxMessageBox(L"ERROR : SetTrgModeSoft()");
		}
	}
}

void CFTech_StUsbCamDlg::OnBnClickedRbtnHard()
{
	for (int i=0; i<MAX_CAM; i++)
	{
		if (m_StCam[i].IsConnected() == false)
			continue;

		bool ret = m_StCam[i].SetTrgModeHard(0,true);
		if (ret == false)
		{
			AfxMessageBox(L"ERROR : SetTrgModeHard()");
		}
	}
}

void CFTech_StUsbCamDlg::OnBnClickedRbtnTimed()
{
	for (int i=0; i<MAX_CAM; i++)
	{
		if (m_StCam[i].IsConnected() == false)
			continue;

		bool ret = m_StCam[i].SetExpModeTimed();
		if (ret == false)
		{
			AfxMessageBox(L"ERROR : SetExpModeTimed()");
		}
	}
}

void CFTech_StUsbCamDlg::OnBnClickedRbtnPulse()
{
	for (int i=0; i<MAX_CAM; i++)
	{
		if (m_StCam[i].IsConnected() == false)
			continue;

		bool ret = m_StCam[i].SetExpModePulse();
		if (ret == false)
		{
			AfxMessageBox(L"ERROR : SetExpModePulse()");
		}
	}
}

BOOL CFTech_StUsbCamDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN && pMsg->hwnd == GetDlgItem(IDC_EDIT_EXPOSURETIME)->GetSafeHwnd())
		{
			for (int i=0; i<MAX_CAM; i++)
			{
				if (m_StCam[i].IsConnected() == false)
					continue;

				int nValue = GetDlgItemInt(IDC_EDIT_EXPOSURETIME);
				bool ret = m_StCam[i].SetExposureTimeMicroSecond(nValue);
				if (ret == false)
				{
					AfxMessageBox(L"ERROR : SetExposureTime()");
				}
			}

			return TRUE;
		}
		else
		{
			if (pMsg->wParam == VK_ESCAPE)
				return TRUE;
			if (pMsg->wParam == VK_RETURN)
				return TRUE;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CFTech_StUsbCamDlg::OnBnClickedBtnConnection1()
{
	CString caption=L"";
	GetDlgItemText(IDC_BTN_CONNECTION1, caption);

	if (caption == L"Connect")
	{
		CListBox* pLB = (CListBox*)GetDlgItem(IDC_LIST_CAMERA);
		CString name=L"";
		pLB->GetText(pLB->GetCurSel(),name);

		bool ret = m_StCam[0].OnConnect(name);
		if (ret == true)
		{
			int w=0,h=0;
			m_StCam[0].GetWidth(w);
			m_StCam[0].GetHeight(h);
			OnCreateBmpInfo(0,w,h,8);

			SetDlgItemText(IDC_BTN_CONNECTION1, L"Disconnect");
			SetDlgItemText(IDC_LB_CAMERA1, m_StCam[0].GetDeviceName());

			m_StCam[0].SetTrgModeFreeRun();
			m_StCam[0].SetExpModeTimed();

			pLB->DeleteString(pLB->GetCurSel());

			//Set Strobe parameters.
			//0:IO0, 1:IO1, 2:IO2
			m_StCam[0].SetStrobeMode(1,true);
			m_StCam[0].SetStrobeDelayMicroSecond(0);
			m_StCam[0].SetStrobeOnTimeMicroSecond(5000);
		}
	}
	else
	{
		bool ret = m_StCam[0].OnDisconnect();
		if (ret == true)
		{
			SetDlgItemText(IDC_BTN_CONNECTION1, L"Connect");
			SetDlgItemText(IDC_LB_CAMERA1, L"");
		}
	}
}

void CFTech_StUsbCamDlg::OnBnClickedBtnConnection2()
{
	CString caption=L"";
	GetDlgItemText(IDC_BTN_CONNECTION2, caption);

	if (caption == L"Connect")
	{
		CListBox* pLB = (CListBox*)GetDlgItem(IDC_LIST_CAMERA);
		CString name=L"";
		pLB->GetText(pLB->GetCurSel(),name);

		bool ret = m_StCam[1].OnConnect(name);
		if (ret == true)
		{
			int w=0,h=0;
			m_StCam[1].GetWidth(w);
			m_StCam[1].GetHeight(h);
			OnCreateBmpInfo(1,w,h,8);

			SetDlgItemText(IDC_BTN_CONNECTION2, L"Disconnect");
			SetDlgItemText(IDC_LB_CAMERA2, m_StCam[1].GetDeviceName());

			m_StCam[1].SetTrgModeFreeRun();
			m_StCam[1].SetExpModeTimed();

			pLB->DeleteString(pLB->GetCurSel());
		}
	}
	else
	{
		bool ret = m_StCam[1].OnDisconnect();
		if (ret == true)
		{
			SetDlgItemText(IDC_BTN_CONNECTION2, L"Connect");
			SetDlgItemText(IDC_LB_CAMERA2, L"");
		}
	}
}


void CFTech_StUsbCamDlg::OnBnClickedBtnConnection3()
{
	CString caption=L"";
	GetDlgItemText(IDC_BTN_CONNECTION3, caption);

	if (caption == L"Connect")
	{
		CListBox* pLB = (CListBox*)GetDlgItem(IDC_LIST_CAMERA);
		CString name=L"";
		pLB->GetText(pLB->GetCurSel(),name);

		bool ret = m_StCam[2].OnConnect(name);
		if (ret == true)
		{
			int w=0,h=0;
			m_StCam[2].GetWidth(w);
			m_StCam[2].GetHeight(h);
			OnCreateBmpInfo(2,w,h,8);

			SetDlgItemText(IDC_BTN_CONNECTION3, L"Disconnect");
			SetDlgItemText(IDC_LB_CAMERA3, m_StCam[2].GetDeviceName());

			m_StCam[2].SetTrgModeFreeRun();
			m_StCam[2].SetExpModeTimed();

			pLB->DeleteString(pLB->GetCurSel());
		}
	}
	else
	{
		bool ret = m_StCam[2].OnDisconnect();
		if (ret == true)
		{
			SetDlgItemText(IDC_BTN_CONNECTION3, L"Connect");
			SetDlgItemText(IDC_LB_CAMERA3, L"");
		}
	}
}


void CFTech_StUsbCamDlg::OnBnClickedBtnConnection4()
{
	CString caption=L"";
	GetDlgItemText(IDC_BTN_CONNECTION4, caption);

	if (caption == L"Connect")
	{
		CListBox* pLB = (CListBox*)GetDlgItem(IDC_LIST_CAMERA);
		CString name=L"";
		pLB->GetText(pLB->GetCurSel(),name);

		bool ret = m_StCam[3].OnConnect(name);
		if (ret == true)
		{
			int w=0,h=0;
			m_StCam[3].GetWidth(w);
			m_StCam[3].GetHeight(h);
			OnCreateBmpInfo(3,w,h,8);

			SetDlgItemText(IDC_BTN_CONNECTION4, L"Disconnect");
			SetDlgItemText(IDC_LB_CAMERA4, m_StCam[3].GetDeviceName());

			m_StCam[3].SetTrgModeFreeRun();
			m_StCam[3].SetExpModeTimed();

			pLB->DeleteString(pLB->GetCurSel());
		}
	}
	else
	{
		bool ret = m_StCam[3].OnDisconnect();
		if (ret == true)
		{
			SetDlgItemText(IDC_BTN_CONNECTION4, L"Connect");
			SetDlgItemText(IDC_LB_CAMERA4, L"");
		}
	}
}
