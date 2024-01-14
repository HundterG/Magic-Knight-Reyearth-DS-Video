#include <nds.h>
#include <filesystem.h>
#include <maxmod9.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define QOI16_IMPLEMENTATION
#define QOI16_NO_ENCODE
#include "qoi16.h"

FILE *audioFilePtr = NULL;
FILE *videoFilePtr = NULL;

uint8_t videoDataBuffer[150 * 1024];
uint16_t framedata[(192*2)*256];

mm_word on_stream_request( mm_word length, mm_addr dest, mm_stream_formats format )
{
	s16 *target = dest;
	mm_word r = fread(target, sizeof(s16), length, audioFilePtr);
	if(r <= 0)
		dmaFillHalfWords(0, target, length*2);
	return r;
}

int main(int argc, char **argv)
{
	if (!nitroFSInit(NULL))
	{
		consoleDemoInit();
    	iprintf("nitroFSInit failure: terminating\n");
		while(1)
		{
			swiWaitForVBlank();
			scanKeys();
			if(keysDown()&KEY_START) return 0;
		}
	}

	chdir("nitro:/");

	audioFilePtr = fopen("/Audio.pcm","rb");
	if(!audioFilePtr) audioFilePtr = fopen("Audio.pcm","rb");
	if(!audioFilePtr)
	{
		consoleDemoInit();
		iprintf("audio file not open: terminating\n");
		while(1)
		{
			swiWaitForVBlank();
			scanKeys();
			if(keysDown()&KEY_START) return 0;
		}
	}

	videoFilePtr = fopen("/Video.qoia","rb");
	if(!videoFilePtr) videoFilePtr = fopen("Video.qoia","rb");
	if(!videoFilePtr)
	{
		consoleDemoInit();
		iprintf("video file not open: terminating\n");
		while(1)
		{
			swiWaitForVBlank();
			scanKeys();
			if(keysDown()&KEY_START) return 0;
		}
	}

	u64 audioTime = 0;
	u64 fileIOTime = 0;
	u64 qoiTime = 0;
	u64 idleTime = 0;
	u32 droppedFrames = 0;
	u32 streamVideoTime = 0;

	videoSetMode(MODE_FB0);
	videoSetModeSub(MODE_5_2D);

#if 0
	oamInit(&oamSub, SpriteMapping_Bmp_2D_256, false);
 
	int x = 0;
	int y = 0;
 
	int id = 0;

	//set up a 4x3 grid of 64x64 sprites to cover the screen
	for(y = 0; y < 3; y++)
	for(x = 0; x < 4; x++)
	{
	/*	u16 *offset = &SPRITE_GFX_SUB[(x * 64) + (y * 64 * 256)];
 
		oamSet(&oamSub, x + y * 4, x * 64, y * 64, 0, 15, SpriteSize_64x64, 
			SpriteColorFormat_Bmp, offset, -1, false,false,false,false,false);
	*/
		oamSub.oamMemory[id].attribute[0] = ATTR0_BMP | ATTR0_SQUARE | (64 * y);
		oamSub.oamMemory[id].attribute[1] = ATTR1_SIZE_64 | (64 * x);
		oamSub.oamMemory[id].attribute[2] = ATTR2_ALPHA(1) | (8 * 32 * y) | (8 * x);
		id++;
	}
 
	swiWaitForVBlank();
 
	oamUpdate(&oamSub);
#endif

	vramSetBankA(VRAM_A_LCD);
	vramSetBankC(VRAM_C_SUB_BG);
int subbg = bgInitSub(2, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
//videoBgEnableSub(3);
//vramSetBankD(VRAM_D_SUB_SPRITE);
//REG_DISPCAPCNT = DCAP_BANK(3) | DCAP_ENABLE | DCAP_SIZE(3);

	// show and process first frame
	{
		cpuStartTiming(1);
		int size = 0;
		int r = fread(&size, sizeof(int), 1, videoFilePtr);
		if(r != 1 || 150*1024 < size)
		{
			consoleDemoInit();
			iprintf("error loading first frame:\nDid not get size\nterminating\n");
			while(1)
			{
				swiWaitForVBlank();
				scanKeys();
				if(keysDown()&KEY_START) return 0;
			}
		}
		int readLen = 0;
		while(readLen < size)
		{
			int r = fread(videoDataBuffer, sizeof(s8), size - readLen, videoFilePtr);
			if(r <= 0)
			{
				consoleDemoInit();
				iprintf("error loading first frame:\nDid not get full file\nterminating\n");
				while(1)
				{
					swiWaitForVBlank();
					scanKeys();
					if(keysDown()&KEY_START) return 0;
				}
			}
			readLen += r;
		}
		fileIOTime += cpuEndTiming();

		cpuStartTiming(1);
		qoi16_decode(videoDataBuffer, size, framedata);
		dmaCopyHalfWordsAsynch(0, framedata, VRAM_A, 192*256*2);
		dmaCopyHalfWordsAsynch(1, framedata+(192*256), /*bgGetGfxPtr(subbg)*/ BG_GFX_SUB, 192*256*2);
		qoiTime += cpuEndTiming();
	}

	{
		cpuStartTiming(1);
		mm_ds_system sys;
		sys.mod_count 			= 0;
		sys.samp_count			= 0;
		sys.mem_bank			= 0;
		sys.fifo_channel		= FIFO_MAXMOD;
		mmInit( &sys );
	
		mm_stream mystream;
		mystream.sampling_rate	= 44100;					// sampling rate = 25khz
		mystream.buffer_length	= 10000;						// buffer length = 1200 samples
		mystream.callback		= on_stream_request;		// set callback function
		mystream.format			= MM_STREAM_16BIT_MONO;	// format = stereo 16-bit
		mystream.timer			= MM_TIMER0;				// use hardware timer 0
		mystream.manual			= true;						// use manual filling
		mmStreamOpen( &mystream );
		audioTime += cpuEndTiming();
	}

	int readError = 0;

	while(1)
	{
		// update audio
		cpuStartTiming(1);
		mmStreamUpdate();
		audioTime += cpuEndTiming();

		if(streamVideoTime + 3675 < mmStreamGetPosition() * 2)
		{
			streamVideoTime += 3675;
			while(streamVideoTime + 3675 < mmStreamGetPosition() * 2)
			{
				streamVideoTime += 3675;
				++droppedFrames;
				int size;
				int r = fread(&size, sizeof(int), 1, videoFilePtr);
				if(r != 1)
				{
					break;
				}
				if(150*1024 < size)
				{
					readError = 1;
					break;
				}
				int currPos = ftell(videoFilePtr);
				fseek(videoFilePtr, currPos+size, SEEK_SET);
			}

			if(readError == 1) break;

			int size;
			cpuStartTiming(1);
			{
				int r = fread(&size, sizeof(int), 1, videoFilePtr);
				if(r != 1)
				{
					break;
				}
				if(150*1024 < size)
				{
					readError = 1;
					break;
				}
				int readLen = 0;
				while(readLen < size)
				{
					int r = fread(videoDataBuffer, sizeof(s8), size - readLen, videoFilePtr);
					if(r <= 0)
					{
						readError = 1;
						break;
					}
					readLen += r;
				}
			}
			fileIOTime += cpuEndTiming();

			if(readError == 1) break;

			cpuStartTiming(1);
			qoi16_decode(videoDataBuffer, size, framedata);
			dmaCopyHalfWordsAsynch(0, framedata, VRAM_A, 192*256*2);
			dmaCopyHalfWordsAsynch(1, framedata+(192*256), /*bgGetGfxPtr(subbg)*/ BG_GFX_SUB, 192*256*2);
			qoiTime += cpuEndTiming();
		}

		cpuStartTiming(1);
		swiWaitForVBlank();
		idleTime += cpuEndTiming();
	}
	mmStreamClose();

	for(int i=0 ; i<60 ; ++i)
	{
		swiWaitForVBlank();
	}

	{
		consoleDemoInit();
		if(readError == 1)
			iprintf("There was an error\n");
		else
			iprintf("Finished\n");
		double totalTick = audioTime + fileIOTime + qoiTime + idleTime;
		iprintf("Perf Stuff\n");
		iprintf("\tAudio: %f\n", (float)(audioTime/totalTick));
		iprintf("\tVideo IO: %f\n", (float)(fileIOTime/totalTick));
		iprintf("\tQOI Decode: %f\n", (float)(qoiTime/totalTick));
		iprintf("\tIdle: %f\n", (float)(idleTime/totalTick));
		iprintf("\nDropped Frames %i\n", (int)(droppedFrames));
	}
	while(1)
	{
		swiWaitForVBlank();
		scanKeys();
		if(keysDown()&KEY_START) return 0;
	}
	return 0;
}
