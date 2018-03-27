/**********************************************************************
Copyright:		YiXiong AUTO S&T Co., Ltd.
Description:	�������������ݴ�����
History:
	<author>	<time>		<desc>
************************************************************************/
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ds_lib.h"
#include "../formula/formula_comply.h"
#include <assert.h>
#include "../command/command.h"
#include "../public/public.h"
#include "../public/protocol_config.h"
#include "../InitConfigFromXml/init_config_from_xml_lib.h"
#include "../formula_parse/interface.h"
#include "../protocol/iso_15765.h"
#include "../protocol/iso_14230.h"
#include "..\SpecialFunction\special_function.h"

//��ʱ�������������漰״̬�����飬�ֲ�����
STRUCT_DS_STORAGE_BUFFER * s_stDSBufferTempGroup = NULL;

STRUCT_SELECT_FUN stReadDataStreamFunGroup[] =
{
	{READ_GENERAL_DATA_STREAM, process_read_general_data_stream},
	{TOGETHER_READ_GENERAL_DATA_STREAM, process_together_read_general_data_stream},
};

/*************************************************
Description:	��ȡ�����ȡ����������
Input:
	cType		��������
Output:	����
Return:	pf_general_function ����ָ��
Others:
*************************************************/
pf_general_function get_read_data_stream_fun( byte cType )
{
	int i = 0;

	for( i = 0; i < sizeof( stReadDataStreamFunGroup ) / sizeof( stReadDataStreamFunGroup[0] ); i++ )
		if( cType == stReadDataStreamFunGroup[i].cType )
			return stReadDataStreamFunGroup[i].pFun;

	return 0;
}
/*************************************************
Description:	��������
Input:	pIn		�������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
void process_read_data_stream( void* pIn, void* pOut )
{
	pf_general_function pFun = NULL;

	pFun = get_read_data_stream_fun( g_p_stProcessFunConfig->cDSFunOffset );

	assert( pFun );

	pFun( pIn, pOut );

}


/*************************************************
Description: ��ȡ15765Э��ECU�ظ�������������
Input:	piCmdIndex ���͵������
Output:	int 	����״̬
Return:	����
Others:
*************************************************/
int process_DS_receive_cmd_by_iso_15765(const int* piCmdIndex )
{
	int i =0;
	byte cBufferOffset = 0;
    byte cReceiveBuffer[20] = {0};
	int iReceiveResult = TIME_OUT;
	for (i = 0; i < piCmdIndex[0]; i++)
	{
		cBufferOffset = g_stInitXmlGobalVariable.m_p_stCmdList[piCmdIndex[i + 1]].cBufferOffset;
		//�ѵ�ǰ�������Ļ������ݱ��浽��ʱ�Ļ�����
        iReceiveResult = process_CAN_receive_Cmd( cBufferOffset, cReceiveBuffer );
		if( ( iReceiveResult != SUCCESS ) && ( iReceiveResult != NEGATIVE ) ) //���״̬�Ȳ���SUCCESS�ֲ���NEGATIVE����Ϊ����
		{
			return  iReceiveResult;
		}
		if ( g_DSProrocess) // ����������
		{
			s_stDSBufferTempGroup[i].iValidLen = g_stBufferGroup[cBufferOffset].iValidLen;

			memcpy( s_stDSBufferTempGroup[i].cBuffer, g_stBufferGroup[cBufferOffset].cBuffer,
				g_stBufferGroup[cBufferOffset].iValidLen );

			if( iReceiveResult == NEGATIVE )
			{
				s_stDSBufferTempGroup[i].bNegativeFlag = true;
			}
			else
			{
				s_stDSBufferTempGroup[i].bNegativeFlag = false;
			}
		}
	}

	return SUCCESS;

}


/*************************************************
Description: ��ȡ14230Э��ECU�ظ�������������
Input:	piCmdIndex ���͵������
Output:	int 	����״̬
Return:	����
Others:
*************************************************/
int process_DS_receive_cmd_by_iso_14230(const int* piCmdIndex )
{
	int i =0;
	byte cBufferOffset = 0;
	byte cReceiveBuffer[512] = {0};
	int iReceiveResult = TIME_OUT;
	for (i = 0; i < piCmdIndex[0]; i++)
	{

		cBufferOffset = g_stInitXmlGobalVariable.m_p_stCmdList[piCmdIndex[i + 1]].cBufferOffset;
		//�ѵ�ǰ�������Ļ������ݱ��浽��ʱ�Ļ�����
		iReceiveResult = process_KWP_receive_Cmd( cBufferOffset, cReceiveBuffer );
		if( ( iReceiveResult != SUCCESS ) && ( iReceiveResult != NEGATIVE ) ) //���״̬�Ȳ���SUCCESS�ֲ���NEGATIVE����Ϊ����
		{
			return  iReceiveResult;
		}

		if (g_DSProrocess)
		{
			s_stDSBufferTempGroup[i].iValidLen = g_stBufferGroup[cBufferOffset].iValidLen;
			if (s_stDSBufferTempGroup[i].iValidLen != 0)
			{
				memcpy( s_stDSBufferTempGroup[i].cBuffer, g_stBufferGroup[cBufferOffset].cBuffer,
					g_stBufferGroup[cBufferOffset].iValidLen );
			}

			if( iReceiveResult == NEGATIVE )
			{
				s_stDSBufferTempGroup[i].bNegativeFlag = true;
			}
			else
			{
				s_stDSBufferTempGroup[i].bNegativeFlag = false;
			}

		}
	}

	return SUCCESS;

}

/*************************************************
Description: ��ȡECU�ظ�������������
Input:	piCmdIndex ���͵������
Output:	int 	����״̬
Return:	����
Others:   С�ģ� ��������ȡ���֡���շŵ���һ�� ��g_DSProrocess �ж��ǲ�������������
*************************************************/
int process_DS_receive_Cmd(const int *piCmdIndex)
{
	int iReturnStatus = TIME_OUT;

	switch( g_stInitXmlGobalVariable.m_cProtocolConfigType ) //�������Э������ģ�����ͽ�������
	{
	case ISO15765_CONFIG://CAN
		iReturnStatus = process_DS_receive_cmd_by_iso_15765( piCmdIndex );
		break;

	case ISO14230_CONFIG://K��
		iReturnStatus = process_DS_receive_cmd_by_iso_14230( piCmdIndex );
		break;

	default:
		iReturnStatus = PROTOCOL_ERROR;
		break;
	}

	return iReturnStatus;
}
/*************************************************
Description: ����15765Э������������
Input:	piCmdIndex ���͵������
Output:	int 	����״̬
Return:	����
Others:
*************************************************/

bool process_DS_send_cmd_by_iso_15765(const int*  piCmdIndex )
{
	STRUCT_CMD stCopySendCmd = {0};
	int i = 0;
	int iNumber = 0;
	bool bSendStatus = false;

	stCopySendCmd.iCmdLen = 5;
	for (i = 0; i < piCmdIndex[0]; i++)
	{
		stCopySendCmd.iCmdLen += g_stInitXmlGobalVariable.m_p_stCmdList[piCmdIndex[i + 1]].iCmdLen + 1;
	}
	stCopySendCmd.pcCmd = ( byte* )malloc( sizeof( byte ) * (stCopySendCmd.iCmdLen+1) );


	stCopySendCmd.pcCmd[0] = 5;  //VDI ���͵�֡���ʱ��
	stCopySendCmd.pcCmd[1] = 6;  //֡����  ���ճ�ʱ���ٷ�����һ֡
	stCopySendCmd.pcCmd[2] = 0x03;
	stCopySendCmd.pcCmd[3] = 0xe8;  //VDI �ȴ�ECU�ظ���ʱʱ��  1S 

	stCopySendCmd.pcCmd[4] = piCmdIndex[0];
	iNumber = 5;
	for( i = 0; i < piCmdIndex[0]; i++ )   //��װ����
	{
		stCopySendCmd.pcCmd[iNumber] = g_stInitXmlGobalVariable.m_p_stCmdList[piCmdIndex[i + 1]].iCmdLen; //�����ֽ���
		iNumber++;
		memcpy(stCopySendCmd.pcCmd+iNumber,g_stInitXmlGobalVariable.m_p_stCmdList[piCmdIndex[i + 1]].pcCmd,stCopySendCmd.pcCmd[iNumber-1]);
		iNumber += stCopySendCmd.pcCmd[iNumber-1];
	}

	bSendStatus = package_and_send_frame( FRAME_HEAD_MULTI_FRAME, &stCopySendCmd, g_p_stISO15765Config->cReserved );
	if( NULL != stCopySendCmd.pcCmd )
	{
		free( stCopySendCmd.pcCmd );
		stCopySendCmd.pcCmd = NULL;
	}
	return bSendStatus;
}

/*************************************************
Description: ����14230Э������������
Input:	piCmdIndex ���͵������
Output:	int 	����״̬
Return:	����
Others:
*************************************************/

bool process_DS_send_cmd_by_iso_14230(const int*  piCmdIndex )
{
	STRUCT_CMD stCopySendCmd = {0};
	int i = 0;
	int iNumber = 0;
	bool bSendStatus = false;

	stCopySendCmd.iCmdLen = 5;
	for (i = 0; i < piCmdIndex[0]; i++)
	{
		stCopySendCmd.iCmdLen += g_stInitXmlGobalVariable.m_p_stCmdList[piCmdIndex[i + 1]].iCmdLen + 1;
	}
	stCopySendCmd.pcCmd = ( byte* )malloc( sizeof( byte ) * (stCopySendCmd.iCmdLen+1) );


	stCopySendCmd.pcCmd[0] = g_DsMulConfig.cDsFrameTime;  //VDI ���͵�֡���ʱ��
	stCopySendCmd.pcCmd[1] = g_DsMulConfig.cDsMode;  //֡����  ���ճ�ʱ���ٷ�����һ֡
	stCopySendCmd.pcCmd[2] = 0;
	stCopySendCmd.pcCmd[3] = g_DsMulConfig.cDsTimeOut;  //VDI ECU�ظ���ɵ�ʱ��  ���ֽڼ��ʱ��󼴿�

	stCopySendCmd.pcCmd[4] = piCmdIndex[0];
	iNumber = 5;
	for( i = 0; i < piCmdIndex[0]; i++ )   //��װ����
	{
		stCopySendCmd.pcCmd[iNumber] = g_stInitXmlGobalVariable.m_p_stCmdList[piCmdIndex[i + 1]].iCmdLen; //�����ֽ���
		iNumber++;
		pre_process_cmd( &g_stInitXmlGobalVariable.m_p_stCmdList[piCmdIndex[i + 1]] ); //����У���
		memcpy(stCopySendCmd.pcCmd+iNumber,g_stInitXmlGobalVariable.m_p_stCmdList[piCmdIndex[i + 1]].pcCmd,stCopySendCmd.pcCmd[iNumber-1]);
		iNumber += stCopySendCmd.pcCmd[iNumber-1];
	}

	bSendStatus = package_and_send_frame( FRAME_HEAD_MULTI_FRAME, &stCopySendCmd, g_p_stISO14230Config->cTimeBetweenBytes );
	if( NULL != stCopySendCmd.pcCmd )
	{
		free( stCopySendCmd.pcCmd );
		stCopySendCmd.pcCmd = NULL;
	}
	return bSendStatus;
}

/*************************************************
Description: ��������������
Input:	piCmdIndex ���͵������
Output:	int 	����״̬
Return:	����
Others:
*************************************************/
bool process_DS_send_Cmd(const int *piCmdIndex)
{
	bool bSendStatus = false;
	switch( g_stInitXmlGobalVariable.m_cProtocolConfigType ) //�������Э������ģ�����ͽ�������
	{
	case ISO15765_CONFIG://CAN
		bSendStatus = process_DS_send_cmd_by_iso_15765( piCmdIndex );
		break;

	case ISO14230_CONFIG://K��
		bSendStatus = process_DS_send_cmd_by_iso_14230( piCmdIndex );
		break;

	default:
		bSendStatus = false;
		break;
	}

	return bSendStatus;

}


/*************************************************
Description: ѡ���ط�����
Input:	void
Output:	byte 	�ط�����
Return:	����
Others:
*************************************************/
byte select_ResendTime_config()
{
	byte	cRetransTime = 0;
	switch( g_stInitXmlGobalVariable.m_cProtocolConfigType ) //�������Э������ģ�����ͽ�������
	{
	case ISO15765_CONFIG://CAN
		cRetransTime			= g_p_stISO15765Config->cRetransTime;
		break;

	case ISO14230_CONFIG://K��
		cRetransTime			= g_p_stISO14230Config->cRetransTime;
		break;

	default:
		cRetransTime = 0xff;
		break;
	}
	return cRetransTime;
}

/*************************************************
Description:	protocol��һ��������ȫ������VDI��VDI�ѷŻصĽ��һ���ϴ�
Input:	pIn		�������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
void process_together_read_general_data_stream( void* pIn, void* pOut )
{
	UNN_2WORD_4BYTE iDsId;
	int iReceiveResult = TIME_OUT;
	int piCmdIndex[255] = {0};
	int i = 0, j = 0, k = 0;
	byte cItemSum = 0, CmdSum = 0;
	bool bSendStatus = false, bNegativeFlag;
	byte cRetransTime = 0;
	byte ReceiveBuffer[10] = { 0 };
	int iDataLen, iValidByteLenSum, iStartOffset, iValidByteLen;
	byte cContentCache[128] = { 0 };
	
	cRetransTime = select_ResendTime_config();
	if (cRetransTime == 0xff)
	{
		general_load_tipID( "Multi_frame_Resend_Time_False", pOut );
		return;
	}
    g_DSProrocess = true;

	//��ȡ��ǰ��������������
	cItemSum = get_current_screen_DS_config( pIn );

	for( i = 0; i < cItemSum; i++ )
	{
		memcpy( piCmdIndex + 1 + CmdSum, g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[i].piDSCmdID, g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[i].iDsCmdIDLen * sizeof(int) );
		CmdSum += g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[i].iDsCmdIDLen;
	}

	piCmdIndex[0] = remove_duplicate_int_element( piCmdIndex + 1, CmdSum );

	s_stDSBufferTempGroup = malloc( sizeof( STRUCT_DS_STORAGE_BUFFER ) * CmdSum ); 
	
	for( i = 0; i < CmdSum; i++ )
	{
		s_stDSBufferTempGroup[i].iCmdID = piCmdIndex[i + 1];
	}

	g_MulCmdOffset = g_KwpFunSaveOffset.cDsOffset; //K����������ȡ��֡����ƫ��

	while(1)
	{
		bSendStatus = process_DS_send_Cmd(piCmdIndex);
		if( !bSendStatus )
		{
			g_DSProrocess = false;
			iReceiveResult =  FAIL;
			general_return_status( iReceiveResult, NULL, 0, pOut );
			free(s_stDSBufferTempGroup);
			s_stDSBufferTempGroup = NULL;
			return;
		}

		iReceiveResult = process_DS_receive_Cmd(piCmdIndex);
		if( ( iReceiveResult != SUCCESS ) && ( iReceiveResult != NEGATIVE ) ) //���״̬�Ȳ���SUCCESS�ֲ���NEGATIVE����Ϊ����
		{
			if (( --cRetransTime ) == 0)
			{
				g_DSProrocess = false;
				general_return_status( iReceiveResult, NULL, 0, pOut );
				free(s_stDSBufferTempGroup);
				s_stDSBufferTempGroup = NULL;
				return;
			} 
			else
			{
				continue;
			}			
		}
		else
		{
			break;
		}
	}
	iDataLen = special_return_status( PROCESS_OK | NO_JUMP | NO_TIP, NULL, NULL, 0, pOut );

	for( i = 0; i < cItemSum; i++ )
	{
		iDsId.u32Bit = g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[i].iDSID;

		memset( cContentCache, 0, sizeof( cContentCache ) );//��������ʼ��
		
		iValidByteLenSum = 0;
		bNegativeFlag = true;

		for( k = 0; k < g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[i].iDsCmdIDLen; k++ )
		{
			for ( j = 0; j < CmdSum; j++ )
			{
				if (g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[i].piDSCmdID[k] == s_stDSBufferTempGroup[j].iCmdID)
				{
					if ( s_stDSBufferTempGroup[j].bNegativeFlag )
					{
						//��Ϊ������Ӧ�����˫��ѭ�������ٲ��Ҵ�����������������
						k = g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[i].iDsCmdIDLen;
						bNegativeFlag = true;
					}
					else
					{
						iStartOffset = g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[i].pcValidByteOffset[k];
						iValidByteLen = g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[i].pcValidByteNumber[k];
						memcpy( ReceiveBuffer + iValidByteLenSum, s_stDSBufferTempGroup[j].cBuffer + iStartOffset, iValidByteLen );
						iValidByteLenSum += iValidByteLen;
						bNegativeFlag = false;
					}
					break;
				}
				
			}
		}
		if ( bNegativeFlag )
		{
			strcpy(cContentCache,"ID_STR_NEG_DS");
		}
		else
		{
			if( PROTOCOL_PARSER == g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[i].cFormulaType )
			{
				//����protocol�ļ��㹫ʽ
				process_normal_ds_calculate( iDsId.u32Bit, ReceiveBuffer, iValidByteLenSum, cContentCache );
			}
			else
			{
				//���ù�ʽ������
				calculate( ReceiveBuffer, iValidByteLenSum,
					g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[i].pcFormula,
					g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[i].iFormulaLen,
					g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[i].pStrFormat,
					cContentCache
					);
			}
		}
		iDataLen = add_data_and_controlsID( iDataLen, iDsId.u8Bit, 4, DEC_LITTLE_ENDIAN, cContentCache, pOut );
	}

	g_DSProrocess = false;
	free(s_stDSBufferTempGroup);
	s_stDSBufferTempGroup = NULL;
}
/*************************************************
Description:	���Ͷ�ȡ����������
Input:	ReceiveBuffer ����ظ�����Ļ���ƫ��
        iValidByteNum �������Ч�ֽڸ���
		DsId ������ID
		CmdSum �������� ��ȥ�أ�
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
int send_receive_ds_cmd( byte * ReceiveBuffer, int * iValidByteNum, int DsId, int CmdSum )
{
	int k = 0, j = 0; 
	int KeyByteNum = 0; 
	byte cBufferOffset = 0;
	int iValidByteOffset, iValidByteNumber;
	int piCmdIndex[2] = { 1, 0 };
	int iReceiveResult = TIME_OUT;

	*iValidByteNum = 0;//����Ч�ֽڸ�����ʼ��Ϊ0

	for ( k = 0; k < g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[DsId].iDsCmdIDLen; k++ )
	{
		iValidByteOffset = g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[DsId].pcValidByteOffset[k];
		iValidByteNumber = g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[DsId].pcValidByteNumber[k];
		*iValidByteNum += iValidByteNumber;

		for( j = 0; j < CmdSum; j++ )
		{
			if ( s_stDSBufferTempGroup[j].iCmdID != -1 )
			{
				if( g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[DsId].piDSCmdID[k] == s_stDSBufferTempGroup[j].iCmdID )
				{
					if( s_stDSBufferTempGroup[j].bNegativeFlag == true )
						iReceiveResult = NEGATIVE;
					else
					{
						memcpy( ReceiveBuffer + KeyByteNum, s_stDSBufferTempGroup[j].cBuffer + iValidByteOffset, iValidByteNumber );
						KeyByteNum += iValidByteNumber;
						iReceiveResult = SUCCESS;
					}
					break;
				}
			}
			else
			{
				piCmdIndex[1] = g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[DsId].piDSCmdID[k];
				s_stDSBufferTempGroup[j].iCmdID = piCmdIndex[1];
				cBufferOffset = g_stInitXmlGobalVariable.m_p_stCmdList[piCmdIndex[1]].cBufferOffset;
				iReceiveResult = send_and_receive_cmd( piCmdIndex );
				if ( iReceiveResult == SUCCESS)
				{
					s_stDSBufferTempGroup[j].bNegativeFlag = false;
					memcpy( ReceiveBuffer + KeyByteNum, g_stBufferGroup[cBufferOffset].cBuffer + iValidByteOffset, iValidByteNumber );
					memcpy( s_stDSBufferTempGroup[j].cBuffer, g_stBufferGroup[cBufferOffset].cBuffer, g_stBufferGroup[cBufferOffset].iValidLen );
					KeyByteNum += iValidByteNumber;
					break;
				}
				else if ( iReceiveResult == NEGATIVE )
				{
					s_stDSBufferTempGroup[j].bNegativeFlag = true;
					return iReceiveResult;
				}
				else
					return iReceiveResult;
			}
		}
	}
	return iReceiveResult;
}
/*************************************************
Description:	�����ȡ��ͨ��������
Input:	pIn		�������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
void process_read_general_data_stream( void* pIn, void* pOut )
{
	UNN_2WORD_4BYTE iDsId;
	int iReceiveResult = TIME_OUT;
	int i = 0;
	byte cItemSum = 0;
	byte CmdSum = 0;
	byte ReceiveBuffer[512] = { 0 };
	int iValidByteLen = 0;
	byte cContentCache[128] = {0};
	int iDataLen = 0;
    byte g_bStopCallProtocol = false;
	byte cNegtiveDisplay[] = {"ID_STR_NEG_DS"};

	//��ȡ��ǰ��������������
	cItemSum = get_current_screen_DS_config( pIn );

	for( i = 0; i < cItemSum; i++ )
	{
		CmdSum += g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[i].iDsCmdIDLen;
	}

	s_stDSBufferTempGroup = malloc( sizeof( STRUCT_DS_STORAGE_BUFFER ) * CmdSum ); 
	for ( i = 0; i < CmdSum; i++ )
	{
		s_stDSBufferTempGroup[i].iCmdID = -1; //��ʼ��Ϊ-1
	}

	g_MulCmdOffset = g_KwpFunSaveOffset.cDsOffset; //K����������ȡ��֡����ƫ��

	iDataLen = special_return_status( PROCESS_OK | NO_JUMP | NO_TIP, NULL, NULL, 0, pOut );
	for( i = 0; i < cItemSum; i++ )
	{
		if (g_bStopCallProtocol) 
		{
			special_return_status( PROCESS_OK|NO_JUMP|NO_TIP, NULL, NULL, 0, pOut );
			return ;
		} 
		iDsId.u32Bit = g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[i].iDSID;

		iReceiveResult = send_receive_ds_cmd( ReceiveBuffer, &iValidByteLen, i, CmdSum );//���Ͷ�ȡ����������

		memset( cContentCache, 0, sizeof( cContentCache ) );

		if ( iReceiveResult == SUCCESS )//����ͳɹ������չ�ʽ������
		{
			if( PROTOCOL_PARSER == g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[i].cFormulaType )
			{
				//����protocol�ļ��㹫ʽ
				process_normal_ds_calculate( iDsId.u32Bit, ReceiveBuffer, iValidByteLen, cContentCache );
			}
			else
			{
				//���ù�ʽ������
				calculate( ReceiveBuffer, iValidByteLen,
					g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[i].pcFormula,
					g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[i].iFormulaLen,
					g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[i].pStrFormat,
					cContentCache
					);
			}
		}
		else if ( iReceiveResult == NEGATIVE )//�ظ�������Ӧ������ʾ������֧��
		{
			strcpy(cContentCache,"ID_STR_NEG_DS");
		}
		else//�����ʧ������ʾ
		{
			general_return_status( iReceiveResult, NULL, 0, pOut );
			free(s_stDSBufferTempGroup);
			s_stDSBufferTempGroup = NULL;
			return;
		}
		iDataLen = add_data_and_controlsID( iDataLen, iDsId.u8Bit, 4, DEC_LITTLE_ENDIAN, cContentCache, pOut );
	}
	free(s_stDSBufferTempGroup);
	s_stDSBufferTempGroup = NULL;
}

/*************************************************
Description:	��ȡ��������������
Input:
	pstDSConfig		ָ�����������õ�ָ��
	pIn				��������

Output:	��
Return:	int ��ǰ������ռ�ռ��С
Others: ע��:�ڵ��ñ�����ʱ��ʹ��calloc��������
		STRUCT_DS_FORMULA_CONFIG���ͱ������ڴ棬
		��ʹ����������ʱ������ʼ����
*************************************************/
int get_ds_config( STRUCT_DS_FORMULA_CONFIG * pstDSConfig, const byte * pcSource )
{

	byte cTemp[15] = {0};
	const byte * pcTemp = NULL;
	const byte * pcHead = NULL;
	byte * pcFormulaType = NULL;
	int iCmdSum = 0;
	int iLen = 0, iDataNum;
	uint32 u32Temp = 0;
	uint32 pu32Temp[5] = { 0 }; 

	if( NULL != pstDSConfig->pcValidByteNumber )
	{
		free( pstDSConfig->pcValidByteNumber );
		pstDSConfig->pcValidByteNumber = NULL;
	}

	if( NULL != pstDSConfig->pcValidByteOffset )
	{
		free( pstDSConfig->pcValidByteOffset );
		pstDSConfig->pcValidByteOffset = NULL;
	}

	if( NULL != pstDSConfig->piDSCmdID )
	{
		free( pstDSConfig->piDSCmdID );
		pstDSConfig->piDSCmdID = NULL;
	}

	if( NULL != pstDSConfig->pcFormula )
	{
		free( pstDSConfig->pcFormula );
		pstDSConfig->pcFormula = NULL;
	}

	if( NULL != pstDSConfig->pStrFormat )
	{
		free( pstDSConfig->pStrFormat );
		pstDSConfig->pStrFormat = NULL;
	}

	pcTemp = pcSource;
	pcHead = pcTemp;

	memcpy( cTemp, pcTemp, 4 );
	cTemp[4] = '\0';

	iLen = 4;

	iLen += strtol( cTemp, NULL, 16 );

	get_config_data( &u32Temp, pcTemp, iLen );

	pstDSConfig->iDSID = ( int )u32Temp; //���DS��ID

	pcTemp += iLen;

	memcpy( cTemp, pcTemp, 4 );
	cTemp[4] = '\0';

	iLen = 4;

	iLen += strtol( cTemp, NULL, 16 );


	iCmdSum = get_config_data( pu32Temp, pcTemp, iLen );

	pstDSConfig->iDsCmdIDLen = iCmdSum;

	pstDSConfig->piDSCmdID = ( int * )malloc( sizeof( int ) * ( iCmdSum ) );

	memcpy( pstDSConfig->piDSCmdID, pu32Temp, iCmdSum * sizeof(int) ); //�������

	pcTemp += iLen;

	iLen = get_command_one_block_config_data( cTemp, &iDataNum, pcTemp );

	pstDSConfig->iValidByteOffsetLen = (byte)iDataNum;

	pstDSConfig->pcValidByteOffset = ( byte * )malloc( iDataNum ); //�����ڴ�ռ�

	memcpy( pstDSConfig->pcValidByteOffset, cTemp, iDataNum ); //�����Ч�ֽ���ʼƫ��

	pcTemp += iLen;

	iLen = get_command_one_block_config_data( cTemp, &iDataNum, pcTemp );

	pstDSConfig->iValidByteNumberLen = (byte)iDataNum;

	pstDSConfig->pcValidByteNumber = ( byte * )malloc( iDataNum ); //�����ڴ�ռ�

	memcpy( pstDSConfig->pcValidByteNumber, cTemp, iDataNum ); //�����Ч�ֽڸ���

	pcTemp += iLen;

	memcpy( cTemp, pcTemp, 4 );
	cTemp[4] = '\0';

	iLen = 4;

	iLen += strtol( cTemp, NULL, 16 );

	pcFormulaType = ( byte * )malloc( sizeof( byte ) * ( iLen - 4 + 1 ) );

	memcpy( pcFormulaType, pcTemp + 4, iLen - 4 ); //��ù�ʽ����

	pcFormulaType[iLen - 4] = '\0';

	pstDSConfig->cFormulaType = FORMULA_PARSER;//Ĭ��Ϊ����������

	if( 0 == strcmp( pcFormulaType, "FORMULA_PARSER" ) )
		pstDSConfig->cFormulaType = FORMULA_PARSER;

	if( 0 == strcmp( pcFormulaType, "PROTOCOL_PARSER" ) )
		pstDSConfig->cFormulaType = PROTOCOL_PARSER;

	free( pcFormulaType );
	pcTemp += iLen;

	memcpy( cTemp, pcTemp, 4 );
	cTemp[4] = '\0';

	iLen = 4;

	iLen += strtol( cTemp, NULL, 16 );

	pstDSConfig->pcFormula = ( byte * )malloc( sizeof( byte ) * ( iLen - 4 + 1 ) );

	memcpy( pstDSConfig->pcFormula, pcTemp + 4, iLen - 4 ); //��ù�ʽ����
	pstDSConfig->iFormulaLen = iLen - 4;//��ù�ʽ���ݵĳ���

	pstDSConfig->pcFormula[iLen - 4] = '\0';

	pcTemp += iLen;

	memcpy( cTemp, pcTemp, 4 );
	cTemp[4] = '\0';

	iLen = 4;

	iLen += strtol( cTemp, NULL, 16 );

	pstDSConfig->pStrFormat = ( byte * )malloc( sizeof( byte ) * ( iLen - 4 + 1 ) );

	memcpy( pstDSConfig->pStrFormat, pcTemp + 4, iLen - 4 ); //�����ʾ����

	pstDSConfig->pStrFormat[iLen - 4] = '\0';

	pcTemp += iLen;

	return ( int )( pcTemp - pcHead );

}

/*************************************************
Description:	��ȡ��ǰ����������������
Input:
	pIn				��������

Output:	��
Return:	byte		��ǰ������������
Others: ��������Ϊ����������+�������������á�
ע������g_stGeneralDSFormulaGroupConfig.cItemSum
����������ֵ���������ÿ�ε�������
*************************************************/
byte get_current_screen_DS_config( void* pIn )
{
	STRUCT_CHAIN_DATA_INPUT* pstParam = ( STRUCT_CHAIN_DATA_INPUT* )pIn;
	byte cTemp	= 0;
	byte * pcTemp = NULL;
	byte j = 0;
	byte cItemSum = 0;
	int iLen = 0;
	int temp = 0;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	pcTemp = pstParam->pcData;

	iLen = get_command_one_block_config_data( &cTemp, &temp, pcTemp );
	cItemSum = cTemp;//��õ�ǰ������������
	pcTemp += iLen;

	if( g_stGeneralDSFormulaGroupConfig.cItemSum < cItemSum ) //�����ǰ����������С����Ҫ�������·���
	{
		free_general_DS_formula_config_space();//�ͷ��ѷ����

		g_stGeneralDSFormulaGroupConfig.cItemSum = cItemSum;

		g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig =
		    ( STRUCT_DS_FORMULA_CONFIG * )calloc( cItemSum, sizeof( STRUCT_DS_FORMULA_CONFIG ) );
	}

	for( j = 0; j < cItemSum; j++ )
	{
		iLen = get_ds_config( ( STRUCT_DS_FORMULA_CONFIG * ) & ( g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[j] ), pcTemp );
		pcTemp += iLen;
	}

	return cItemSum;
}

/*************************************************
Description:	�ͷŴ����ͨ��������ʽ���õĿռ�
Input:	��
Output:	����
Return:	��
Others: ÿ���һ�����þ��ڸú����������Ӧ���ͷŴ���
		����quit_system_lib.c��free_xml_config_space
		�����е��øú�����
*************************************************/
void free_general_DS_formula_config_space( void )
{
	byte j = 0;

	if( NULL != g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig )
	{
		for( j = 0; j < g_stGeneralDSFormulaGroupConfig.cItemSum; j++ )
		{
			if( NULL != g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[j].pcValidByteNumber )
			{
				free( g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[j].pcValidByteNumber );
				g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[j].pcValidByteNumber = NULL;
			}

			if( NULL != g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[j].pcValidByteOffset )
			{
				free( g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[j].pcValidByteOffset );
				g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[j].pcValidByteOffset = NULL;
			}

			if( NULL != g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[j].piDSCmdID )
			{
				free( g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[j].piDSCmdID );
				g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[j].piDSCmdID = NULL;
			}

			if( NULL != g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[j].pcFormula )
			{
				free( g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[j].pcFormula );
				g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[j].pcFormula = NULL;
			}

			if( NULL != g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[j].pStrFormat )
			{
				free( g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[j].pStrFormat );
				g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig[j].pStrFormat = NULL;
			}
		}

		free( g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig );
		g_stGeneralDSFormulaGroupConfig.pstDSFormulaConfig = NULL;
	}

}

/*************************************************
Description:	���Ͷ�֡���ݷ��ͽ��պ���
Input:  piCmdIndex:  �����1 + �����2 + ......
        CmdSum:  �����ܸ���
Output:	
Return:	bool
Others:
*************************************************/
bool process_Multi_frame_cmd_To_VDI_processing(int CmdSum,byte* piCmd,void* pOut)
{
	int iReceiveResult = TIME_OUT;
	bool bSendStatus = false;
	int piCmdIndex[300] = {0};
	int i = 0;
	byte	cRetransTime			= 0;

	cRetransTime = select_ResendTime_config();
	if (cRetransTime == 0xff)
	{
		//special_return_status(PROCESS_FAIL|NO_JUMP|HAVE_TIP,NULL,"Multi_frame_Resend_Time_False",NULL,0,ORIGINAL,NULL,pOut);
		general_load_tipID( "Multi_frame_Resend_Time_False", pOut );
		return false;
	}

	piCmdIndex[0] = CmdSum;
	for (i = 0 ; i < CmdSum; i++)
	{
		piCmdIndex[i+1] = piCmd[i];
	}
	while(1)
	{
		bSendStatus = process_DS_send_Cmd(piCmdIndex);
		if( !bSendStatus )
		{
			iReceiveResult =  FAIL;			
			general_return_status( iReceiveResult, NULL, 0, pOut );
			return false;
		}

		iReceiveResult = process_DS_receive_Cmd(piCmdIndex);
		if( ( iReceiveResult != SUCCESS ) && ( iReceiveResult != NEGATIVE ) ) //���״̬�Ȳ���SUCCESS�ֲ���NEGATIVE����Ϊ����
		{
			if (( --cRetransTime ) == 0)
			{			
				general_return_status( iReceiveResult, NULL, 0, pOut );
				return false;
			} 
			else
			{
				continue;
			}			
		}
		else
		{
			break;
		}
	}
	return true;
}


