#include <nds.h>
#include <filesystem.h>
#include <maxmod9.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <math.h>
#include <inttypes.h>

FILE *audioFilePtr = NULL;
FILE *videoFilePtr = NULL;

uint8_t videoDataBuffer[18944];
u16* spireteVRAM[24];

mm_word on_stream_request( mm_word length, mm_addr dest, mm_stream_formats format )
{
	s16 *target = dest;
	//if(feof(audioFilePtr) != 0 || ferror(audioFilePtr) != 0)
	{
//iprintf("Here 1\n");
		dmaFillHalfWords(0, target, length*2);
		return length;
	}
//	mm_word r = fread(target, sizeof(s16), length, audioFilePtr);
//	if(r <= 0)
//	{
//iprintf("Here 2\n");
//		dmaFillHalfWords(0, target, length*2);
//		return length;
//	}
//	return r;
}

void DecodeMegaTile(uint8_t *data, u16 *out)
{
	union
	{
		uint32_t v;
		struct
		{
			uint8_t i[4];
		};
	} colors;
	uint32_t indexes;

	for(int i=0 ; i<8*8*2 ; ++i)
	{
		colors.v = *(uint32_t*)(data);
		data += 4;
		indexes = *(uint32_t*)(data);
		data += 4;
		u16 outTemp = 0;

		outTemp = colors.i[indexes & 0x03]; indexes >>= 2;
		outTemp |= (u16)(colors.i[indexes & 0x03]) << 8; indexes >>= 2;
		*out = outTemp; ++out;
		outTemp = colors.i[indexes & 0x03]; indexes >>= 2;
		outTemp |= (u16)(colors.i[indexes & 0x03]) << 8; indexes >>= 2;
		*out = outTemp; ++out;

		outTemp = colors.i[indexes & 0x03]; indexes >>= 2;
		outTemp |= (u16)(colors.i[indexes & 0x03]) << 8; indexes >>= 2;
		*out = outTemp; ++out;
		outTemp = colors.i[indexes & 0x03]; indexes >>= 2;
		outTemp |= (u16)(colors.i[indexes & 0x03]) << 8; indexes >>= 2;
		*out = outTemp; ++out;

		outTemp = colors.i[indexes & 0x03]; indexes >>= 2;
		outTemp |= (u16)(colors.i[indexes & 0x03]) << 8; indexes >>= 2;
		*out = outTemp; ++out;
		outTemp = colors.i[indexes & 0x03]; indexes >>= 2;
		outTemp |= (u16)(colors.i[indexes & 0x03]) << 8; indexes >>= 2;
		*out = outTemp; ++out;

		outTemp = colors.i[indexes & 0x03]; indexes >>= 2;
		outTemp |= (u16)(colors.i[indexes & 0x03]) << 8; indexes >>= 2;
		*out = outTemp; ++out;
		outTemp = colors.i[indexes & 0x03]; indexes >>= 2;
		outTemp |= (u16)(colors.i[indexes & 0x03]) << 8; indexes >>= 2;
		*out = outTemp; ++out;

		indexes = *(uint32_t*)(data);
		data += 4;

		outTemp = colors.i[indexes & 0x03]; indexes >>= 2;
		outTemp |= (u16)(colors.i[indexes & 0x03]) << 8; indexes >>= 2;
		*out = outTemp; ++out;
		outTemp = colors.i[indexes & 0x03]; indexes >>= 2;
		outTemp |= (u16)(colors.i[indexes & 0x03]) << 8; indexes >>= 2;
		*out = outTemp; ++out;

		outTemp = colors.i[indexes & 0x03]; indexes >>= 2;
		outTemp |= (u16)(colors.i[indexes & 0x03]) << 8; indexes >>= 2;
		*out = outTemp; ++out;
		outTemp = colors.i[indexes & 0x03]; indexes >>= 2;
		outTemp |= (u16)(colors.i[indexes & 0x03]) << 8; indexes >>= 2;
		*out = outTemp; ++out;

		outTemp = colors.i[indexes & 0x03]; indexes >>= 2;
		outTemp |= (u16)(colors.i[indexes & 0x03]) << 8; indexes >>= 2;
		*out = outTemp; ++out;
		outTemp = colors.i[indexes & 0x03]; indexes >>= 2;
		outTemp |= (u16)(colors.i[indexes & 0x03]) << 8; indexes >>= 2;
		*out = outTemp; ++out;

		outTemp = colors.i[indexes & 0x03]; indexes >>= 2;
		outTemp |= (u16)(colors.i[indexes & 0x03]) << 8; indexes >>= 2;
		*out = outTemp; ++out;
		outTemp = colors.i[indexes & 0x03]; indexes >>= 2;
		outTemp |= (u16)(colors.i[indexes & 0x03]) << 8; indexes >>= 2;
		*out = outTemp; ++out;
	}
}

void ProccessFrame(u16 **dest, u16 *paletteDest)
{
	dmaCopyHalfWordsAsynch(1, videoDataBuffer, paletteDest, 256*sizeof(u16));
	u8 *dataStart = videoDataBuffer + 256*sizeof(u16);

	DecodeMegaTile(dataStart, *dest);
	dataStart += 16*8*12;
	++dest;

	DecodeMegaTile(dataStart, *dest);
	dataStart += 16*8*12;
	++dest;

	DecodeMegaTile(dataStart, *dest);
	dataStart += 16*8*12;
	++dest;

	DecodeMegaTile(dataStart, *dest);
	dataStart += 16*8*12;
	++dest;

	DecodeMegaTile(dataStart, *dest);
	dataStart += 16*8*12;
	++dest;

	DecodeMegaTile(dataStart, *dest);
	dataStart += 16*8*12;
	++dest;

	DecodeMegaTile(dataStart, *dest);
	dataStart += 16*8*12;
	++dest;

	DecodeMegaTile(dataStart, *dest);
	dataStart += 16*8*12;
	++dest;

	DecodeMegaTile(dataStart, *dest);
	dataStart += 16*8*12;
	++dest;

	DecodeMegaTile(dataStart, *dest);
	dataStart += 16*8*12;
	++dest;

	DecodeMegaTile(dataStart, *dest);
	dataStart += 16*8*12;
	++dest;

	DecodeMegaTile(dataStart, *dest);
	dataStart += 16*8*12;
	++dest;
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

	audioFilePtr = fopen("/Audio2.pcm","rb");
	if(!audioFilePtr) audioFilePtr = fopen("Audio2.pcm","rb");
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

	videoFilePtr = fopen("/Video2.dxta","rb");
	if(!videoFilePtr) videoFilePtr = fopen("Video2.dxta","rb");
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
	u32 frameCount = 1;

	videoSetMode(MODE_0_2D);
	videoSetModeSub(MODE_0_2D);
	oamInit(&oamMain, SpriteMapping_Bmp_1D_128, false);
	oamInit(&oamSub, SpriteMapping_Bmp_1D_128, false);
	vramSetBankA(VRAM_A_MAIN_SPRITE);
	vramSetBankD(VRAM_D_SUB_SPRITE);

	for(int i = 0; i < 12; i++)
		spireteVRAM[i] = oamAllocateGfx(&oamMain, SpriteSize_64x64, SpriteColorFormat_256Color);
	for(int i = 12; i < 24; i++)
		spireteVRAM[i] = oamAllocateGfx(&oamSub, SpriteSize_64x64, SpriteColorFormat_256Color);

	for(int i = 0; i < 12; i++)
	{
		oamSet(
			&oamMain, //sub display 
			i,       //oam entry to set
			(i*64)%256, ((i*64)/256)*64, //position 
			0, //priority
			0, //palette for 16 color sprite or alpha for bmp sprite
			SpriteSize_64x64, 
			SpriteColorFormat_256Color, 
			spireteVRAM[i], 
			0, 
			false, //double the size of rotated sprites
			false, //don't hide the sprite
			false, false, //vflip, hflip
			false //apply mosaic
		);
	}
	for(int i = 12; i < 24; i++)
	{
		oamSet(
			&oamSub, //sub display 
			i,       //oam entry to set
			((i-12)*64)%256, (((i-12)*64)/256)*64, //position 
			0, //priority
			0, //palette for 16 color sprite or alpha for bmp sprite
			SpriteSize_64x64, 
			SpriteColorFormat_256Color, 
			spireteVRAM[i], 
			0, 
			false, //double the size of rotated sprites
			false, //don't hide the sprite
			false, false, //vflip, hflip
			false //apply mosaic
		);
	}

	oamUpdate(&oamMain);
	oamUpdate(&oamSub);


	// show and process first frame
	{
		cpuStartTiming(1);
		int readLen = 0;
		while(readLen < 18944)
		{
			int r = fread(videoDataBuffer, sizeof(s8), 18944 - readLen, videoFilePtr);
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
		ProccessFrame(spireteVRAM, SPRITE_PALETTE);
		qoiTime += cpuEndTiming();

		cpuStartTiming(1);
		readLen = 0;
		while(readLen < 18944)
		{
			int r = fread(videoDataBuffer, sizeof(s8), 18944 - readLen, videoFilePtr);
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
		ProccessFrame(spireteVRAM + 12, SPRITE_PALETTE_SUB);
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
		mystream.sampling_rate	= 32000;					// sampling rate = 25khz
		mystream.buffer_length	= 10 * 1024;						// buffer length = 1200 samples
		mystream.callback		= on_stream_request;		// set callback function
		mystream.format			= MM_STREAM_16BIT_MONO;	// format = stereo 16-bit
		mystream.timer			= MM_TIMER0;				// use hardware timer 0
		mystream.manual			= true;						// use manual filling
		mmStreamOpen( &mystream );
		audioTime += cpuEndTiming();
	}

	int done = 0;

	while(frameCount < 2211)
	{
		// update audio
		cpuStartTiming(1);
		mmStreamUpdate();
		audioTime += cpuEndTiming();

		if(streamVideoTime + 4000 < mmStreamGetPosition() * 3)
		{
			streamVideoTime += 4000;
			while(streamVideoTime + 4000 < mmStreamGetPosition() * 3)
			{
				streamVideoTime += 4000;
				++droppedFrames;
				++frameCount;
				int currPos = ftell(videoFilePtr);
				fseek(videoFilePtr, currPos+(18944*2), SEEK_SET);
			}

			if(2211 <= frameCount)
				break;

			cpuStartTiming(1);
			int readLen = 0;
			while(readLen < 18944)
			{
				if(feof(videoFilePtr) != 0 || ferror(videoFilePtr) != 0)
				{
					done = 1;
					break;
				}
				int r = fread(videoDataBuffer, sizeof(s8), 18944 - readLen, videoFilePtr);
				if(r <= 0)
				{
					done = 1;
					break;
				}
				readLen += r;
			}
			fileIOTime += cpuEndTiming();

			if(done == 1) break;

			cpuStartTiming(1);
			ProccessFrame(spireteVRAM, SPRITE_PALETTE);
			qoiTime += cpuEndTiming();

			cpuStartTiming(1);
			readLen = 0;
			while(readLen < 18944)
			{
				if(feof(videoFilePtr) != 0 || ferror(videoFilePtr) != 0)
				{
					done = 1;
					break;
				}
				int r = fread(videoDataBuffer, sizeof(s8), 18944 - readLen, videoFilePtr);
				if(r <= 0)
				{
					done = 1;
					break;
				}
				readLen += r;
			}
			fileIOTime += cpuEndTiming();

			if(done == 1) break;

			cpuStartTiming(1);
			ProccessFrame(spireteVRAM + 12, SPRITE_PALETTE_SUB);
			qoiTime += cpuEndTiming();
			++frameCount;
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
		iprintf("Finished\n");
		u64 totalTick = audioTime + fileIOTime + qoiTime + idleTime;
		iprintf("Perf Stuff\n");
		iprintf("\tAudio: %"PRIu64"\n", audioTime);
		iprintf("\tVideo IO: %"PRIu64"\n", fileIOTime);
		iprintf("\tDecode: %"PRIu64"\n", qoiTime);
		iprintf("\tIdle: %"PRIu64"\n", idleTime);
		iprintf("\tTotal: %"PRIu64"\n", totalTick);
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
