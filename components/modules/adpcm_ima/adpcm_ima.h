#ifndef _ADPCM_IMA
#define _ADPCM_IMA

int adpcm_decode_block (int16_t *outbuf, const uint8_t *inbuf, size_t inbufsize, int channels);

#endif	// _ADPCM_IMA
