/*-----------------------------------------------------------------------
|                            FILE DESCRIPTION                           |
-----------------------------------------------------------------------*/
/*----------------------------------------------------------------------
  - File name     : STC8x_RST.c
  - Author        : zeweni
  - Update date   : 2020.02.06
  -	Copyright(C)  : 2020-2021 zeweni. All rights reserved.
-----------------------------------------------------------------------*/
/*------------------------------------------------------------------------
|                            COPYRIGHT NOTICE                            |
------------------------------------------------------------------------*/
/*
 * Copyright (C) 2021, zeweni (17870070675@163.com)

 * This file is part of 8051 ELL low-layer libraries.

 * 8051 ELL low-layer libraries is free software: you can redistribute 
 * it and/or modify it under the terms of the Apache-2.0 License.

 * 8051 ELL low-layer libraries is distributed in the hope that it will 
 * be useful,but WITHOUT ANY WARRANTY; without even the implied warranty 
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 * Apache-2.0 License for more details.

 * You should have received a copy of the Apache-2.0 License.8051 ELL 
 * low-layer libraries. If not, see <http://www.apache.org/licenses/>.
**/
/*-----------------------------------------------------------------------
|                               INCLUDES                                |
-----------------------------------------------------------------------*/
#include "STC8x_RST.h"
/*-----------------------------------------------------------------------
|                                 DATA                                  |
-----------------------------------------------------------------------*/
/* None */
/*-----------------------------------------------------------------------
|                               FUNCTION                                |
-----------------------------------------------------------------------*/
#if (PER_LIB_RST_CTRL == 1)
	
	#if (PER_LIB_RST_INIT_CTRL == 1)

		/**
		 * @brief       ????????????????????????????????????????????????????????????
		 * @details     Low voltage reset function, 
		 *              you can select different low voltage reset standards.
		 * @param[in]   lvDiv   ?????????????????????low voltage reset standards.
		 * @param[in]   run     ?????????????????? Enable control bit.
		 * @return      FSC_SUCCESS ???????????????Return to success.
		 * @return      FSC_FAIL    ???????????????Return to fail.
		**/
		FSCSTATE LVD_RST_Init(LVDRstDiv_Type lvDiv, BOOL run)
		{
			RSTCFG = (RSTCFG & 0xFC) | lvDiv;
			RSTCFG = (RSTCFG & 0xEF) | (run << 6);
			return FSC_SUCCESS;
		}

	#endif
	#if (PER_LIB_RST_WORK_CTRL == 1)
		/**
		 * @brief        ???????????????????????????
		 * @details     Software reset function.
		 * @param[in]   choice   ?????????????????????Reset mode selection.
		 * @param[in]   run      ?????????????????? Enable control bit.
		 * @return      FSC_SUCCESS ???????????????Return to success.
		 * @return      FSC_FAIL    ???????????????Return to fail.
		**/
		FSCSTATE SFW_RST_Ctrl(RSTChoice_Type choice, BOOL run)
		{
			IAP_CONTR = (IAP_CONTR & 0xBF) | (choice << 6);
			IAP_CONTR = (IAP_CONTR & 0xDF) | (run << 5);
			return FSC_SUCCESS;
		}


		/**
		 * @brief    ??????????????????????????????????????????????????????????????????
		 *           ?????????STC-ISP??????????????????????????????10???0x7F???
		 * @details  Software reset automatic download function, 
		 *			 need to be called in serial interrupt,
		 *			 The download password needs to be 
		 *			 set in the STC-ISP assistant: 10 0x7F.
		 * @param    None.
		 * @return   None.
		**/
		void Auto_RST_download(void)
		{
		  static uint8_t semCont = 0;
		  if(SBUF == 0x7F || SBUF == 0x80) 
		  {
			if(++semCont >= 10)
			{
			  semCont = 0;
			  IAP_CONTR = 0x60;
			}
		  }
		  else
		  {
			semCont = 0;
		  }
		}
		
	#endif
		
#endif
/*-----------------------------------------------------------------------
|                   END OF FLIE.  (C) COPYRIGHT zeweni                  |
-----------------------------------------------------------------------*/



