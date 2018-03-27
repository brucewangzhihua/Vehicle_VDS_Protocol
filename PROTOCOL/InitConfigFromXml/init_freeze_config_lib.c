/**********************************************************************
Copyright:		YiXiong AUTO S&T Co., Ltd.
Description:	�����xml��ȡ����֡���ô�����
History:
	<author>	<time>		<desc>

***********************************************************************/
#include "..\interface\protocol_define.h"
#include "..\public\protocol_config.h"
#include "init_config_from_xml_lib.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*************************************************
Description:	��ȡ����֡����������
Input:
	PIn				������������
Output:	����
Return:	��
Others: ���ݲ�ͬ������ģ�����Ӧ�ķ�֧
*************************************************/
void get_freeze_dtc_config_data( void* pIn )
{
	STRUCT_CHAIN_DATA_INPUT* pstParam = ( STRUCT_CHAIN_DATA_INPUT* )pIn;
	byte cConfigTemp[50] = {0};

	byte cDtcConfigType = 0;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	get_cmd_config_content_data( cConfigTemp, pstParam->pcData, NO_LENGTH_LIMIT );

	cDtcConfigType	= cConfigTemp[1];

	switch( cDtcConfigType )
	{
	case UDS_FREEZE_DTC_CONFIG:
	{
		get_UDS_freeze_dtc_config_data( cConfigTemp );
	}
	break;

	default:
		break;
	}

}

/*************************************************
Description:	��ȡUDS����֡����������
Input:
	PIn				������������
Output:	����
Return:	��
Others:
*************************************************/
void get_UDS_freeze_dtc_config_data( const byte * pcSource )
{
	byte cConfigOffset = 0;

	cConfigOffset = pcSource[0];

	if( g_p_stUDSFreezeDtcConfigGroup[cConfigOffset] == NULL )
		g_p_stUDSFreezeDtcConfigGroup[cConfigOffset] = ( STRUCT_UDS_FREEZE_DTC_CONFIG * )malloc( sizeof( STRUCT_UDS_FREEZE_DTC_CONFIG ) );

	g_p_stUDSFreezeDtcConfigGroup[cConfigOffset]->cFreezeDtcStartOffset	= pcSource[2];
	g_p_stUDSFreezeDtcConfigGroup[cConfigOffset]->cDtcBytesInCmd		= pcSource[3];
	g_p_stUDSFreezeDtcConfigGroup[cConfigOffset]->cDtcBytesInDisplay	= pcSource[4];
	g_p_stUDSFreezeDtcConfigGroup[cConfigOffset]->cModifyOffset			= pcSource[5];
	g_p_stUDSFreezeDtcConfigGroup[cConfigOffset]->cModifyBytes			= pcSource[6];
	g_p_stUDSFreezeDtcConfigGroup[cConfigOffset]->cRecordOffsetInDTC	= pcSource[7];

}

/*************************************************
Description:	ѡ�����������
Input:
	iConfigOffset		����ƫ��
	g_cConfigType			��������������
Output:	����
Return:	��
Others: ���ݼ����������ͺ�����ƫ��ѡ����Ӧ������
*************************************************/
void select_freeze_dtc_config( int iConfigOffset, const byte cConfigType )
{
	switch( cConfigType )
	{
	case UDS_FREEZE_DTC_CONFIG:
		g_p_stUDSFreezeDtcConfig = g_p_stUDSFreezeDtcConfigGroup[iConfigOffset];
		break;

	default:
		break;
	}
}

/*************************************************
Description:	�ͷŴ�Ŷ���֡���������õĿռ�
Input:	��
Output:	����
Return:	��
Others: ÿ���һ�����þ��ڸú����������Ӧ���ͷŴ���
����quit_system_lib.c��free_xml_config_space
�����е��øú�����
*************************************************/
void free_freeze_dtc_config_space( void )
{
	int i = 0;

	for( i = 0; i < sizeof( g_p_stUDSFreezeDtcConfigGroup ) / sizeof( g_p_stUDSFreezeDtcConfigGroup[0] ); i++ )
	{
		if( NULL != g_p_stUDSFreezeDtcConfigGroup[i] )
		{
			free( g_p_stUDSFreezeDtcConfigGroup[i] );
			g_p_stUDSFreezeDtcConfigGroup[i] = NULL;
		}
	}

}

/*************************************************
Description:	��ȡ����֡����������
Input:
	PIn				������������
Output:	����
Return:	��
Others: ���ݲ�ͬ������ģ�����Ӧ�ķ�֧
*************************************************/
void get_freeze_ds_config_data( void* pIn )
{
	STRUCT_CHAIN_DATA_INPUT* pstParam = ( STRUCT_CHAIN_DATA_INPUT* )pIn;
	byte * pcTemp = NULL;
	byte cConfigTemp[15] = {0};
	byte cDSConfigType = 0;
	int iLen = 0;
	int temp = 0;
	byte cConfigID = 0;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );

	pcTemp = pstParam->pcData;

	iLen = get_command_one_block_config_data( cConfigTemp, &temp, pcTemp ); //���ID
	pcTemp += iLen;

	cConfigID = cConfigTemp[0];

	iLen = get_command_one_block_config_data( cConfigTemp, &temp, pcTemp ); //���ģ��ż���������
	pcTemp += iLen;

	cDSConfigType	= cConfigTemp[0];

// 	switch( cDSConfigType )
// 	{
// 	case UDS_FREEZE_DS_CONFIG:
// 	{
// 		get_UDS_freeze_ds_config_data( cConfigID, pcTemp );
// 	}
// 	break;
// 
// 	default:
// 		break;
// 	}

}

