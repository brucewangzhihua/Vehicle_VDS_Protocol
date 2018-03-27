/**********************************************************************
Copyright:		YiXiong AUTO S&T Co., Ltd.
Description:	�������⹦�ܺ�����������غ����������
History:
<author>	<time>		<desc>

************************************************************************/
#ifndef _SCAN_ECU_VERSION_FUNCTION
#define _SCAN_ECU_VERSION_FUNCTION

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "..\InitConfigFromXml\init_config_from_xml_lib.h"
#include "..\SpecialFunction\special_function.h"
#include "active_ecu_lib.h"
#include "idle_link_lib.h"


/********************************ͨ�ú�������**************************************/
void Read_ECU_Version( STRUCT_CHAIN_DATA_INPUT* pstParam, void* pOut );
void Check_ECU_Version(STRUCT_CHAIN_DATA_INPUT* pstParam, void* pOut);
void process_ECU_version_function( void* pIn, void* pOut );

#endif