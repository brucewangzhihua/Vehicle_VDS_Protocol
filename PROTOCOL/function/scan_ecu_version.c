/**********************************************************************
Copyright:		YiXiong AUTO S&T Co., Ltd.
Description:	ϵͳ����ʱɨ��ECU�汾�ŵĺ��������ļ���
History:
<author>    ��Ө
<time>		2016��10��26��
<desc>      
************************************************************************/
#include <ctype.h>
#include "scan_ecu_version.h"

/*************************************************
Description:	��ȡECU�汾������ECU�汾������Ӧ�ļ������
Input:
pIn		�������ȡECU�汾�йص��������ݺʹ�UI���������
Output:	pOut	������ݵ�ַ
Return:	void
Others:	���ݵ�һ����������ִ�в�ͬ�Ĺ��ܺ���
*************************************************/
void Read_ECU_Version( STRUCT_CHAIN_DATA_INPUT* pstParam, void* pOut )
{
	int piActiveCmdIndex[2] = {1,0};
	int iReceiveResult = TIME_OUT;
	byte cBufferOffset = 0;
	int  SoftwareVersionOffset = 0;
	int  SoftwareVersionLength = 0;
	byte SoftwareVersionComper[255] = {0};//Ҫ�Ƚϵİ汾�ַ��������255����������������Ĵ����鳤��
	int ipOutLen = 0;
	int i = 0, j = 0;
	byte* pcECUversion = NULL;
	bool bMatch = false;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );
	get_command_config_data( pstParam, &( g_stInitXmlGobalVariable.m_p_stCmdList ) ); //��ȡ��������

	cBufferOffset = g_stInitXmlGobalVariable.m_p_stCmdList[piActiveCmdIndex[1]].cBufferOffset;

	iReceiveResult = send_and_receive_cmd( piActiveCmdIndex );

	pstParam = pstParam->pNextNode; 
	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );
	SoftwareVersionOffset =  atoi( pstParam->pcData );

	pstParam = pstParam->pNextNode; 

	if( iReceiveResult == SUCCESS )
	{
		while (pstParam != NULL)
		{
			memcpy(SoftwareVersionComper,pstParam->pcData,pstParam->iLen);
			SoftwareVersionLength = pstParam->iLen;

			pstParam = pstParam->pNextNode;
			
			pcECUversion = g_stBufferGroup[cBufferOffset].cBuffer + SoftwareVersionOffset;

			if ( g_stBufferGroup[cBufferOffset].iValidLen - SoftwareVersionOffset < SoftwareVersionLength ) 
			{//���ظ�����Ȱ汾�ų��ȶ̣���ƥ��ʧ��
				bMatch = false;
			}
			else
			{
				i = 0;
				j = SoftwareVersionOffset;
				while ( 1 )
				{
					if ( isdigit( *pcECUversion ) )//�ж��Ƿ�Ϊ���֣���Ϊ��������бȽϣ����򲻱ȶ�
					{
						if ( SoftwareVersionComper[i++] == *pcECUversion )
						{
							if ( i >=  SoftwareVersionLength )
							{
								bMatch = true;
								break;
							}
						}
						else
						{
							bMatch = false;
							break;
						}
					}
					if( ++j >= g_stBufferGroup[cBufferOffset].iValidLen )//���ظ������ѱȶ��꣬���˳�ѭ��
					{
						bMatch = false;
						break;
					}
					pcECUversion++;
				}
			}
			if ( bMatch )
			{
				ipOutLen = special_return_status( PROCESS_OK | HAVE_JUMP | HAVE_TIP_DATA, pstParam->pcData, "SoftwareVersion", 0, pOut);
				add_data_and_controlsID(ipOutLen,g_stBufferGroup[cBufferOffset].cBuffer + SoftwareVersionOffset, j,ORIGINAL,NULL,pOut);
				return; //���ҵ���ƥ��İ汾������ֹwhileѭ��������
			}
			else//�汾��ƥ�䣬�������һ���ıȽ�
			{
				pstParam = pstParam->pNextNode;
			}
		}
		//���е��˴�˵��δƥ�䵽��Ӧ�İ汾��������Ĭ�ϰ汾ִ��
		ipOutLen = special_return_status( PROCESS_OK | HAVE_JUMP | HAVE_TIP_DATA, "DEFAULT", "ECU_version_not_support", 0, pOut );
		add_data_and_controlsID( ipOutLen, g_stBufferGroup[cBufferOffset].cBuffer+SoftwareVersionOffset, g_stBufferGroup[cBufferOffset].iValidLen - SoftwareVersionOffset, ORIGINAL, NULL, pOut);
	}
	else if (  iReceiveResult == NEGATIVE )
	{
		//�ظ�������Ӧ������Ĭ�ϰ汾
		ipOutLen = special_return_status( PROCESS_OK | HAVE_JUMP | HAVE_TIP_DATA, "DEFAULT", "ECU_version_not_support", 0, pOut );
		add_data_and_controlsID( ipOutLen, g_stBufferGroup[cBufferOffset].cBuffer, 3, ORIGINAL, NULL, pOut );
		return;
	}
	else
	{
		//ʧ��������ACTIVE_FAIL����
		special_return_status( PROCESS_FAIL | HAVE_JUMP | NO_TIP, "ACTIVE_FAIL", NULL, 0, pOut );
		return;
	}

}


void Check_ECU_Version(STRUCT_CHAIN_DATA_INPUT* pstParam, void* pOut)
{
	int piActiveCmdIndex[2] = {1,0};
	int iReceiveResult = TIME_OUT;
	int iReceiveValidLen = 0;//���յ�����Ч�ֽڳ���
	int ComperSize = 0;
	byte cBufferOffset = 0;
	int  SoftwareVersionOffset = 0;
	int  SoftwareVersionLength = 0;
	byte SoftwareVersionComper[50] = {0};
	int iAppendDataOffset = 0;
	byte idleLinks_Station = 0;


	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );
	get_command_config_data( pstParam, &( g_stInitXmlGobalVariable.m_p_stCmdList ) ); //��ȡ��������

	cBufferOffset = g_stInitXmlGobalVariable.m_p_stCmdList[piActiveCmdIndex[1]].cBufferOffset;

	iReceiveResult = send_and_receive_cmd( piActiveCmdIndex );

	iReceiveValidLen = g_stBufferGroup[cBufferOffset].iValidLen;

	pstParam = pstParam->pNextNode; 
	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );
	SoftwareVersionOffset =  atoi( pstParam->pcData );

	pstParam = pstParam->pNextNode; 
	switch( iReceiveResult )
	{
	case SUCCESS:
		{
			assert( pstParam->pcData );
			assert( pstParam->iLen != 0 );
			memcpy(SoftwareVersionComper,pstParam->pcData,pstParam->iLen);
			SoftwareVersionLength = pstParam->iLen;
			if (!memcmp(SoftwareVersionComper,g_stBufferGroup[cBufferOffset].cBuffer+SoftwareVersionOffset,SoftwareVersionLength))
			{
				iAppendDataOffset = special_return_status( PROCESS_OK | HAVE_JUMP | HAVE_TIP_DATA, pstParam->pcData, "SoftwareVersion", 0, pOut);
				add_data_and_controlsID(iAppendDataOffset,SoftwareVersionComper,SoftwareVersionLength,ORIGINAL,NULL,pOut);
				pstParam = pstParam->pNextNode; 
				assert( pstParam->pcData );
				assert( pstParam->iLen != 0 );
				idleLinks_Station = atoi( pstParam->pcData );
				set_idle_link( idleLinks_Station );
				if (1 == idleLinks_Station)
				{
					g_iActiveECUStatus = ACTIVE_ECU_SUCCESS;
				}
				special_return_status( PROCESS_OK | NO_JUMP | NO_TIP, NULL, NULL, 0, pOut );
				break;
			}
			else
			{
				iAppendDataOffset = special_return_status( PROCESS_FAIL | HAVE_JUMP | HAVE_TIP_DATA, "ACTIVE_FAIL", "ECU_version_not_support", 0, pOut );
				add_data_and_controlsID(iAppendDataOffset,g_stBufferGroup[cBufferOffset].cBuffer+SoftwareVersionOffset,SoftwareVersionLength,ORIGINAL,NULL,pOut);
			}

		}
		break;

	case NEGATIVE:
	case FRAME_TIME_OUT:
	case TIME_OUT:
	default:
		special_return_status( PROCESS_FAIL | HAVE_JUMP | NO_TIP, "ACTIVE_FAIL", NULL, 0, pOut );
		break;
	}
}

/*************************************************
Description:	ɨ��ECU�汾������
Input:
pIn		������ɨ��汾�йص��������ݺʹ�UI���������
Output:	pOut	������ݵ�ַ
Return:	void
Others:	���ݵ�һ����������ִ�в�ͬ�Ĺ��ܺ���
*************************************************/
void process_ECU_version_function( void* pIn, void* pOut )
{
	STRUCT_CHAIN_DATA_INPUT* pstParam = ( STRUCT_CHAIN_DATA_INPUT* )pIn;
	byte FunctionSlect = 0;
	int ReadECUVersion = 0;

	assert( pstParam->pcData );
	assert( pstParam->iLen != 0 );
	FunctionSlect = atoi( pstParam->pcData );

	pstParam = pstParam->pNextNode;
	switch( FunctionSlect )
	{
	case 0:
		{
			Read_ECU_Version(pstParam,pOut);
		}
		break;

	case 1:
		{
			Check_ECU_Version(pstParam,pOut);
		}
		break;

	default:
		break;
	}
}
