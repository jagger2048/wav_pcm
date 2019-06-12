#ifndef __WAV_PCM_H__
#define __WAV_PCM_H__

#ifdef __cplusplus
extern "C" {
#endif
/*
 * A basic implementation of the 16-bit pcm wave file header.
 * Links explaining the RIFF-WAVE standard:
 *      http://www.topherlee.com/software/pcm-tut-wavformat.html
 *      https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
	
/* Standard wav file header */
typedef struct _WaveHeader_t {
char chunkId[4];            // 1-4      "RIFF"
int32_t chunkSize;          // 5-8		 overall size of file in bytes (36 + data_size)
char format[4];				// 9-12     "WAVE"
char subchunkId[4];         // 13-16    "fmt\0"
int32_t subchunkSize;       // 17-20     16 for PCM.  This is the size of the rest of the Subchunk which follows this number.
uint16_t audioFormat;       // 21-22    format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
uint16_t numChannels;       // 23-24    Mono = 1, Stereo = 2
int32_t sampleRate;         // 25-28	
int32_t byteRate;			// 29-32    SampleRate * NumChannels * BitsPerSample/8 byteRate
uint16_t blockAlign;        // 33-34	NumChannels * BitsPerSample/8
uint16_t bitsPerSample;     // 35-36    BitsPerSample 16bit support only 
char dataId[4];             // 37-40    "data"
int32_t dataSize;           // 41-44
} WaveHeader_t;

typedef struct _WaveData_t {
int16_t *pcmData;			// pcm data
long long pcmPos;			// the position of the pcm data in FILE ptr
int32_t pcmSize;
int32_t sampleRate;
uint16_t bitsPerSample;
size_t numChannels;
size_t samplesPerChannel;	//  number samples per channel
} WaveData_t;

/*
* Prototypes
*/

/* printf wav file information */
void printWavInfo(WaveData_t hdr);

/* read wav file */
WaveData_t* wavRead(FILE* fp);

/* dump pcm data from wav file */
void dumpPcmData(WaveData_t waveData);

/* seek id between startIndex and endIndex */
int seekId(FILE* fp, char* id, size_t idSize, size_t startIndex, size_t endIndex);
	
/* Create a wav header */
WaveHeader_t* createWavHeader(size_t numChannels, size_t samplerate, size_t bitsPerSample);

/* write wav header to stream */
int writeHeader(FILE* fo, size_t pcmSize, size_t numChannels, size_t sampleRate, size_t bitsPerSample);

/* write stream to strem */
int writeToStream(FILE* fi, FILE* fo, size_t writeCount);


/*
* DEBUG 
*/
void printHeaderInfo(WaveHeader_t);			


#ifdef __cplusplus
}
#endif

#endif // !__WAV_PCM_H__
