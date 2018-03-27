/**********************************************************************
Copyright:		YiXiong AUTO S&T Co., Ltd.
Description:	�������ISO15765Э�鴦���շ����ݵ���غ���
History:
	<author>	<time>		<desc>

************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "iso_15765.h"
#include "../interface/protocol_interface.h"
#include "../public/protocol_config.h"
#include "../command/command.h"
#include "../InitConfigFromXml/init_config_from_xml_lib.h"
#include "../function/idle_link_lib.h"
#include "../public/public.h" 

/*************************************************
Description:	����ISO15765Э���շ�����
Input:	piCmdIndex	����������ַ

Output:	none
Return:	int	�շ�����ʱ��״̬
Others:	���������������Է��ͺͽ��ն������
		��ECU������Ӧ�����ݷŵ�������Ӧ�Ļ�����
ע�⣺������ECU�����Ǵ�SID�Ļظ���ʼ�ģ��磺
7E0  03 22 15 08 00 00 00 00
7E8  04 62 15 08 79 00 00 00
�Ǵ�62��ʼ���棬��Ϊ������Ӧ���7F��ʼ���档
*************************************************/
int send_and_receive_cmd_by_iso_15765( const int* piCmdIndex )
{
	int iStatus = TIME_OUT;
	int i = 0;
	int iCmdSum = piCmdIndex[0];
	byte cReceiveBuffer[20] = {0};
	STRUCT_CMD stSendCmd = {0};

	for( i = 0; i < iCmdSum; i++ )
	{
		iStatus = send_and_receive_single_cmd_by_iso_15765( ( STRUCT_CMD * )&stSendCmd, piCmdIndex[1 + i], cReceiveBuffer );

		if( NULL != stSendCmd.pcCmd )
			free( stSendCmd.pcCmd );

		if( ( iStatus != SUCCESS ) && ( iStatus != NEGATIVE ) ) //���״̬�Ȳ���SUCCESS�ֲ���NEGATIVE����Ϊ����
		{
			break;
		}

		time_delay_ms( g_p_stISO15765Config->u16TimeBetweenFrames );
	}

	return iStatus;
}
/*************************************************
Description:	�շ����������
Input:
	cCmdIndex		��������
	pcReceiveBuffer	���ECU�ظ����ݵĻ���

Output:	none
Return:	int	�շ�����ʱ��״̬
Others:	����ISO15765Э�鴦��
*************************************************/
int send_and_receive_single_cmd_by_iso_15765( STRUCT_CMD *pstSendCmd, const int cCmdIndex, byte* pcReceiveBuffer )
{
	int		i = 0;
	int		j = 0;
	int		iReceiveStatus = TIME_OUT;
	byte	cIso15765ReservedByte	= g_p_stISO15765Config->cReserved;
	byte	cRetransTime			= g_p_stISO15765Config->cRetransTime;
	uint16	u16ECUResTimeout		= g_p_stISO15765Config->u16ECUResTimeout;
	bool	bSendStatus = false;
	int		iCmdLen = 0;
	bool	bMultiFrame = false;//��֡��־
	int 	iReceiveFCStatus = FAIL;//�ȴ�FC֡��־
	byte	cFrameSum = 0;
	STRUCT_CMD stCopySendCmd = {0};

	byte	cBufferOffset = 0;
	const byte ExtendCAN = 2;
	const byte StandardCAN = 0;

	//���� ��չCAN֧�� 2015��10��20��16:46:37
	 
	if (g_CANoffset != 0 && g_CANoffset != 2)
	{
		return FORMAT_ERORR;
	}

	pstSendCmd->cBufferOffset	= g_stInitXmlGobalVariable.m_p_stCmdList[cCmdIndex].cBufferOffset;
	pstSendCmd->cReserved		= g_stInitXmlGobalVariable.m_p_stCmdList[cCmdIndex].cReserved;
	pstSendCmd->iCmdLen			= g_stInitXmlGobalVariable.m_p_stCmdList[cCmdIndex].iCmdLen;
	pstSendCmd->pcCmd			= ( byte* )malloc( sizeof( byte ) * pstSendCmd->iCmdLen );
	memcpy( pstSendCmd->pcCmd, g_stInitXmlGobalVariable.m_p_stCmdList[cCmdIndex].pcCmd, pstSendCmd->iCmdLen );

	cBufferOffset = pstSendCmd->cBufferOffset;

	if( pstSendCmd->iCmdLen > 2 && ( pstSendCmd->pcCmd[2 + g_CANoffset] & 0x10 ) == 0x10 )
	{
		bMultiFrame = true;
		iCmdLen = pstSendCmd->iCmdLen;

		cFrameSum = ( iCmdLen - 10 - g_CANoffset) / 7;
		cFrameSum += ( ( iCmdLen - 10 - g_CANoffset) % 7 == 0 ) ? 0 : 1;
	}

	//��֡���ͣ���װ��һ֡����
	if( bMultiFrame )
	{
		i = j = cRetransTime;  //���ʹ���

		while( 1 )
		{
			stCopySendCmd.iCmdLen = pstSendCmd->iCmdLen + cFrameSum * (4 + g_CANoffset) + 4;   //�����
			stCopySendCmd.pcCmd = ( byte* )malloc( sizeof( byte ) * stCopySendCmd.iCmdLen );

			stCopySendCmd.pcCmd[0] = ( byte )g_p_stISO15765Config->cMultiframestime;	//��֡֡ʱ����

			stCopySendCmd.pcCmd[1] = 0x01; //֡����  0x01: CAN��֡��ʽ����һ֡��30֡�ظ��� 0x02:��һ֡�ȴ��ظ�һ֡
			stCopySendCmd.pcCmd[2] = cFrameSum + 1; //֡����

			stCopySendCmd.pcCmd[3] = 10 + g_CANoffset; //��һ֡������ֽڸ���

			memcpy( stCopySendCmd.pcCmd + 4, pstSendCmd->pcCmd, 10 + g_CANoffset );

			for( i = 0; i < cFrameSum; i++ )
			{
				if(g_CANoffset == StandardCAN)
				{
					stCopySendCmd.pcCmd[14 + i * 11 + 1] = stCopySendCmd.pcCmd[4]; //ID
					stCopySendCmd.pcCmd[14 + i * 11 + 2] = stCopySendCmd.pcCmd[5]; //ID
					stCopySendCmd.pcCmd[14 + i * 11 + 3] = 0x20 | ( ( i + 1 ) & 0x0F );
				}
				else if (g_CANoffset == ExtendCAN)
				{
					stCopySendCmd.pcCmd[16 + i * 13 + 1] = stCopySendCmd.pcCmd[4]; //ID
					stCopySendCmd.pcCmd[16 + i * 13 + 2] = stCopySendCmd.pcCmd[5]; //ID
					stCopySendCmd.pcCmd[16 + i * 13 + 3] = stCopySendCmd.pcCmd[6]; //ID
					stCopySendCmd.pcCmd[16 + i * 13 + 4] = stCopySendCmd.pcCmd[7]; //ID
					stCopySendCmd.pcCmd[16 + i * 13 + 5] = 0x20 | ( ( i + 1 ) & 0x0F );
				}

				if( i != cFrameSum - 1 ) //����������һ֡
				{
					memcpy( stCopySendCmd.pcCmd + (14 + g_CANoffset) + i * (11 + g_CANoffset) + (4 + g_CANoffset), pstSendCmd->pcCmd + (10 + g_CANoffset) + i * 7, 7 );
					stCopySendCmd.pcCmd[14 + g_CANoffset + i * (11 + g_CANoffset)] = 2 + g_CANoffset + 1 + 7; //ÿ֡������ֽڸ���

				}
				else
				{
					memcpy( stCopySendCmd.pcCmd + (14 + g_CANoffset) + i * (11 + g_CANoffset) + (4 + g_CANoffset), pstSendCmd->pcCmd + (10 + g_CANoffset) + i * 7, iCmdLen - (10 + g_CANoffset) - i * 7 );
					stCopySendCmd.pcCmd[14 + g_CANoffset + i * (11 + g_CANoffset)] = 2 + g_CANoffset + 1 + iCmdLen - (10 + g_CANoffset) - i * 7; //ÿ֡������ֽڸ���
				}
			}

			bSendStatus = package_and_send_frame( FRAME_HEAD_MULTI_FRAME, &stCopySendCmd, cIso15765ReservedByte );

			//�ͷ��ڴ�
			if( NULL != stCopySendCmd.pcCmd )
			{
				free( stCopySendCmd.pcCmd );
				stCopySendCmd.pcCmd = NULL;
			}

			if( !bSendStatus )
			{
				return FAIL;
			}

			iReceiveStatus = process_CAN_receive_Cmd( cBufferOffset, pcReceiveBuffer );

			if( TIME_OUT == iReceiveStatus )
			{
				if( ( --j ) == 0 )
					return iReceiveStatus;
			}
			else
				return iReceiveStatus;
		}//end while
	}

	while( 1 )
	{
		bSendStatus = package_and_send_frame( FRAME_HEAD_NORMAL_FRAME, pstSendCmd, cIso15765ReservedByte );

		if( !bSendStatus )
		{
			return FAIL;
		}

		iReceiveStatus = process_CAN_receive_Cmd( cBufferOffset, pcReceiveBuffer );

		switch( iReceiveStatus )
		{
		case TIME_OUT:
		{
			if( ( --cRetransTime ) == 0 )
				return iReceiveStatus;


		}
		break;

		case SUCCESS:
		case FRAME_TIME_OUT:
		case NEGATIVE:

		default:
			return iReceiveStatus;

		}
	}
}
/*************************************************
Description:	�����׼CAN�Ľ���
Input:	cBufferOffset	�洢����ƫ��
		pcSource		����ECU�ظ����ݵĻ���

Output:	none
Return:	int	�շ�����ʱ��״̬
Others:	����ISO15765Э�鴦����գ�����
		cBufferOffset��Ӧ����g_stBufferGroup
		��pcSource�ǵ���receive_cmd����ʱ�ݴ�
		���ݵĵ�ַ��
*************************************************/
int process_CAN_receive_Cmd( const byte cBufferOffset, byte* pcSource )
{
	int	iReceiveStatus	= FAIL;
	bool	bSendStatus		= false;

	uint16	u16Code7F78Timeout	= g_p_stISO15765Config->u16Code7F78Timeout;
	uint16	u16ECUResTimeout	= g_p_stISO15765Config->u16ECUResTimeout;
	byte	cReserved			= g_p_stISO15765Config->cReserved;

	byte*	pcDestination		= g_stBufferGroup[cBufferOffset].cBuffer;

	byte	cBSmax				= 0;

	int		i = 0;
	int		iNegativeResponseCounter = 0;
	int		iValidLen = 0;

	//����ECU�ظ�����ĵ�һ֡
	iReceiveStatus = process_CAN_receive_single_Cmd( pcSource, u16ECUResTimeout );

	if( iReceiveStatus != SUCCESS )
	{
		return iReceiveStatus;
	}

	if( ( pcSource[2 + g_CANoffset] == 0x03 ) && ( pcSource[3 + g_CANoffset] == 0x7f ) ) //�ж�������Ӧ
	{
		if( pcSource[5 + g_CANoffset] == 0x78 )
		{
			while( 1 ) //���while����������ȴ����7F78��Ŀǰ����200��
			{
				iReceiveStatus = process_CAN_receive_single_Cmd( pcSource, u16Code7F78Timeout );

				if( iReceiveStatus != SUCCESS )
				{
					return iReceiveStatus;
				}

				if( ( pcSource[2 + g_CANoffset] == 0x03 ) && ( pcSource[3 + g_CANoffset] == 0x7f ) && ( pcSource[5 + g_CANoffset] == 0x78 ) )
					iNegativeResponseCounter++;
				else if( ( pcSource[2 + g_CANoffset] == 0x03 ) && ( pcSource[3 + g_CANoffset] == 0x7f ) )
				{
					memcpy( pcDestination, &pcSource[3 + g_CANoffset], 3 ); //����������Ӧ����
					g_stBufferGroup[cBufferOffset].iValidLen = 3;
					return NEGATIVE;
				}
				else
					break;

				if( iNegativeResponseCounter == 200 )
					return TIME_OUT;

			}

		}//end if
		else
		{
			memcpy( pcDestination, &pcSource[3 + g_CANoffset], 3 ); //����������Ӧ����
			g_stBufferGroup[cBufferOffset].iValidLen = 3;
			return NEGATIVE;
		}

	}//end if

	if( ( pcSource[2 + g_CANoffset] & 0x10 ) != 0x10 ) //��֡
	{
		iValidLen = pcSource[2 + g_CANoffset];

		g_stBufferGroup[cBufferOffset].iValidLen = iValidLen;

		memcpy( pcDestination, &pcSource[3 + g_CANoffset], iValidLen );

		return SUCCESS;
	}

	//���������Ϊ�Ƕ�֡

	//cBSmax	= g_stInitXmlGobalVariable.m_p_stCmdList[g_p_stISO15765Config->cFCCmdOffset].pcCmd[3];
	iValidLen = ( pcSource[2 + g_CANoffset] & 0x0F );
	iValidLen <<= 8;
	iValidLen += pcSource[3 + g_CANoffset];

	g_stBufferGroup[cBufferOffset].iValidLen = iValidLen;

	memcpy( pcDestination, &pcSource[4 + g_CANoffset], 6 ); //�ȱ����һ֡����

	pcDestination += 6;
	/*

		//����FC֡
		bSendStatus = package_and_send_frame( FRAME_HEAD_NORMAL_FRAME,
											( STRUCT_CMD * ) & (g_stInitXmlGobalVariable.m_p_stCmdList[g_p_stISO15765Config->cFCCmdOffset]), cReserved );

		if( !bSendStatus )
		{
			return FAILE;
		}*/

	for( ; i < ( ( iValidLen - 6 ) / 7 ); i++ )
	{
		iReceiveStatus = process_CAN_receive_single_Cmd( pcSource, u16ECUResTimeout );

		if( iReceiveStatus != SUCCESS )
		{
			return iReceiveStatus;
		}

		memcpy( &pcDestination[i * 7], &pcSource[3 + g_CANoffset], 7 ); //��������һ֡

	}//end for

	if( ( iValidLen - 6 ) % 7  == 0 ) //���������֡�ͷ��سɹ�
	{
		return SUCCESS;
	}

	//����ʣ���ֽڣ�����1�������ֽڡ�2���ֽ�ID��CF֡�����ֽں�ʣ�����Ч�ֽ�
	iReceiveStatus = process_CAN_receive_single_Cmd( pcSource, u16ECUResTimeout );

	if( iReceiveStatus != SUCCESS )
	{
		return iReceiveStatus;
	}

	memcpy( &pcDestination[i * 7], &pcSource[3 + g_CANoffset], ( ( iValidLen - 6 ) % 7 ) ); //�����������֡

	return SUCCESS;
}


/*************************************************
Description:	�����׼CAN�Ľ���

Input:
	pcSource		����ECU�ظ����ݵĻ���
	u16Timeout		����һ֡����ĳ�ʱ

Output:	pcSource	����ECU�ظ����ݵĻ���
Return:	int	�շ�����ʱ��״̬
Others:	VCI�������ݵĸ�ʽ�ǣ�1���ֽ���Ч�ֽڳ���
							+2���ֽ�ID+��Ч�ֽ�
*************************************************/
int process_CAN_receive_single_Cmd( byte* pcSource, const uint16 u16Timeout )
{
	bool	bReceiveStatus	= false;
	byte	cValidLen = 0;//

	//�Ƚ���CAN֡���ݵ���Ч�ֽڳ���
	bReceiveStatus = ( bool )receive_cmd( &cValidLen, 1, u16Timeout );

	if( !bReceiveStatus )
		return TIME_OUT;

	//����ID+��Ч����
	bReceiveStatus = ( bool )receive_cmd( pcSource, cValidLen + 2 + g_CANoffset, u16Timeout );

	if( !bReceiveStatus )
		return TIME_OUT;

	return SUCCESS;

}

