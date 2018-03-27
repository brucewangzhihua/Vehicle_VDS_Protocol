#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

#include "Parser.h"
#include "interface.h"

typedef enum {false = 0, true} bool;

int calculate( const char* pcDataSource, const int nDataSourceLen,
               const char* pcFormula, const int nFormulaLen,
               char *pStrFormat, char* pOutStringData )
{
	char szBuffer[MAX_SIZE] = {'\0'};
	char szFormula[MAX_SIZE] = {'\0'};
	//char szSwitch[MAX_SIZE] = {'\0'};
	int nFormulaRealLen = 0;
	int nSwitchLen = 0;
	bool PiecewizeFlag = false;
	
	double dResult = 0;
	OPERATE operate[MAX_SIZE];
	int nResult = 0;
	int i = 0;
	int n = 0;
	int nType = 0;

	//��ʼ��������
	memset( szBuffer, '\0', MAX_SIZE );
	memset( operate, '\0', sizeof( OPERATE ) * MAX_SIZE );

	//�ж�����Դ�Ƿ�Ϊ��
	if( NULL == pcDataSource )
	{
		return -1;//��ʾ����ԴΪ��
	}

	//�ж�����ַ���ָ���Ƿ�Ϊ��
	if( NULL == pOutStringData )
	{
		return -2;//��ʾ����ַ���ָ��Ϊ��
	}

	//�жϸ�ʽ���ַ����Ƿ�Ϊ��
	if( NULL == pStrFormat )
	{
		return -3;//�жϸ�ʽ���ַ����Ƿ�Ϊ��
	}

	nType = GetFormatType( pStrFormat );

	if( nType == 0 )
	{
		//ASCII��ʾ
		for( i = 0; i < nDataSourceLen; i++ )
		{
			sprintf( ( pOutStringData + i ), "%c", *( pcDataSource + i ) );
		}

		return 1;
	}
	else if( nType == 1 )
	{
		//Hex��ʾ
		for( i = 0; i < nDataSourceLen; i++ )
		{
			sprintf( pOutStringData, "%02X", ( unsigned char )( *( pcDataSource + i ) ) );
			pOutStringData += 2;
		}

		return 1;
	}
	else if( nType == 2 )
	{
		//Decimalʮ������ʾ
		for( i = 0; i < nDataSourceLen; i++ )
		{
			n = sprintf( pOutStringData, "%d", *( pcDataSource + i ) );
			pOutStringData += n;
		}

		return 1;
	}
	else if( 0 != nFormulaLen )//�жϹ�ʽ�Ƿ�Ϊ��
	{
		char* szSwitch = malloc( sizeof( char ) * nFormulaLen );
		memset( szSwitch, '\0', sizeof( char ) * nFormulaLen );

		//��ֹ�ʽ��Switch
		ParseFormulaAndSwitch( pcFormula, nFormulaLen, szFormula, szSwitch );

		nFormulaRealLen = ( int )strlen( szFormula );
		nSwitchLen = ( int )strlen( szSwitch );

		if( nFormulaRealLen > 0 )
		{
			//�û���ʽ�е�ռλ��A-Z
			PutDataToFormula( pcDataSource, nDataSourceLen, szFormula, nFormulaRealLen, szBuffer );

			//����ʽ�еĸ���
			HandleMinus( szBuffer );

			//�õ�����ĸ�滻��ѧ���ʽ
			FormulaProc( szBuffer );

			//��������������ȼ���ʽ����ʽ
			FormatFormula( szBuffer, operate );

			//������ֵ
			dResult = CalculateValue( operate, 3.14159266 );
			if (nSwitchLen > 0)
			{
				//������ֵ�ҵ���Ӧ���ַ���ID
				nResult = ( unsigned int )dResult;
				SwitchFunction( nResult, szSwitch, nSwitchLen, pOutStringData );
				if ( *pOutStringData =='%' )
				{
					memset( pStrFormat, '\0', strlen( pStrFormat ) );
					for ( i = 0; i < (int)strlen(pOutStringData); i++ )
					{
						if ( i != 0 && *(pOutStringData + i) =='%' )
						{
							i++;
							break;
						}
						*(pStrFormat + i) = *(pOutStringData + i);
					}
					nType = GetFormatType( pStrFormat );
					nFormulaRealLen = (int)strlen(pOutStringData + i);
					//�û���ʽ�е�ռλ��A-Z
					PutDataToFormula( pcDataSource, nDataSourceLen, pOutStringData + i, nFormulaRealLen, szBuffer );

					//����ʽ�еĸ���
					HandleMinus( szBuffer );

					//�õ�����ĸ�滻��ѧ���ʽ
					FormulaProc( szBuffer );

					//��������������ȼ���ʽ����ʽ
					FormatFormula( szBuffer, operate );

					//������ֵ
					dResult = CalculateValue( operate, 3.14159266 );
				}
			}

			if( nType == 3 )
			{
				//�������ݵĸ�ʽ��
				nResult = ( int )dResult;
				sprintf( pOutStringData, pStrFormat, nResult );
			}
			else if( nType == 4 )
			{
				//�����͵����ݸ�ʽ��
				sprintf( pOutStringData, pStrFormat, dResult );
			}
			else if (nType == 6)
			{
				//�������ݸ�ʽ�����޷����������
				nResult = (unsigned int)dResult;
				sprintf(pOutStringData, pStrFormat, nResult );
			}
		}
		free( szSwitch );
	}
	return 1;
}