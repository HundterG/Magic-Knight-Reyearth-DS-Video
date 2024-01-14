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
FILE *palletFilePtr = NULL;

uint8_t videoDataBuffer[18432];
u16* spireteVRAM[24];
u32 palletThreshHold[27] = {86, 133, 505, 617, 650, 696, 715, 776, 852, 897, 931, 953, 997, 1041, 1090, 1116, 1294, 1500, 1626, 1680, 1713, 1769, 1837, 1956, 1989, 2149, 9999};
u16 palletData[26][2][255];
u16 tempPallet[255];

//s16 audioBuffer[8][4 * 1024];
//int audioEmpty[8] = {0, 0, 0, 0, 0, 0, 0, 0};
//int audioFrame = 0;
int audioFrameProgress = 0;

s8 audioFileData[2030159];

void AudioCopy(s16* dest, s16* begin, s16* end)
{
	while(begin<end)
{
*dest = *begin;
++dest;
++begin;
}
}

mm_word on_stream_request( mm_word length, mm_addr dest, mm_stream_formats format )
{
#if 0
	s16 *target = dest;
	s16 *endDst = target + length;
	while(target < endDst)
	{
		int end = (audioFrameProgress+length < 4*1024) ? (audioFrameProgress+length) : (4*1024);
		//dmaCopyHalfWords(0, audioBuffer+audioFrameProgress, target+filled, (end-audioFrameProgress)*sizeof(s16));
		//memcpy(target+filled, audioBuffer[audioFrame]+audioFrameProgress, (end-audioFrameProgress)*sizeof(s16));
		//AudioCopy(target+filled, audioBuffer[audioFrame]+audioFrameProgress, audioBuffer[audioFrame]+end);
		s16 *sourceStart = audioBuffer[audioFrame]+audioFrameProgress;
		s16 *sourceEnd = audioBuffer[audioFrame]+end;
while(sourceStart<sourceEnd)
{
*target = *sourceStart;
++target;
++sourceStart;
}
		//filled += end-audioFrameProgress;
		if(end == 4*1024)
		{
			audioEmpty[audioFrame] = 1;
			audioFrame = (audioFrame+1) & 0x7;
			audioFrameProgress = 0;
		}
		else
			audioFrameProgress = end;
	}
	return length;
#else
	s8 *target = dest;
int i=0;
	for( ; i<length && audioFrameProgress<2030159 ; ++i)
	{
		*target = audioFileData[audioFrameProgress];
		++target;
		++audioFrameProgress;
	}
if(i<length)
{
for( ; i<length ; ++i)
{
*target = 0;
++target;
}
}
return length;
#endif
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

void ProccessFrame(u16 **dest)
{
	u8 *dataStart = videoDataBuffer;

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

	audioFilePtr = fopen("/Audio3.pcm","rb");
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

	palletFilePtr = fopen("/Pallet.pala","rb");
	if(!palletFilePtr) palletFilePtr = fopen("Pallet.pala","rb");
	if(!palletFilePtr)
	{
		consoleDemoInit();
		iprintf("pallet file not open: terminating\n");
		while(1)
		{
			swiWaitForVBlank();
			scanKeys();
			if(keysDown()&KEY_START) return 0;
		}
	}

	videoFilePtr = fopen("/Video3.dxta","rb");
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
	u32 nextPallet = 0;
	int audioFrameLoad = 0;

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
		while(readLen < 255)
		{
			int r = fread(tempPallet+readLen, sizeof(u16), 255 - readLen, palletFilePtr);
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

		dmaCopyHalfWords(1, tempPallet, SPRITE_PALETTE+1, 255*sizeof(u16));

		cpuStartTiming(1);
		readLen = 0;
		while(readLen < 255)
		{
			int r = fread(tempPallet+readLen, sizeof(u16), 255 - readLen, palletFilePtr);
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

		dmaCopyHalfWords(2, tempPallet, SPRITE_PALETTE_SUB+1, 255*sizeof(u16));

		cpuStartTiming(1);
		readLen = 0;
		while(readLen < 18432)
		{
			int r = fread(videoDataBuffer+readLen, sizeof(s8), 18432 - readLen, videoFilePtr);
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
		ProccessFrame(spireteVRAM);
		qoiTime += cpuEndTiming();

		cpuStartTiming(1);
		readLen = 0;
		while(readLen < 18432)
		{
			int r = fread(videoDataBuffer+readLen, sizeof(s8), 18432 - readLen, videoFilePtr);
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
		ProccessFrame(spireteVRAM + 12);
		qoiTime += cpuEndTiming();
	}

	// load rest of pallet data
	{
		cpuStartTiming(1);
		int readLen = 0;
		while(readLen < 26*255*2)
		{
			int r = fread(palletData+readLen, sizeof(u16), (26*255*2) - readLen, palletFilePtr);
			if(r <= 0)
			{
				consoleDemoInit();
				iprintf("error loading inital data:\nDid not get full file\nterminating\n");
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
		fclose(palletFilePtr);
		palletFilePtr = NULL;
	}

	// audio
	{
		cpuStartTiming(1);
#if 0
		int readLen = 0;
		while(readLen < 8 * 4 * 1024)
		{
			int r = fread(audioBuffer+readLen, sizeof(s16), (8 * 4 * 1024) - readLen, audioFilePtr);
			if(r <= 0)
			{
				consoleDemoInit();
				iprintf("error loading inital data:\nDid not get full file\nterminating\n");
				while(1)
				{
					swiWaitForVBlank();
					scanKeys();
					if(keysDown()&KEY_START) return 0;
				}
			}
			readLen += r;
		}
#elif 1
		int readLen = 0;
		while(readLen < 2030159)
		{
			int r = fread(audioFileData+readLen, sizeof(s8), (2030159) - readLen, audioFilePtr);
			if(r <= 0)
			{
				consoleDemoInit();
				iprintf("error loading inital data:\nDid not get full file\nterminating\n");
				while(1)
				{
					swiWaitForVBlank();
					scanKeys();
					if(keysDown()&KEY_START) return 0;
				}
			}
			readLen += r;
		}
fclose(audioFilePtr);
		audioFilePtr = NULL;
#else
		for(int b=0 ; b<8 ; ++b)
		{
		int readLen = 0;
		while(readLen < 4 * 1024)
		{
			*(audioBuffer[b]+readLen) = 0;
			++readLen;
		}
		}
#endif

		fileIOTime += cpuEndTiming();
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
		mystream.sampling_rate	= 22050;					// sampling rate = 25khz
		mystream.buffer_length	= 4 * 1024;						// buffer length = 1200 samples
		mystream.callback		= on_stream_request;		// set callback function
		mystream.format			= MM_STREAM_8BIT_MONO;	// format = stereo 16-bit
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

#if 0
		while(audioEmpty[audioFrameLoad] == 1)
		{
				cpuStartTiming(1);
#if 1
				int readLen = 0;
				while(readLen < 4 * 1024)
				{
					int r = fread(audioBuffer[audioFrameLoad]+readLen, sizeof(s16), (4 * 1024) - readLen, audioFilePtr);
					if(r <= 0)
					{
						// end of file
						dmaFillHalfWords(0, audioBuffer[audioFrameLoad]+readLen, ((4 * 1024) - readLen) * sizeof(s16));
						break;
					}
					readLen += r;
				}
#else
				int readLen = 0;
				while(readLen < 4 * 1024)
				{
					*(audioBuffer[b]+readLen) = 0;
					++readLen;
				}
#endif
				fileIOTime += cpuEndTiming();
				audioEmpty[audioFrameLoad] = 0;
				audioFrameLoad = (audioFrameLoad+1) & 0x7;
		}
#endif

		if(streamVideoTime + 3657 < mmStreamGetPosition() * 4)
		{
			streamVideoTime += 3657;
			while(streamVideoTime + 3657 < mmStreamGetPosition() * 4)
			{
				streamVideoTime += 3657;
				++droppedFrames;
				++frameCount;
				int currPos = ftell(videoFilePtr);
				fseek(videoFilePtr, currPos+(18432*2), SEEK_SET);
			}

			if(2211 <= frameCount)
				break;

			cpuStartTiming(1);
			int readLen = 0;
			while(readLen < 18432)
			{
				if(feof(videoFilePtr) != 0 || ferror(videoFilePtr) != 0)
				{
					done = 1;
					break;
				}
				int r = fread(videoDataBuffer+readLen, sizeof(s8), 18432 - readLen, videoFilePtr);
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
			ProccessFrame(spireteVRAM);
			qoiTime += cpuEndTiming();

			cpuStartTiming(1);
			readLen = 0;
			while(readLen < 18432)
			{
				if(feof(videoFilePtr) != 0 || ferror(videoFilePtr) != 0)
				{
					done = 1;
					break;
				}
				int r = fread(videoDataBuffer+readLen, sizeof(s8), 18432 - readLen, videoFilePtr);
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
			ProccessFrame(spireteVRAM + 12);
			qoiTime += cpuEndTiming();

			int palletChanged = 0;
			while(palletThreshHold[nextPallet] <= frameCount)
			{
				palletChanged = 1;
				++nextPallet;
			}
			if(palletChanged)
			{
				dmaCopyHalfWordsAsynch(1, palletData[nextPallet-1][0], SPRITE_PALETTE+1, 255*sizeof(u16));
				dmaCopyHalfWordsAsynch(2, palletData[nextPallet-1][1], SPRITE_PALETTE_SUB+1, 255*sizeof(u16));
			}

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
