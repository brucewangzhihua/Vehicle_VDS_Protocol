/**********************************************************************
Copyright:		YiXiong AUTO S&T Co., Ltd.
Description:	������������ݴ�����
History:
	<author>	<time>		<desc>

************************************************************************/
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dtc_lib.h"
#include "../public/public.h"
#include <assert.h>
#include "../InitConfigFromXml/init_config_from_xml_lib.h"
#include "../SpecialFunction/special_function.h"

STRUCT_SELECT_FUN stReadCurrentDTCFunGroup[] =
{
	{GENERAL_CURRENT_DTC, process_general_read_current_Dtc},
	{SEA1939_CURRENT_DTC, process_SEA1939_read_current_Dtc},
	{SPECIAL_CURRENT_DTC, process_special_read_current_Dtc},
};

/*************************************************
Description:	��ȡ����ǰ�뺯��
Input:
	cType		��������
Output:	����
Return:	pf_general_function ����ָ��
Others:
*************************************************/
pf_general_function get_read_current_DTC_fun( byte cType )
{
	int i = 0;

	for( i = 0; i < sizeof( stReadCurrentDTCFunGroup ) / sizeof( stReadCurrentDTCFunGroup[0] ); i++ )
		if( cType == stReadCurrentDTCFunGroup[i].cType )
			return stReadCurrentDTCFunGroup[i].pFun;

	return 0;
}

/*************************************************
Description:	��ȡ��ǰ��
Input:	pIn		���������������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
void process_read_current_Dtc( void* pIn, void* pOut )
{
	pf_general_function pFun = NULL;
	pFun = get_read_current_DTC_fun( g_p_stProcessFunConfig->cCurrentDTCFunOffset );

	assert( pFun );

	pFun( pIn, pOut );
}

STRUCT_SELECT_FUN stReadHistoryDTCFunGroup[] =
{
	{GENERAL_HISTORY_DTC, process_general_read_history_Dtc},
	{SEA1939_HISTORY_DTC, process_SEA1939_general_read_history_Dtc},
};
/*************************************************
Description:	��ȡ����ʷ�뺯��
Input:
	cType		��������
Output:	����
Return:	pf_general_function ����ָ��
Others:
*************************************************/
pf_general_function get_read_history_DTC_fun( byte cType )
{
	int i = 0;

	for( i = 0; i < sizeof( stReadHistoryDTCFunGroup ) / sizeof( stReadHistoryDTCFunGroup[0] ); i++ )
		if( cType == stReadHistoryDTCFunGroup[i].cType )
			return stReadHistoryDTCFunGroup[i].pFun;

	return 0;
}

/*************************************************
Description:	��ȡ��ʷ��
Input:	pIn		���������������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
void process_read_history_Dtc( void* pIn, void* pOut )
{
	pf_general_function pFun = NULL;

	pFun = get_read_history_DTC_fun( g_p_stProcessFunConfig->cHistoryDTCFunOffset );

	assert( pFun );

	pFun( pIn, pOut );
}
/*************************************************
Description:	��ȡ��ǰ�����룬����֡���ÿ֡�������һ��������
Input:	pIn		���������������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
void process_special_read_current_Dtc( void* pIn, void* pOut )
{
	STRUCT_CHAIN_DATA_INPUT* pstParam = ( STRUCT_CHAIN_DATA_INPUT* )pIn;
	int iReceiveResult = TIME_OUT;

	int iDTCConfigOffset = 0;
	byte cDtcConfigType = 0;

	int iProcessStatus;//����״̬
	byte cDtcMask = 0;

	byte DTCHead[255];
	byte *DTCTemp;
	byte cDtcStartOffset;
	byte cDtcBytesInCmd;
	int iDtcValidLen = 0;//�����������ֽڳ���
	uint32 u32CmdTemp[255] = {0};//�������������255��
	uint32 piCmdIndex[2] = {1,0};
	byte cBufferOffset = 0;
	int i = 0, j = 0, iSum = 0;;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	iDTCConfigOffset = atoi( pstParam->pcData );//��ù���������ID

	pstParam = pstParam->pNextNode;
	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	cDtcConfigType = ( byte )atoi( pstParam->pcData ); //��ù���������ģ���

	select_dtc_config( iDTCConfigOffset, cDtcConfigType );

	cDtcStartOffset	    = g_p_stGeneralReadDtcConfig->cDtcStartOffset;	//��Ч�ظ��б���DTC��ʼƫ�ƣ���SID�ظ���ʼ
	cDtcBytesInCmd		= g_p_stGeneralReadDtcConfig->cDtcBytesInCmd;	//�����м����ֽڱ�ʾһ��������
	cDtcMask			= g_p_stGeneralReadDtcConfig->cCurrentDtcMask;	//��ǰ��״̬

	DTCTemp = DTCHead + cDtcStartOffset;//Ϊ���ݹ����봦�������ӹ�������ʼ�ֽڿ�ʼ�洢���
	iDtcValidLen = cDtcStartOffset;

	pstParam = pstParam->pNextNode;
	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	//��ȡ����������������������
	iSum = get_string_type_data_to_uint32( u32CmdTemp, pstParam->pcData, pstParam->iLen );

	//���Ͷ���������������Ĺ����뱣������
	for( i = 0; i < iSum; i++ )
	{
		//���ڴ˴����ӶԶ�������Ĵ���
		piCmdIndex[1] = u32CmdTemp[i];
		iReceiveResult = send_and_receive_cmd( piCmdIndex );
		cBufferOffset = g_stInitXmlGobalVariable.m_p_stCmdList[ piCmdIndex[1] ].cBufferOffset;

		switch( iReceiveResult )
		{
			case SUCCESS:
				for ( j = 0; j < cDtcBytesInCmd; j ++ )
				{
					//���Ϊ0����Ϊ����
					if ( 0 != g_stBufferGroup[cBufferOffset].cBuffer[ cDtcStartOffset + j ])
					{
						break;
					}
				}
				if ( j == cDtcBytesInCmd)
				{
					continue;
				}
				//�洢���
				memcpy( DTCTemp, g_stBufferGroup[cBufferOffset].cBuffer + cDtcStartOffset, cDtcBytesInCmd );
				DTCTemp = DTCTemp + cDtcBytesInCmd;
				iDtcValidLen += cDtcBytesInCmd;
				break;

			case NEGATIVE:
			case FRAME_TIME_OUT:
			case TIME_OUT:
			default:
				general_return_status( iReceiveResult, g_stBufferGroup[cBufferOffset].cBuffer, 3, pOut );
				break;
		}

	}
	//������ź�״̬
	iProcessStatus = process_Dtc_data( DTCHead, iDtcValidLen, pOut, cDtcMask , 1);
	if( !iProcessStatus )
	{
		special_return_status( PROCESS_OK | HAVE_JUMP | HAVE_TIP, "return", "NO_CURRENT_DTC", 0,pOut );
	}
}
/*************************************************
Description:	��ȡ��ǰ������
Input:	pIn		���������������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
void process_general_read_current_Dtc( void* pIn, void* pOut )
{
	STRUCT_CHAIN_DATA_INPUT* pstParam = ( STRUCT_CHAIN_DATA_INPUT* )pIn;
	int iReceiveResult = TIME_OUT;
	int iReceiveValidLen = 0;//���յ�����Ч�ֽڳ���

	int iDtcNum;//������״̬

	byte cDtcMask = 0;

	byte cBufferOffset = 0;
	int iDTCConfigOffset = 0;
	byte cConfigTemp[4] = {0};
	uint32 u32CmdIndex[6] = {0};
	int i = 0;
	byte cDtcConfigType = 0;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	iDTCConfigOffset = atoi( pstParam->pcData );//��ù���������ID

	pstParam = pstParam->pNextNode;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	cDtcConfigType = ( byte )atoi( pstParam->pcData ); //��ù���������ģ���

	select_dtc_config( iDTCConfigOffset, cDtcConfigType );

	pstParam = pstParam->pNextNode;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	//��ȡ����������������������
	u32CmdIndex[0] = get_string_type_data_to_uint32( u32CmdIndex + 1, pstParam->pcData, pstParam->iLen );

	g_MulCmdOffset = g_p_stGeneralReadDtcConfig->cDtcStartOffset;

	iReceiveResult = send_and_receive_cmd( u32CmdIndex );
	cBufferOffset = g_stInitXmlGobalVariable.m_p_stCmdList[ u32CmdIndex[ u32CmdIndex[0] ] ].cBufferOffset;
	iReceiveValidLen = g_stBufferGroup[cBufferOffset].iValidLen;
	cDtcMask = g_p_stGeneralReadDtcConfig->cCurrentDtcMask;

	pstParam = pstParam->pNextNode;//DRIVINGRECORD_DTC_judgeֵ

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	switch( iReceiveResult )
	{
		case SUCCESS:
		{
			iDtcNum = process_Dtc_data( g_stBufferGroup[cBufferOffset].cBuffer, iReceiveValidLen, pOut, cDtcMask, 1);
			if( !iDtcNum )
			{
				if(0 == atoi( pstParam->pcData ))//����ʱ�ж��г���¼����ʾ
				{
				special_return_status( PROCESS_OK | HAVE_JUMP | HAVE_TIP, "return", "NO_CURRENT_DTC", 0,pOut );  
				}
			}
		}
		break;

		case NEGATIVE:
		case FRAME_TIME_OUT:
		case TIME_OUT:
		default:
			general_return_status( iReceiveResult, g_stBufferGroup[cBufferOffset].cBuffer, 3, pOut );
			break;
	}
}



/*************************************************
Description:	��ȡ��ǰ������
Input:	pIn		���������������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
void process_SEA1939_read_current_Dtc( void* pIn, void* pOut )
{
	STRUCT_CHAIN_DATA_INPUT* pstParam = ( STRUCT_CHAIN_DATA_INPUT* )pIn;
	int iReceiveResult = TIME_OUT;
	int iReceiveValidLen = 0;//���յ�����Ч�ֽڳ���

	int iDtcNum;//������״̬

	byte cDtcMask = 0;

	byte cBufferOffset = 0;
	int iDTCConfigOffset = 0;
	byte cConfigTemp[4] = {0};
	uint32 u32CmdIndex[6] = {0};
	int i = 0;
	byte cDtcConfigType = 0;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	iDTCConfigOffset = atoi( pstParam->pcData );//��ù���������ID  

	pstParam = pstParam->pNextNode;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	cDtcConfigType = ( byte )atoi( pstParam->pcData ); //��ù���������ģ���

	select_dtc_config( iDTCConfigOffset, cDtcConfigType );

	pstParam = pstParam->pNextNode;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	//��ȡ����������������������
	u32CmdIndex[0] = get_string_type_data_to_uint32( u32CmdIndex + 1, pstParam->pcData, pstParam->iLen );

	iReceiveResult = send_and_receive_cmd( u32CmdIndex );
	cBufferOffset = g_stInitXmlGobalVariable.m_p_stCmdList[ u32CmdIndex[u32CmdIndex[0]] ].cBufferOffset;
	iReceiveValidLen = g_stBufferGroup[cBufferOffset].iValidLen;
	cDtcMask = g_p_stGeneralReadDtcConfig->cCurrentDtcMask;   

	switch( iReceiveResult )
	{
		case SUCCESS:
			iDtcNum = process_SEA_1939_Dtc_data( g_stBufferGroup[cBufferOffset].cBuffer, iReceiveValidLen, pOut, cDtcMask );

			if( !iDtcNum )
			{
				special_return_status( PROCESS_OK | HAVE_JUMP | HAVE_TIP, "return", "NO_CURRENT_DTC", 0,pOut );
			}
			break;

		case NEGATIVE:
		case FRAME_TIME_OUT:
		case TIME_OUT:
		default:
			general_return_status( iReceiveResult, g_stBufferGroup[cBufferOffset].cBuffer, 3, pOut );
			break;
	}
}


/*************************************************
Description:	��ȡ1939��ʷ������
Input:	pIn		���������������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
void process_SEA1939_general_read_history_Dtc( void* pIn, void* pOut )
{
	STRUCT_CHAIN_DATA_INPUT* pstParam = ( STRUCT_CHAIN_DATA_INPUT* )pIn;
	int iReceiveResult = TIME_OUT;
	int iReceiveValidLen = 0;//���յ�����Ч�ֽڳ���

	int iDtcNum;//������״̬

	byte cDtcMask = 0;

	byte cBufferOffset = 0;
	int iDTCConfigOffset = 0;
	byte cConfigTemp[4] = {0};
	uint32 u32CmdIndex[6] = {0};
	int i = 0;
	byte cDtcConfigType = 0;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	iDTCConfigOffset = atoi( pstParam->pcData );//��ù���������ID

	pstParam = pstParam->pNextNode;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	cDtcConfigType = ( byte )atoi( pstParam->pcData ); //��ù���������ģ���

	select_dtc_config( iDTCConfigOffset, cDtcConfigType );

	pstParam = pstParam->pNextNode;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	//��ȡ����������������������
	u32CmdIndex[0] = get_string_type_data_to_uint32( u32CmdIndex + 1, pstParam->pcData, pstParam->iLen );

	iReceiveResult = send_and_receive_cmd( u32CmdIndex );

	cBufferOffset = g_stInitXmlGobalVariable.m_p_stCmdList[ u32CmdIndex[u32CmdIndex[0]] ].cBufferOffset;
	iReceiveValidLen = g_stBufferGroup[cBufferOffset].iValidLen;
	cDtcMask = g_p_stGeneralReadDtcConfig->cHistoryDtcMask;

	switch( iReceiveResult )
	{
		case SUCCESS:
			iDtcNum = process_SEA_1939_Dtc_data( g_stBufferGroup[cBufferOffset].cBuffer, iReceiveValidLen, pOut, cDtcMask );

			if( !iDtcNum )
			{
				special_return_status( PROCESS_OK | HAVE_JUMP | HAVE_TIP, "return", "NO_HISTORY_DTC", 0,pOut );
			}
			break;

		case NEGATIVE:
		case FRAME_TIME_OUT:
		case TIME_OUT:
		default:
			general_return_status( iReceiveResult, g_stBufferGroup[cBufferOffset].cBuffer, 3, pOut );
			break;
	}
}

/*************************************************
Description:	��ȡ��ʷ������
Input:	pIn		���������������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
void process_general_read_history_Dtc( void* pIn, void* pOut )
{
	STRUCT_CHAIN_DATA_INPUT* pstParam = ( STRUCT_CHAIN_DATA_INPUT* )pIn;
	int iReceiveResult = TIME_OUT;
	int iReceiveValidLen = 0;//���յ�����Ч�ֽڳ���

	int iDtcNum;//������״̬

	byte cDtcMask = 0;

	byte cBufferOffset = 0;
	int iDTCConfigOffset = 0;
	byte cConfigTemp[4] = {0};
	uint32 u32CmdIndex[6] = {0};
	int i = 0;
	byte cDtcConfigType = 0;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	iDTCConfigOffset = atoi( pstParam->pcData );//��ù���������ID

	pstParam = pstParam->pNextNode;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	cDtcConfigType = ( byte )atoi( pstParam->pcData ); //��ù���������ģ���

	select_dtc_config( iDTCConfigOffset, cDtcConfigType );

	pstParam = pstParam->pNextNode;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );
	
	g_MulCmdOffset = (g_KwpFunSaveOffset.cDtcOffset != 0)?g_KwpFunSaveOffset.cDtcOffset:g_p_stGeneralReadDtcConfig->cDtcStartOffset;/* �������֡ƫ�� */
	 
	//��ȡ����������������������
	u32CmdIndex[0] = get_string_type_data_to_uint32( u32CmdIndex + 1, pstParam->pcData, pstParam->iLen );

	iReceiveResult = send_and_receive_cmd( u32CmdIndex );

	cBufferOffset = g_stInitXmlGobalVariable.m_p_stCmdList[ u32CmdIndex[u32CmdIndex[0]] ].cBufferOffset;
	iReceiveValidLen = g_stBufferGroup[cBufferOffset].iValidLen;
	cDtcMask = g_p_stGeneralReadDtcConfig->cHistoryDtcMask;

	switch( iReceiveResult )
	{
		case SUCCESS:
		{
			iDtcNum = process_Dtc_data( g_stBufferGroup[cBufferOffset].cBuffer, iReceiveValidLen, pOut, cDtcMask, 1);
			if( !iDtcNum )
			{
				special_return_status( PROCESS_OK | HAVE_JUMP | HAVE_TIP, "return", "NO_HISTORY_DTC", 0,pOut );
			}
		}
		break;

		case NEGATIVE:
		case FRAME_TIME_OUT:
		case TIME_OUT:
		default:
			general_return_status( iReceiveResult, g_stBufferGroup[cBufferOffset].cBuffer, 3, pOut );
			break;
	}
}

/*************************************************
Description:	�������������
Input:
	pcDctData	������ظ�����洢��ַ
	iValidLen	��Ч����
	cDtcMask	������״̬

Output:	pstDtc	�������ָ��
Return:	int		���������
Others:
*************************************************/
int process_Dtc_data( byte* pcDctData, int iValidLen, void* pOut, byte cDtcMask, bool bFunction )
{
	int i = 0;
	int iDtcNum = 0;
	int iDataLen;
	byte *pcDtcStart = NULL;//��������ʼ��ŵ�ַ
	byte cDtcStatusCache[256] = {0};
	byte cDtcStartOffset = 0, cDtcBytesInCmd = 0, cStatusOffset = 0, cDtcBytesInDisplay = 0;

	if (bFunction)
	{
		cDtcStartOffset	    = g_p_stGeneralReadDtcConfig->cDtcStartOffset;	//��Ч�ظ��б���DTC��ʼƫ�ƣ���SID�ظ���ʼ
		cDtcBytesInCmd		= g_p_stGeneralReadDtcConfig->cDtcBytesInCmd;	//�����м����ֽڱ�ʾһ��������
		cStatusOffset		= g_p_stGeneralReadDtcConfig->cStatusOffset;	//������״̬��һ�������������е�ƫ��
		cDtcBytesInDisplay  = g_p_stGeneralReadDtcConfig->cDtcBytesInDisplay;//һ����������Ҫ��ʾ�����ֽ�	
	}
	else
	{
		cDtcStartOffset		= g_p_stUDSFreezeDtcConfig->cFreezeDtcStartOffset;	//��Ч�ظ��б������DTC��ʼƫ�ƣ���SID�ظ���ʼ
		cDtcBytesInCmd		= g_p_stUDSFreezeDtcConfig->cDtcBytesInCmd;	//�����м����ֽڱ�ʾһ��������
		cDtcBytesInDisplay	= g_p_stUDSFreezeDtcConfig->cDtcBytesInDisplay;//һ����������Ҫ��ʾ�����ֽ�
	}
	if (iValidLen >= cDtcStartOffset)//��Ч���ݱ�ƫ�ƴ�
	{
		iDtcNum = ( iValidLen -  cDtcStartOffset ) / cDtcBytesInCmd;
	}
	else
	{
		return 0;
	}
	iDataLen = special_return_status( PROCESS_OK | NO_JUMP | NO_TIP, NULL, NULL, 0, pOut );

	pcDtcStart = pcDctData + cDtcStartOffset;
		
	for( i = 0; i < iDtcNum; i++ )
	{
		//���������״̬���
		get_Dtc_status( pcDtcStart[cStatusOffset + i * cDtcBytesInCmd], cDtcStatusCache, cDtcMask );

		iDataLen = add_data_and_controlsID( iDataLen, pcDtcStart + ( i * cDtcBytesInCmd ), cDtcBytesInDisplay, PCBU_PRINT, cDtcStatusCache, pOut  );
	}
	return iDtcNum;
}


/*************************************************
Description:	SEA1939�������������
Input:
	pcDctData	������ظ�����洢��ַ
	iValidLen	��Ч����
	cDtcMask	������״̬

Output:	pstDtc	�������ָ��
Return:	int		���������
Others:
*************************************************/
int process_SEA_1939_Dtc_data( byte* pcDctData, int iValidLen, void* pOut, byte cDtcMask )
{
	int i = 0;
	int j = 0;
	byte* pOutTemp = (byte*)pOut;
	int iDtcNum = 0;
	byte *pcDtcStart = NULL;//��������ʼ��ŵ�ַ
	byte cDtcStatusCache[256] = {0};
    UNN_2WORD_4BYTE DTCByte;
	int iDataLen;
	byte DTC_SPN[50] = {0};

	byte cDtcStartOffset	= g_p_stGeneralReadDtcConfig->cDtcStartOffset;	//��Ч�ظ��б���DTC��ʼƫ�ƣ���SID�ظ���ʼ
	byte cDtcBytesInCmd		= g_p_stGeneralReadDtcConfig->cDtcBytesInCmd;	//�����м����ֽڱ�ʾһ��������
	byte cStatusOffset		= g_p_stGeneralReadDtcConfig->cStatusOffset;	//������״̬��һ�������������е�ƫ��
	byte cDtcBytesInDisplay = g_p_stGeneralReadDtcConfig->cDtcBytesInDisplay;//һ����������Ҫ��ʾ�����ֽ�

	iDtcNum = ( iValidLen -  cDtcStartOffset ) / cDtcBytesInCmd;
	pcDtcStart = pcDctData + cDtcStartOffset;

	if((iValidLen == 8)&&(pcDtcStart[0] == 0)&&(pcDtcStart[1] == 0)&&(pcDtcStart[2] == 0))
	{
		return 0;
	}

	iDataLen = special_return_status( PROCESS_OK | NO_JUMP | NO_TIP, NULL, NULL, 0, pOut );
	
	for( i = 0; i < iDtcNum; i++ )
	{
        DTCByte.u32Bit = 0;
		DTCByte.u8Bit[2] = (pcDtcStart[i * cDtcBytesInCmd+2]>>5)&0x07;
		DTCByte.u8Bit[1] = pcDtcStart[i * cDtcBytesInCmd+1];
        DTCByte.u8Bit[0] = pcDtcStart[i * cDtcBytesInCmd+0];

		get_J1939Dtc_status( pcDtcStart[cStatusOffset + i * cDtcBytesInCmd], cDtcStatusCache, cDtcMask );

		iDataLen = add_data_and_controlsID( iDataLen, DTCByte.u8Bit, 3, J1939_SPN, cDtcStatusCache, pOut );
	}
	return iDtcNum;
}
