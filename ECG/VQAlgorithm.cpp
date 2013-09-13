#include "stdafx.h"
#include "ECG.h"
#include "ECGDlg.h"
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <wchar.h>
#include "CodeBook.h"
#include "gainCodebook.h"
#include "VQClass.h"
#include "gainPredictor.h"
#include "dsc.h"
#include "constants.h"


/*INT vqcb.dimension;
INT vqcb.samplePoint;
INT vqcb.M;*/
const BYTE ST_CYCLE = 1;
const BYTE ST_PERIOD = 3;
const BYTE ST_VQIndex = 4;
const BYTE ST_BREAK = 6;

const int GAIN_HEADER_LENGTH = 5;
const int GAIN_CONTENT_LENGTH = 14; 
const int PERIOD_HEADER_LENGTH = 7;
const int PERIOD_CONTENT_LENGTH = 3;
const int Y_HEADER_LENGTH = 2;
//const int Y_CONTENT_LENGTH = cNormalizedLen / vqcb.samplePoint;
const int DATA_HEADER_LENGTH = 5;

const char *GAIN_HEADER = "GAIN=";
const char *PERIOD_HEADER = "PERIOD=";
const char *Y_HEADER = "Y=";
const char *DATA_HEADER = "DATA=";

const int DSC_OUTPUT_BIT_NUM = 2;

DWORD CECGDlg::VQAlgThread(LPVOID lparam)
{
	CECGDlg * pDlg = (CECGDlg *) lparam;
	pDlg->UpdateData(TRUE);

	//open an file to record original ECG data
#ifdef Debug_PrintPeriodNormalizedData
	FILE *m_EcgFp_int = fopen("int_linear_ECG_bth_source_is_period_normalized.txt","w");
	FILE *m_EcgFp_double = fopen("double_linear_ECG_bth_source_is_period_normalized.txt","w");
#endif
#ifdef Debug_PrintGainValue
	FILE *fp_Gain = fopen("gain.txt", "w");
#endif
#ifdef Debug_PrintVQIndex
	FILE *tt = fopen("index.txt", "w");
#endif
#ifdef Debug_PrintDSCBits
	FILE *fp_dsc = fopen("dsc.txt", "w");
#endif

/*
	static bool firstGainCalculated = true;
	static int PackIndex = 0;
	static int PackGainIndex = 0;
*/

	/***********************************************************************
	The format of data received from bluetooth tells us that every two bytes
	data compose a ECG sample. So a sample is two bytes long.
	So we use UINT16 instead of UINT to save memory space.
	***********************************************************************/
/*

	double *PackedNumber = new double [vqcb.samplePoint];
	if(PackedNumber == NULL)
	{
		pDlg->UpdateStatus(L"In VQAlgThread: new PackedNumber error!", ADDSTR2STATUS);
        return 1;
	}

	INT CompressedDataIndex = 0;
	INT CompressedDataIndexCnt = 0;
	INT CompressedGainIndex = 0;
	INT CompressedGainIndexCnt = 0;
    INT bits_cnt = 0;
    INT tx_data = 0;



	VQClass vq(vqcb.dimension, vqcb.M, vqcb.samplePoint, pDlg->m_strY);
*/	
	
	//vq.showBitTable(vqcb.dimension);
	//vq.showY();
/*
	char *indexData = new char [BufSize*4/vqcb.samplePoint];
	if(indexData == NULL)
	{
		pDlg->UpdateStatus(L"In VQAlgThread: new indexData error!", ADDSTR2STATUS);
        return 1;
	}

	int DataAlignment = SIZEofCHAR;
*/
	double *buf = new double [BufSize*4];

	int *sendbuf = new int[BufSize*4];
	int sendlen = 0;

	bool flag = true;
	char dbgstr[30];

	if(buf == NULL)
	{
		pDlg->UpdateStatus(L"In VQAlgThread: new buf error!", ADDSTR2STATUS);
        return 1;
	}
	DWORD buflen;

	int periodCnt = 0;
	int *periodLen = new int [cPeriodNum];
	if(periodLen == NULL)
	{
		pDlg->UpdateStatus(L"In BTHRecvThread: new periodLen error!", ADDSTR2STATUS);
        return FALSE;
	}
	else
		memset(periodLen, 0, sizeof(int) * cPeriodNum);

/*
	gainPredictor generateGain(default_alpha);

	double gain = 0.0;

	int j;
	BYTE CurrentState;
	CurrentState = ST_CYCLE;
	int YCnt = 0;
	int YdataLen = vq.getVectorPerCycle();
    bool YSent = false;
*/
	while(TRUE)
	{
		if(WaitForSingleObject(pDlg->m_ExitVQThreadEvent, 0) == WAIT_OBJECT_0)
			break;

		EnterCriticalSection(&m_csBTHDataBuf);

		if(SharedMem_BTHDataBufLen == 0)
		{
			LeaveCriticalSection(&m_csBTHDataBuf);
			continue;
		}
		buflen = SharedMem_BTHDataBufLen;
		memcpy(buf, SharedMem_BTHDataBuf, sizeof(double) * SharedMem_BTHDataBufLen);
		SharedMem_BTHDataBufLen = 0;

		periodCnt = SharedMem_PeriodCnt;
		memcpy(periodLen, SharedMem_PeriodLen, sizeof(int) * periodCnt);

		LeaveCriticalSection(&m_csBTHDataBuf);

		
		

		
		for(short i=0 ;i<buflen; i++)
			sendbuf[i]=buf[i];

		memmove(sendbuf+1, sendbuf, buflen*sizeof(int));
		sendbuf[0] = periodCnt;


#ifdef Debug_PrintPeriodNormalizedData
		if(flag){
			for(short  j = 0 ; j < buflen ; j++ ){
				fprintf(m_EcgFp_double,"%04lf\n", buf[j]);
				fprintf(m_EcgFp_int,"%d\n", sendbuf[j]);
			}
			fprintf(m_EcgFp_int,"%d\n", sendbuf[buflen]);
			flag = false;
		}
#endif


		//Important! java socket on PC only accept big-endian data, which is the same endianess of the network's
		for(short i=0 ;i<buflen+1; i++)
			sendbuf[i]=::htonl(sendbuf[i]);


		//note that the following line can only be executed after wifi is connected
		sendlen = (buflen+1)*sizeof(int);
		sprintf(dbgstr,"Send %d bytes!", sendlen);



		if( ::send(pDlg->s, (char*)sendbuf, sendlen , 0) != sendlen ){
			pDlg->UpdateStatus(L"Send Data Failed!", ADDSTR2STATUS);
		}else{
			pDlg->UpdateStatus(CString(dbgstr), ADDSTR2STATUS);
		}

    /*
		CompressedDataIndexCnt = 0;
		CompressedGainIndexCnt = 0;
		//implement VQ Algorithm here!

	

		for(DWORD i = 0 ; i < buflen ; i++ )
		{
			PackedNumber[PackIndex] = buf[i];
			PackIndex++;

			if(PackIndex == vqcb.samplePoint)
			{
				PackIndex = 0;
#ifdef Debug_PrintPeriodNormalizedData
				for( j = 0 ; j < vqcb.samplePoint ; j++ )
					fprintf(m_EcgFp,"%04lf\n", PackedNumber[j]);
#endif
				gain = generateGain.computeGain(firstGainCalculated, PackedNumber, vq.getACodeBookEntry(CompressedDataIndex), vqcb.samplePoint);
#ifdef Debug_PrintGainValue
				fprintf(fp_Gain, "%lf\n", gain);
#endif
				for( j = 0 ; j < vqcb.samplePoint ; j++ )
					PackedNumber[j] /= gain;


				vq.LookUpCodeBook(PackedNumber, CompressedDataIndex);

				//The following three lines are originally commented by WeiYing Tsai

				//WCHAR strErr[128];
				//wsprintf(strErr, L"%lf\n", gain);
				//pDlg->UpdateStatus(strErr, ADDSTR2STATUS);

				indexData[CompressedDataIndexCnt++] = char(CompressedDataIndex);
			}
		}


#ifdef Debug_PrintVQIndex
		for( int i = 0 ; i < CompressedDataIndexCnt ; i++)
		{
			fprintf(tt, "%03d\n", int(indexData[i]));
		}
#endif
		WCHAR strLen[40];

		if( CompressedDataIndexCnt == 0 )
			continue;
		EnterCriticalSection(&m_csCompressedDataBuf);
		SharedMem_CompressedDataBufLen = 0;
		while(1)
		{
			switch (CurrentState)
			{
			case ST_CYCLE:
				SharedMem_CompressedDataBuf[0] = char(periodCnt);
				SharedMem_CompressedDataBufLen += 1;
				CurrentState = ST_PERIOD;
				break;
			case ST_PERIOD:
				for( int i = 0 ; i < periodCnt ; i++ )
				{
					sprintf(SharedMem_CompressedDataBuf+SharedMem_CompressedDataBufLen, "%03d", periodLen[i]);
					SharedMem_CompressedDataBufLen += PERIOD_CONTENT_LENGTH;
				}
				CurrentState = ST_VQIndex;
				break;
			case ST_VQIndex:
				memcpy(SharedMem_CompressedDataBuf+SharedMem_CompressedDataBufLen, indexData, sizeof(char)*CompressedDataIndexCnt);
				SharedMem_CompressedDataBufLen += CompressedDataIndexCnt;
				CurrentState = ST_BREAK;
				break;
			}

			if(CurrentState == ST_BREAK)
			{
				sprintf(SharedMem_CompressedDataBuf+SharedMem_CompressedDataBufLen, "%s", "\n");
				SharedMem_CompressedDataBufLen += 1;
				CurrentState = ST_CYCLE;
				break;
			}
		}
		LeaveCriticalSection(&m_csCompressedDataBuf);

	*/


	}
	


	delete [] sendbuf;

	delete [] buf;
//	delete [] PackedNumber;
//	delete [] indexData;
	delete [] periodLen;



#ifdef Debug_PrintGainValue
	fclose(fp_Gain);
#endif
#ifdef Debug_PrintVQIndex
	fclose(tt);
#endif
#ifdef Debug_PrintDSCBits
	fclose(fp_dsc);
#endif
#ifdef Debug_PrintPeriodNormalizedData
	fclose(m_EcgFp_int);
	fclose(m_EcgFp_double);
#endif
	
	return 0;
}

BOOL CECGDlg::LoadCodeBook()
{
	UpdateData(TRUE);
	int len = m_strCodeBookFilename.GetLength();
	char *pfilename = new char [len];
	if(pfilename == NULL)
	{
		UpdateStatus(L"In LoadCodeBook: new pfilename error!", ADDSTR2STATUS);
        return FALSE;
	}
	for(int i = 0 ; i < len ; i++)
		pfilename[i] = (char)m_strCodeBookFilename.GetAt(i);
	pfilename[len] = '\0';
	
	FILE *CodeBookfp = fopen(pfilename, "r");

	if( !CodeBookfp )
	{
		UpdateStatus(L"Failed to Open Code Book!", ADDSTR2STATUS);
        return FALSE;
	}
	delete [] pfilename;

	vqcb.dimension = _ttoi(m_strDimension);
	vqcb.samplePoint = _ttoi(m_strSamplePoint);
	vqcb.M = countM(vqcb.dimension);

	BOOL ret = CreateCodeWord(vqcb.codeWord, vqcb.dimension, vqcb.samplePoint);
	if(ret == FALSE)
	{
		UpdateStatus(L"In CreateCodeWord: new codeWord error!", ADDSTR2STATUS);
		return FALSE;
	}

	/*vqcb.codeWord = new double *[vqcb.dimension];
	if(vqcb.codeWord == NULL)
	{
		UpdateStatus(L"In LoadCodeBook: new codeWord error!", ADDSTR2STATUS);
        return FALSE;
	}
	for( INT i = 0 ; i < vqcb.dimension ; i++)
	{
		vqcb.codeWord[i] = new double [vqcb.samplePoint];
		if(vqcb.codeWord[i] == NULL)
		{
			UpdateStatus(L"In LoadCodeBook: new codeWord[i] error!", ADDSTR2STATUS);
			return FALSE;
		}
	}*/

	
	for( INT i = 0 ; i < vqcb.dimension ; i++)
	{
		for( INT j = 0 ; j < vqcb.samplePoint ; j++)
		{
			fscanf(CodeBookfp, "%lf", &vqcb.codeWord[i][j]);
		}
	}
	fclose(CodeBookfp);

	//For testing load code book successfully.
#ifdef Debug_PrintShapeCodebook
	FILE *t = fopen("myBook.txt", "w");
	for( INT i = 0 ; i < vqcb.dimension ; i++)
	{
		for( INT j = 0 ; j < vqcb.samplePoint ; j++)
		{
			fprintf(t, "\t%0.20lf", vqcb.codeWord[i][j]);
		}
		fprintf(t, "\n");
	}
	fclose(t);
#endif

	return TRUE;
}

BOOL CECGDlg::LoadGainCodeBook()
{
	UpdateData(TRUE);
	int len = m_strGainCodeBookFilename.GetLength();
	char *pfilename = new char [len];
	if(pfilename == NULL)
	{
		UpdateStatus(L"In LoadCodeBook: new pfilename error!", ADDSTR2STATUS);
        return FALSE;
	}
	for(int i = 0 ; i < len ; i++)
		pfilename[i] = (char)m_strGainCodeBookFilename.GetAt(i);
	pfilename[len] = '\0';
	
	FILE *CodeBookfp = fopen(pfilename, "r");

	if( !CodeBookfp )
	{
		UpdateStatus(L"Failed to Open Code Book!", ADDSTR2STATUS);
        return FALSE;
	}
	delete [] pfilename;

	gcb.dimension = _ttoi(m_strGainDimension);
	gcb.samplePoint = _ttoi(m_strGainSamplePoint);
	gcb.M = countM(gcb.dimension);

	BOOL ret = CreateCodeWord(gcb.codeWord, gcb.dimension, gcb.samplePoint);
	if(ret == FALSE)
	{
		UpdateStatus(L"In CreateCodeWord: new codeWord error!", ADDSTR2STATUS);
		return FALSE;
	}

	/*gcb.codeWord = new double *[gcb.dimension];
	if(gcb.codeWord == NULL)
	{
		UpdateStatus(L"In LoadGainCodeBook: new codeWord error!", ADDSTR2STATUS);
        return FALSE;
	}
	for( INT i = 0 ; i < gcb.dimension ; i++)
	{
		gcb.codeWord[i] = new double [gcb.samplePoint];
		if(gcb.codeWord[i] == NULL)
		{
			UpdateStatus(L"In LoadGainCodeBook: new codeWord[i] error!", ADDSTR2STATUS);
			return FALSE;
		}
	}*/

	
	for( INT i = 0 ; i < gcb.dimension ; i++)
	{
		for( INT j = 0 ; j < gcb.samplePoint ; j++)
		{
			fscanf(CodeBookfp, "%lf", &gcb.codeWord[i][j]);
		}
	}
	fclose(CodeBookfp);

	//For testing load code book successfully.
#ifdef Debug_PrintGainCodebook
	FILE *t = fopen("myGainBook.txt", "w");
	for( INT i = 0 ; i < gcb.dimension ; i++)
	{
		for( INT j = 0 ; j < gcb.samplePoint ; j++)
		{
			fprintf(t, "\t%0.20lf", gcb.codeWord[i][j]);
		}
		fprintf(t, "\n");
	}
	fclose(t);
#endif

	return TRUE;
}

void CECGDlg::OnBnClickedStart()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	int ret;

	//Load codebook first
	ret = LoadCodeBook();
	if( !ret )
		return;

	ret = LoadGainCodeBook();
	if( !ret )
		return;

	DWORD IDThread;
	HANDLE hVQAlgThread;

	SendStartCommand();

	//InitializeCriticalSection(&m_csCompressedDataBuf);

	m_ExitVQThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    hVQAlgThread = CreateThread(0, 0, VQAlgThread, this, 0, &IDThread);
    if(hVQAlgThread == NULL)
    {
		UpdateStatus(L"Failed to create VQ Algorithm threads!", ADDSTR2STATUS);
        return;
    }
    CloseHandle(hVQAlgThread);

	m_BtnStart.EnableWindow(FALSE);
	m_BtnEnd.EnableWindow(TRUE);
	m_BtnCloseBTSPP.EnableWindow(FALSE);

	//Display status
	UpdateStatus(L"Start!", ADDSTR2STATUS);
}

BOOL CECGDlg::End()
{
	DeleteCodeWord(vqcb.codeWord, vqcb.dimension);
	DeleteCodeWord(gcb.codeWord, gcb.dimension);
	
	//DeleteCriticalSection(&m_csCompressedDataBuf);
	return 0;
}

void CECGDlg::OnBnClickedEnd()
{
	// TODO: 在此加入控制項告知處理常式程式碼
	if (m_ExitVQThreadEvent != NULL)
	{
		SetEvent(m_ExitVQThreadEvent);
		Sleep(1000);
		CloseHandle(m_ExitVQThreadEvent);
		m_ExitVQThreadEvent = NULL;
	}

	SendEndCommand();

	m_BtnEnd.EnableWindow(FALSE);
	m_BtnStart.EnableWindow(TRUE);
	m_BtnCloseBTSPP.EnableWindow(TRUE);

	End();

	//Display status
	UpdateStatus(L"End!", ADDSTR2STATUS);
}

BOOL CECGDlg::SendStartCommand()
{
	// TODO: Add your control notification handler code here
	DWORD dwactlen;
	if(m_hComm == INVALID_HANDLE_VALUE)
	{
		UpdateStatus(L"Serial port is not open!!", ADDSTR2STATUS);
		return FALSE;
	}

	
	//Important to delay 100 miniseconds.
	Sleep(100);
	WriteFile(m_hComm, "\r", 1, &dwactlen, NULL);
	Sleep(100);
	WriteFile(m_hComm, "INIT\r", 5, &dwactlen, NULL);
	Sleep(100);
	WriteFile(m_hComm, "W+\r", 3, &dwactlen, NULL);
	Sleep(100);
	t1 = CTime::GetCurrentTime();
	Sleep(100);
	WriteFile(m_hComm, "S+\r", 3, &dwactlen, NULL);

	return TRUE;
}

BOOL CECGDlg::SendEndCommand()
{
	// TODO: Add your control notification handler code here
	if(m_hComm == INVALID_HANDLE_VALUE)
	{
		MessageBox(_T("Serial port is not open!"));
		return FALSE;
	}
	DWORD dwactlen;

	//Important to delay 100 miniseconds.
	Sleep(100);
	WriteFile(m_hComm, "S-\r", 3, &dwactlen, NULL);
	Sleep(100);
	WriteFile(m_hComm, "W-\r", 3, &dwactlen, NULL);
	Sleep(100);
	t2 = CTime::GetCurrentTime();
	Sleep(100);
	WriteFile(m_hComm, "RS\r", 3, &dwactlen, NULL);
	Sleep(100);

	FILE *ofp = fopen("time.txt", "w");
	fwprintf(ofp, L"%s\n", t1.Format(L"%Y-%m-%d  %H:%M:%S"));
	fwprintf(ofp, L"%s\n", t2.Format(L"%Y-%m-%d  %H:%M:%S"));
	fclose(ofp);

	return TRUE;
}

BOOL CECGDlg::CreateCodeWord(double **&codeWord, INT dimension, INT samplePoint)
{
	codeWord = new double *[dimension];
	if(codeWord == NULL)
        return FALSE;
	for( INT i = 0 ; i < dimension ; i++)
	{
		codeWord[i] = new double [samplePoint];
		if(codeWord[i] == NULL)
			return FALSE;
	}

	return TRUE;
}

void CECGDlg::DeleteCodeWord(double **&codeWord, INT dimension)
{
	INT i;
	for( i = 0 ; i < dimension ; i++)
	{
		if(codeWord[i] != NULL)
		{
			delete [] codeWord[i];
			codeWord[i] = NULL;
		}
	}
	if(codeWord != NULL)
	{
		delete [] codeWord;
		codeWord = NULL;
	}
}