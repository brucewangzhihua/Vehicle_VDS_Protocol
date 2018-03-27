/**********************************************************************
Copyright:		YiXiong AUTO S&T Co., Ltd.
Description:
	1.�����ȡ������״̬������
	2.���崦��汾��Ϣ��ʾ��ʽ������
	3.���崦����ͨ���������㺯����
	4.���崦����֡���������㺯����
	5.���崦��ȫ�����㷨������

History:
	<author>	<time>		<desc>
************************************************************************/

#include <string.h>
#include "formula_comply.h"
#include "formula.h"
#include "../command/command.h"
#include "../public/public.h"
#include "../InitConfigFromXml/init_config_from_xml_lib.h"

int	g_iDefaultSessionCmdIndex[]		= {1, 0};
int	g_iExtendSessionCmdIndex[]		= {1, 0};
int	g_iRequestSeedCmdIndex[]		= {1, 0};
int	g_iSendKeyCmdIndex[]			= {1, 0};


//�������״̬λΪ1����ʾ
byte *DTCStatusBitTrueArry[] =
{
	"DTC_BIT0_TRUE",
	"DTC_BIT1_TRUE",
	"DTC_BIT2_TRUE",
	"DTC_BIT3_TRUE",
	"DTC_BIT4_TRUE",
	"DTC_BIT5_TRUE",
	"DTC_BIT6_TRUE",
	"DTC_BIT7_TRUE"

};
//�������״̬λΪ0����ʾ
byte *DTCStatusBitFalseArry[] =
{
	"DTC_BIT0_FALSE",
	"DTC_BIT1_FALSE",
	"DTC_BIT2_FALSE",
	"DTC_BIT3_FALSE",
	"DTC_BIT4_FALSE",
	"DTC_BIT5_FALSE",
	"DTC_BIT6_FALSE",
	"DTC_BIT7_FALSE"
};
//J1939 FMI
byte *DTCDescriptionArry[] = 
{
	"ID_STR_J1939_FMI_00",
	"ID_STR_J1939_FMI_01",
	"ID_STR_J1939_FMI_02",
	"ID_STR_J1939_FMI_03",
	"ID_STR_J1939_FMI_04",
	"ID_STR_J1939_FMI_05",
	"ID_STR_J1939_FMI_06",
	"ID_STR_J1939_FMI_07",
	"ID_STR_J1939_FMI_08",
	"ID_STR_J1939_FMI_09",
	"ID_STR_J1939_FMI_0A",
	"ID_STR_J1939_FMI_0B",
	"ID_STR_J1939_FMI_0C",
	"ID_STR_J1939_FMI_0D",
	"ID_STR_J1939_FMI_0E",
	"ID_STR_J1939_FMI_0F",
	"ID_STR_J1939_FMI_10",
	"ID_STR_J1939_FMI_11",
	"ID_STR_J1939_FMI_12",
	"ID_STR_J1939_FMI_13",
	"ID_STR_J1939_FMI_14",
	"ID_STR_J1939_FMI_15",
	"ID_STR_J1939_FMI_16",
	"ID_STR_J1939_FMI_17",
	"ID_STR_J1939_FMI_18",
	"ID_STR_J1939_FMI_19",
	"ID_STR_J1939_FMI_1A",
	"ID_STR_J1939_FMI_1B",
	"ID_STR_J1939_FMI_1C",
	"ID_STR_J1939_FMI_1D",
	"ID_STR_J1939_FMI_1E",
	"ID_STR_J1939_FMI_1F",
};


/*************************************************
Description:	��ù�����״̬
Input:
	cDctStatusData	������״̬�ֽ�
	cDtcMask		������maskֵ

Output:	pcOut	��������ַ
Return:	int		�ù�����֧�ֵ�״̬����
Others:
*************************************************/

int get_Dtc_status( byte cDctStatusData, byte *pcOut, byte cDtcMask )
{
	int i = 0;
	int iSupportStatusCounter = 0;//֧�ֵ�״̬����
	byte temp_Status = 0;
	byte temp_SupportStatus = 0;
  
	while( i < 8 )
	{
		temp_SupportStatus = ( ( cDtcMask >> i ) & 0x01 );
		temp_Status = ( ( cDctStatusData >> i ) & 0x01 );

		if( 0x01 == temp_SupportStatus )//λΪ1
		{

			if( iSupportStatusCounter != 0 )
			{
				*pcOut = ',';
				pcOut++;
			}

			if( 0x01 == temp_Status )
			{
				memcpy( pcOut, DTCStatusBitTrueArry[i], strlen( DTCStatusBitTrueArry[i] ) );
				pcOut += strlen( DTCStatusBitTrueArry[i] );
			}
			//else//λΪ0
			//{
			//	memcpy( pcOut, DTCStatusBitFalseArry[i], strlen( DTCStatusBitFalseArry[i] ) );
			//	pcOut += strlen( DTCStatusBitFalseArry[i] );
			//}

			iSupportStatusCounter++;
		}

		i++;

	}

	*pcOut = '\0';

	return iSupportStatusCounter;
}


/*************************************************
Description:	��ù�����״̬
Input:
cDctStatusData	������״̬�ֽ�
cDtcMask		������maskֵ

Output:	pcOut	��������ַ
Return:	int		�ù�����֧�ֵ�״̬����
Others:
*************************************************/

int get_J1939Dtc_status( byte cDctStatusData, byte *pcOut, byte cDtcMask )
{
	memcpy( pcOut, DTCDescriptionArry[cDctStatusData&cDtcMask], strlen( DTCDescriptionArry[cDctStatusData&cDtcMask] ) );
	pcOut += strlen( DTCDescriptionArry[cDctStatusData&cDtcMask] );
	*pcOut = '\0';
	return 1;
}



/*************************************************
Description:	����汾��Ϣ��ʾ��ʽ
Input:
	pcSource	ȡֵ��ַ
	cIvalidLen	��Ч����
	cStyle		��ʾ��ʽ

Output:	pcOut	��������ַ
Return:	void
Others:
*************************************************/

void process_inform_format( const byte* pcSource, byte cIvalidLen, byte cStyle, byte* pcOut )
{
	switch( cStyle )
	{
	case 'A'://ASCII�뷽ʽ����
		DisASCII( pcSource, cIvalidLen, pcOut );
		break;

	case 'H':
	case 'B':
		DisHex( pcSource, cIvalidLen, pcOut );
		break;

	case 'D':
		break;

	default:
		break;
	}
}
/************************************************************************/
/* �������������ַ�����ʽ�õ���                                         */
/************************************************************************/
/*************************************************
typedef struct _STRUCT_STRING_UNIT
{
	byte cCompareData;		//Ҫ�Ƚϵ�����
	byte* pcOutputString;	//Ҫ������ַ���
} STRUCT_STRING_UNIT;
*************************************************/

STRUCT_STRING_UNIT stStringUintArray000[] =
{
	0x00, "ID_STR_DS_CSTRING_000",//��	Off
	0x01, "ID_STR_DS_CSTRING_001",//��	On
};

/*************************************************
typedef struct _STRUCT_DIS_STRING
{
byte cCompareDataSum;					//��Ҫ�Ƚ����ݸ���
byte* pcDefaultOutputString;			//������Ҫ��ʱ������ַ���
STRUCT_STRING_UNIT *pstStringUnitGroup;	//����������
} STRUCT_DIS_STRING;
*************************************************/
STRUCT_DIS_STRING stDisStringArray[] =
{
	{0x02, "----", stStringUintArray000}, //00  bit.0=1:�� bit.0=0:��
};
/*************************************************
Description:	����������ID�������������㹫ʽ
Input:
	iDsId		��������ID
	pcDsSource	ȡֵ��ַ

Output:	pcDsValueOut	��������ַ
Return:	void
Others:
DisplayString(pcDsSource,stDisStringArraypcDsSource,stDisStringArray,0,0xff,0,pcDsValueOut);
*************************************************/

void process_normal_ds_calculate( int iDsId, const byte* pcDsSource, int iValidByteNum, byte* pcDsValueOut )
{
	switch( iDsId )
	{
	case   0:
		OneByteOperation( pcDsSource, 0, 0, 1, 10, "%4.1f", pcDsValueOut ); //x/10
		break;
	default:
		break;
	}
}
/*************************************************
Description:	�汾��Ϣ������
Input:
iDsId
pcDsSource	ȡֵ��ַ

Output:	pcDsValueOut	��������ַ
Return:	void
Others:
DisplayString(pcDsSource,stDisStringArraypcDsSource,stDisStringArray,0,0xff,0,pcDsValueOut);
*************************************************/
void process_normal_infor_calculate( int iDsId, const byte* pcDsSource, const int iByteLen, byte* pcDsValueOut )
{
	switch( iDsId )
	{
	case   0:
		OneByteOperation( pcDsSource, 0, 0, 1, 10, "%4.1f", pcDsValueOut ); //x/10
		break;
	default:
		break;
	}
}
/*************************************************
Description:	���ݶ���֡������ID�������������㹫ʽ
Input:
	iDsId		��������ID
	pcDsSource	ȡֵ��ַ

Output:	pcDsValueOut	��������ַ
Return:	void
Others:
*************************************************/

void process_freeze_ds_calculate( int iDsId, const byte* pcDsSource, byte* pcDsValueOut )
{
	switch( iDsId )
	{
	case   0:
// 		DisplayString( pcDsSource, stDisStringArray, 1, 0X01, 0, pcDsValueOut ); //ACC.0=1~Present   ACC.0=0~Not Present
		break;
	default:
		break;
	}
}

/************************************************************************/
/* �����ǰ�ȫ������㹫ʽ                                               */
/************************************************************************/

/*************************************************
  Description:	���㰲ȫ�㷨
  Calls:	seedToKey;
  Called By:	process_security_access_algorithm;

  Input:	Group	����ECU�ظ�seed���׵�ַ

  Output:	Group	��������ַ
  Return:	byte	keyռ���ֽ���
  Others:	�����������ϵͳ����
*************************************************/
byte security_calculate( byte *Group )
{
	//�˴�д��ȫ�㷨��ʽ


	return 2;//Key��Ҫ�����ֽ�
}
/*************************************************
Description:	���ݰ�ȫ�ȼ�����ȫ�㷨
Input:	cAccessLevel	��ȫ�ȼ�

Output:	pOut	��������ַ
Return:	bool	�㷨ִ��״̬���ɹ���ʧ�ܣ�
Others:	��������ʵ�ֻ���ϵͳ����
*************************************************/

bool process_security_access_algorithm( byte cAccessLevel, void* pOut )
{
	bool bProcessSingleCmdStatus = false;
	byte cBufferOffset = 0;//����ƫ��
	byte cRequestSeedCmdOffset = 0;
	byte cSendKeyCmdOffset = 0;

	byte cDataArray[10] = {0};
	byte cNeedBytes = 0;

	int i = 0;

	//���ݰ�ȫ�ȼ�ȷ������ƫ��
	switch( cAccessLevel )
	{
		case 0:
			cRequestSeedCmdOffset	= g_iRequestSeedCmdIndex[1];
			cSendKeyCmdOffset		= g_iSendKeyCmdIndex[1];
			break;

		default:
			return false;
	}

	cBufferOffset = g_stInitXmlGobalVariable.m_p_stCmdList[ cRequestSeedCmdOffset ].cBufferOffset;
	bProcessSingleCmdStatus = process_single_cmd_without_subsequent_processing( cRequestSeedCmdOffset, pOut );
	if( !bProcessSingleCmdStatus )
	{
		return false;
	}


	memcpy( cDataArray, ( g_stBufferGroup[cBufferOffset].cBuffer + 2 ), 4 );

	//�ж�seed�Ƿ�ȫΪ0����ȫΪ0�򲻷���key
	for ( i = 0; i < ( g_stBufferGroup[cBufferOffset].iValidLen - 2) ; i++ )
	{
		if ( cDataArray[i] != 0x00 )
		{
			break;
		}
		if ( i == ( g_stBufferGroup[cBufferOffset].iValidLen - 3))
		{
			return true;
		}
	}
	//���ݰ�ȫ�ȼ�ȷ�����㹫ʽ
	switch( cAccessLevel )
	{
	case 0://��ȫ�ȼ�Ϊ0
	{

		cNeedBytes = security_calculate( cDataArray );
	}
	break;

	default:
		break;
	}

	memcpy( ( g_stInitXmlGobalVariable.m_p_stCmdList[cSendKeyCmdOffset].pcCmd + 5), cDataArray, cNeedBytes );

	bProcessSingleCmdStatus = process_single_cmd_without_subsequent_processing( cSendKeyCmdOffset, pOut );

	if( !bProcessSingleCmdStatus )
	{
		return false;
	}

	return true;

}