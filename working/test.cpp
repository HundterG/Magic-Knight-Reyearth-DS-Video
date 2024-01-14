#include <cstdint>
#include <cstddef>
#include <cstdlib>
#define QOI16_IMPLEMENTATION
#include "qoi16.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define RGB15(r,g,b) ((r)|((g)<<5)|((b)<<10))
uint16_t img16[256*(192*2)];
uint16_t img16de[256*(192*2)];
unsigned char imgFull[(1229-652)*(435*2)*3];
unsigned char img24[256*(192*2)*3];
uint8_t img8[256*192];
uint16_t workingbigpallet[500 * 1024 * 1024];
int workingbigpalletcount = 0;

unsigned char filebuffer[150 * 1024];

struct PalletCut
{
	int frame;
	bool small;
	bool end;
	uint16_t palletTop[16];
	uint16_t palletBottom[16];
} palletCuts[] = 
{
	{86, false, false, {2114, 4162, 4288, 6272, 6336, 8320, 10368, 14722, 19224, 21272, 23320, 23382, 25368, 25430, 27416, 27480}, {2114, 2116, 2178, 4162, 4164, 4226, 8384, 16772, 23318, 25366, 25368, 25432, 25434, 27478, 27480, 27482}},
	{133, false, false, {2114, 4160, 4162, 4224, 6208, 6272, 8320, 14594, 25436, 27484, 27548, 29532, 29596, 29598, 29662, 31646}, {2114, 4160, 4162, 4224, 6210, 6272, 6336, 8320, 23248, 27484, 27548, 29596, 29598, 31644, 31646, 31710}},
	{505, true, false, {2136}, {2136}},
	{617, false, false, {0, 2, 4, 6, 2114, 2116, 2178, 4162, 17310, 19358, 21470, 23518, 25566, 27614, 29662, 31710}, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2114}},
	{650, false, false, {0, 2048, 2112, 4096, 4160, 6144, 6208, 8256, 23516, 27550, 27612, 29598, 29660, 29662, 31708, 31710}, {2048, 2050, 2112, 2114, 4096, 4160, 6208, 8256, 21204, 23054, 23252, 23316, 25300, 25302, 25366, 27414}},
	{696, false, false, {2048, 2048, 2112, 4096, 4160, 6208, 8256, 10304, 23516, 25438, 27550, 29598, 29660, 29662, 31708, 31710}, {2048, 2112, 2114, 4096, 4160, 4162, 6208, 8256, 23054, 23252, 25300, 25302, 25364, 25366, 27350, 27414}},
	{715, false, false, {70, 2116, 2118, 2178, 2180, 4228, 6276, 8386, 19422, 21470, 23518, 25566, 27614, 29596, 29662, 31710}, {2, 4, 66, 68, 2112, 2114, 2176, 10762, 17374, 19422, 21470, 23518, 25566, 27614, 29662, 31710}},
	{776, true, false, {31710}, {31710}},
	{852, true, false, {25302}, {25302}},
	{897, false, false, {2116, 4162, 4164, 6210, 6212, 6274, 8258, 8322, 23248, 23322, 23388, 25370, 25434, 25436, 27482, 27484}, {68, 4162, 4180, 6210, 6274, 8258, 8322, 10306, 13018, 15066, 16926, 18974, 19036, 21022, 21148, 23196}},
	{931, false, false, {2, 64, 2050, 2112, 2176, 4160, 4224, 18818, 23326, 23388, 25374, 27486, 27548, 27550, 29534, 29596}, {4, 2052, 2112, 2176, 4160, 4224, 6208, 6272, 17182, 19230, 23180, 23390, 25438, 27486, 27548, 29596}},
	{953, false, false, {68, 130, 2114, 2116, 2178, 4162, 4226, 6274, 19160, 23388, 25374, 25436, 25438, 27484, 27486, 27548}, {64, 128, 2050, 2112, 4160, 6208, 8256, 10304, 17180, 19166, 19228, 21276, 23388, 25434, 25436, 27484}},
	{997, true, false, {2114}, {2114}},
	{1041, false, false, {2, 64, 66, 128, 2048, 2112, 2176, 4298, 19230, 21278, 21342, 23326, 23390, 25438, 27548, 27550}, {2, 4, 66, 2048, 2050, 2052, 2114, 4162, 6420, 19226, 21212, 23324, 25372, 25434, 27482, 27484}},
	{1090, false, false, {64, 2050, 2112, 4160, 4224, 6208, 6272, 8256, 16852, 23194, 23256, 23258, 25242, 25304, 25306, 25370}, {2, 64, 2048, 2050, 2112, 4160, 6208, 8256, 21208, 23256, 23256, 23318, 23320, 25368, 25370, 27416}},
	{1116, false, false, {4, 2052, 2112, 2114, 4160, 6208, 8256, 10304, 23258, 23320, 23322, 25306, 25368, 25370, 27416, 27418}, {2, 2048, 2112, 2114, 2176, 4160, 4224, 6208, 23258, 23320, 23322, 25306, 25370, 25434, 27354, 27418}},
	{1294, false, false, {2, 4, 66, 2050, 2052, 2112, 2114, 2128, 4162, 14804, 14866, 14868, 16916, 16980, 18964, 19026}, {66, 2052, 2112, 2114, 2114, 2176, 4160, 4224, 21146, 21210, 23194, 23258, 23320, 23322, 25368, 25370}},
	{1500, true, false, {29594}, {29594}},
	{1626, false, false, {4, 6, 68, 2114, 2116, 4162, 6210, 8258, 15126, 17174, 19222, 19224, 21270, 21272, 21334, 23320}, {2, 4, 66, 2050, 2112, 2114, 4160, 4162, 17114, 19226, 21270, 21274, 23322, 23384, 25308, 25370}},
	{1680, false, false, {2, 64, 66, 2048, 2112, 4160, 6208, 8256, 21274, 23322, 23324, 25370, 25372, 25434, 25436, 27482}, {2, 64, 2050, 2112, 2114, 4160, 6208, 8256, 23322, 23386, 25370, 25372, 25432, 25434, 27418, 27482}},
	{1713, false, false, {2, 4, 64, 66, 128, 2050, 2052, 6534, 21276, 23324, 23386, 23388, 25372, 25434, 25436, 27482}, {2, 64, 128, 2050, 2112, 2176, 4098, 4224, 17104, 19226, 21274, 21338, 23324, 23386, 25434, 27482}},
	{1769, false, false, {2, 4, 66, 68, 2114, 4162, 4226, 6210, 23322, 25370, 25434, 25436, 27482, 27482, 27484, 29530}, {2114, 4162, 4164, 4226, 4228, 6276, 6278, 6342, 19024, 19026, 21140, 23188, 23190, 25434, 27482, 27482}},
	{1837, false, false, {2, 4, 66, 2048, 2050, 2052, 2112, 2114, 19098, 21084, 21148, 23258, 23320, 23322, 25368, 25370}, {2, 64, 128, 2050, 2112, 2114, 4160, 6208, 19162, 21210, 21274, 23322, 25370, 25372, 25434, 27480}},
	{1956, true, false, {29662}, {29662}},
	{1989, false, false, {2, 64, 2048, 2050, 2112, 2114, 4096, 4160, 19228, 21276, 23324, 25372, 25434, 25436, 25438, 27420}, {2, 2050, 2114, 2116, 2180, 4162, 4164, 4228, 10772, 15000, 15064, 17048, 17112, 19160, 21208, 23256}},
	{2149, true, true, {31710}, {31710}}
};

#include <algorithm>
#include <string>
#include <cstring>

#define PALLET_COLOR_COUNT 256

struct DVec3
{
	union
	{
		struct
		{
			stbi_uc x, y, z;
		};
		stbi_uc a[3];
	};
};

struct DCube
{
	DVec3 min, max;
	int size;
	DVec3 *data;

	void Shrink(void)
	{
		min.x = 0xff; min.y = 0xff; min.z = 0xff;
		max.x = 0x00; max.y = 0x00; max.z = 0x00;
		for(int i=0 ; i<size ; ++i)
		{
			DVec3 &cp = data[i];

			if(cp.x < min.x) min.x = cp.x;
			if(cp.y < min.y) min.y = cp.y;
			if(cp.z < min.z) min.z = cp.z;

			if(max.x < cp.x) max.x = cp.x;
			if(max.y < cp.y) max.y = cp.y;
			if(max.z < cp.z) max.z = cp.z;
		}
	}

	int GetLargest(void)
	{
		DVec3 diff;
		diff.x = max.x - min.x;
		diff.y = max.y - min.y;
		diff.z = max.z - min.z;
		int ret = 0;
		if(diff.x < diff.y)
		{
			diff.x = diff.y;
			ret = 1;
		}
		if(diff.x < diff.z)
			ret = 2;
		return ret;
	}

	void GetAverage(stbi_uc color[3])
	{
		color[0] = stbi_uc( (int(max.x) + min.x) / 2 );
		color[1] = stbi_uc( (int(max.y) + min.y) / 2 );
		color[2] = stbi_uc( (int(max.z) + min.z) / 2 );
	}
};

void GetPalette(stbi_uc *pixles, int x, int y, stbi_uc palette[PALLET_COLOR_COUNT*4])
{
	int size = PALLET_COLOR_COUNT;
	DCube *cubes = new DCube[size];
	DVec3 *copy = new DVec3[x*y];
	for(int i=0 ; i<x*y ; ++i)
	{
		stbi_uc *inp = pixles + (i * 3);
		DVec3 &outp = copy[i];
		outp.x = inp[0];
		outp.y = inp[1];
		outp.z = inp[2];
	}

//std::cout << "Before Cube\n";

	cubes[0].data = copy;
	cubes[0].size = x*y;
	cubes[0].Shrink();

	for(int level=1, parentCube=0, itCount=0 ; level <= /*4*/ std::sqrt(PALLET_COLOR_COUNT) && itCount < 2000000000 ; ++itCount)
	{
		int largest = cubes[parentCube].GetLargest();
        if(1<cubes[parentCube].size)
        {
		std::sort(cubes[parentCube].data, cubes[parentCube].data + cubes[parentCube].size, 
			[largest](DVec3 &lhs, DVec3 &rhs)
			{
				return lhs.a[largest] < rhs.a[largest];
			});
        }
else if(cubes[parentCube].size<0)
{
std::sort(cubes[parentCube].data, (copy + (x*y)), 
			[largest](DVec3 &lhs, DVec3 &rhs)
			{
				return lhs.a[largest] < rhs.a[largest];
			});
}

		stbi_uc mid = cubes[parentCube].data[ cubes[parentCube].size / 2 ].a[largest];
		int childCubeOffset = size / std::pow(2, level);

		cubes[parentCube + childCubeOffset] = cubes[parentCube];
		cubes[parentCube].max.a[largest] = mid;
		cubes[parentCube + childCubeOffset].min.a[largest] = mid+1;

		cubes[parentCube].size = 0;
		if(mid != 255)
			while(cubes[parentCube].data[ cubes[parentCube].size ].a[largest] <= mid)
			{
				if(cubes[parentCube].data + cubes[parentCube].size + 1 < copy + (x*y))
					++cubes[parentCube].size;
				else
					break;
			}
		cubes[parentCube + childCubeOffset].data += cubes[parentCube].size;
		cubes[parentCube + childCubeOffset].size -= cubes[parentCube].size;
		// make sure that data and size exist in the copy buffer
		if(!(copy <= cubes[parentCube + childCubeOffset].data && cubes[parentCube + childCubeOffset].data < copy + (x*y)))
		{
			std::cout << "Data Is Invalid\n";
			throw 5;
		}

		if(cubes[parentCube + childCubeOffset].data + cubes[parentCube + childCubeOffset].size < copy)
		{
			cubes[parentCube + childCubeOffset].size = copy - cubes[parentCube + childCubeOffset].data;
		}
		else if(copy + (x*y*25) <= cubes[parentCube + childCubeOffset].data + cubes[parentCube + childCubeOffset].size)
		{
			// adjust to bounds
			cubes[parentCube + childCubeOffset].size = cubes[parentCube + childCubeOffset].data - (copy + (x*y));
		}

		cubes[parentCube].Shrink();
		cubes[parentCube + childCubeOffset].Shrink();

		if(parentCube + (childCubeOffset * 2) < size)
			parentCube = parentCube + (childCubeOffset * 2);
		else
		{
			parentCube = 0;
			++level;
		}
	}

//std::cout << "After Cube\n";

	for(int i=1 ; i<PALLET_COLOR_COUNT ; ++i)
	{
		stbi_uc *c = palette + (i * 4);
		cubes[i-1].GetAverage(c);
		c[3] = 0xff;
	}

	delete[] copy;
	delete[] cubes;
}

uint8_t GetClosest(stbi_uc color[3], stbi_uc palette[PALLET_COLOR_COUNT*4])
{
	int index = 0;
	int lastDist = 255 * 255 * 3 + 1;// Max int?
	for(int i=0 ; i<PALLET_COLOR_COUNT ; ++i)
	{
		stbi_uc *c = palette + (i * 4);
		int dr = color[0] - c[0];
		int dg = color[1] - c[1];
		int db = color[2] - c[2];
		int d = (dr*dr) + (dg*dg) + (db*db);
		if(d < lastDist)
		{
			lastDist = d;
			index = i;
		}
	}

	color[0] = palette[(index*4)];
	color[1] = palette[(index*4) + 1];
	color[2] = palette[(index*4) + 2];
return index;
}

void DoDitherDiff(stbi_uc color[3], int dx, int dy, int dz, int f)
{
	color[0] = stbi_uc(std::max(0, std::min(int(color[0]) + (dx * f / 16), 255)));
	color[1] = stbi_uc(std::max(0, std::min(int(color[1]) + (dy * f / 16), 255)));
	color[2] = stbi_uc(std::max(0, std::min(int(color[2]) + (dz * f / 16), 255)));
}

void Dither(stbi_uc *pixles, int x, int y, stbi_uc palette[PALLET_COLOR_COUNT*4], uint8_t *img8)
{
	for(int cy=0 ; cy<y ; ++cy)
	{
		for(int cx=0 ; cx<x ; ++cx)
		{
			stbi_uc *cp = pixles + (((cy*x) + cx) * 3);
			DVec3 np;
			np.x = cp[0];
			np.y = cp[1];
			np.z = cp[2];
			img8[(cy*x) + cx] = GetClosest(np.a, palette);
			int diffx = int(cp[0]) - np.x;
			int diffy = int(cp[1]) - np.y;
			int diffz = int(cp[2]) - np.z;
			cp[0] = np.x;
			cp[1] = np.y;
			cp[2] = np.z;

			if(cx+1 < x)
				DoDitherDiff(pixles + (((cy*x) + cx + 1) * 4), diffx, diffy, diffz, 7);
			if(cy+1 < y)
			{
				if(0 < cx-1)
					DoDitherDiff(pixles + ((((cy+1)*x) + cx - 1) * 3), diffx, diffy, diffz, 3);
				DoDitherDiff(pixles + ((((cy+1)*x) + cx) * 3), diffx, diffy, diffz, 3);
				if(cx+1 < x)
					DoDitherDiff(pixles + ((((cy+1)*x) + cx + 1) * 3), diffx, diffy, diffz, 3);
			}
		}
	}
}

void Get256Pallet16(uint16_t allColors[], int allColorsSize, uint16_t palette[256])
{
	std::memset(palette, 0, 16*sizeof(uint16_t));

	if(allColorsSize < 255)
	{
		for(int i=0 ; i<allColorsSize ; ++i)
		{
			palette[i+1] = allColors[i];
		}
std::sort(palette, palette+256);
		return;
	}

	for(int p=1 ; p<256 ; ++p)
	{
		int colorIndex = -1;
		int delta = 0;

		int lastR = palette[p-1] & 0x1F;
		int lastG = (palette[p-1] & 0x3E0) >> 5;
		int lastB = (palette[p-1] & 0x7C00) >> 10;

		for(int ac=0 ; ac<allColorsSize ; ++ac)
		{
			if(allColors[ac] != 0)
			{
				int aCR = allColors[ac] & 0x1F;
				int aCG = (allColors[ac] & 0x3E0) >> 5;
				int aCB = (allColors[ac] & 0x7C00) >> 10;

				int dR = lastR - aCR;
				int dG = lastG - aCG;
				int dB = lastB - aCB;

				int diff = (dR*dR) + (dG*dG) + (dB+dB);
				if(delta < diff)
				{
					delta = diff;
					colorIndex = ac;
				}
			}
		}
		palette[p] = allColors[colorIndex];
		allColors[colorIndex] = 0;
	}

	std::sort(palette, palette+256);
}

void Get16Pallet16(uint16_t allColors[], int allColorsSize, uint16_t palette[16])
{
	std::memset(palette, 0, 16*sizeof(uint16_t));

	if(allColorsSize <= 16)
	{
		for(int i=0 ; i<allColorsSize ; ++i)
		{
			palette[i] = allColors[i];
		}
std::sort(palette, palette+16);
		return;
	}

	palette[0] = allColors[0];
	for(int p=1 ; p<16 ; ++p)
	{
		int colorIndex = -1;
		int64_t delta = 0;

		for(int ac=0 ; ac<allColorsSize ; ++ac)
		{
			if(allColors[ac] != 0)
			{
				int aCR = allColors[ac] & 0x1F;
				int aCG = (allColors[ac] & 0x3E0) >> 5;
				int aCB = (allColors[ac] & 0x7C00) >> 10;
				int64_t diff = 0;

				for(int lp=0 ; lp<p ; ++lp)
				{
					int lastR = palette[lp] & 0x1F;
					int lastG = (palette[lp] & 0x3E0) >> 5;
					int lastB = (palette[lp] & 0x7C00) >> 10;

					int dR = lastR - aCR;
					int dG = lastG - aCG;
					int dB = lastB - aCB;

					diff += (dR*dR) + (dG*dG) + (dB+dB);
				}

				if(delta < diff)
				{
					delta = diff;
					colorIndex = ac;
				}
			}
		}
		palette[p] = allColors[colorIndex];
		allColors[colorIndex] = 0;
	}

	std::sort(palette, palette+16);
}

void GetNPallet16(uint16_t allColors[], int allColorsSize, uint16_t palette[], int palletSize)
{
	std::memset(palette, 0, palletSize*sizeof(uint16_t));

	if(allColorsSize < palletSize)
	{
		for(int i=0 ; i<allColorsSize ; ++i)
		{
			palette[i+1] = allColors[i];
		}
std::sort(palette, palette+palletSize);
		return;
	}

	for(int p=1 ; p<palletSize ; ++p)
	{
		int colorIndex = -1;
		int delta = 0;

		int lastR = palette[p-1] & 0x1F;
		int lastG = (palette[p-1] & 0x3E0) >> 5;
		int lastB = (palette[p-1] & 0x7C00) >> 10;

		for(int ac=0 ; ac<allColorsSize ; ++ac)
		{
			if(allColors[ac] != 0)
			{
				int aCR = allColors[ac] & 0x1F;
				int aCG = (allColors[ac] & 0x3E0) >> 5;
				int aCB = (allColors[ac] & 0x7C00) >> 10;

				int dR = lastR - aCR;
				int dG = lastG - aCG;
				int dB = lastB - aCB;

				int diff = (dR*dR) + (dG*dG) + (dB+dB);
				if(delta < diff)
				{
					delta = diff;
					colorIndex = ac;
				}
			}
		}
		palette[p] = allColors[colorIndex];
		allColors[colorIndex] = 0;
	}

	std::sort(palette, palette+palletSize);
}

uint8_t GetClosest16(unsigned char color[3], uint16_t palette[256])
{
	int index = 0;
	int lastDist = 255 * 255 * 3 + 1;// Max int?
	for(int i=0 ; i<256 ; ++i)
	{
		int dr = color[0] - ((palette[i] & 0x1F) << 3);
		int dg = color[1] - (((palette[i] & 0x3E0) >> 5) << 3);
		int db = color[2] - (((palette[i] & 0x7C00) >> 10) << 3);
		int d = (dr*dr) + (dg*dg) + (db*db);
		if(d < lastDist)
		{
			lastDist = d;
			index = i;
		}
	}

	color[0] = (palette[index] & 0x1F) << 3;
	color[1] = ((palette[index] & 0x3E0) >> 5) << 3;
	color[2] = (((palette[index] & 0x7C00) >> 10) << 3);
return index;
}

void Dither16(stbi_uc *img24, int x, int y, uint16_t palette[256], uint8_t *img8)
{
	for(int cy=0 ; cy<y ; ++cy)
	{
		for(int cx=0 ; cx<x ; ++cx)
		{
			stbi_uc *cp = img24 + (((cy*x) + cx) * 3);
			DVec3 np;
			np.x = cp[0];
			np.y = cp[1];
			np.z = cp[2];
			img8[(cy*x) + cx] = GetClosest16(np.a, palette);
			int diffx = int(cp[0]) - np.x;
			int diffy = int(cp[1]) - np.y;
			int diffz = int(cp[2]) - np.z;
			cp[0] = np.x;
			cp[1] = np.y;
			cp[2] = np.z;

			if(cx+1 < x)
				DoDitherDiff(img24 + (((cy*x) + cx + 1) * 3), diffx, diffy, diffz, 7);
			if(cy+1 < y)
			{
				if(0 < cx-1)
					DoDitherDiff(img24 + ((((cy+1)*x) + cx - 1) * 3), diffx, diffy, diffz, 3);
				DoDitherDiff(img24 + ((((cy+1)*x) + cx) * 3), diffx, diffy, diffz, 3);
				if(cx+1 < x)
					DoDitherDiff(img24 + ((((cy+1)*x) + cx + 1) * 3), diffx, diffy, diffz, 3);
			}
		}
	}
}

uint8_t GetClosest161616(unsigned char color[3], uint16_t palette[16])
{
	int index = 0;
	int lastDist = 255 * 255 * 3 + 1;// Max int?
	for(int i=0 ; i<16 ; ++i)
	{
		int dr = color[0] - ((palette[i] & 0x1F) << 3);
		int dg = color[1] - (((palette[i] & 0x3E0) >> 5) << 3);
		int db = color[2] - (((palette[i] & 0x7C00) >> 10) << 3);
		int d = (dr*dr) + (dg*dg) + (db*db);
		if(d < lastDist)
		{
			lastDist = d;
			index = i;
		}
	}

	color[0] = (palette[index] & 0x1F) << 3;
	color[1] = ((palette[index] & 0x3E0) >> 5) << 3;
	color[2] = (((palette[index] & 0x7C00) >> 10) << 3);
return index;
}

void Dither1616(stbi_uc *img24, int x, int y, uint16_t palette[16], uint8_t *img8)
{
	for(int cy=0 ; cy<y ; ++cy)
	{
		for(int cx=0 ; cx<x ; ++cx)
		{
			stbi_uc *cp = img24 + (((cy*x) + cx) * 3);
			DVec3 np;
			np.x = cp[0];
			np.y = cp[1];
			np.z = cp[2];
			img8[(cy*x) + cx] = GetClosest161616(np.a, palette);
			int diffx = int(cp[0]) - np.x;
			int diffy = int(cp[1]) - np.y;
			int diffz = int(cp[2]) - np.z;
			cp[0] = np.x;
			cp[1] = np.y;
			cp[2] = np.z;

			if(cx+1 < x)
				DoDitherDiff(img24 + (((cy*x) + cx + 1) * 3), diffx, diffy, diffz, 7);
			if(cy+1 < y)
			{
				if(0 < cx-1)
					DoDitherDiff(img24 + ((((cy+1)*x) + cx - 1) * 3), diffx, diffy, diffz, 3);
				DoDitherDiff(img24 + ((((cy+1)*x) + cx) * 3), diffx, diffy, diffz, 3);
				if(cx+1 < x)
					DoDitherDiff(img24 + ((((cy+1)*x) + cx + 1) * 3), diffx, diffy, diffz, 3);
			}
		}
	}
}

void DoSingleFrame(char const *fileName, char const *outname)
{
	// load file
std::ifstream filein(fileName);
int readLen = 0;
while(readLen < (256*192*2))
{
filein.read(reinterpret_cast<char*>(img16) + readLen, (256*192*2) - readLen);
readLen += filein.gcount();
}
	// get list of unique colors
int uniqueCount = 1;
img16de[0] = img16[0];
for(int wi=1 ; wi<(256*(192*2)) ; ++wi)
{
bool in = false;
for(int j=0 ; j<uniqueCount ; ++j)
{
if(img16de[j] == img16[wi])
{
in = true;
break;
}
}
if(!in)
{
img16de[uniqueCount] = img16[wi];
++uniqueCount;
}
}
	// paletize
uint16_t palette[256];
Get256Pallet16(img16de, uniqueCount, palette);
	// do dither
unsigned char *imgWalk = img24;
for(int wi=0 ; wi<(256*(192)) ; ++wi)
{
int r = img16[wi] & 0x1F;
int g = (img16[wi] & 0x3E0) >> 5;
int b = (img16[wi] & 0x7C00) >> 10;
r = r << 3;
g = g << 3;
b = b << 3;
imgWalk[0] = r;
imgWalk[1] = g;
imgWalk[2] = b;
imgWalk += 3;
}
Dither16(img24, 256, 192, palette, img8);
	// save as dither
	// save as bmp
std::string outfileName("pallet/");
outfileName += outname;
outfileName += ".pal";
std::ofstream file(outfileName.c_str(), std::ofstream::binary);
	file.write((char*)(palette), sizeof(palette));
file.write((char*)(img8), 256*192);
file.close();

outfileName = "png/";
outfileName += outname;
outfileName += ".bmp";
stbi_write_bmp(outfileName.c_str(), 256, 192, 3, img24);
}

uint8_t GetClosest1616(uint16_t color, uint16_t palette[256])
{
	int index = 0;
	int lastDist = 255 * 255 * 3 + 1;// Max int?
uint8_t r = color & 0x1F;
uint8_t g = (color & 0x3E0) >> 5;
uint8_t b = (color & 0x7C00) >> 10;
r = r << 3;
g = g << 3;
b = b << 3;
	for(int i=0 ; i<256 ; ++i)
	{
		int dr = r - ((palette[i] & 0x1F) << 3);
		int dg = g - (((palette[i] & 0x3E0) >> 5) << 3);
		int db = b - (((palette[i] & 0x7C00) >> 10) << 3);
		int d = (dr*dr) + (dg*dg) + (db*db);
		if(d < lastDist)
		{
			lastDist = d;
			index = i;
		}
	}
return index;
}

uint8_t GetClosest164(uint16_t color, uint16_t palette[4])
{
	int index = 0;
	int lastDist = 255 * 255 * 3 + 1;// Max int?
uint8_t r = color & 0x1F;
uint8_t g = (color & 0x3E0) >> 5;
uint8_t b = (color & 0x7C00) >> 10;
r = r << 3;
g = g << 3;
b = b << 3;
	for(int i=0 ; i<4 ; ++i)
	{
		int dr = r - ((palette[i] & 0x1F) << 3);
		int dg = g - (((palette[i] & 0x3E0) >> 5) << 3);
		int db = b - (((palette[i] & 0x7C00) >> 10) << 3);
		int d = (dr*dr) + (dg*dg) + (db*db);
		if(d < lastDist)
		{
			lastDist = d;
			index = i;
		}
	}
return index;
}

void PalitizeTile(int count[16], uint8_t unique[16], int uniqueCount, uint8_t pixels[16], uint16_t const palette[256])
{
uint8_t endPallet[4];
// find max count, add as first color
int max = count[0];
int maxIndex = 0;
for(int i=1 ; i<uniqueCount ; ++i)
{
if(max < count[i])
{
max = count[i];
maxIndex = i;
}
}
endPallet[0] = unique[maxIndex];
// do paletization loop
	for(int p=1 ; p<4 ; ++p)
{
		int colorIndex = -1;
		int delta = 0;

		int lastR = palette[endPallet[p-1]] & 0x1F;
		int lastG = (palette[endPallet[p-1]] & 0x3E0) >> 5;
		int lastB = (palette[endPallet[p-1]] & 0x7C00) >> 10;

		for(int ac=0 ; ac<uniqueCount ; ++ac)
		{
			if(unique[ac] != endPallet[0])
			{
				int aCR = palette[unique[ac]] & 0x1F;
				int aCG = (palette[unique[ac]] & 0x3E0) >> 5;
				int aCB = (palette[unique[ac]] & 0x7C00) >> 10;

				int dR = lastR - aCR;
				int dG = lastG - aCG;
				int dB = lastB - aCB;

				int diff = (dR*dR) + (dG*dG) + (dB+dB);
				if(delta < diff)
				{
					delta = diff;
					colorIndex = ac;
				}
			}
		}
		endPallet[p] = unique[colorIndex];
		unique[colorIndex] = endPallet[0];
	}

// recolor extra pixels
uint16_t endColors[4];
endColors[0] = palette[endPallet[0]];
endColors[1] = palette[endPallet[1]];
endColors[2] = palette[endPallet[2]];
endColors[3] = palette[endPallet[3]];
for(int ac=0 ; ac<uniqueCount ; ++ac)
		{
			if(unique[ac] != endPallet[0])
			{
				// find closest
uint8_t closest = endPallet[GetClosest164(palette[unique[ac]], endColors)];
// update pixels array
for(int i=0 ; i<16 ; ++i)
{
if(pixels[i] == unique[ac])
pixels[i] = closest;
}
			}
		}

unique[0] = endPallet[0];
unique[1] = endPallet[1];
unique[2] = endPallet[2];
unique[3] = endPallet[3];
}

void EncodeDXTTile(uint8_t pixels[16], uint8_t *out, uint16_t const palette[256])
{
int count[16];
uint8_t unique[16];
int uniqueCount = 1;
count[0] = 1;
unique[0] = pixels[0];
for(int i=1 ; i<16 ; ++i)
{
bool in = false;
for(int j=0 ; j<uniqueCount ; ++j)
{
if(unique[j] == pixels[i])
{
in = true;
++count[j];
break;
}
}
if(!in)
{
count[uniqueCount] = 1;
unique[uniqueCount] = pixels[i];
++uniqueCount;
}
}

if(4 < uniqueCount)
{
PalitizeTile(count, unique, uniqueCount, pixels, palette);
}

out[0] = unique[0];
out[1] = unique[1];
out[2] = unique[2];
out[3] = unique[3];

uint32_t indexField = 0;
for(int i=15 ; 0<=i ; --i)
{
if(pixels[i] == unique[0]) indexField = (indexField << 2) | 0;
else if(1<uniqueCount && pixels[i] == unique[1]) indexField = (indexField << 2) | 1;
else if(2<uniqueCount && pixels[i] == unique[2]) indexField = (indexField << 2) | 2;
else if(3<uniqueCount && pixels[i] == unique[3]) indexField = (indexField << 2) | 3;
else throw "Error in encoding";
}
*(uint32_t*)(out+4) = indexField;
}

void EncodeImageDXT(uint8_t *out, uint16_t const palette[256])
{
for(int mty=0 ; mty<3 ; ++mty)
{
for(int mtx=0 ; mtx<4 ; ++mtx)
{
for(int sty=0 ; sty<8 ; ++sty)
{
for(int stx=0 ; stx<8 ; ++stx)
{
for(int y=0 ; y<2 ; ++y)
{
uint8_t *line1 = img8+(((mty*64)+(sty*8)+(y*4))*256) + (mtx*64) + (stx*8);
uint8_t *line2 = line1+256;
uint8_t *line3 = line2+256;
uint8_t *line4 = line3+256;
for(int x=0 ; x<2 ; ++x)
{
uint8_t tile[16];
tile[0] = line1[0];
tile[1] = line1[1];
tile[2] = line1[2];
tile[3] = line1[3];
line1 += 4;

tile[4] = line2[0];
tile[5] = line2[1];
tile[6] = line2[2];
tile[7] = line2[3];
line2 += 4;

tile[8] = line3[0];
tile[9] = line3[1];
tile[10] = line3[2];
tile[11] = line3[3];
line3 += 4;

tile[12] = line4[0];
tile[13] = line4[1];
tile[14] = line4[2];
tile[15] = line4[3];
line4 += 4;

EncodeDXTTile(tile, out, palette);
out += 8;
}
}
}
}
}
}
}

void PalitizeTile2(int count[32], uint8_t unique[32], int uniqueCount, uint8_t pixels[32], uint16_t const palette[256])
{
uint8_t endPallet[4];
// find max count, add as first color
int max = count[0];
int maxIndex = 0;
for(int i=1 ; i<uniqueCount ; ++i)
{
if(max < count[i])
{
max = count[i];
maxIndex = i;
}
}
endPallet[0] = unique[maxIndex];
// do paletization loop
	for(int p=1 ; p<4 ; ++p)
{
		int colorIndex = -1;
		int delta = 0;

		int lastR = palette[endPallet[p-1]] & 0x1F;
		int lastG = (palette[endPallet[p-1]] & 0x3E0) >> 5;
		int lastB = (palette[endPallet[p-1]] & 0x7C00) >> 10;

		for(int ac=0 ; ac<uniqueCount ; ++ac)
		{
			if(unique[ac] != endPallet[0])
			{
				int aCR = palette[unique[ac]] & 0x1F;
				int aCG = (palette[unique[ac]] & 0x3E0) >> 5;
				int aCB = (palette[unique[ac]] & 0x7C00) >> 10;

				int dR = lastR - aCR;
				int dG = lastG - aCG;
				int dB = lastB - aCB;

				int diff = (dR*dR) + (dG*dG) + (dB+dB);
				if(delta < diff)
				{
					delta = diff;
					colorIndex = ac;
				}
			}
		}
		endPallet[p] = unique[colorIndex];
		unique[colorIndex] = endPallet[0];
	}

// recolor extra pixels
uint16_t endColors[4];
endColors[0] = palette[endPallet[0]];
endColors[1] = palette[endPallet[1]];
endColors[2] = palette[endPallet[2]];
endColors[3] = palette[endPallet[3]];
for(int ac=0 ; ac<uniqueCount ; ++ac)
		{
			if(unique[ac] != endPallet[0])
			{
				// find closest
uint8_t closest = endPallet[GetClosest164(palette[unique[ac]], endColors)];
// update pixels array
for(int i=0 ; i<32 ; ++i)
{
if(pixels[i] == unique[ac])
pixels[i] = closest;
}
			}
		}

unique[0] = endPallet[0];
unique[1] = endPallet[1];
unique[2] = endPallet[2];
unique[3] = endPallet[3];
}

void EncodeDXTTile2(uint8_t pixels[32], uint8_t *out, uint16_t const palette[256])
{
int count[32];
uint8_t unique[32];
int uniqueCount = 1;
count[0] = 1;
unique[0] = pixels[0];
for(int i=1 ; i<32 ; ++i)
{
bool in = false;
for(int j=0 ; j<uniqueCount ; ++j)
{
if(unique[j] == pixels[i])
{
in = true;
++count[j];
break;
}
}
if(!in)
{
count[uniqueCount] = 1;
unique[uniqueCount] = pixels[i];
++uniqueCount;
}
}

if(4 < uniqueCount)
{
PalitizeTile2(count, unique, uniqueCount, pixels, palette);
}

out[0] = unique[0];
out[1] = unique[1];
out[2] = unique[2];
out[3] = unique[3];

uint64_t indexField = 0;
for(int i=31 ; 0<=i ; --i)
{
if(pixels[i] == unique[0]) indexField = (indexField << 2) | 0;
else if(1<uniqueCount && pixels[i] == unique[1]) indexField = (indexField << 2) | 1;
else if(2<uniqueCount && pixels[i] == unique[2]) indexField = (indexField << 2) | 2;
else if(3<uniqueCount && pixels[i] == unique[3]) indexField = (indexField << 2) | 3;
else throw "Error in encoding";
}
*(uint64_t*)(out+4) = indexField;
}

void EncodeImageDXT2(uint8_t *out, uint16_t const palette[256])
{
for(int mty=0 ; mty<3 ; ++mty)
{
for(int mtx=0 ; mtx<4 ; ++mtx)
{
for(int sty=0 ; sty<8 ; ++sty)
{
for(int stx=0 ; stx<8 ; ++stx)
{
for(int y=0 ; y<2 ; ++y)
{
uint8_t *line1 = img8+(((mty*64)+(sty*8)+(y*4))*256) + (mtx*64) + (stx*8);
uint8_t *line2 = line1+256;
uint8_t *line3 = line2+256;
uint8_t *line4 = line3+256;
uint8_t tile[32];
tile[0] = line1[0];
tile[1] = line1[1];
tile[2] = line1[2];
tile[3] = line1[3];
line1 += 4;

tile[4] = line1[0];
tile[5] = line1[1];
tile[6] = line1[2];
tile[7] = line1[3];
line1 += 4;

tile[8] = line2[0];
tile[9] = line2[1];
tile[10] = line2[2];
tile[11] = line2[3];
line2 += 4;

tile[12] = line2[0];
tile[13] = line2[1];
tile[14] = line2[2];
tile[15] = line2[3];
line2 += 4;

tile[16] = line3[0];
tile[17] = line3[1];
tile[18] = line3[2];
tile[19] = line3[3];
line3 += 4;

tile[20] = line3[0];
tile[21] = line3[1];
tile[22] = line3[2];
tile[23] = line3[3];
line3 += 4;

tile[24] = line4[0];
tile[25] = line4[1];
tile[26] = line4[2];
tile[27] = line4[3];
line4 += 4;

tile[28] = line4[0];
tile[29] = line4[1];
tile[30] = line4[2];
tile[31] = line4[3];
line4 += 4;

EncodeDXTTile2(tile, out, palette);
out += 12;
}
}
}
}
}
}

void PalitizeTile3(int count[32], uint8_t unique[32], int uniqueCount, uint8_t pixels[32], uint16_t const palette[16], uint8_t palletOffset)
{
uint8_t endPallet[4];
// find max count, add as first color
int max = count[0];
int maxIndex = 0;
for(int i=1 ; i<uniqueCount ; ++i)
{
if(max < count[i])
{
max = count[i];
maxIndex = i;
}
}
endPallet[0] = unique[maxIndex];
// do paletization loop
	for(int p=1 ; p<4 ; ++p)
{
		int colorIndex = -1;
		int delta = 0;

		int lastR = palette[endPallet[p-1] - palletOffset] & 0x1F;
		int lastG = (palette[endPallet[p-1] - palletOffset] & 0x3E0) >> 5;
		int lastB = (palette[endPallet[p-1] - palletOffset] & 0x7C00) >> 10;

		for(int ac=0 ; ac<uniqueCount ; ++ac)
		{
			if(unique[ac] != endPallet[0])
			{
				int aCR = palette[unique[ac] - palletOffset] & 0x1F;
				int aCG = (palette[unique[ac] - palletOffset] & 0x3E0) >> 5;
				int aCB = (palette[unique[ac] - palletOffset] & 0x7C00) >> 10;

				int dR = lastR - aCR;
				int dG = lastG - aCG;
				int dB = lastB - aCB;

				int diff = (dR*dR) + (dG*dG) + (dB+dB);
				if(delta < diff)
				{
					delta = diff;
					colorIndex = ac;
				}
			}
		}
		endPallet[p] = unique[colorIndex];
		unique[colorIndex] = endPallet[0];
	}

// recolor extra pixels
uint16_t endColors[4];
endColors[0] = palette[endPallet[0] - palletOffset];
endColors[1] = palette[endPallet[1] - palletOffset];
endColors[2] = palette[endPallet[2] - palletOffset];
endColors[3] = palette[endPallet[3] - palletOffset];
for(int ac=0 ; ac<uniqueCount ; ++ac)
		{
			if(unique[ac] != endPallet[0])
			{
				// find closest
uint8_t closest = endPallet[GetClosest164(palette[unique[ac] - palletOffset], endColors)];
// update pixels array
for(int i=0 ; i<32 ; ++i)
{
if(pixels[i] == unique[ac])
pixels[i] = closest;
}
			}
		}

unique[0] = endPallet[0];
unique[1] = endPallet[1];
unique[2] = endPallet[2];
unique[3] = endPallet[3];
}

void EncodeDXTTile3(uint8_t pixels[32], uint8_t *out, uint16_t const palette[16], uint8_t palletOffset)
{
int count[32];
uint8_t unique[32];
int uniqueCount = 1;
count[0] = 1;
unique[0] = pixels[0];
for(int i=1 ; i<32 ; ++i)
{
bool in = false;
for(int j=0 ; j<uniqueCount ; ++j)
{
if(unique[j] == pixels[i])
{
in = true;
++count[j];
break;
}
}
if(!in)
{
count[uniqueCount] = 1;
unique[uniqueCount] = pixels[i];
++uniqueCount;
}
}

if(4 < uniqueCount)
{
PalitizeTile3(count, unique, uniqueCount, pixels, palette, palletOffset);
}

out[0] = unique[0];
out[1] = unique[1];
out[2] = unique[2];
out[3] = unique[3];

uint64_t indexField = 0;
for(int i=31 ; 0<=i ; --i)
{
if(pixels[i] == unique[0]) indexField = (indexField << 2) | 0;
else if(1<uniqueCount && pixels[i] == unique[1]) indexField = (indexField << 2) | 1;
else if(2<uniqueCount && pixels[i] == unique[2]) indexField = (indexField << 2) | 2;
else if(3<uniqueCount && pixels[i] == unique[3]) indexField = (indexField << 2) | 3;
else throw "Error in encoding";
}
*(uint64_t*)(out+4) = indexField;
}

void EncodeImageDXT3(uint8_t *out, uint16_t const palette[16], uint8_t palletOffset)
{
for(int mty=0 ; mty<3 ; ++mty)
{
for(int mtx=0 ; mtx<4 ; ++mtx)
{
for(int sty=0 ; sty<8 ; ++sty)
{
for(int stx=0 ; stx<8 ; ++stx)
{
for(int y=0 ; y<2 ; ++y)
{
uint8_t *line1 = img8+(((mty*64)+(sty*8)+(y*4))*256) + (mtx*64) + (stx*8);
uint8_t *line2 = line1+256;
uint8_t *line3 = line2+256;
uint8_t *line4 = line3+256;
uint8_t tile[32];
tile[0] = line1[0] + palletOffset;
tile[1] = line1[1] + palletOffset;
tile[2] = line1[2] + palletOffset;
tile[3] = line1[3] + palletOffset;
line1 += 4;

tile[4] = line1[0] + palletOffset;
tile[5] = line1[1] + palletOffset;
tile[6] = line1[2] + palletOffset;
tile[7] = line1[3] + palletOffset;
line1 += 4;

tile[8] = line2[0] + palletOffset;
tile[9] = line2[1] + palletOffset;
tile[10] = line2[2] + palletOffset;
tile[11] = line2[3] + palletOffset;
line2 += 4;

tile[12] = line2[0] + palletOffset;
tile[13] = line2[1] + palletOffset;
tile[14] = line2[2] + palletOffset;
tile[15] = line2[3] + palletOffset;
line2 += 4;

tile[16] = line3[0] + palletOffset;
tile[17] = line3[1] + palletOffset;
tile[18] = line3[2] + palletOffset;
tile[19] = line3[3] + palletOffset;
line3 += 4;

tile[20] = line3[0] + palletOffset;
tile[21] = line3[1] + palletOffset;
tile[22] = line3[2] + palletOffset;
tile[23] = line3[3] + palletOffset;
line3 += 4;

tile[24] = line4[0] + palletOffset;
tile[25] = line4[1] + palletOffset;
tile[26] = line4[2] + palletOffset;
tile[27] = line4[3] + palletOffset;
line4 += 4;

tile[28] = line4[0] + palletOffset;
tile[29] = line4[1] + palletOffset;
tile[30] = line4[2] + palletOffset;
tile[31] = line4[3] + palletOffset;
line4 += 4;

EncodeDXTTile3(tile, out, palette, palletOffset);
out += 12;
}
}
}
}
}
}

// [start, end]
void GetPalletForFrames(int startFrame, int endFrame, int screen, uint16_t staticPallet[], int staticPalletSize, uint16_t pallet[], int palletSize)
{
	workingbigpalletcount = 0;
	for(int i=startFrame ; i<=endFrame ; ++i)
	{
		{
			std::string outfileName("raw16/");
			outfileName += std::to_string(i);
			outfileName += "_";
			outfileName += std::to_string(screen);
			outfileName += ".raw";
			std::ifstream file(outfileName.c_str());
			int readLen = 0;
while(readLen < (256*192*2))
{
file.read(reinterpret_cast<char*>(img16) + readLen, (256*192*2) - readLen);
readLen += file.gcount();
}
			file.close();
		}

		for(int wi=0 ; wi<(256*(192)) ; ++wi)
		{
			bool in = false;
			for(int j=0 ; j<staticPalletSize ; ++j)
			{
				if(staticPallet[j] == img16[wi])
				{
					in = true;
					break;
				}
			}
			if(!in)
			{
				for(int j=0 ; j<workingbigpalletcount ; ++j)
				{
					if(workingbigpallet[j] == img16[wi])
					{
						in = true;
						break;
					}
				}
			}
			if(!in)
			{
				workingbigpallet[workingbigpalletcount] = img16[wi];
				++workingbigpalletcount;
			}
		}
	}
	std::cout << "*";
	GetNPallet16(workingbigpallet, workingbigpalletcount, pallet, palletSize);
	std::cout << "*";
}

int main(void)
{
#if 0
bool front = true;
	std::ifstream palletFile("Pallet.pala");
	uint16_t palletTop[256];
	uint16_t palletBottom[256];
	palletTop[0] = 0;
	palletBottom[0] = 0;
	int readLen = 0;
	while(readLen < (255*2))
	{
		palletFile.read(reinterpret_cast<char*>(palletTop+1) + readLen, (255*2) - readLen);
		readLen += palletFile.gcount();
	}
	readLen = 0;
	while(readLen < (255*2))
	{
		palletFile.read(reinterpret_cast<char*>(palletBottom+1) + readLen, (255*2) - readLen);
		readLen += palletFile.gcount();
	}

	int palIndex = 0;
	bool palEnd = false;

	for(int i=1 ; i<=2211 ;)
	{
		if(!palEnd && (palletCuts[palIndex].frame == i))
		{
			if(palletCuts[palIndex].small)
			{
				// just do the compression on a full array of same value
				std::memset(img8, (front) ? 1 : 255, sizeof(img8));
				EncodeImageDXT2(filebuffer, nullptr);
				for(int j=0 ; j<2 ; ++j)
				{
					for(int screen=0 ; screen<2 ; ++screen)
					{
						std::string outfileName("dxt3/");
outfileName += std::to_string(i+j);
				outfileName += "_";
				outfileName += std::to_string(screen);
outfileName += ".dxt";
std::ofstream file(outfileName.c_str(), std::ofstream::binary);
file.write((char*)(filebuffer), 12*16*8*12); // 16*16*12*8
file.close();
					}
				}
			}
			else
			{
				// use a 16 color function for the dithering
				for(int screen=0 ; screen<2 ; ++screen)
				{
					uint16_t *thisPallet = (screen == 0) ? palletCuts[palIndex].palletTop : palletCuts[palIndex].palletBottom;
					for(int j=0 ; j<2 ; ++j)
					{
						{
						std::string outfileName("raw16/");
						outfileName += std::to_string(i+j);
						outfileName += "_";
						outfileName += std::to_string(screen);
						outfileName += ".raw";
						std::ifstream file(outfileName.c_str());
						int readLen = 0;
						while(readLen < (256*192*2))
						{
						file.read(reinterpret_cast<char*>(img16) + readLen, (256*192*2) - readLen);
						readLen += file.gcount();
						}
						file.close();
						}

						unsigned char *imgWalk = img24;
						for(int wi=0 ; wi<(256*(192)) ; ++wi)
						{
						int r = img16[wi] & 0x1F;
						int g = (img16[wi] & 0x3E0) >> 5;
						int b = (img16[wi] & 0x7C00) >> 10;
						r = r << 3;
						g = g << 3;
						b = b << 3;
						imgWalk[0] = r;
						imgWalk[1] = g;
						imgWalk[2] = b;
						imgWalk += 3;
						}

						Dither1616(img24, 256, 192, thisPallet, img8);
						EncodeImageDXT3(filebuffer, thisPallet, (front) ? 1 : (256-16));
std::string outfileName("dxt3/");
outfileName += std::to_string(i+j);
				outfileName += "_";
				outfileName += std::to_string(screen);
outfileName += ".dxt";
std::ofstream file(outfileName.c_str(), std::ofstream::binary);
file.write((char*)(filebuffer), 12*16*8*12); // 16*16*12*8
file.close();
					}
				}
			}
			front = !front;
			if(!palEnd)
			{
				int readLen = 0;
	while(readLen < (255*2))
	{
		palletFile.read(reinterpret_cast<char*>(palletTop+1) + readLen, (255*2) - readLen);
		readLen += palletFile.gcount();
	}
	readLen = 0;
	while(readLen < (255*2))
	{
		palletFile.read(reinterpret_cast<char*>(palletBottom+1) + readLen, (255*2) - readLen);
		readLen += palletFile.gcount();
	}
			}
			palEnd = palletCuts[palIndex].end;
			++palIndex;
			i += 2;
		}
		else
		{
			for(int screen=0 ; screen<2 ; ++screen)
			{
				{
				std::string outfileName("raw16/");
				outfileName += std::to_string(i);
				outfileName += "_";
				outfileName += std::to_string(screen);
				outfileName += ".raw";
				std::ifstream file(outfileName.c_str());
				int readLen = 0;
				while(readLen < (256*192*2))
				{
				file.read(reinterpret_cast<char*>(img16) + readLen, (256*192*2) - readLen);
				readLen += file.gcount();
				}
				file.close();
				}

				unsigned char *imgWalk = img24;
				for(int wi=0 ; wi<(256*(192)) ; ++wi)
				{
				int r = img16[wi] & 0x1F;
				int g = (img16[wi] & 0x3E0) >> 5;
				int b = (img16[wi] & 0x7C00) >> 10;
				r = r << 3;
				g = g << 3;
				b = b << 3;
				imgWalk[0] = r;
				imgWalk[1] = g;
				imgWalk[2] = b;
				imgWalk += 3;
				}
				uint16_t *palette = (screen==0) ? palletTop : palletBottom;
				Dither16(img24, 256, 192, palette, img8);
				EncodeImageDXT2(filebuffer, palette);
std::string outfileName("dxt3/");
outfileName += std::to_string(i);
				outfileName += "_";
				outfileName += std::to_string(screen);
outfileName += ".dxt";
std::ofstream file(outfileName.c_str(), std::ofstream::binary);
file.write((char*)(filebuffer), 12*16*8*12); // 16*16*12*8
file.close();
			}
			++i;
			// load the frame
	// dither with dither16
	// dxt compress
	//write the data
		}
	}
#endif

#if 0
	bool front = true;
	bool lastIteration = false;
	int last = -1;
	int curr = 0;
	int commonFrameStart = 1;
	std::ofstream fileout("Pallet.pala", std::ofstream::binary);
	for(;;)
	{
		uint16_t staticPalletTop[255];
		uint16_t staticPalletBottom[255];
		int staticPalletSize = 0;

		if(0 <= last)
		{
			for(int c=0 ; c<((palletCuts[last].small) ? 1 : 16) ; ++c)
			{
				staticPalletTop[staticPalletSize] = palletCuts[last].palletTop[c];
				staticPalletBottom[staticPalletSize] = palletCuts[last].palletBottom[c];
				++staticPalletSize;
			}
		}

		if(!lastIteration)
		{
			for(int c=0 ; c<((palletCuts[curr].small) ? 1 : 16) ; ++c)
			{
				staticPalletTop[staticPalletSize] = palletCuts[curr].palletTop[c];
				staticPalletBottom[staticPalletSize] = palletCuts[curr].palletBottom[c];
				++staticPalletSize;
			}
		}

		int endFrame = (lastIteration) ? 2211 : palletCuts[curr].frame-1;

		uint16_t commonPalletTop[255];
		uint16_t commonPalletBottom[255];
		int commonPalletSize = 255 - staticPalletSize;

		GetPalletForFrames(commonFrameStart, endFrame, 0, staticPalletTop, staticPalletSize, commonPalletTop, commonPalletSize);
		GetPalletForFrames(commonFrameStart, endFrame, 1, staticPalletBottom, staticPalletSize, commonPalletBottom, commonPalletSize);
		commonFrameStart = endFrame + 3;

		uint16_t finalPalletTop[255];
		uint16_t finalPalletBottom[255];
		int finalCommonStart = 0;

		if(0 <= last)
		{
			int lastStart = 0;
			if(front) // last was in the back
				lastStart = (palletCuts[last].small) ? 254 : (255-16);
			else // last in front
				finalCommonStart = (palletCuts[last].small) ? 1 : 16;
			for(int c=0 ; c<((palletCuts[last].small) ? 1 : 16) ; ++c)
			{
				finalPalletTop[lastStart+c] = palletCuts[last].palletTop[c];
				finalPalletBottom[lastStart+c] = palletCuts[last].palletBottom[c];
			}
		}

		if(!lastIteration)
		{
			int currStart = 0;
			if(front)
				finalCommonStart = (palletCuts[curr].small) ? 1 : 16;
			else
				currStart = (palletCuts[curr].small) ? 254 : (255-16);
			for(int c=0 ; c<((palletCuts[curr].small) ? 1 : 16) ; ++c)
			{
				finalPalletTop[currStart+c] = palletCuts[curr].palletTop[c];
				finalPalletBottom[currStart+c] = palletCuts[curr].palletBottom[c];
			}
		}

		for(int c=0 ; c<commonPalletSize ; ++c)
		{
			finalPalletTop[finalCommonStart+c] = commonPalletTop[c];
			finalPalletBottom[finalCommonStart+c] = commonPalletBottom[c];
		}

		fileout.write(reinterpret_cast<char*>(finalPalletTop), sizeof(finalPalletTop));
		fileout.write(reinterpret_cast<char*>(finalPalletBottom), sizeof(finalPalletBottom));

		if(lastIteration)
		{
			break;
		}
		else
		{
			lastIteration = palletCuts[curr].end;
			last = curr;
			++curr;
			front = !front;
		}
	}
#endif
#if 0
	int cuts[] = {86, 133, 617, 650, 696, 715, 897, 931, 953, 1041, 1090, 1116, 1294, 1626, 1680, 1713, 1769, 1837, 1989};
	int smallcuts[] = {505, 776, 852, 997, 1500, 1956, 2149};
#if 1
	for(int c : cuts)
	{
		//std::cout << c << " - " << c+1 << "\n";
		// top
		{
			std::string outfileName("raw16/");
			outfileName += std::to_string(c);
			outfileName += "_";
			outfileName += "0";
			outfileName += ".raw";
			std::ifstream file(outfileName.c_str());
			//file.read((char*)(img16), 256*192*2);
			int readLen = 0;
while(readLen < (256*192*2))
{
file.read(reinterpret_cast<char*>(img16) + readLen, (256*192*2) - readLen);
readLen += file.gcount();
}
			file.close();
		}
		{
			std::string outfileName("raw16/");
			outfileName += std::to_string(c+1);
			outfileName += "_";
			outfileName += "0";
			outfileName += ".raw";
			std::ifstream file(outfileName.c_str());
			//file.read((char*)(img16+256*192), 256*192*2);
			int readLen = 0;
while(readLen < (256*192*2))
{
file.read(reinterpret_cast<char*>(img16+256*192) + readLen, (256*192*2) - readLen);
readLen += file.gcount();
}
			file.close();
		}
		workingbigpallet[0] = img16[0];
		workingbigpalletcount = 1;
		for(int wi=1 ; wi<(256*(192*2)) ; ++wi)
		{
			bool in = false;
			for(int j=0 ; j<workingbigpalletcount ; ++j)
			{
				if(workingbigpallet[j] == img16[wi])
				{
					in = true;
					break;
				}
			}
			if(!in)
			{
				workingbigpallet[workingbigpalletcount] = img16[wi];
				++workingbigpalletcount;
			}
		}

		uint16_t pallet[16];
		Get16Pallet16(workingbigpallet, workingbigpalletcount, pallet);

		std::cout << c << " - {" << pallet[0];
		for(int p=1 ; p<16 ; ++p)
			std::cout << ", " << pallet[p];
		std::cout << "}\n";
	}
#endif

#if 1
	for(int c : smallcuts)
	{
		{
			std::string outfileName("raw16/");
			outfileName += std::to_string(c);
			outfileName += "_";
			outfileName += "0";
			outfileName += ".raw";
			std::ifstream file(outfileName.c_str());
			//file.read((char*)(img16), 256*192*2);
			int readLen = 0;
while(readLen < (256*192*2))
{
file.read(reinterpret_cast<char*>(img16) + readLen, (256*192*2) - readLen);
readLen += file.gcount();
}
			file.close();
		}
		std::cout << c << " - " << img16[0] << "\n";
	}
#endif




#endif

#if 0
for(int i=1 ; i<=2211 ; ++i)
{
for(int j=0 ; j<2 ; ++j)
{
std::string name = std::to_string(i) + "_" + std::to_string(j);
std::string path = "raw16/";
path += name;
path += ".raw";
DoSingleFrame(path.c_str(), name.c_str());
}
}
#endif

#if 0
for(int i=1 ; i<=2211 ; ++i)
{
for(int j=0 ; j<2 ; ++j)
{
std::string name = std::to_string(i) + "_" + std::to_string(j);
std::string path = "pallet/";
path += name;
path += ".pal";
uint16_t palette[256];
std::ifstream filein(path.c_str());
int readLen = 0;
while(readLen < sizeof(palette))
{
filein.read(reinterpret_cast<char*>(palette) + readLen, sizeof(palette) - readLen);
readLen += filein.gcount();
}
readLen = 0;
while(readLen < (256*192))
{
filein.read(reinterpret_cast<char*>(img8) + readLen, (256*192) - readLen);
readLen += filein.gcount();
}
EncodeImageDXT2(filebuffer, palette);
std::string outfileName("dxt2/");
outfileName += name;
outfileName += ".dxt";
std::ofstream file(outfileName.c_str(), std::ofstream::binary);
	file.write((char*)(palette), sizeof(palette));
file.write((char*)(filebuffer), 12*16*8*12); // 16*16*12*8
file.close();
}
}
#endif

#if 1
std::ofstream fileout("Video3.dxta", std::ofstream::binary);
for(int i=1 ; i<=2211 ; ++i)
{
for(int j=0 ; j<2 ; ++j)
{
std::string name = std::to_string(i) + "_" + std::to_string(j);
std::string path = "dxt3/";
path += name;
path += ".dxt";
// 25088
// 18944
// 18432
std::ifstream filein(path.c_str());
int readLen = 0;
while(readLen < 18432)
{
filein.read(reinterpret_cast<char*>(filebuffer) + readLen, 18432 - readLen);
readLen += filein.gcount();
}
fileout.write((char*)(filebuffer), 18432);
}
}
#endif

#if 0
for(int i=1 ; i<=2211 ; ++i)
{
int wx, wy, channels;
std::string fileName("frames/FullVideo-2400_");
fileName += std::to_string(i);
fileName += ".png";
stbi_uc *pixels = stbi_load(fileName.c_str(), &wx, &wy, &channels, 3);
unsigned char *imgWalk = imgFull;
for(int y=0 ; y<435; ++y)
{
for(int x=652 ; x<1229 ; ++x)
{
imgWalk[0] = pixels[(((wx*y)+x)*3)];
imgWalk[1] = pixels[(((wx*y)+x)*3)+1];
imgWalk[2] = pixels[(((wx*y)+x)*3)+2];
imgWalk += 3;
}
}
for(int y=645 ; y<1080; ++y)
{
for(int x=652 ; x<1229 ; ++x)
{
imgWalk[0] = pixels[(((wx*y)+x)*3)];
imgWalk[1] = pixels[(((wx*y)+x)*3)+1];
imgWalk[2] = pixels[(((wx*y)+x)*3)+2];
imgWalk += 3;
}
}
stbi_image_free(pixels);

stbir_resize(imgFull, 1229-652, 435*2, 0, img24, 256, 192*2, 0, STBIR_TYPE_UINT8, 3, STBIR_ALPHA_CHANNEL_NONE, 0, STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP, STBIR_FILTER_DEFAULT, STBIR_FILTER_DEFAULT, STBIR_COLORSPACE_LINEAR, nullptr);

//std::string outfilePNG("resizeframes/");
//outfilePNG += std::to_string(i);
//outfilePNG += ".bmp";
//stbi_write_bmp(outfilePNG.c_str(), 256, 192*2, 3, img24);

imgWalk = img24;
for(int wi=0 ; wi<(256*(192*2)) ; ++wi)
{
int r = imgWalk[0];
int g = imgWalk[1];
int b = imgWalk[2];
r = r >> 3;
g = g >> 3;
b = b >> 3;
imgWalk += 3;
img16[wi] = RGB15(r,g,b) & 0x7BDE;
}

#if 1
for(int screen=0 ; screen<2 ; ++screen)
{
std::string outfileName("raw16/");
outfileName += std::to_string(i);
outfileName += "_";
outfileName += std::to_string(screen);
outfileName += ".raw";
std::ofstream file(outfileName.c_str(), std::ofstream::binary);
file.write((char*)(img16+screen*256*192), 256*192*2);
file.close();
}
#endif

#if 0
// count unique
uint16_t uniqe[4196];
int uniqueCount = 1;
uniqe[0] = img16[0];
for(int wi=1 ; wi<(256*(192*2)) ; ++wi)
{
bool in = false;
for(int j=0 ; j<uniqueCount ; ++j)
{
if(uniqe[j] == img16[wi])
{
in = true;
break;
}
}
if(!in)
{
uniqe[uniqueCount] = img16[wi];
++uniqueCount;
if(4195 < uniqueCount) break;
}
}

if(uniqueCount <= 4195)
{
std::cout << "Frame " << i << " color count: " << uniqueCount << "\n";
}
else
{
std::cout << "Frame " << i << " color count: " << "OVER" << "\n";
}
#endif


#if 0
std::string outfileName("qoiframes/");
outfileName += std::to_string(i);
outfileName += ".qoi";
int outlen = 0;
	uint8_t *cdata = qoi16_encode(img16, 256*(192*2), &outlen);
qoi16_decode(cdata, outlen, img16de);
if(std::memcmp(img16, img16de, sizeof(img16)) == 0)
{
	std::ofstream file(outfileName.c_str(), std::ofstream::binary);
	file.write((char*)(cdata), outlen);
file.close();
}
else
{
std::cout << "Frame " << i << " did not compress correctly.\n";
}
free(cdata);
#endif

#if 0
// convert back to 24bit
imgWalk = img24;
for(int wi=0 ; wi<(256*(192*2)) ; ++wi)
{
int r = img16[wi] & 0x1F;
int g = (img16[wi] & 0x3E0) >> 5;
int b = (img16[wi] & 0x7C00) >> 10;
r = r << 3;
g = g << 3;
b = b << 3;
imgWalk[0] = r;
imgWalk[1] = g;
imgWalk[2] = b;
imgWalk += 3;
}

for(int screen=0 ; screen<2 ; ++screen)
{
	std::cout << i << " - " << screen << "\n";
uint8_t pallet[PALLET_COLOR_COUNT*4];
pallet[0] = pallet[1] = pallet[2] = 0;
GetPalette(img24+(screen*256*192*3), 256, 192, pallet);
// do dithering stuff
	//for(int cy=0 ; cy<192 ; ++cy)
	//{
	//	for(int cx=0 ; cx<256 ; ++cx)
	//	{
	//		stbi_uc *cp = (img24+(screen*256*192*3)) + (((cy*256) + cx) * 4);
	//		img8[((cy*256) + cx)] = GetClosest(cp, pallet);
	//	}
	//}
//std::cout << "Before Dither\n";
	Dither(img24+(screen*256*192*3), 256, 192, pallet, img8);

//std::cout << "Before Crunch\n";

imgWalk = pallet;
for(int p=0 ; p<PALLET_COLOR_COUNT ; ++p)
{
int r = imgWalk[0];
int g = imgWalk[1];
int b = imgWalk[2];
r = r >> 3;
g = g >> 3;
b = b >> 3;
imgWalk += 4;
img16[p] = RGB15(r,g,b) & 0x7BDE;
}

//std::cout << "Before File\n";

std::string outfileName("pallet/");
outfileName += std::to_string(i);
outfileName += "_";
outfileName += std::to_string(screen);
outfileName += ".pal";
std::ofstream file(outfileName.c_str(), std::ofstream::binary);
	file.write((char*)(img16), PALLET_COLOR_COUNT*2);
file.write((char*)(img8), 256*192);
file.close();

outfileName = "png/";
outfileName += std::to_string(i);
outfileName += "_";
outfileName += std::to_string(screen);
outfileName += ".bmp";
stbi_write_bmp(outfileName.c_str(), 256, 192, 3, img24+(screen*256*192*3));
}
#endif
}
#endif

#if 0
uint32_t largest = 0;
std::ofstream outFile("Video.qoia", std::ofstream::binary);
for(int i=1 ; i<=2211 ; ++i)
{
std::string outfileName("qoiframes/");
outfileName += std::to_string(i);
outfileName += ".qoi";
std::ifstream file(outfileName.c_str(), std::ifstream::binary | std::ifstream::ate);
uint32_t size = file.tellg();
if(largest < size) largest = size;
outFile.write((char*)(&size), 4);
file.seekg(0);
uint32_t readLen = 0;
while(readLen < size)
					{
						file.read(reinterpret_cast<char*>(filebuffer + readLen), size - readLen);
						readLen += file.gcount();
					}
if(readLen != size)
{
std::cout << "error: file\n";
return 0;
}
outFile.write((char*)(filebuffer), size);
}
outFile.close();
std::cout << largest << "\n";
#endif
	
}
