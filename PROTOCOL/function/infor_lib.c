/**********************************************************************
Copyright:		YiXiong AUTO S&T Co., Ltd.
Description:	����汾��Ϣ���ݴ�����
History:
	<author>	<time>		<desc>

***********************************************************************/
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "infor_lib.h"
#include "../formula/formula_comply.h"
// #include "../command/command.h"
#include <assert.h>
#include "../public/public.h"
#include "../InitConfigFromXml/init_config_from_xml_lib.h"
// #include "../public/protocol_config.h"
#include "../formula_parse/interface.h"
#include "ds_lib.h"
#include "..\SpecialFunction\special_function.h"


STRUCT_SELECT_FUN stReadInformationFunGroup[] =
{
	{GENERAL_READ_INFORMATION, process_read_general_ECU_information},
};

/*************************************************
Description:	��ȡ�����ȡ�汾��Ϣ����
Input:
	cType		��������
Output:	����
Return:	pf_general_function ����ָ��
Others:
*************************************************/
pf_general_function get_read_information_fun( byte cType )
{
	int i = 0;

	for( i = 0; i < sizeof( stReadInformationFunGroup ) / sizeof( stReadInformationFunGroup[0] ); i++ )
		if( cType == stReadInformationFunGroup[i].cType )
			return stReadInformationFunGroup[i].pFun;

	return 0;
}

/*************************************************
Description:	���汾��Ϣ
Input:	pIn		���������������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
void process_read_ECU_information( void* pIn, void* pOut )
{
	pf_general_function pFun = NULL;

	pFun = get_read_information_fun( g_p_stProcessFunConfig->cInforFunOffset );

	assert( pFun );

	pFun( pIn, pOut );
}
/*************************************************
Description:	��ͨ���汾��Ϣ
Input:	pIn		��������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
1. ������һ�������ֱ�ӱ���������
2. 
*************************************************/
void process_read_general_ECU_information( void* pIn, void* pOut )
{
	STRUCT_CHAIN_DATA_INPUT* pstParam = ( STRUCT_CHAIN_DATA_INPUT* )pIn;
	UNN_2WORD_4BYTE iDisplayID;
	int iReceiveResult = TIME_OUT;
	byte cItemSum = 0;//Ҫ��ʾ������
	int iGroupID = 0;
	int iDataLen= 0;
	int SendCmd[] = {1,0};
	byte cBufferOffset = 0;
	int i = 0, j = 0;
	byte cContentCache[128] = { 0 };
	byte ReceiveBuffer[512] = { 0 };
	byte cValidByteLen = 0;
	int iAllByteLen = 0;
	byte cStartOffset = 0;

	byte cNegtiveDisplay[] = {"ID_STR_NEG_DS"};

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	iGroupID = atoi( pstParam->pcData );
	cItemSum = g_p_stInformationGroupConfigGroup[iGroupID]->cItemSum;

	iDataLen = special_return_status( PROCESS_OK | NO_JUMP | NO_TIP, NULL, NULL, 0, pOut );
	for( i = 0; i < cItemSum; i++ )
	{
		memset( ReceiveBuffer, 0, 512);//���ݻ����ʼ��
		iDisplayID.u32Bit = g_p_stInformationGroupConfigGroup[iGroupID]->pstDSFormulaConfig[i].iDSID;
		iAllByteLen = 0;
		for( j = 0; j < g_p_stInformationGroupConfigGroup[iGroupID]->pstDSFormulaConfig[i].iDsCmdIDLen; j++ )
		{
			SendCmd[1] = g_p_stInformationGroupConfigGroup[iGroupID]->pstDSFormulaConfig[i].piDSCmdID[j];
			cBufferOffset = g_stInitXmlGobalVariable.m_p_stCmdList[SendCmd[1]].cBufferOffset;
			if (g_KwpFunSaveOffset.cInforOffset != 0)
			{
				g_MulCmdOffset = g_KwpFunSaveOffset.cInforOffset;
			}
			else
			{
				g_MulCmdOffset = g_p_stInformationGroupConfigGroup[iGroupID]->pstDSFormulaConfig[i].pcValidByteOffset[j];
			}
			iReceiveResult = send_and_receive_cmd( SendCmd );
			if ( iReceiveResult == SUCCESS )
			{
				cStartOffset		= g_p_stInformationGroupConfigGroup[iGroupID]->pstDSFormulaConfig[i].pcValidByteOffset[j];
				cValidByteLen		= g_p_stInformationGroupConfigGroup[iGroupID]->pstDSFormulaConfig[i].pcValidByteNumber[j];
				if (cValidByteLen == 0xff)//֧��ECU��Ϣ��֡����ʾ
				{
					cValidByteLen = g_stBufferGroup[cBufferOffset].iValidLen - cStartOffset;
				}
				memcpy( ReceiveBuffer + iAllByteLen, g_stBufferGroup[cBufferOffset].cBuffer + cStartOffset, cValidByteLen );
				iAllByteLen += cValidByteLen;
			}
			else if( iReceiveResult == NEGATIVE )
			{
				break;
			}
			else
			{
				general_return_status( iReceiveResult, g_stBufferGroup[cBufferOffset].cBuffer, 3, pOut );
				return;
			}
		}
		
		memset( cContentCache, 0, sizeof( cContentCache ) );
		if ( iReceiveResult == NEGATIVE )
		{
			strcpy( cContentCache, cNegtiveDisplay);
		}
		else
		{
			if( PROTOCOL_PARSER == g_p_stInformationGroupConfigGroup[iGroupID]->pstDSFormulaConfig[i].cFormulaType)
			{
				//����protocol�ļ��㹫ʽ
				process_normal_infor_calculate( iDisplayID.u32Bit, ReceiveBuffer, iAllByteLen, cContentCache );
			}
			else
			{
				//���ù�ʽ������
				calculate( ReceiveBuffer, iAllByteLen,
					g_p_stInformationGroupConfigGroup[iGroupID]->pstDSFormulaConfig[i].pcFormula,
					g_p_stInformationGroupConfigGroup[iGroupID]->pstDSFormulaConfig[i].iFormulaLen,
					g_p_stInformationGroupConfigGroup[iGroupID]->pstDSFormulaConfig[i].pStrFormat,
					cContentCache);			 
			}
		}
		iDataLen = add_data_and_controlsID( iDataLen, iDisplayID.u8Bit, 4, DEC_LITTLE_ENDIAN, cContentCache, pOut );
	}
}