/*
** adpcm.h - include file for adpcm coder.
**
** Version 1.0, 7-Jul-92.
*/

#ifndef ADPCM_FANGTANG_H
#define ADPCM_FANGTANG_H

#ifdef __cplusplus
extern "C" {
#endif


typedef  struct {
    int	    valprev;	/* Previous output value */
    int	    index;		/* Index into stepsize table */
	short   peak_level;
	int     write_out_index;
	int     write_in_index;
}adpcm_state;

typedef struct
{
	unsigned char *out;
    short *inp;
    int index;
    int imaSize;
    
    int imaAnd;
    int imaOr ;
    int predictedSample;
    int len;
} adpcm_encode_state;


/*========================================
 *       adpcm interface api
 =========================================*/
//encode
int adpcm_coder(short *indata, unsigned char *outdata, int len, adpcm_state *state);
int adpcm_decoder(unsigned char *indata, short *outdata, int len, adpcm_state *state);

void encode_adpcm(adpcm_encode_state *state);

//decode
void decode_adpcm_16to4(unsigned char *indata, unsigned num_bytes, short *outdata, adpcm_state *state);
void decode_adpcm_16to3(unsigned char *indata, unsigned num_bytes, short *outdata, adpcm_state *state);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* ADPCM_H*/
