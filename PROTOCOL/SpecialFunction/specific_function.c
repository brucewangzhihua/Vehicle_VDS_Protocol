/**********************************************************************
Copyright:		YiXiong AUTO S&T Co., Ltd.
Description:	���⹦�ܵĺ��������ļ��������еĴ�����д�� process_special_function �ϱߡ���������д����������
                �������⹦�ܵĲ�������һ���ļ��ڽ��С�
History:
	<author>    ��ѧ��
	<time>		2015��5��18��14:08:25
	<desc>      �ļ������ ����ģ�壬�������ο�
************************************************************************/
#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "special_function.h"
#include "..\InitConfigFromXml\init_config_from_xml_lib.h"
#include <assert.h>
#include <time.h>



/*************************************************
Description:	д�봦����
Input:
pIn		���������⹦���йص���������
�ʹ�UI���������
Output:	pOut	������ݵ�ַ
Return:	void
Others:	���ݵ�һ����������ִ�в�ͬ�Ĺ��ܺ���
*************************************************/
bool write_Function_S( byte * cSpecialCmdData, const STRUCT_CHAIN_DATA_INPUT* pstParam, void * pOut )
{
	int i = 0;
	int iCmdSum = 0;
	uint32 u8CmdIndex[30];//���⹦����������
	int iReceiveResult = TIME_OUT;
	byte cBufferOffset = 0;//����ƫ��
	int ipCmdIndex[2] = {1, 0};
	int WriteCmd = 0, ReadCmd = 0;
	byte InputData[20] = {0};
	uint32 user_permission = 0;  //�߼�Ȩ��

	iCmdSum  = get_string_type_data_to_uint32( u8CmdIndex, pstParam->pcData, pstParam->iLen );

	WriteCmd = u8CmdIndex[0];
	ReadCmd  = u8CmdIndex[1];

	//ָ����һ�����
	pstParam = pstParam->pNextNode;

	if( pstParam->iLen != cSpecialCmdData[2] )
	{
		general_return_status( INVALID_INPUT, NULL, 0, pOut );
		return false;
	}

	for( i = 0; i < pstParam->iLen; i++ )
	{
		if( !(( pstParam->pcData[i] >= '0' && pstParam->pcData[i] <= '9' ) || ( pstParam->pcData[i] >= 'A' && pstParam->pcData[i] <= 'Z' )))
		{
			general_return_status( INVALID_INPUT, NULL, 0, pOut );
			return false;
		}
		InputData[i] = pstParam->pcData[i];
	}

	//���������ж�
	switch (cSpecialCmdData[3])
	{
	case 1:  //�޶�������ʼ�����ַ�Ϊ��LNB��
		if ( pstParam->pcData[0] != 'L' || pstParam->pcData[1] != 'N' || pstParam->pcData[2] != 'B' )
		{
			general_return_status( INVALID_INPUT, NULL, 0, pOut );
			return false;
		}

		pstParam = pstParam->pNextNode;

		for ( i = 0; i < pstParam->iLen; i ++ )
		{
			user_permission = (pstParam->pcData[i]-0x30) + user_permission * 10;
		}

		if ( 1 != ( user_permission >> 31 ) )  //��Ϊ�߼�Ȩ��ʱ
		{
			cBufferOffset = g_stInitXmlGobalVariable.m_p_stCmdList[ReadCmd].cBufferOffset;
			ipCmdIndex[1] = ReadCmd;
			iReceiveResult = send_and_receive_cmd( ipCmdIndex );
			if( iReceiveResult != SUCCESS )
			{
				general_return_status( iReceiveResult, g_stBufferGroup[cBufferOffset].cBuffer, 3, pOut );
				return false;
			}
			if ( 0 == memcmp(g_stBufferGroup[cBufferOffset].cBuffer + 3,"LNB",3) )
			{
				special_return_status( PROCESS_OK | HAVE_JUMP | HAVE_TIP, "ID_LABLE_VIN_ALREAD_WRITE", NULL,0, pOut );
				return false;
			}
		}
		break;
	default:
		break;
	}

	iReceiveResult = process_single_cmd_without_subsequent_processing( g_iExtendSessionCmdIndex[1], pOut );
	if( !iReceiveResult )
	{
		return false;
	}
	iReceiveResult = process_security_access_algorithm( 0, pOut );
	if( !iReceiveResult )
	{
		return false;
	}

	memcpy( g_stInitXmlGobalVariable.m_p_stCmdList[WriteCmd].pcCmd + cSpecialCmdData[1], InputData, cSpecialCmdData[2] );

	cBufferOffset = g_stInitXmlGobalVariable.m_p_stCmdList[WriteCmd].cBufferOffset;
	ipCmdIndex[1] = WriteCmd;
	iReceiveResult = send_and_receive_cmd( ipCmdIndex );
	if( iReceiveResult != SUCCESS )
	{
		general_return_status( iReceiveResult, g_stBufferGroup[cBufferOffset].cBuffer, 3, pOut );
		return false;
	}

	//��ʾ�ɹ�
	special_return_status( PROCESS_OK | HAVE_JUMP | NO_TIP, "ACTUATOR_TEST_SUCCESS", NULL, 0, pOut );

	return true;
}

/*************************************************
Description:	д��Programming Date������
Input:
pIn		���������⹦���йص���������
�ʹ�UI���������
Output:	pOut	������ݵ�ַ
Return:	void
Others:	���ݵ�һ����������ִ�в�ͬ�Ĺ��ܺ���
*************************************************/
bool write_DATE_S( byte * cSpecialCmdData, const STRUCT_CHAIN_DATA_INPUT* pstParam, void * pOut )
{
	int i = 0;
	int iCmdSum = 0;
	uint32 u8CmdIndex[30];//���⹦����������
	int iReceiveResult = TIME_OUT;
	byte cBufferOffset = 0;//����ƫ��
	int ipCmdIndex[2] = {1, 0};
	byte InputDATE[6];

	time_t nowTime;
	struct tm *sysTime;
	int year;
	int mon;
	int day;                   

	iCmdSum  = get_string_type_data_to_uint32( u8CmdIndex, pstParam->pcData, pstParam->iLen );

	//���ϵͳ����
	time(&nowTime);                 
	sysTime = localtime(&nowTime);
	year = sysTime->tm_year+1900; //��-2016
	mon = sysTime->tm_mon+1; //��-12    
	day = sysTime->tm_mday;  //��-13

	InputDATE[0] =0X30+(year%100)/10;
	InputDATE[1] =0X30+(year%100)%10;
	InputDATE[2] =0X30+mon/10;
	InputDATE[3] =0X30+mon%10;
	InputDATE[4] =0X30+day/10;
	InputDATE[5] =0X30+day%10;

	//������չ�㣬�����Ͱ�ȫ�㷨
	iReceiveResult = process_single_cmd_without_subsequent_processing( g_iExtendSessionCmdIndex[1], pOut );
	if( !iReceiveResult )
	{
		return false;
	}
	iReceiveResult = process_security_access_algorithm( 0, pOut );
	if( !iReceiveResult )
	{
		return false;
	}

	//д��������
	ipCmdIndex[1] = u8CmdIndex[0];
	cBufferOffset = g_stInitXmlGobalVariable.m_p_stCmdList[ipCmdIndex[1]].cBufferOffset;
	memcpy( g_stInitXmlGobalVariable.m_p_stCmdList[ipCmdIndex[1]].pcCmd + cSpecialCmdData[1], InputDATE, 6 );
	iReceiveResult = send_and_receive_cmd( ipCmdIndex );
	if( iReceiveResult != SUCCESS )
	{
		general_return_status( iReceiveResult, g_stBufferGroup[cBufferOffset].cBuffer, 3, pOut );

		return false;
	}

	//��ʾ�ɹ�
	special_return_status( PROCESS_OK | HAVE_JUMP | NO_TIP, "ACTUATOR_TEST_SUCCESS", NULL, 0, pOut );
	return true;
}

/*************************************************
Description:	���⹦�ܴ�����
Input:
pIn		���������⹦���йص���������
�ʹ�UI���������
Output:	pOut	������ݵ�ַ
Return:	void
Others:	���ݵ�һ����������ִ�в�ͬ�Ĺ��ܺ���
*************************************************/
void process_special_function( void* pIn, void* pOut )
{

	byte cSpecialCmdData[40] = {0};//������⹦����������
	STRUCT_CHAIN_DATA_INPUT* pstParam = ( STRUCT_CHAIN_DATA_INPUT* )pIn;
	byte FunctionSlect = 0;
	bool bStatus = false;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );
	FunctionSlect = atoi( pstParam->pcData );

	get_string_type_data_to_byte( cSpecialCmdData, pstParam->pcData, pstParam->iLen );
	//ָ����һ�����
	pstParam = pstParam->pNextNode;

	/* ���⹦�ܵĺ������ */
	switch( FunctionSlect )
	{
		/* e.g.   process_Key_Match( cSpecialCmdData, pstParam, pOut );   */
	case 0:		//��ȡ����
		bStatus = read_function( cSpecialCmdData, pstParam, pOut );
		break;

	case 1:		//д�빦��
		bStatus = write_function( cSpecialCmdData, pstParam, pOut );
		break;

	case 2:		//д��VIN
		bStatus = write_Function_S( cSpecialCmdData, pstParam, pOut );  //cSpecialCmdData��1��Ҫ�޸������ƫ��λ�ã�2�������ַ����ȣ�3���޶����뷶Χ���������
		break;

	case 3:		//д�������ڣ�ʹ��ϵͳʱ��
		bStatus = write_DATE_S( cSpecialCmdData, pstParam, pOut );
		break;


	default:
		break;
	}

}

/*************************************************
Description:	��ȡ������ASCII����ʾ��
Input:
pIn		���������⹦���йص��������ݺʹ�UI���������
Output:	pOut	������ݵ�ַ
Return:	bool
Others:
*************************************************/
bool read_function( byte* cSpecialCmdData, const STRUCT_CHAIN_DATA_INPUT* pstParam, void* pOut )
{
	byte i = 0;
	byte iCmdSum = 0;
	uint32 u8CmdIndex[30];//���⹦����������
	bool iReceiveResult;
	byte cBufferOffset = 0;//����ƫ��
	byte ReadCmd;
	byte* pReadData = NULL;
	int iDataLen;

	iCmdSum  = get_string_type_data_to_uint32( u8CmdIndex, pstParam->pcData, pstParam->iLen );

	ReadCmd  = u8CmdIndex[iCmdSum - 1];

	for ( i = 0; i < iCmdSum; i++ )
	{
		iReceiveResult = process_single_cmd_without_subsequent_processing( u8CmdIndex[i], pOut);
		if( !iReceiveResult )
		{
			return false;
		}
	}
	cBufferOffset = g_stInitXmlGobalVariable.m_p_stCmdList[ReadCmd].cBufferOffset;
	pReadData = g_stBufferGroup[cBufferOffset].cBuffer + cSpecialCmdData[1];
	iDataLen = special_return_status( PROCESS_OK | NO_JUMP | NO_TIP, NULL, NULL, 0, pOut );
	add_data_and_controlsID( iDataLen,pReadData, cSpecialCmdData[2], ORIGINAL, "ID_TEXT_CODE", pOut );
	return true;
}
/*************************************************
Description:	д�뺯����ASCII��д�룩
Input:
pIn		���������⹦���йص��������ݺʹ�UI���������
Output:	pOut	������ݵ�ַ
Return:	bool
Others:	
*************************************************/
bool write_function( byte* cSpecialCmdData, const STRUCT_CHAIN_DATA_INPUT* pstParam, void* pOut )
{
	byte i = 0;
	byte iCmdSum = 0;
	uint32 u8CmdIndex[30];//���⹦����������
	int iReceiveResult = TIME_OUT;
	byte cBufferOffset = 0;//����ƫ��
	byte WriteCmd;

	iCmdSum  = get_string_type_data_to_uint32( u8CmdIndex, pstParam->pcData, pstParam->iLen );

	WriteCmd  = u8CmdIndex[iCmdSum - 1];

	//ָ����һ�����
	pstParam = pstParam->pNextNode;

	if( pstParam->iLen != cSpecialCmdData[2] )
	{
		general_return_status( INVALID_INPUT, NULL, 0, pOut );
		return false;
	}
	//�ж������ַ��Ƿ���0-9��A-Z֮�����
	for( i = 0; i < pstParam->iLen; ++i )
	{
		if( !(( pstParam->pcData[i] >= '0' && pstParam->pcData[i] <= '9' ) || ( pstParam->pcData[i] >= 'A' && pstParam->pcData[i] <= 'Z' )))
		{
			general_return_status( INVALID_INPUT, NULL, 0, pOut );
			return false;
		}
	}

	memcpy( g_stInitXmlGobalVariable.m_p_stCmdList[WriteCmd].pcCmd + cSpecialCmdData[1], pstParam->pcData, cSpecialCmdData[2] );

	switch( cSpecialCmdData[3] ) /* ������չ�㣬���а�ȫ���� */
	{
		case 1://���Ͱ�ȫ����������Ͱ�ȫ�㷨
			iReceiveResult = process_single_cmd_without_subsequent_processing( g_iExtendSessionCmdIndex[1], pOut );
			if (!iReceiveResult)
			{
				return false;
			}
			//break;
		case 2://ֻ���Ͱ�ȫ�㷨
			iReceiveResult = process_security_access_algorithm( 0, pOut );
			if (!iReceiveResult)
			{
				return false;
			}
			break;
		default:
			break;
	}

	for ( i = 0; i < iCmdSum; i++ )
	{
		iReceiveResult = process_single_cmd_without_subsequent_processing( u8CmdIndex[i], pOut);
		if( !iReceiveResult )
		{
			return false;
		}
	}

	//��ʾ�ɹ�
	special_return_status( PROCESS_OK | NO_JUMP | HAVE_TIP, NULL, "SUCCESS", 0, pOut );

	return true;
}
