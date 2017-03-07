//----------------------------------------------------------
// Sentech USB Camera Class
// Trigger SDK 3.0.7 version
//----------------------------------------------------------
// Programmed by William Kim
//----------------------------------------------------------
// Last Update : 2017-03-07 11:02
// Modified by William Kim
//----------------------------------------------------------

#pragma once

#include "SDK/include/StTrgApi.h"

#ifdef _WIN64
#pragma comment(lib, "SDK/lib/x64/StTrgApi.lib")
#else //_WIN64
#pragma comment(lib, "SDK/lib/x86/StTrgApi.lib")
#endif //_WIN64

typedef struct StInterfaceInfo
{
	HANDLE hCamera;
	CString strName;

}StInterfaceInfo;

class CStUSbInterface
{
public :
	CStUSbInterface(void);
	~CStUSbInterface(void);

public :
	int GetDeviceCount();
	bool GetDeviceName(int nIndex, CString &strValue);

private :
	CArray<StInterfaceInfo> m_arrCamInfo;
};

class CStCamUSB
{
public:
	CStCamUSB(void);
	~CStCamUSB(void);

public :
	//----- Connection -----//
	bool OnConnect(CString strName);
	bool OnDisconnect();

	//----- Acquisition -----//
	bool OnStartAcquisition();
	bool OnStopAcquisition();
	bool OnTriggerEvent();

	bool SetExpModeTimed();
	bool SetExpModePulse();
	bool SetTrgModeFreeRun();
	bool SetTrgModeSoft();
	bool SetTrgModeHard(int nIONum, bool bPolarity);
	bool SetStrobeMode(int nIONum, bool bPolarity);

	//----- Get & Set Parameters -----//
	bool GetOffsetX(int &nValue);
	bool GetOffsetY(int &nValue);
	bool GetWidth(int &nValue);
	bool GetHeight(int &nValue);
	bool GetBpp(int &nValue);
	bool GetGain(int &nValue);
	bool GetExposureTimeMicroSecond(int &nValue);
	bool GetFrameRate(float &fValue);
	CString GetDeviceName() { return m_strName; }
	bool SetOffsetX(int nValue);
	bool SetOffsetY(int nValue);
	bool SetWidth(int nValue);
	bool SetHeight(int nValue);
	bool SetGain(int nValue);
	bool SetExposureTimeMicroSecond(int nValue);
	bool SetStrobeDelayMicroSecond(int nValue);
	bool SetStrobeOnTimeMicroSecond(int nValue);

	bool SetMirrorMode(bool bHor, bool bVer);

	//----- Check Status -----//
	bool IsConnected() { return m_bConnected; }
	bool IsActived() { return m_bActived; }

	//----- Buffer & Event -----//
	void OnMemcpy(BYTE* pBuffer, int nWidth, int nHeight) { 
		memcpy(m_pBuffer, pBuffer, nWidth*nHeight); }
	BYTE* GetBufferPointer() { return m_pBuffer; }
	HANDLE GetHandleGrabDone() { return m_hGrabDone; }
	void OnSetEventHandle() { SetEvent(m_hGrabDone); }
	void OnResetEventHandle() { ResetEvent(m_hGrabDone); }
private :
	HANDLE m_hCamera;
	HANDLE m_hGrabDone;
	CString m_strName;
	bool m_bConnected;
	bool m_bActived;
	BYTE *m_pBuffer;
	
};

