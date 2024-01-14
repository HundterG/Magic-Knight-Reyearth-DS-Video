/* 
this is a re-creation of the qoi encoder/decoder made for 16bit images
it's really just an rle encoder with a history
16bit colors are denoted as 5551 with the 1 being an extra unused bit

frames
0xxxxxxx xxxxxxxx - raw color
10xxxxxx - index
11xxxxxx - run - the value in the x's are run-1 since a run of 1 can just be index
*/

#define QOI16_OP_COLOR 0x00
#define QOI16_OP_INDEX 0x80
#define QOI16_OP_RUN 0xC0

#define QOI16_MASK_1 0x80 /*10000000*/
#define QOI16_MASK_2 0xC0 /*11000000*/

#define QOI16_COLOR_HASH(__color) ((((__color)&0x0C00)>>6) | (((__color)&0x0060)>>3) | (((__color)&0x0003)))

#ifndef QOI16_NO_ENCODE
uint8_t *qoi16_encode(uint16_t const *data, int len, int *out_len);
#ifdef QOI16_IMPLEMENTATION
uint8_t *qoi16_encode(uint16_t const *data, int len, int *out_len)
{
	uint8_t *out_buffer, *out_buffer_writer;
	uint16_t index[64];
	uint16_t last = 0;
	uint16_t const *data_end = data + len;

	if(len <= 0) return NULL;
	out_buffer = (uint8_t*)malloc(len * sizeof(uint16_t));
	out_buffer_writer = out_buffer;
	if(out_len) *out_len = 0;
	if(!out_buffer) return NULL;
	for(int i=0 ; i<64 ; ++i) index[i] = 0;

	while(data<data_end)
	{
		if(*data == last)
		{
			++data;
			int c = 0;
			while(data<data_end && c<64 && *data == last)
			{
				++c;
				++data;
			}
			if(c == 0)
			{
				int i = QOI16_COLOR_HASH(last);
				*out_buffer_writer = (uint8_t)(i) | QOI16_OP_INDEX;
				++out_buffer_writer;
				if(out_len) *out_len += 1;
			}
			else
			{
				*out_buffer_writer = (uint8_t)(c) | QOI16_OP_RUN;
				++out_buffer_writer;
				if(out_len) *out_len += 1;
			}
		}
		else
		{
			int i = QOI16_COLOR_HASH(*data);
			if(*data == index[i])
			{
				*out_buffer_writer = (uint8_t)(i) | QOI16_OP_INDEX;
				last = *data;
				++data;
				++out_buffer_writer;
				if(out_len) *out_len += 1;
			}
			else
			{
				uint8_t b[2];
				*(uint16_t*)(b) = *data & ~0x8000;
				*out_buffer_writer = b[1];
				++out_buffer_writer;
				*out_buffer_writer = b[0];
				++out_buffer_writer;
				last = *data;
				index[i] = last;
				++data;
				if(out_len) *out_len += 2;
			}
		}
	}

	return out_buffer;
}
#endif
#endif

uint16_t *qoi16_decode_alloc(uint8_t const *data, int len, int x, int y);
void qoi16_decode(uint8_t const *data, int len, uint16_t *out_data);
#ifdef QOI16_IMPLEMENTATION
uint16_t *qoi16_decode_alloc(uint8_t const *data, int len, int x, int y)
{
	uint16_t *buffer = (uint16_t*)malloc(x*y*sizeof(uint16_t));
	qoi16_decode(data, len, buffer);
	return buffer;
}

void qoi16_decode(uint8_t const *data, int len, uint16_t *out_data)
{
	uint16_t index[64];
	uint16_t last = 0;
	uint8_t const *data_end = data + len;
	uint8_t b[2];
	for(int i=0 ; i<64 ; ++i) index[i] = 0;

	while(data<data_end)
	{
		b[0] = *data;
		++data;
		if((b[0] & QOI16_MASK_1) == QOI16_OP_COLOR)
		{
			b[1] = b[0];
			b[0] = *data;
			++data;
			last = *(uint16_t*)(b);
			*out_data = last;
			++out_data;
			index[QOI16_COLOR_HASH(last)] = last;
		}
		else if((b[0] & QOI16_MASK_2) == QOI16_OP_RUN)
		{
			int run = (b[0] & ~QOI16_MASK_2) + 1;
#if 1
			while(run)
			{
				*out_data = last;
				++out_data;
				--run;
			}
#else
dmaFillHalfWords(last, out_data, run*2);
out_data += run;
#endif
		}
		else if((b[0] & QOI16_MASK_2) == QOI16_OP_INDEX)
		{
			last = index[b[0] & ~QOI16_MASK_2];
			*out_data = last;
			++out_data;
		}
	}
}
#endif
