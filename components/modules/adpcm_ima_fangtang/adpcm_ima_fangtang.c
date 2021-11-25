/*
** Intel/DVI ADPCM coder/decoder.
**
** The algorithm for this coder was taken from the IMA Compatability Project
**
** Version 1.2, 18-Dec-92.
*/
//#include <log/log.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "adpcm_ima_fangtang.h"


/* Intel ADPCM step variation table */
static int indexTable[16] =
{
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8,
};

static int stepsizeTable[89] =
{
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

int adpcm_coder(short *indata, unsigned char *outdata, int len, adpcm_state *state)
{
    int val;            /* Current input sample value */
    unsigned int delta; /* Current adpcm output value */
    int diff;           /* Difference between val and valprev */
    int step;           /* Stepsize */
    int valpred;        /* Predicted output value */
    int vpdiff;         /* Current change to valpred */
    int index;          /* Current step change index */
    unsigned int outputbuffer = 0;/* place to keep previous 4-bit value */
    int count = 0;      /* the number of bytes encoded */

    valpred = state->valprev;
    index = (int)state->index;
    step = stepsizeTable[index];

    while (len > 0 )
    {
        /* Step 1 - compute difference with previous value */
        val = *indata++;
        diff = val - valpred;
        if(diff < 0)
        {
            delta = 8;
            diff = (-diff);
        }
        else
        {
            delta = 0;
        }

        /* Step 2 - Divide and clamp */
        /* Note:
        ** This code *approximately* computes:
        **    delta = diff*4/step;
        **    vpdiff = (delta+0.5)*step/4;
        ** but in shift step bits are dropped. The net result of this is
        ** that even if you have fast mul/div hardware you cannot put it to
        ** good use since the fixup would be too expensive.
        */
        vpdiff = (step >> 3);

        if ( diff >= step )
        {
            delta |= 4;
            diff -= step;
            vpdiff += step;
        }
        step >>= 1;
        if ( diff >= step  )
        {
            delta |= 2;
            diff -= step;
            vpdiff += step;
        }
        step >>= 1;
        if ( diff >= step )
        {
            delta |= 1;
            vpdiff += step;
        }

        /* Phil Frisbie combined steps 3 and 4 */
        /* Step 3 - Update previous value */
        /* Step 4 - Clamp previous value to 16 bits */
        if ( (delta&8) != 0 )
        {
            valpred -= vpdiff;
            if ( valpred < -32768 )
                valpred = -32768;
        }
        else
        {
            valpred += vpdiff;
            if ( valpred > 32767 )
                valpred = 32767;
        }

        /* Step 5 - Assemble value, update index and step values */
        index += indexTable[delta];
        if ( index < 0 ) index = 0;
        else if ( index > 88 ) index = 88;
        step = stepsizeTable[index];

        /* Step 6 - Output value */
        outputbuffer = (delta << 4);

        /* Step 1 - compute difference with previous value */
        val = *indata++;
        diff = val - valpred;
        if(diff < 0)
        {
            delta = 8;
            diff = (-diff);
        }
        else
        {
            delta = 0;
        }

        /* Step 2 - Divide and clamp */
        /* Note:
        ** This code *approximately* computes:
        **    delta = diff*4/step;
        **    vpdiff = (delta+0.5)*step/4;
        ** but in shift step bits are dropped. The net result of this is
        ** that even if you have fast mul/div hardware you cannot put it to
        ** good use since the fixup would be too expensive.
        */
        vpdiff = (step >> 3);

        if ( diff >= step )
        {
            delta |= 4;
            diff -= step;
            vpdiff += step;
        }
        step >>= 1;
        if ( diff >= step  )
        {
            delta |= 2;
            diff -= step;
            vpdiff += step;
        }
        step >>= 1;
        if ( diff >= step )
        {
            delta |= 1;
            vpdiff += step;
        }

        /* Phil Frisbie combined steps 3 and 4 */
        /* Step 3 - Update previous value */
        /* Step 4 - Clamp previous value to 16 bits */
        if ( (delta&8) != 0 )
        {
            valpred -= vpdiff;
            if ( valpred < -32768 )
                valpred = -32768;
        }
        else
        {
            valpred += vpdiff;
            if ( valpred > 32767 )
                valpred = 32767;
        }

        /* Step 5 - Assemble value, update index and step values */
        index += indexTable[delta];
        if ( index < 0 ) index = 0;
        else if ( index > 88 ) index = 88;
        step = stepsizeTable[index];

        /* Step 6 - Output value */
#if 0
        *outdata++ = (unsigned char)(delta | outputbuffer);
#else  //DYC changed
        *outdata = (unsigned char)(delta | outputbuffer);
        *outdata = (*outdata&0xf0)>>4 | ((*outdata&0x0f)<<4);        //reverse
        outdata++;
#endif

        count++;
        len -= 2;
    }

    state->valprev = (short)valpred;
    state->index = (char)index;

    return count;
}

int adpcm_decoder(unsigned char *indata, short *outdata, int len, adpcm_state *state)
{
    unsigned int delta; /* Current adpcm output value */
    int step;           /* Stepsize */
    int valpred;        /* Predicted value */
    int vpdiff;         /* Current change to valpred */
    int index;          /* Current step change index */
    unsigned int inputbuffer = 0;/* place to keep next 4-bit value */
    int count = 0;

    valpred = state->valprev;
    index = (int)state->index;
    step = stepsizeTable[index];

    /* Loop unrolling by Phil Frisbie */
    /* This assumes there are ALWAYS an even number of samples */
    while ( len-- > 0 )
    {

        /* Step 1 - get the delta value */
        inputbuffer = (unsigned int)*indata++;
        delta = (inputbuffer >> 4);

        /* Step 2 - Find new index value (for later) */
        index += indexTable[delta];
        if ( index < 0 ) index = 0;
        else if ( index > 88 ) index = 88;


        /* Phil Frisbie combined steps 3, 4, and 5 */
        /* Step 3 - Separate sign and magnitude */
        /* Step 4 - Compute difference and new predicted value */
        /* Step 5 - clamp output value */
        /*
        ** Computes 'vpdiff = (delta+0.5)*step/4', but see comment
        ** in adpcm_coder.
        */
        vpdiff = step >> 3;
        if ( (delta & 4) != 0 ) vpdiff += step;
        if ( (delta & 2) != 0 ) vpdiff += step>>1;
        if ( (delta & 1) != 0 ) vpdiff += step>>2;

        if ( (delta & 8) != 0 )
        {
            valpred -= vpdiff;
            if ( valpred < -32768 )
                valpred = -32768;
        }
        else
        {
            valpred += vpdiff;
            if ( valpred > 32767 )
                valpred = 32767;
        }

        /* Step 6 - Update step value */
        step = stepsizeTable[index];

        /* Step 7 - Output value */
        *outdata++ = (short)valpred;

        /* Step 1 - get the delta value */
        delta = inputbuffer & 0xf;

        /* Step 2 - Find new index value (for later) */
        index += indexTable[delta];
        if ( index < 0 ) index = 0;
        else if ( index > 88 ) index = 88;


        /* Phil Frisbie combined steps 3, 4, and 5 */
        /* Step 3 - Separate sign and magnitude */
        /* Step 4 - Compute difference and new predicted value */
        /* Step 5 - clamp output value */
        /*
        ** Computes 'vpdiff = (delta+0.5)*step/4', but see comment
        ** in adpcm_coder.
        */
        int vpdiff = step;  /* the 4 is for rounding */
        if ((delta & 4) != 0)
        {
            vpdiff += (int)(step << 3);
        }
        if ((delta & 2) != 0)
        {
            vpdiff += (int)(step << 2);
        }
        if ((delta & 1) != 0)
        {
            vpdiff += (int)(step << 1);
        }
        vpdiff >>= 3;

        if ( (delta & 8) != 0 )
        {
            valpred -= vpdiff;
            if ( valpred < -32768 )
                valpred = -32768;
        }
        else
        {
            valpred += vpdiff;
            if ( valpred > 32767 )
                valpred = 32767;
        }

        /* Step 6 - Update step value */
        step = stepsizeTable[index];

        /* Step 7 - Output value */
        *outdata++ = (short)valpred;
        count += 2;
    }

    state->valprev = (short)valpred;
    state->index = (char)index;

    return count;
}

static void decode_adpcm_frame(unsigned char nibble, adpcm_state *state,
                               short *outdata)
{
    int step_index = state->index;
    int value = state->valprev;
    int step = stepsizeTable[step_index];
    int diff;

    diff = step;
    if ((nibble & 4) != 0)
    {
        diff += (int) (step << 3);
    }
    if ((nibble & 2) != 0)
    {
        diff += (int) (step << 2);
    }
    if ((nibble & 1) != 0)
    {
        diff += (int) (step << 1);
    }
    diff >>= 3;

    if (nibble & 8)
    {
        value -= diff;
        if (value < -32768)
            value = 32768;
    }
    else
    {
        value += diff;
        if (value > 32767)
            value = 32767;
    }
    state->valprev = value;

    /* copy to stream */
    if (outdata != NULL)
    {
        *(outdata + state->write_out_index) = (short)value;
        state->write_out_index ++;
    }
    if (value > state->peak_level)
        state->peak_level = value;

    /* update step_index */
    step_index += indexTable[nibble];
    /* clamp step_index */
    if (step_index < 0)
        step_index = 0;
    else if (step_index > 88)
        step_index = 88;
    state->index = step_index;
}


void decode_adpcm_16to3(unsigned char *indata, unsigned int num_bytes, short *outdata, adpcm_state *state)
{
    unsigned int group = 0, pack_int = 0, index = 0;
    unsigned short pack_short = 0;
    unsigned char data = 0;
    //ALOGE(" decode_adpcm_16to3 : num_bytes: %ud, state->valprev: %d, state->index: %d, state->peak_level: %hd, state->write_in_index: %d",
    //  num_bytes, state->valprev, state->index, state->peak_level, state->write_in_index);

    for (index = 0; index < (num_bytes/3*3); index += 3)
    {
        pack_int = (unsigned int)((*(indata + index)<<24)|(*(indata + index + 1)<<16)|(*(indata + index + 2)<<8));
        for (int j = 0; j < 8; j++)
        {
            data = ((pack_int >> 28) & 0x0E) | 0x01;
            pack_int <<= 3;
            decode_adpcm_frame(data, state, outdata);
        }
    }

    group = (num_bytes%3) *8 /3;
    if (num_bytes%3 == 1)
        pack_short = (unsigned int)((*(indata + index)<<8));
    else if (num_bytes%3 == 2)
        pack_short = (unsigned int)((*(indata + index)<<8)|(*(indata + index + 1)));
    for (unsigned short j = 0; j<(group); j++)
    {
        data = ((pack_short >> 12) & 0x0E) | 0x01;
        pack_short <<= 3;
        decode_adpcm_frame(data, state, outdata);
    }
}
void decode_adpcm_16to4(unsigned char *indata, unsigned int num_bytes, short *outdata, adpcm_state *state)
{
    unsigned int group = 0, pack_int = 0, index = 0;
    unsigned short pack_short = 0;
    unsigned char data = 0;

    for (index = 0; index < (num_bytes); index ++)
    {
        unsigned char image_data;
        image_data = indata[index];
        for (int j = 0; j < 2; j++)
        {
            data = (( image_data >> 4) & 0x0F);
            image_data <<= 4;
            decode_adpcm_frame(data, state, outdata);
        }
    }
}


void encode_adpcm(adpcm_encode_state *state)
{
    int i, j;
    int16_t *ptr = state->inp;
    int32_t predictedSample = (int32_t)state->predictedSample;
    int index = state->index;
    int stepSize = stepsizeTable[index];
    uint8_t *optr = state->out;
    uint16_t outBuf = 0;
    int size = state->imaSize;
    int outMsb = 16 - size;
    int imaAnd = state->imaAnd;
    int imaOr = state->imaOr;
    int shift = 4 - size;


    for (i = 0; i < state->len; i++)
    {
        int inp = *ptr++;
        int32_t diff = inp - predictedSample;
        int sign = diff;
        uint8_t newIma = 0;
        if (sign < 0)
        {
            diff = -diff;

        }
        int mask = 4;
        int32_t tempStepSize = stepSize << 3;
        diff <<= 3;
        for (j = 0; j < 3; j++)
        {
            if (diff > tempStepSize)
            {
                newIma |= mask;
                diff -= tempStepSize;
            }
            tempStepSize >>= 1;
            mask >>= 1;
        }
        int storeIma = newIma;

        if (sign < 0)
        {
            storeIma |= 8;
        }
        storeIma = storeIma >> shift;

        outBuf |= (storeIma << outMsb);
        if (outMsb < 8)
        {
            *optr++ = (uint8_t)(outBuf >> 8);
            outMsb += 8;
            outBuf = (uint16_t)(outBuf << 8);

        }
        outMsb  -= size;
        newIma = newIma & imaAnd;
        newIma |= imaOr;

        int32_t predictedDiff = (newIma * (int32_t)stepSize) + (stepSize >> 1);
        predictedDiff >>= 2;
        predictedSample += (sign < 0) ? - predictedDiff : predictedDiff;
        if (predictedSample > 32767)
        {
            predictedSample = 32767;
        }
        if (predictedSample < - 32768)
        {
            predictedSample = 32768;
        }

        index += indexTable[newIma];
        if (index < 0)
        {
            index = 0;
        }
        else if (index > 88)
        {
            index = 88;
        }
        stepSize = stepsizeTable[index];
    }
    * optr++ = (uint8_t)(outBuf >> 8);
    state->index = index;
    state->predictedSample = predictedSample;

}
