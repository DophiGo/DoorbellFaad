#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <android/log.h>
#include "faad.h"

#define LOG_TAG "door_bell_decode_aac"
#define LOG_LEVEL 8
#define LOGI(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__);}
#define LOGE(level, ...) if (level <= LOG_LEVEL) {__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__);}

int isInit = 0;

int aac2pcm( void *sample_buffer,unsigned int samples,char *data)
{
	int ret;
	unsigned int i;
	short *sample_buffer16 = (short*)sample_buffer;

	for (i = 0; i < samples; i++)
	{
		data[i*2] = (char)(sample_buffer16[i] & 0xFF);
		data[i*2+1] = (char)((sample_buffer16[i] >> 8) & 0xFF);
	}

	return ret;
}

NeAACDecHandle aac_decode_init()
{
	NeAACDecHandle hDecoder = NeAACDecOpen();
	
	NeAACDecConfigurationPtr pDecodeConfig = NeAACDecGetCurrentConfiguration(hDecoder);
	
	pDecodeConfig->defSampleRate	= 8000;
	pDecodeConfig->defObjectType	= LC;
	pDecodeConfig->outputFormat 	= FAAD_FMT_16BIT;
	pDecodeConfig->downMatrix		= 0;
	pDecodeConfig->useOldADTSFormat = 0;

	NeAACDecSetConfiguration(hDecoder, pDecodeConfig);
	isInit = 0;

	LOGE(9, "aac_decode_init.");

    return hDecoder;
}

int aac_decode(NeAACDecHandle hDecoder, char *pInputBuf, uint32_t dwInputSize, short* pOutputBuf, uint32_t* dwOutputSize)
{
	if (!isInit)
	{
		long lRealUse =0;

		unsigned long lRealSampleRate	;
		unsigned char ucRealChans		;

		if ((lRealUse = NeAACDecInit(hDecoder, pInputBuf,
				dwInputSize, &lRealSampleRate, &ucRealChans)) < 0)
		{
			/* If some error initializing occured, skip the file */
			LOGE(9, "Error initializing decoder library.\n");
			NeAACDecClose(hDecoder);
			return 1;
		}

		dwInputSize -= lRealUse;
	    isInit = 1;
	}

	NeAACDecFrameInfo frameInfo;

	do
	{
		void* pcm_data = NeAACDecDecode(hDecoder, &frameInfo, pInputBuf, dwInputSize);
		dwInputSize -= frameInfo.bytesconsumed;
	    aac2pcm(pcm_data, frameInfo.samples, pOutputBuf);
		*dwOutputSize = frameInfo.samples*16*sizeof(char)/8;
		LOGE(1, "aac_decode %d, %d", dwInputSize, *dwOutputSize);
	} while (dwInputSize > 0);

	getchar();
	return 0;
}

