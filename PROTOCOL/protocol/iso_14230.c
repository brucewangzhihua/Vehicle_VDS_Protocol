/**********************************************************************
Copyright:		YiXiong AUTO S&T Co., Ltd.
Description:	�������ISO14230Э�鴦���շ����ݵ���غ���
History:
	<author>	<time>		<desc>

************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "iso_14230.h"
#include "../interface/protocol_interface.h"
#include "../public/protocol_config.h"
#include "../command/command.h"
#include "../public/public.h" 
#include "../InitConfigFromXml/init_config_from_xml_lib.h"
#include "../function/idle_link_lib.h"

/*************************************************
Description:	����ISO14230Э���շ�����
Input:	piCmdIndex	����������ַ
Output:	none
Return:	int	�շ�����ʱ��״̬
Others:	���������������Է��ͺͽ��ն������
		��ECU������Ӧ�����ݷŵ�������Ӧ�Ļ�����
ע�⣺������ECU�����Ǵ�SID�Ļظ���ʼ�ģ��磺
CMD:80 59 F1 01 81 4C RET:80 F1 59 03 C1 EA 8F 07
�Ǵ�C1��ʼ���棬��Ϊ������Ӧ���7F��ʼ���档
*************************************************/
int send_and_receive_cmd_by_iso_14230( const int* piCmdIndex )
{
	int iStatus = TIME_OUT;
	int i = 0;
	int iCmdSum = piCmdIndex[0];
	byte cReceiveBuffer[256] = {0};
	STRUCT_CMD stSendCmd = {0};

	for( i = 0; i < iCmdSum; i++ )
	{
		iStatus = send_and_receive_single_cmd_by_iso_14230( ( STRUCT_CMD * )&stSendCmd, piCmdIndex[1 + i], cReceiveBuffer );

		if( NULL != stSendCmd.pcCmd )
			free( stSendCmd.pcCmd );

		if( ( iStatus != SUCCESS ) && ( iStatus != NEGATIVE ) ) //���״̬�Ȳ���SUCCESS�ֲ���NEGATIVE����Ϊ����
		{
			break;
		}

		//time_delay_ms( g_p_stISO14230Config->u16TimeBetweenFrames );
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
Others:	����ISO14230Э�鴦��
*************************************************/
int send_and_receive_single_cmd_by_iso_14230( STRUCT_CMD *pstSendCmd, const int cCmdIndex, byte* pcReceiveBuffer )
{
	int		iReceiveStaus = TIME_OUT;
	byte	cTimeBetweenBytes		= g_p_stISO14230Config->cTimeBetweenBytes;
	byte	cRetransTime			= g_p_stISO14230Config->cRetransTime;
	bool	bSendStatus = false;

	byte	cBufferOffset = 0;


	pstSendCmd->cBufferOffset	= g_stInitXmlGobalVariable.m_p_stCmdList[cCmdIndex].cBufferOffset;
	pstSendCmd->cReserved		= g_stInitXmlGobalVariable.m_p_stCmdList[cCmdIndex].cReserved;
	pstSendCmd->iCmdLen			= g_stInitXmlGobalVariable.m_p_stCmdList[cCmdIndex].iCmdLen;
	pstSendCmd->pcCmd			= ( byte* )malloc( sizeof( byte ) * pstSendCmd->iCmdLen );
	memcpy( pstSendCmd->pcCmd, g_stInitXmlGobalVariable.m_p_stCmdList[cCmdIndex].pcCmd, pstSendCmd->iCmdLen );

	cBufferOffset = pstSendCmd->cBufferOffset;
	pre_process_cmd( pstSendCmd ); //����У���

	while( 1 )
	{
		switch( g_p_stVCI_params_config->cCommunicationType )
		{
		case 0x00:
			bSendStatus = package_and_send_frame( FRAME_HEAD_NORMAL_FRAME, pstSendCmd, cTimeBetweenBytes );
			break;

		case 0x01:
		case 0x50:
		case 0x51:/* ��ʱ���ϴ� */
			bSendStatus = package_and_send_frame_time( FRAME_HEAD_NORMAL_FRAME, pstSendCmd, cTimeBetweenBytes );
			break;

		default:
			return PROTOCOL_ERROR;
			break;
		}


		if( !bSendStatus )
		{
			return FAIL;
		}

		iReceiveStaus = process_KWP_receive_Cmd( cBufferOffset, pcReceiveBuffer );

		switch( iReceiveStaus )
		{

		case TIME_OUT:
		{
			if( ( --cRetransTime ) == 0 )
				return iReceiveStaus;
		}
		break;

		case SUCCESS:
		case FRAME_TIME_OUT:
		case NEGATIVE:

		default:
			return iReceiveStaus;
		}
	}
}


/*************************************************
Description:	�����׼KWP�Ľ���
Input:
	cBufferOffset	�洢����ƫ��
	pcSource		����ECU�ظ����ݵĻ���

Output:	none
Return:	int	�շ�����ʱ��״̬
Others:	����ISO14230Э�鴦����գ�����
		cBufferOffset��Ӧ����g_stBufferGroup
		��pcSource�ǵ���receive_cmd����ʱ�ݴ�
		���ݵĵ�ַ��
*************************************************/
int process_KWP_receive_Cmd( const byte cBufferOffset, byte* pcSource )
{
	uint16	u16Code7F78Timeout	= g_p_stISO14230Config->u16Code7F78Timeout;
	uint16	u16ECUResTimeout	= g_p_stISO14230Config->u16ECUResTimeout;

	byte*	pcDestination		= g_stBufferGroup[cBufferOffset].cBuffer;

	int		iNegativeResponseCounter = 0;
	int		iValidLen = 0;
	int 	iReceiveStatus	= FAIL;

	iReceiveStatus = process_KWP_receive_single_Cmd( &pcSource, &iValidLen, u16ECUResTimeout );

	if( iReceiveStatus != SUCCESS )
	{
		return iReceiveStatus;
	}
	
	if (iValidLen == 0)
	{
		g_stBufferGroup[cBufferOffset].iValidLen = 0;
		return SUCCESS;
	}

	if( ( iValidLen == 0x03 ) && ( pcSource[0] == 0x7f ) ) //�ж�������Ӧ
	{
		if( pcSource[2] == 0x78 )
		{
			while( 1 ) //���while����������ȴ����7F78��Ŀǰ����200��
			{
				iReceiveStatus = process_KWP_receive_single_Cmd( &pcSource, &iValidLen, u16Code7F78Timeout );

				if( iReceiveStatus != SUCCESS )
				{
					return iReceiveStatus;
				}

				if( ( iValidLen == 0x03 ) && ( pcSource[0] == 0x7f ) && ( pcSource[2] == 0x78 ) )
					iNegativeResponseCounter++;
				else if( ( iValidLen == 0x03 ) && ( pcSource[0] == 0x7f ) )
				{
					memcpy( pcDestination, &pcSource[0], 3 ); //����������Ӧ����
					g_stBufferGroup[cBufferOffset].iValidLen = 3;
					return NEGATIVE;
				}
				else
					break;

				if( iNegativeResponseCounter == 200 )
					return TIME_OUT;

			}//end while

		}//end if
		else
		{
			memcpy( pcDestination, &pcSource[0], 3 ); //����������Ӧ����
			g_stBufferGroup[cBufferOffset].iValidLen = 3;
			return NEGATIVE;
		}

	}//end if

	//������Ч���ݵ�ָ���Ļ�����
	memcpy( pcDestination, pcSource, iValidLen );
	//�޸Ļ������Ч�ֽ�
	g_stBufferGroup[cBufferOffset].iValidLen = iValidLen;

	return SUCCESS;
}
/************************************************************************
Description�� ��ȡ��֡����                                                                      
Input      :  ���ݴ洢�� ��Ч�ֽڣ� �ܳ���
              ���õݹ��㷨��������
*************************************************************************/
static int process_receive_Mul_Cmd(byte **ppcSource, int* piValidLen, byte TotolLength)
{
	byte SingleFrameLength;
	bool bReceiveStatus = false;
	int  TempValidLen= 0;

	bReceiveStatus = ( bool )receive_cmd( *ppcSource, 1, 100 );

	switch( ( *ppcSource)[0] & 0xC0 )
	{
	case 0X00://û�е�ַ��Ϣ��
		{
			TempValidLen = ( *ppcSource )[0];
			bReceiveStatus = ( bool )receive_cmd( *ppcSource, g_MulCmdOffset, 100);	
			if( !bReceiveStatus ) //
				return FRAME_TIME_OUT;		
			
			bReceiveStatus = ( bool )receive_cmd( *ppcSource, *piValidLen + 1 - g_MulCmdOffset, 100);	
			if( !bReceiveStatus ) //
				return FRAME_TIME_OUT;		

			SingleFrameLength = TempValidLen + 2;
		}
		break;

	case 0XC0:
	case 0X80:
		{
			if( ( ( *ppcSource )[0] != 0XC0 ) && ( ( *ppcSource )[0] != 0X80 ) ) //�Ȳ���0X80Ҳ����0XC0
			{
				TempValidLen = (( *ppcSource )[0] & 0X3F );
				bReceiveStatus = ( bool )receive_cmd( *ppcSource, 2 + g_MulCmdOffset, 100 );	
				if( !bReceiveStatus ) //�жϽ���ʣ�ಿ��ʱ��״̬
					return FRAME_TIME_OUT;			

				bReceiveStatus = ( bool )receive_cmd( *ppcSource, TempValidLen + 1 - g_MulCmdOffset, 100 );
				if( !bReceiveStatus ) //�жϽ���ʣ�ಿ��ʱ��״̬
					return FRAME_TIME_OUT;			

				SingleFrameLength = TempValidLen + 4;				
			}
			else
			{
				bReceiveStatus = ( bool )receive_cmd( *ppcSource, 2 + 1, 100); //���������ֽڵ�ַ��һ���ֽڳ���
				if( !bReceiveStatus )
					return FRAME_TIME_OUT;

				TempValidLen = ( *ppcSource )[2];

				bReceiveStatus = ( bool )receive_cmd( *ppcSource, g_MulCmdOffset, 100 ); //
				if( !bReceiveStatus ) //�жϽ���ʣ�ಿ��ʱ��״̬
					return FRAME_TIME_OUT;		

				bReceiveStatus = ( bool )receive_cmd( *ppcSource, TempValidLen + 1 - g_MulCmdOffset, 100 ); //���ݳ����ֽڽ���
				if( !bReceiveStatus ) //�жϽ���ʣ�ಿ��ʱ��״̬
					return FRAME_TIME_OUT;

				SingleFrameLength = TempValidLen + 5;
			}
		}
		break;	 

	default:
		return FORMAT_ERORR;
	}
	*piValidLen += (TempValidLen - g_MulCmdOffset);
	
	if (TotolLength - SingleFrameLength <= 0)
	{
		return SUCCESS;
	}
	else
	{
		*ppcSource += (TempValidLen - g_MulCmdOffset);
		return process_receive_Mul_Cmd( ppcSource, piValidLen, TotolLength - SingleFrameLength);
	}
}
/************************************************************************
Description�� 9141��ȡ��֡                                                                      
Input      :  
*************************************************************************/
static void process_9141_Mul_Cmd(byte** ppcSource, int *piValidLen, UNN_2WORD_4BYTE Key)
{
 	byte count = 0;
	byte *pSecond = NULL, *pHead = NULL;	
	byte *pTempSource = NULL;
	int Len = *piValidLen;
	bool bFindFlag = false;

	*piValidLen = 0;
	pSecond = *ppcSource;
	pTempSource = *ppcSource;
	for ( ;pSecond <= (*ppcSource + Len - 3); pSecond++ )
	{
		if (  *pSecond == Key.u8Bit[0] && 
			 *(pSecond + 1) == Key.u8Bit[1] && 
			 *(pSecond + 2) == Key.u8Bit[2] ) /* �ҵ���ͬ���ֽ� */
		{
			bFindFlag = true;
			if (pHead == NULL)/* ��һ���ҵ�����Դ��ַ�ƶ�����һ֡��������β�� */
			{				 
				*piValidLen += (byte)(pSecond - pTempSource - 1);
				pHead = pSecond;
				pTempSource += (pHead - pTempSource - 1);				
			}
			else
			{				 
				count = (byte)(pSecond - pHead) - 4 - g_MulCmdOffset;/*ʵ����Ч����*/
				*piValidLen += count;			
				memcpy(pTempSource, pHead + g_MulCmdOffset + 3, count);
				pTempSource += count;
				pHead = pSecond;
			}
		}

		if (bFindFlag)
		{
			if (pSecond == *ppcSource + Len - 3)
			{
				count = (byte)(pSecond + 3 - pHead) - 4 - g_MulCmdOffset;/*ʵ����Ч����*/
				*piValidLen += count;			
				memcpy(pTempSource, pHead + g_MulCmdOffset + 3, count);
			}
		}		
	}
	if (*piValidLen == 0)
	{
		*piValidLen = Len;
	}
}

/*************************************************
Description:	������KWPЭ�鷽ʽ���յ����ظ�����
Input:	u16FrameTimeout			֡��ʱʱ��

Output:	ppcSource	��Ž������ݵ�ַ��ָ��ĵ�ַ
		piValidLen	����������Ч�ֽ�

Return:	int	����ECU�ظ���״̬
Others: ���ֽ��ϴ� ���ϴ��ľ�����Ч����
        ��ʱ���ϴ��� �ϴ��ĵ�һ���ֽ�����Ч���ݵĳ��ȣ��Ժ���ŵ�����Ч����
*************************************************/
int process_KWP_receive_single_Cmd( byte** ppcSource, int* piValidLen, const uint16 u16FrameTimeout )
{
	bool	bReceiveStatus	= false;
	uint16	u16FrameContentTimeout = g_p_stISO14230Config->u16ECUResTimeout;
	byte TimeTotalFrameLength = 0, SingleFrameLength = 0;
	byte *pTempSource = NULL;	     
	int status = TIME_OUT;
	UNN_2WORD_4BYTE ISO9141Head;
	ISO9141Head.u32Bit = 0;
 
	bReceiveStatus = ( bool )receive_cmd( *ppcSource, 1, u16FrameTimeout ); //���յ�һ���ֽ�
	*piValidLen = **ppcSource;
	TimeTotalFrameLength = *piValidLen;

	if( !bReceiveStatus)
	{
		return TIME_OUT;
	}

	if (*piValidLen == 0)
	{
		return SUCCESS;
	}

	if( g_p_stGeneralActiveEcuConfig->cActiveMode == 0x06 ) /* ����1���ֽ� */
	{
		return SUCCESS;
	}
STATR_RECEIVE:
 	switch( g_p_stVCI_params_config->cCommunicationType ) //����ͨ�����õ�ģʽ
	{
	case 0x01:
	case 0x50:
	case 0x51:/* ��ʱ���ϴ�,�Ƚ���һ���ֽ� */
	{
		bReceiveStatus = ( bool )receive_cmd( *ppcSource, 1, u16FrameTimeout ); //���յ�һ���ֽ�
		if( !bReceiveStatus )
		{
			return TIME_OUT;
		}
	}
	break;

	default: /* Ĭ��������� ���ֽ��ϴ� */
		break;
	}

	switch( ( *ppcSource )[0] & 0xC0 )
	{
	case 0X00://û�е�ַ��Ϣ��
	{
		*piValidLen = ( *ppcSource )[0];
		bReceiveStatus = ( bool )receive_cmd( *ppcSource, *piValidLen + 1, u16FrameContentTimeout );
		SingleFrameLength += *piValidLen + 2;
	}
	break;

	case 0XC0:
	case 0X80:
	{
		if( ( ( *ppcSource )[0] != 0XC0 ) && ( ( *ppcSource )[0] != 0X80 ) ) //�Ȳ���0X80Ҳ����0XC0
		{
			*piValidLen = ( ( *ppcSource )[0] & 0X3F );
			bReceiveStatus = ( bool )receive_cmd( *ppcSource, *piValidLen + 3, u16FrameContentTimeout );
			if( !bReceiveStatus ) //�жϽ���ʣ�ಿ��ʱ��״̬
				return FRAME_TIME_OUT;			
			
			*ppcSource += 2;//���������ֽڵĵ�ַ	
			SingleFrameLength += *piValidLen + 4;
		}
		else
		{
			bReceiveStatus = ( bool )receive_cmd( *ppcSource, 2 + 1, u16FrameContentTimeout ); //���������ֽڵ�ַ��һ���ֽڳ���

			if( !bReceiveStatus )
				return FRAME_TIME_OUT;

			*piValidLen = ( *ppcSource )[2];

			bReceiveStatus = ( bool )receive_cmd( *ppcSource, *piValidLen + 1, u16FrameContentTimeout ); //���ݳ����ֽڽ���
			if( !bReceiveStatus ) //�жϽ���ʣ�ಿ��ʱ��״̬
				return FRAME_TIME_OUT;

			SingleFrameLength += *piValidLen + 5;
		}
	}
	break;

	case 0X40://CARBģʽ
	{
		ISO9141Head.u8Bit[0] = *ppcSource[0];
		bReceiveStatus = ( bool )receive_cmd( *ppcSource, 2, u16FrameContentTimeout ); //6B + ECU��ַ
		if( !bReceiveStatus )
		{
			return FRAME_TIME_OUT;
		}
		if (*piValidLen <= 3)
		{
			return FRAME_TIME_OUT;
		}		
		ISO9141Head.u8Bit[1] = (*ppcSource)[0];
		ISO9141Head.u8Bit[2] = (*ppcSource)[1];
		bReceiveStatus = ( bool )receive_cmd( *ppcSource, *piValidLen - 3, u16FrameContentTimeout ); //����ʣ���ֽ�
		if( !bReceiveStatus )
		{
			return FRAME_TIME_OUT;
		}
		*piValidLen -= 3;
		process_9141_Mul_Cmd(ppcSource, piValidLen, ISO9141Head); /* ��ȡ9141������ */	
		return SUCCESS;
	}
	default:
		return FORMAT_ERORR;
	}

	if ( (TimeTotalFrameLength > SingleFrameLength) && ((*piValidLen == 3) && ((*ppcSource)[0] ==0x7F) && ((*ppcSource)[2] == 0X78)) )
		goto STATR_RECEIVE;

	//����ʱ���ϴ� �ٶȹ��죬��֡�ϲ�Ϊһ֡�ϴ������
	if(g_p_stVCI_params_config->cCommunicationType == 0x01  ||  
		 g_p_stVCI_params_config->cCommunicationType == 0x50 || 
		g_p_stVCI_params_config->cCommunicationType == 0x51)
	{
		if (TimeTotalFrameLength > SingleFrameLength)
		{
			pTempSource = *ppcSource;
			*ppcSource += *piValidLen;
			status = process_receive_Mul_Cmd(ppcSource, piValidLen, TimeTotalFrameLength - SingleFrameLength);  		      
			*ppcSource = pTempSource ;
			return status;
			 
 		}
	}

	return SUCCESS;
}
/*************************************************
Description:	�������������֡(ʱ����Կ���)��
Input:
cFrameHead		����֡ͷ
pstFrameContent	��������ṹ��ָ��
cReservedByte	�����б����ֽ�

Output:	none
Return:	bool	���ط���״̬���ɹ���ʧ�ܣ�
Others:	�ú����᳢�Է������Σ������շ�װ��
�ظ�����
*************************************************/
bool package_and_send_frame_time( const byte cFrameHead, STRUCT_CMD* pstFrameContent, const byte cReservedByte )
{
	UNN_2WORD_4BYTE uFrameLen;
	bool bReturnStatus = false;
	byte *pcSendCache = NULL;
	byte cCheckNum = 0;
	int i = 0;

	uFrameLen.u32Bit = 1 + 2 + 1 + pstFrameContent->iCmdLen + 1 + 2;

	pcSendCache = ( byte * )malloc( ( uFrameLen.u32Bit ) * sizeof( byte ) ); //����֡�Ļ���

	pcSendCache[0] = cFrameHead;
	pcSendCache[1] = uFrameLen.u8Bit[1];
	pcSendCache[2] = uFrameLen.u8Bit[0];
	pcSendCache[3] = cReservedByte;

	pcSendCache[4] = 0x00;
	pcSendCache[5] = 0x37;

	memcpy( &pcSendCache[6], pstFrameContent->pcCmd, pstFrameContent->iCmdLen );

	for( i = 0; ( i < ( int )uFrameLen.u32Bit - 1 ); i++ )
	{
		cCheckNum += pcSendCache[i];
	}

	pcSendCache[ uFrameLen.u32Bit - 1] = cCheckNum;
	send_cmd( pcSendCache, uFrameLen.u32Bit );

	bReturnStatus = receive_confirm_byte( 3000 );

	free( pcSendCache );

	return bReturnStatus;
}
