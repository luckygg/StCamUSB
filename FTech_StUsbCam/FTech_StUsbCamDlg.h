
// FTech_StUsbCamDlg.h : header file
//

#pragma once
#include "StCamUSB.h"

#define MAX_CAM 4
// CFTech_StUsbCamDlg dialog
class CFTech_StUsbCamDlg : public CDialogEx
{
// Construction
public:
	CFTech_StUsbCamDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_FTECH_STUSBCAM_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

public :
	CStUsbCam m_StCam[MAX_CAM];
	bool m_bThdDisplay[MAX_CAM];
	BITMAPINFO *m_pBitmapInfo[MAX_CAM];
	void OnCreateBmpInfo(int nIndex, int nWidth, int nHeight, int nBpp);

	void OnDisplayCamera1();
	void OnDisplayCamera2();
	void OnDisplayCamera3();
	void OnDisplayCamera4();
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnScan();
	afx_msg void OnBnClickedBtnConnection1();
	afx_msg void OnBnClickedBtnAcquisition();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnTrgevent();
	afx_msg void OnBnClickedRbtnFree();
	afx_msg void OnBnClickedRbtnSoft();
	afx_msg void OnBnClickedRbtnHard();
	afx_msg void OnBnClickedRbtnTimed();
	afx_msg void OnBnClickedRbtnPulse();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedBtnConnection2();
	afx_msg void OnBnClickedBtnConnection3();
	afx_msg void OnBnClickedBtnConnection4();
};
