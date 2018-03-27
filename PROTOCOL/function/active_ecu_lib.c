/**********************************************************************
Copyright:		YiXiong AUTO S&T Co., Ltd.
Description:	���弤�����
History:
	<author>	<time>		<desc>
************************************************************************/
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#include "active_ecu_lib.h"
#include "..\public\public.h"
#include "..\command\command.h"
#include "..\public\protocol_config.h"
#include "..\interface\protocol_interface.h"
#include "..\InitConfigFromXml\init_config_from_xml_lib.h"
#include <assert.h>
#include "..\SpecialFunction\special_function.h"
#include "idle_link_lib.h"
#include "quit_system_lib.h"
#include <string.h>


int g_iActiveECUStatus;

STRUCT_ACTIVE_FUN stActiveECUFunGroup[] =
{
// 	{ACTIVE_ECU_BY_GENERAL_CAN, NULL},
	{ACTIVE_ECU_BY_KWP, process_active_ECU_by_low_high_voltage},
	{ACTIVE_ECU_BY_5_BAUD_ADDRESS, process_active_ECU_by_5_baud_address},
};

/*************************************************
Description:	��ȡ�������
Input:
	cType		��������
Output:	����
Return:	pf_general_function ����ָ��
Others:
*************************************************/
pf_active_function get_active_ECU_fun( byte cType )
{
	int i = 0;

	for( i = 0; i < sizeof( stActiveECUFunGroup ) / sizeof( stActiveECUFunGroup[0] ); i++ )
		if( cType == stActiveECUFunGroup[i].cType )
			return stActiveECUFunGroup[i].pFun;

	return 0;
}
/*************************************************
Description:	�������
Input:	pIn		���������������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
void process_active_ECU( void* pIn, void* pOut )
{
	STRUCT_CHAIN_DATA_INPUT* pstParam = ( STRUCT_CHAIN_DATA_INPUT* )pIn;

	int * piActivateCmdIndex = NULL;
	int iActiveCmdSum = 0;
	int i = 0;
	byte cConfigType = 0;
	int iConfigOffset = 0;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	iConfigOffset = atoi( pstParam->pcData ); //��ô���������ID

	pstParam = pstParam->pNextNode;
	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	cConfigType = ( byte )atoi( pstParam->pcData ); //��ô���������ģ���

	select_process_fun_config( iConfigOffset, cConfigType );

	pstParam = pstParam->pNextNode;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	iConfigOffset = atoi( pstParam->pcData ); //��ð�ȫ��������ID

	pstParam = pstParam->pNextNode;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	cConfigType = ( byte )atoi( pstParam->pcData ); //��ð�ȫ��������ģ���

	init_security_access_config( iConfigOffset, cConfigType );

	pstParam = pstParam->pNextNode;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	iConfigOffset = atoi( pstParam->pcData );//��ÿ�����������ID

	pstParam = pstParam->pNextNode;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	cConfigType = ( byte )atoi( pstParam->pcData ); //��ÿ�����������ģ���

	select_idle_link_config( iConfigOffset, cConfigType );

	pstParam = pstParam->pNextNode;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );
	//��ȡ������������
	g_stInitXmlGobalVariable.m_iIdleLinkCmdGetFromXmlSum =
	    get_command_config_data( pstParam, &( g_stInitXmlGobalVariable.m_p_stIdleLinkCmdList ) );

	pstParam = pstParam->pNextNode;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	iConfigOffset = atoi( pstParam->pcData ); //��ü�������ID

	pstParam = pstParam->pNextNode;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	cConfigType = ( byte )atoi( pstParam->pcData ); //��ü�������ģ���

	select_active_config( iConfigOffset, cConfigType );

	pstParam = pstParam->pNextNode;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	//��ȡ����������������������
	iActiveCmdSum = get_command_config_data( pstParam, &( g_stInitXmlGobalVariable.m_p_stCmdList ) );

	if( 0 != iActiveCmdSum )
	{
		piActivateCmdIndex = ( int * )malloc( sizeof( int ) * ( iActiveCmdSum + 1 ) );
		//��ż���������������
		piActivateCmdIndex[0] = iActiveCmdSum;

		for( i = 1; i < iActiveCmdSum + 1; i++ )
			piActivateCmdIndex[i] = i - 1;

		if( !send_active_ECU_command( piActivateCmdIndex, pstParam, pOut ) )
		{
			g_iActiveECUStatus = ACTIVE_ECU_FAIL;
			free_xml_config_space(); //ϵͳ����ʧ���ͷ��ڴ档
			if( piActivateCmdIndex != NULL )
			{
				free( piActivateCmdIndex );
			}
			special_return_status( PROCESS_FAIL | HAVE_JUMP | NO_TIP, "ACTIVE_FAIL", NULL, 0, pOut );
			( ( byte * )pOut )[0] = 2;  //����ʧ�ܣ�ͳһ��ʾ��ʱ����0x7F����ʾfail�������ڴ�ͳһ�޸�Ϊtimeout��
			return;
		}
	}
	g_iActiveECUStatus = ACTIVE_ECU_SUCCESS;

	//��һ�ΰ���xml�����ÿ���״̬
	if( !set_idle_link( g_p_stIdleLinkConfig->cIdleLinkStatus ) )
	{
		//��ʾ���ÿ�������ʧ��
		special_return_status( PROCESS_OK | NO_JUMP | HAVE_TIP, NULL, "ID_STR_SET_IDLE_LINK_FAIL", 0, pOut );
	}
	else
	{
		special_return_status( PROCESS_OK | NO_JUMP | NO_TIP, NULL, NULL, 0, pOut );
	}

	if( piActivateCmdIndex != NULL )
	{
		free( piActivateCmdIndex );
	}
}

/*************************************************
Description:	�����������
Input:	pIn		���������������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
bool send_active_ECU_command( int * piActiveCmdIndex, void* pIn, void* pOut )
{
	pf_active_function pFun = NULL;
	bool bResultStatus = false;
	byte *pcOutTemp = ( byte * )pOut;
	int iReceiveResult = TIME_OUT;
	int iReceiveValidLen = 0;//���յ�����Ч�ֽڳ���
	byte piSecuriteIndex[5][2] = { 0 };//���֧�ּ����5�����
	int piSendCmd[2] = { 1, 0 };
	STRUCT_CHAIN_DATA_INPUT* pstParam = ( STRUCT_CHAIN_DATA_INPUT* )pIn;
	int iSum = 0, i = 0, j = 0;
	byte cRetransTime = 0;
	byte cRetransTimeTemp = 1;
	bool bSecuritFlag = false;
	int iRequestSeedCmdIndexBak = g_iRequestSeedCmdIndex[1];
	int iSendKeyCmdIndexBak = g_iSendKeyCmdIndex[1];

	pFun = get_active_ECU_fun( g_p_stProcessFunConfig->cActiveECUFunOffset );

	pstParam = pstParam->pNextNode;
	if( pstParam )	//����¸��ڵ㲻Ϊ�գ����ж��Ƿ�Ϊ��ȫ�㷨��������
	{
		if( !memcmp( pstParam->pcData, "true", 4 ) )//����Ϊtrue��Ϊ�а�ȫ�㷨
		{
			bSecuritFlag = true;
			pstParam = pstParam->pNextNode;
			while (pstParam)
			{	//��ȡ��ȫ�㷨�����λ�úͰ�ȫ�㷨�ȼ�����������������һ������Ϊ��ȡ���������ƫ�ƣ��ڶ�������Ϊ��ȫ�㷨�ȼ�
				iSum = get_string_type_data_to_byte( piSecuriteIndex[i++], pstParam->pcData, pstParam->iLen );
				if ( iSum != 2 )
				{
					return false;
				}
				pstParam = pstParam->pNextNode;
			}
		}
	}

	if ( g_p_stProcessFunConfig->cActiveECUFunOffset != 0)
	{
		cRetransTime		= g_p_stISO14230Config->cRetransTime;
		cRetransTimeTemp = cRetransTime;
		//��ʱ�޸��ط�����Ϊ1
		g_p_stISO14230Config->cRetransTime = 1;
	}

	while( cRetransTimeTemp-- )
	{
		if ( pFun != NULL )
		{
			if( !pFun( pIn, pOut ) )//����ɹ���ִ����������̣����¿�ʼѭ��
				continue;
		}
		if( bSecuritFlag )	
		{
			for ( i = 0, j = 0; i < piActiveCmdIndex[0]; i++ )
			{
				if ( i == piSecuriteIndex[j][0] )//���Ǽ��ܵ���������㰲ȫ�㷨
				{
					g_iRequestSeedCmdIndex[1] = i++;
					g_iSendKeyCmdIndex[1] = i;
					iReceiveResult = process_security_access_algorithm( piSecuriteIndex[j][1], pOut );
					if (!iReceiveResult)
					{
						return false;
					}
					j++;
				}
				else//���Ǽ�����������������
				{
					piSendCmd[1] = piActiveCmdIndex[i + 1];
					iReceiveResult = send_and_receive_cmd( piSendCmd );
					if( ( iReceiveResult != SUCCESS ) && ( iReceiveResult != NEGATIVE ) )//�������ʧ�������forѭ��
					{
						i = piActiveCmdIndex[0];
						bResultStatus = false;
					}
				}
			}
		}
		else//�����Ͱ�ȫ�㷨��ֱ�ӷ�����
		{
			iReceiveResult = send_and_receive_cmd( piActiveCmdIndex );
		}

		if( !iReceiveResult )//������ͳɹ����˳�whileѭ��
		{
			bResultStatus = true;
			break;
		}
	}

	if ( g_p_stProcessFunConfig->cActiveECUFunOffset != 0)
	{
		g_p_stISO14230Config->cRetransTime = cRetransTime;
	}
	
	g_iRequestSeedCmdIndex[1] = iRequestSeedCmdIndexBak;
	g_iSendKeyCmdIndex[1] = iSendKeyCmdIndexBak;
	
	return bResultStatus;
}

/*************************************************
Description:	kwpЭ�鼤�����
Input:	pIn		���������������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
���ʽ:0x01 �ߵ͵�ƽ��25ms����0x04
*************************************************/
bool process_active_ECU_by_low_high_voltage( void* pIn, void* pOut )
{
	bool bSendStatus = true;

	if( g_p_stGeneralActiveEcuConfig->cActiveMode != 0x00 ) /* ������C2���� */
	{
		//���ͼ�����������
		bSendStatus = package_and_send_active_config();
	}
	return bSendStatus;
}

/*************************************************
Description:	��5�����ʵ�ַ�뼤�����
Input:	pIn		���������������
Output:	pOut	������ݵ�ַ
Return:	����
Others:
*************************************************/
bool process_active_ECU_by_5_baud_address( void* pIn, void* pOut )
{
	bool bStatus	= false;

	byte cReceiveKeyByte[3] = {0};
	int i = 0;

	uint16	u16ECUResByteTimeout = g_p_stGeneralActiveEcuConfig->u16ECUResByteTimeout;

	STRUCT_CMD	stActiveCmdTemp = {0};

	//���ͼ�����������
	bStatus = package_and_send_active_config();

	if( !bStatus )
	{
		return bStatus;
	}

	if( g_p_stGeneralActiveEcuConfig->cActiveMode == 0x02 ) /* ��ͨ��5bps���protocolȡ�� */
	{
		//���չؼ��ֽڣ������Լ�ģʽΪ[���Լ�]ʱVCI�ϴ�0x55 0xMM 0xNN�������ϴ�0xMM 0xNN
		bStatus = ( bool )receive_cmd( cReceiveKeyByte, g_p_stGeneralActiveEcuConfig->cReceiveKeyByte[0]
		                                      , u16ECUResByteTimeout * 3 );

		if( !bStatus )
		{
			return bStatus;
		}

		//�жϽ��յ��Ĺؼ��ֽڸ��趨���Ƿ���ͬ
		for( ; i < g_p_stGeneralActiveEcuConfig->cReceiveKeyByte[0]; i++ )
		{
			if( cReceiveKeyByte[i] != g_p_stGeneralActiveEcuConfig->cReceiveKeyByte[i + 1] )
			{
				return false;
			}
			//Ҳ���Խ�����������
		}

		stActiveCmdTemp.cBufferOffset = 0;
		stActiveCmdTemp.cReserved = 0;
		stActiveCmdTemp.iCmdLen = 1;
		stActiveCmdTemp.pcCmd = ( byte* )malloc( sizeof( byte ) );
		stActiveCmdTemp.pcCmd[0] = ~cReceiveKeyByte[1];//�޸ļ�������������һ���ֽ�

		//����ȡ���ֽ�,����03ȡ���Ŀɰ�ȡ�����ֽڵ�����������ʹ�÷ŵ�command.xml��
		if( !package_and_send_frame( FRAME_HEAD_NORMAL_FRAME, ( STRUCT_CMD* )&stActiveCmdTemp, 0xaa ) )
		{
			free( stActiveCmdTemp.pcCmd );
			return false;
		}
	}
	else if( g_p_stGeneralActiveEcuConfig->cActiveMode == 0x05 ||  \
	         g_p_stGeneralActiveEcuConfig->cActiveMode == 0x06 ) /* 0x05 VDIȡ�� ����ϵͳ, 0x06 VDI��ȡ�� */
	{
		bStatus = ( bool )receive_cmd( cReceiveKeyByte, g_p_stGeneralActiveEcuConfig->cReceiveKeyByte[0]
		                                      , u16ECUResByteTimeout * 3 );
	}

	free( stActiveCmdTemp.pcCmd );
	return true;
}

