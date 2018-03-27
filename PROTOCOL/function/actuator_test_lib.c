/**********************************************************************
Copyright:		YiXiong AUTO S&T Co., Ltd.
Description:	����ִ�������Դ�����
History:
	<author>	<time>		<desc>
************************************************************************/
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*#define NDEBUG*/
#include <assert.h>

#include "actuator_test_lib.h"
#include "../public/public.h"
#include "../command/command.h"
#include "../formula/formula_comply.h"
#include "../SpecialFunction/special_function.h"
#include "../InitConfigFromXml/init_config_from_xml_lib.h"

STRUCT_SELECT_FUN stActuatorTestFunGroup[] =
{
	{GENERAL_ACTUATOR_TEST, process_general_actuator_test},
};

/*************************************************
Description:	��ȡ�����ȡ����������
Input:
	cType		��������
Output:	����
Return:	pf_general_function ����ָ��
Others:
*************************************************/
pf_general_function get_actuator_test_fun( byte cType )
{
	int i = 0;

	for( i = 0; i < sizeof( stActuatorTestFunGroup ) / sizeof( stActuatorTestFunGroup[0] ); i++ )
		if( cType == stActuatorTestFunGroup[i].cType )
			return stActuatorTestFunGroup[i].pFun;

	return 0;
}

/*************************************************
Description:	ִ�������Դ�����
Input:
	pIn		�����xml��ȡ��ִ����������������
Output:	pOut	������ݵ�ַ
Return:	void
Others:	��UI��ȡ�������������ַ�����
*************************************************/
void process_actuator_test( void* pIn, void* pOut )
{
	pf_general_function pFun = NULL;

	pFun = get_actuator_test_fun( g_p_stProcessFunConfig->cActuatorTestFunOffset );

	assert( pFun );

	pFun( pIn, pOut );
}

/*************************************************
Description:	ִ�������Դ�����
Input:
	pIn		�����xml��ȡ��ִ����������������
Output:	pOut	������ݵ�ַ
Return:	void
Others:	��UI��ȡ�������������ַ�����
*************************************************/
void process_general_actuator_test( void* pIn, void* pOut )
{
	STRUCT_CHAIN_DATA_INPUT* pstParam = ( STRUCT_CHAIN_DATA_INPUT* )pIn;
	byte cActuatorTestMode		= 0xff;
	bool bProcessStatus = true;
	uint32 u32ActuatorTestCmdData[10] = {0};//ִ�������Բ���
	static bool bExitFlag = false;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	get_string_type_data_to_uint32( u32ActuatorTestCmdData, pstParam->pcData, pstParam->iLen );

	pstParam = pstParam->pNextNode;//��ȡ����ƫ��

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	cActuatorTestMode = ( byte )u32ActuatorTestCmdData[0];

	switch( cActuatorTestMode )
	{
		case 0x00://ֱ��ִ����
			bExitFlag = true;
			bProcessStatus = actuator_just_send_cmd( u32ActuatorTestCmdData, pstParam, pOut );
			break;
		case 0x01://��������
			bProcessStatus = actuator_input_type( u32ActuatorTestCmdData, pstParam, pOut );
			if ( bProcessStatus )
			{
				bExitFlag = true;
			}
			break;

		case 0x02://ִ���������˳�����ȡPID
			if (!bExitFlag) /* û��ִ���κζ�����ֱ���˳� */
			{			
				return;
			}
			bExitFlag = false; 
			actuator_process_exit( u32ActuatorTestCmdData, pstParam, pOut );
			break;

		default:
			break;
	}
	if ( bProcessStatus )
	{	//��ʾ�ɹ�
		special_return_status( PROCESS_OK | HAVE_JUMP | NO_TIP, "ACTUATOR_TEST_SUCCESS", NULL, 0, pOut );
	}
}

/*************************************************
Description:	ֻ��������ִ�������Դ�����
Input:
pIn		�����xml��ȡ��ִ����������������
Output:	pOut	������ݵ�ַ
Return:	void
Others:	��UI��ȡ�������������ַ�����
*************************************************/
bool actuator_just_send_cmd( uint32* u32ActuatorTestCmdData, STRUCT_CHAIN_DATA_INPUT* pstParam, void* pOut )
{
	bool bProcessStatus = false;
	int i = 0;
	int iCmdSum = 0;
	byte cBufferOffset = 0;//����ƫ��
	int piCmdIndex[50] = {0};
	int iReceiveResult = TIME_OUT;

	switch( u32ActuatorTestCmdData[1] ) /* ������չ�㣬���а�ȫ���� */
	{
		case 1://���Ͱ�ȫ����������Ͱ�ȫ�㷨
			bProcessStatus = process_single_cmd_without_subsequent_processing( g_iExtendSessionCmdIndex[1], pOut );
			if (!bProcessStatus)
			{
				return false;
			}
			//break;
		case 2://ֻ���Ͱ�ȫ�㷨
			bProcessStatus = process_security_access_algorithm( 0, pOut );
			if (!bProcessStatus)
			{
				return false;
			}
			break;
		default://�����Ͱ�ȫ����Ͱ�ȫ�㷨
			break;
	}

	piCmdIndex[0] = get_string_type_data_to_uint32( piCmdIndex + 1, pstParam->pcData, pstParam->iLen );

	cBufferOffset = g_stInitXmlGobalVariable.m_p_stCmdList[ piCmdIndex[1] ].cBufferOffset;//�����Ͷ�������ʱ�������������ͳһ

	iReceiveResult = send_and_receive_cmd( piCmdIndex );

	if( iReceiveResult != SUCCESS )
	{
		general_return_status( iReceiveResult, g_stBufferGroup[cBufferOffset].cBuffer, 3, pOut );
		return false;
	}
	return true;
}

/*************************************************
Description:	������ֵ��ִ�������Դ�����
Input:
pIn		�����xml��ȡ��ִ����������������
Output:	pOut	������ݵ�ַ
Return:	void
Others:	��UI��ȡ�������������ַ�����
*************************************************/
bool actuator_input_type( uint32* u32ActuatorTestCmdData, STRUCT_CHAIN_DATA_INPUT* pstParam, void* pOut )
{
	int iReceiveResult = TIME_OUT;
	byte cBufferOffset = 0;//����ƫ��
	bool bProcessStatus = false;
	UNN_2WORD_4BYTE uuValueTemp;
	byte cValueCache[4] = {0};
	byte cEndian				= 0;
	byte cProcessBytes			= 0;
	byte cSaveDataOffset		= 0;
	int piCmdIndex[2] = { 1, 0 };
	double fInputValue = (double)0xffffffff, fMaxValue = (double)0xffffffff, fMinValue = (double)0xffffffff;
	int i;

	piCmdIndex[1] = atoi( pstParam->pcData );

	pstParam = pstParam->pNextNode;//��ȡ��������

	if( ( pstParam == NULL ) || ( pstParam->iLen == 0 ) )
	{
		general_return_status( INVALID_INPUT, NULL, 0, pOut ); //��ʾ��Ч����
		return false;
	}
	for( i = 0; i < pstParam->iLen; ++i )
	{
		if(( pstParam->pcData[i] >= '0' && pstParam->pcData[i] <= '9' )||(pstParam->pcData[0] == '-') || (pstParam->pcData[i] == '.'))
		{
		}
		else
		{
			general_return_status( INVALID_INPUT, NULL, 0, pOut );
			return false;
		}
	}

	fInputValue = atof( pstParam->pcData );

	pstParam = pstParam->pNextNode;//��ȡ��������
	if ( pstParam != NULL )
	{
		fMinValue = atof( pstParam->pcData );
		pstParam = pstParam->pNextNode;//��ȡ��������
		if ( pstParam != NULL )
		{
			fMaxValue = atof( pstParam->pcData );
		}
		//�ж���Сֵ���ֵ
		if (( fInputValue > fMaxValue ) || ( fInputValue < fMinValue ))
		{
			general_return_status( INVALID_INPUT, NULL, 0, pOut );
			return false;
		}
	}

	fInputValue *= ( int )u32ActuatorTestCmdData[3];
	fInputValue /= ( int )u32ActuatorTestCmdData[4];
	fInputValue += ( int )u32ActuatorTestCmdData[1];
	fInputValue -= ( int )u32ActuatorTestCmdData[2];

	cEndian			= u32ActuatorTestCmdData[5];
	cProcessBytes	= u32ActuatorTestCmdData[6];
	cSaveDataOffset = u32ActuatorTestCmdData[7];

	uuValueTemp.u32Bit = ( uint32 )fInputValue;

	memcpy( cValueCache, uuValueTemp.u8Bit, 4 );

	if( ( cEndian == 1 ) && ( cProcessBytes > 1 ) ) //����Ǵ�����ֽ�������1
	{
		cValueCache[0] = uuValueTemp.u8Bit[3];
		cValueCache[1] = uuValueTemp.u8Bit[2];
		cValueCache[2] = uuValueTemp.u8Bit[1];
		cValueCache[3] = uuValueTemp.u8Bit[0];

		memcpy( ( g_stInitXmlGobalVariable.m_p_stCmdList[ piCmdIndex[1] ].pcCmd + cSaveDataOffset )
			, &cValueCache[4 - cProcessBytes], cProcessBytes );
	}
	else
	{
		memcpy( ( g_stInitXmlGobalVariable.m_p_stCmdList[ piCmdIndex[1] ].pcCmd + cSaveDataOffset )
			, &cValueCache[0], cProcessBytes );
	}

	switch( u32ActuatorTestCmdData[8] ) /* ������չ�㣬���а�ȫ���� */
	{
		case 1://���Ͱ�ȫ����������Ͱ�ȫ�㷨
			bProcessStatus = process_single_cmd_without_subsequent_processing( g_iExtendSessionCmdIndex[1], pOut );
			if (!bProcessStatus)
			{
				return false;
			}
			//break;
		case 2://ֻ���Ͱ�ȫ�㷨
			bProcessStatus = process_security_access_algorithm( 0, pOut );
			if (!bProcessStatus)
			{
				return false;
			}
			break;
		default:
			break;
	}

	cBufferOffset = g_stInitXmlGobalVariable.m_p_stCmdList[ piCmdIndex[1] ].cBufferOffset;//

	iReceiveResult = send_and_receive_cmd( piCmdIndex );

	if( iReceiveResult != SUCCESS )
	{
		general_return_status( iReceiveResult, g_stBufferGroup[cBufferOffset].cBuffer, 3, pOut );

		return false;
	}
	return true;
}
/*************************************************
Description:	�˳�ִ�������Դ�����
Input:
pIn		�����xml��ȡ��ִ����������������
Output:	pOut	������ݵ�ַ
Return:	void
Others:	��UI��ȡ�������������ַ�����
*************************************************/
bool actuator_process_exit( uint32* u32ActuatorTestCmdData, STRUCT_CHAIN_DATA_INPUT* pstParam, void* pOut )
{
	int iReceiveResult = TIME_OUT;
	int i = 0;
	int iGetPIDCmdOffset = 0;
	int iSavePIDCmdOffset = 0;
	bool bProcessStatus = false;
	int piCmdIndex[50] = {0};

	if( u32ActuatorTestCmdData[1] == 0x00 ) //������� ��־��Ϊ0x00,���Ը�������ID�����˳�
	{
		pstParam = pstParam->pNextNode;

		assert( pstParam->pcData );
		assert( pstParam->iLen != 0 );

		iGetPIDCmdOffset = atoi( pstParam->pcData );//��ȡ ��ȡPID������ƫ��

		pstParam = pstParam->pNextNode;

		assert( pstParam->pcData );
		assert( pstParam->iLen != 0 );

		iSavePIDCmdOffset = atoi( pstParam->pcData );//��ȡ ����PID������ƫ�ƣ���Ҫ���͵�����

		memcpy(
			g_stInitXmlGobalVariable.m_p_stCmdList[ iSavePIDCmdOffset].pcCmd + u32ActuatorTestCmdData[3],
			g_stInitXmlGobalVariable.m_p_stCmdList[ iGetPIDCmdOffset ].pcCmd + u32ActuatorTestCmdData[2],
			u32ActuatorTestCmdData[4]
		);
		piCmdIndex[0] = 1;
		piCmdIndex[1] = iSavePIDCmdOffset;
		iReceiveResult = send_and_receive_cmd( piCmdIndex );

		if( u32ActuatorTestCmdData[2] != 0 ) /* ����Ĭ�ϲ� */
		{
			piCmdIndex[1] = g_iDefaultSessionCmdIndex[1];
			iReceiveResult = send_and_receive_cmd( piCmdIndex );
		}
	} 
	else if (u32ActuatorTestCmdData[1] == 0x01 )//����ִ���˳�
	{
		pstParam = pstParam->pNextNode;//��ȡ����ƫ��
		assert( pstParam->pcData );
		assert( pstParam->iLen != 0 );

		piCmdIndex[0] = get_string_type_data_to_uint32( piCmdIndex + 1, pstParam->pcData, pstParam->iLen );

		iReceiveResult = send_and_receive_cmd( piCmdIndex );
	}
	special_return_status( PROCESS_OK | NO_JUMP | NO_TIP, NULL, NULL, 0, pOut);
	return true;
}
