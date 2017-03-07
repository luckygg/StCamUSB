#include "StdAfx.h"
#include "StCamUSB.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CStUSbInterface::CStUSbInterface(void)
{

}

CStUSbInterface::~CStUSbInterface(void)
{

}

int CStUSbInterface::GetDeviceCount()
{
	m_arrCamInfo.RemoveAll();

	HANDLE hCamera = INVALID_HANDLE_VALUE;
	do{
		hCamera = StTrg_Open();
		if(INVALID_HANDLE_VALUE != hCamera)
		{
			StInterfaceInfo stInfo;
			DWORD	dwCameraID;
			TCHAR	szCameraID[MAX_PATH];
			memset(szCameraID, 0, sizeof(TCHAR) * MAX_PATH);
			
			StTrg_ReadCameraUserID(hCamera, &dwCameraID, szCameraID, MAX_PATH);

			stInfo.hCamera = hCamera;
			stInfo.strName = (LPCTSTR)szCameraID;
			m_arrCamInfo.Add(stInfo);
		}
	}while(INVALID_HANDLE_VALUE != hCamera);

	int nCnt = m_arrCamInfo.GetSize();

	for (int i=0; i<nCnt; i++)
		StTrg_Close(m_arrCamInfo.GetAt(i).hCamera);

	return nCnt;
}

bool CStUSbInterface::GetDeviceName(int nIndex, CString &strValue)
{
	int nMax = m_arrCamInfo.GetSize();
	if (nIndex >= nMax)
	{
		strValue = L"Index is bigger than current max count.";
		return false;
	}

	strValue = m_arrCamInfo.GetAt(nIndex).strName;

	return true;
}

CStCamUSB::CStCamUSB(void)
{
	m_hCamera = INVALID_HANDLE_VALUE;
	m_bConnected = false;
	m_bActived = false;
	m_strName = L"";
	m_pBuffer = NULL;

	m_hGrabDone = CreateEvent(NULL, TRUE, FALSE, NULL);
	ResetEvent(m_hGrabDone);
}

CStCamUSB::~CStCamUSB(void)
{
	OnDisconnect();

	if (m_pBuffer != NULL)
	{
		delete []m_pBuffer;
		m_pBuffer = NULL;
	}

	DeleteObject(m_hGrabDone);
}

void __stdcall TransferEndCallback(HANDLE hCamera, DWORD dwFrameNo, DWORD dwWidth, DWORD dwHeight, WORD wColorArray, PBYTE pbyteRaw, PVOID pvContext)
{
	CStCamUSB* pMain = (CStCamUSB*)pvContext;

	pMain->OnMemcpy(pbyteRaw, dwWidth, dwHeight);

	pMain->OnSetEventHandle();

	//2016-10-24 ggkim add
	//Check Frame No.
	int count = dwFrameNo;
	CString tmp;
	tmp.Format(L"%d\n",count);

	OutputDebugString(tmp);
}

bool CStCamUSB::OnConnect(CString strName)
{
	CArray<StInterfaceInfo> arrCamInfo;

	HANDLE hCamera = INVALID_HANDLE_VALUE;
	do{
		hCamera = StTrg_Open();
		if(INVALID_HANDLE_VALUE != hCamera)
		{
			StInterfaceInfo stInfo;
			DWORD	dwCameraID;
			TCHAR	szCameraID[MAX_PATH];
			memset(szCameraID, 0, sizeof(TCHAR) * MAX_PATH);

			StTrg_ReadCameraUserID(hCamera, &dwCameraID, szCameraID, MAX_PATH);

			stInfo.hCamera = hCamera;
			stInfo.strName = (LPCTSTR)szCameraID;
			arrCamInfo.Add(stInfo);
			if (stInfo.strName == strName)
			{
				m_hCamera = hCamera;
				m_strName = strName;
			}
		}
	}while(INVALID_HANDLE_VALUE != hCamera);

	for (int i=0; i<arrCamInfo.GetSize(); i++)
	{
		if (arrCamInfo.GetAt(i).hCamera == m_hCamera) continue;

		StTrg_Close(arrCamInfo.GetAt(i).hCamera);
	}

	BOOL ret=FALSE;
	m_bConnected = true;

	int width=0, height=0;
	ret = GetWidth(width);
	if (ret == FALSE)
	{
		StTrg_Close(m_hCamera);
		return false;
	}

	ret = GetHeight(height);
	if (ret == FALSE)
	{
		StTrg_Close(m_hCamera);
		return false;
	}
	
	if (m_pBuffer != NULL)
	{
		delete []m_pBuffer;
		m_pBuffer = NULL;
	}

	m_pBuffer = new BYTE[width*height];
	memset(m_pBuffer, 0, width*height);

	ret = StTrg_SetTransferEndCallback(m_hCamera, TransferEndCallback, this);
	if (ret == FALSE)
	{
		StTrg_Close(m_hCamera);
		return false;
	}

	return true;
}
bool CStCamUSB::OnDisconnect()
{
	if (m_hCamera == NULL) return false;

	StTrg_Close(m_hCamera);
	
	m_hCamera = NULL;
	m_bConnected = false;

	m_strName = L"";

	return true;
}
bool CStCamUSB::OnStartAcquisition()
{
	BOOL ret=FALSE;
	//2016-10-24 ggkim add
	//Reset Frame No.
	ret = StTrg_ResetCounter(m_hCamera);
	if (ret == FALSE) return false;

	ret = StTrg_StartTransfer(m_hCamera);
	if (ret == FALSE) return false;
		
	m_bActived = true;

	return true;
}
bool CStCamUSB::OnStopAcquisition()
{
	BOOL ret=FALSE;
	ret = StTrg_StopTransfer(m_hCamera);
	if (ret == FALSE) return false;

	m_bActived = false;

	return true;
}
bool CStCamUSB::OnTriggerEvent()
{
	BOOL ret=FALSE;
	ret = StTrg_TriggerSoftware(m_hCamera, STCAM_TRIGGER_SELECTOR_FRAME_START);
	if (ret == FALSE) return false;

	return true;
}
bool CStCamUSB::SetExpModeTimed()
{
	BOOL ret=FALSE;
	ret = StTrg_SetExposureMode(m_hCamera, STCAM_EXPOSURE_MODE_TIMED);
	if (ret == FALSE) return false;

	return true;
}
bool CStCamUSB::SetExpModePulse()
{
	BOOL ret=FALSE;
	ret = StTrg_SetExposureMode(m_hCamera, STCAM_EXPOSURE_MODE_TRIGGER_WIDTH);
	if (ret == FALSE) return false;

	return true;
}
bool CStCamUSB::SetTrgModeFreeRun()
{
	BOOL ret=FALSE;
	ret = StTrg_SetTriggerMode(m_hCamera, STCAM_TRIGGER_MODE_TYPE_FREE_RUN);
	if (ret == FALSE) return false;

	return true;
}
bool CStCamUSB::SetTrgModeSoft()
{
	BOOL ret=FALSE;
	//Set Trigger Mode
	ret = StTrg_SetTriggerMode(m_hCamera, STCAM_TRIGGER_MODE_TYPE_TRIGGER);
	if (ret == FALSE) return false;
	//Set Trigger Source
	ret = StTrg_SetTriggerSource(m_hCamera, STCAM_TRIGGER_SELECTOR_FRAME_START, STCAM_TRIGGER_SOURCE_SOFTWARE);
	if (ret == FALSE) return false;
	//Set Trigger Overlap
	ret = StTrg_SetTriggerOverlap(m_hCamera, STCAM_TRIGGER_SELECTOR_FRAME_START, STCAM_TRIGGER_OVERLAP_READ_OUT);
	if (ret == FALSE) return false;

	return true;
}
bool CStCamUSB::SetTrgModeHard(int nIONum, bool bPolarity)
{
	BOOL ret=FALSE;
	//Set Trigger Mode
	ret = StTrg_SetTriggerMode (m_hCamera, STCAM_TRIGGER_MODE_TYPE_TRIGGER | STCAM_TRIGGER_MODE_SOURCE_HARDWARE);
	if (ret == FALSE) return false;
	
	DWORD dwDirection=0;
	//Get IO Direction
	ret = StTrg_GetIOPinDirection(m_hCamera, &dwDirection);
	if (ret == FALSE) return false;

	DWORD dwMask = 1 << nIONum;
	dwDirection &= (~dwMask);
	
	//0:Input, 1:Output
	//Set IO Direction
	ret = StTrg_SetIOPinDirection(m_hCamera, dwDirection);
	if (ret == FALSE) return false;

	//Set IO Mode
	ret = StTrg_SetIOPinMode(m_hCamera, nIONum, STCAM_IN_PIN_MODE_TRIGGER_INPUT); 
	if (ret == FALSE) return false;
	
	DWORD dwPolarity=0;

	//Get IO Polarity
	ret = StTrg_GetIOPinPolarity(m_hCamera, &dwPolarity);
	if (ret == FALSE) return false;

	dwMask = 1 << nIONum;
	if (bPolarity == true)
		dwPolarity &= (~dwMask);
	else
		dwPolarity |= dwMask;

	//0:Positive, 1:Negative
	//Set IO Polarity
	ret = StTrg_SetIOPinPolarity(m_hCamera, dwPolarity);
	if (ret == FALSE) return false;

	return true;
}

bool CStCamUSB::SetStrobeMode(int nIONum, bool bPolarity)
{
	BOOL ret=FALSE;

	DWORD dwDirection=0;
	//Get IO Direction
	ret = StTrg_GetIOPinDirection(m_hCamera, &dwDirection);
	if (ret == FALSE) return false;

	DWORD dwMask = 1 << nIONum;
	dwDirection |= dwMask;

	//0:Input, 1:Output
	//Set IO Direction
	ret = StTrg_SetIOPinDirection(m_hCamera, dwDirection);
	if (ret == FALSE) return false;

	//Set IO Mode
	ret = StTrg_SetIOPinMode(m_hCamera, nIONum, STCAM_OUT_PIN_MODE_STROBE_OUTPUT_PROGRAMMABLE); 
	if (ret == FALSE) return false;

	DWORD dwPolarity=0;

	//Get IO Polarity
	ret = StTrg_GetIOPinPolarity(m_hCamera, &dwPolarity);
	if (ret == FALSE) return false;

	dwMask = 1 << nIONum;
	if (bPolarity == true)
		dwPolarity &= (~dwMask);
	else
		dwPolarity |= dwMask;

	//0:Positive, 1:Negative
	//Set IO Polarity
	ret = StTrg_SetIOPinPolarity(m_hCamera, dwPolarity);
	if (ret == FALSE) return false;
	return true;
}

bool CStCamUSB::GetOffsetX(int &nValue)
{
	BOOL ret=FALSE;
	WORD wScanMode=0;
	DWORD dwOffsetX=0,dwOffsetY=0,dwWidth=0,dwHeight=0;
	ret = StTrg_GetScanMode(m_hCamera, &wScanMode, &dwOffsetX, &dwOffsetY, &dwWidth, &dwHeight);
	if (ret == FALSE) return false;

	nValue = dwOffsetX;
	return true;
}
bool CStCamUSB::GetOffsetY(int &nValue)
{
	BOOL ret=FALSE;
	WORD wScanMode=0;
	DWORD dwOffsetX=0,dwOffsetY=0,dwWidth=0,dwHeight=0;
	ret = StTrg_GetScanMode(m_hCamera, &wScanMode, &dwOffsetX, &dwOffsetY, &dwWidth, &dwHeight);
	if (ret == FALSE) return false;

	nValue = dwOffsetY;
	return true;
}
bool CStCamUSB::GetWidth(int &nValue)
{
	BOOL ret=FALSE;
	WORD wScanMode=0;
	DWORD dwOffsetX=0,dwOffsetY=0,dwWidth=0,dwHeight=0;
	ret = StTrg_GetScanMode(m_hCamera, &wScanMode, &dwOffsetX, &dwOffsetY, &dwWidth, &dwHeight);
	if (ret == FALSE) return false;

	nValue = dwWidth;
	return true;
}
bool CStCamUSB::GetHeight(int &nValue)
{
	BOOL ret=FALSE;
	WORD wScanMode=0;
	DWORD dwOffsetX=0,dwOffsetY=0,dwWidth=0,dwHeight=0;
	ret = StTrg_GetScanMode(m_hCamera, &wScanMode, &dwOffsetX, &dwOffsetY, &dwWidth, &dwHeight);
	if (ret == FALSE) return false;

	nValue = dwHeight;
	return true;
}

bool CStCamUSB::GetBpp(int &nValue)
{
	BOOL ret=FALSE;
	WORD wColorArray=0;
	ret = StTrg_GetColorArray(m_hCamera, &wColorArray);
	if (ret == FALSE) return false;

	if (wColorArray == STCAM_COLOR_ARRAY_MONO)
		nValue = 8;
	else
		nValue = 8;

	return true;
}

bool CStCamUSB::GetGain(int &nValue)
{
	BOOL ret=FALSE;
	WORD wGain=0;

	ret = StTrg_GetGain(m_hCamera, &wGain);
	if (ret == FALSE) return false;

	nValue = wGain;
	return true;
}

bool CStCamUSB::GetFrameRate(float &fValue)
{
	BOOL ret=FALSE;
	float fps=0;
	ret = StTrg_GetOutputFPS(m_hCamera, &fps);
	if (ret == FALSE) return false;

	fValue = fps;
	return true;
}

bool CStCamUSB::GetExposureTimeMicroSecond(int &nValue)
{
	BOOL ret=FALSE;
	float fTime=0;
	DWORD dwClock=0;
	ret = StTrg_GetExposureClock(m_hCamera, &dwClock);
	if (ret == FALSE) return false;
	ret = StTrg_GetExposureTimeFromClock(m_hCamera, dwClock, &fTime);
	if (ret == FALSE) return false;

	nValue = (int)(fTime*1000000);
	return true;
}

bool CStCamUSB::SetOffsetX(int nValue)
{
	BOOL ret=FALSE;
	WORD wScanMode=0;
	DWORD dwOffsetX=0,dwOffsetY=0,dwWidth=0,dwHeight=0;
	ret = StTrg_GetScanMode(m_hCamera, &wScanMode, &dwOffsetX, &dwOffsetY, &dwWidth, &dwHeight);

	dwOffsetX = nValue;
	ret = StTrg_SetScanMode(m_hCamera, wScanMode, dwOffsetX, dwOffsetY, dwWidth, dwHeight);
	if (ret == FALSE) return false;

	return true;
}
bool CStCamUSB::SetOffsetY(int nValue)
{
	BOOL ret=FALSE;
	WORD wScanMode=0;
	DWORD dwOffsetX=0,dwOffsetY=0,dwWidth=0,dwHeight=0;
	ret = StTrg_GetScanMode(m_hCamera, &wScanMode, &dwOffsetX, &dwOffsetY, &dwWidth, &dwHeight);

	dwOffsetY = nValue;
	ret = StTrg_SetScanMode(m_hCamera, wScanMode, dwOffsetX, dwOffsetY, dwWidth, dwHeight);
	if (ret == FALSE) return false;

	return true;
}
bool CStCamUSB::SetWidth(int nValue)
{
	BOOL ret=FALSE;
	WORD wScanMode=0;
	DWORD dwOffsetX=0,dwOffsetY=0,dwWidth=0,dwHeight=0;
	ret = StTrg_GetScanMode(m_hCamera, &wScanMode, &dwOffsetX, &dwOffsetY, &dwWidth, &dwHeight);

	dwWidth = nValue;
	ret = StTrg_SetScanMode(m_hCamera, wScanMode, dwOffsetX, dwOffsetY, dwWidth, dwHeight);
	if (ret == FALSE) return false;

	return true;
}
bool CStCamUSB::SetHeight(int nValue)
{
	BOOL ret=FALSE;
	WORD wScanMode=0;
	DWORD dwOffsetX=0,dwOffsetY=0,dwWidth=0,dwHeight=0;
	ret = StTrg_GetScanMode(m_hCamera, &wScanMode, &dwOffsetX, &dwOffsetY, &dwWidth, &dwHeight);

	dwHeight = nValue;
	ret = StTrg_SetScanMode(m_hCamera, wScanMode, dwOffsetX, dwOffsetY, dwWidth, dwHeight);
	if (ret == FALSE) return false;

	return true;
}

bool CStCamUSB::SetGain(int nValue)
{
	BOOL ret=FALSE;
	WORD wGain=0;

	wGain = nValue;
	ret = StTrg_SetGain(m_hCamera, wGain);
	if (ret == FALSE) return false;

	return true;
}
bool CStCamUSB::SetExposureTimeMicroSecond(int nValue)
{
	BOOL ret=FALSE;
	DWORD dwClock=0;
	float fValue = (float)nValue / 1000000;
	ret = StTrg_GetExposureClockFromTime(m_hCamera, fValue, &dwClock);
	if (ret == FALSE) return false;

	ret = StTrg_SetExposureClock(m_hCamera, dwClock);
	if (ret == FALSE) return false;

	return true;
}

bool CStCamUSB::SetStrobeDelayMicroSecond(int nValue)
{
	BOOL ret=FALSE;
	DWORD dwDelay=0;

	dwDelay = nValue;
	ret = StTrg_SetTriggerTiming(m_hCamera, STCAM_TRIGGER_TIMING_STROBO_START_DELAY, dwDelay);
	if (ret == FALSE) return false;
	return true;
}

bool CStCamUSB::SetStrobeOnTimeMicroSecond(int nValue)
{
	BOOL ret=FALSE;
	DWORD dwEnd=0;

	dwEnd = nValue;
	ret = StTrg_SetTriggerTiming(m_hCamera, STCAM_TRIGGER_TIMING_STROBO_END_DELAY, dwEnd);
	if (ret == FALSE) return false;
	return true;
}

bool CStCamUSB::SetMirrorMode(bool bHor, bool bVer)
{
	BOOL ret=FALSE;
	BOOL HasFunc=FALSE;

	// Check the functions availability by StTrg_HasFunction.
	ret = StTrg_HasFunction(m_hCamera, STCAM_CAMERA_FUNCTION_MIRROR_HORIZONTAL | STCAM_CAMERA_FUNCTION_MIRROR_VERTICAL, &HasFunc);
	if (ret == FALSE) return false;

	BYTE mode = 0;
	if (bHor == true)
		mode |= STCAM_MIRROR_HORIZONTAL_CAMERA;
	if (bVer == true)
		mode |= STCAM_MIRROR_VERTICAL_CAMERA;

	ret = StTrg_SetMirrorMode(m_hCamera,mode);
	if (ret == FALSE) return false;	

	return true;
}