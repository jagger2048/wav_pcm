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
	char chunk_id[4];            // 1-4      "RIFF"
	int32_t chunk_size;          // 5-8		 overall size of file in bytes (36 + data_size)
	char format[4];				// 9-12     "WAVE"
	char subchunk_id[4];         // 13-16    "fmt\0"
	int32_t subchunk_size;       // 17-20     16 for PCM.  This is the size of the rest of the Subchunk which follows this number.
	uint16_t audio_format;       // 21-22    format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
	uint16_t num_channels;       // 23-24    Mono = 1, Stereo = 2
	int32_t sample_rate;         // 25-28
	int32_t byte_rate;			// 29-32    SampleRate * NumChannels * BitsPerSample/8 byte_rate
	uint16_t block_align;        // 33-34	NumChannels * BitsPerSample/8
	uint16_t bits_per_sample;     // 35-36    BitsPerSample 16bit support only
	char data_id[4];             // 37-40    "data"
	int32_t data_size;           // 41-44
} WaveHeader_t;

typedef struct _WaveData_t {
	int16_t *pcm_data;			// pcm data
	long long pcm_pos;			// the position of the pcm data in FILE ptr
	int32_t pcm_size;
	int32_t sample_rate;
	uint16_t bits_per_sample;
	size_t num_channels;
	size_t samples_per_ch;	//  number samples per channel
} WaveData_t;

/*
* Prototypes
*/

/* printf wav file information */
void print_wav_info(WaveData_t hdr);

/* read wav file */
WaveData_t* wav_read(FILE* fp);

/* dump pcm data from wav file */
void dump_pcm_data(WaveData_t wav_data);

/* seek id between start_index and end_index */
int seek_id(FILE* fp, char* id, size_t id_size, size_t start_index, size_t end_index);

/* Create a wav header */
WaveHeader_t* create_wav_header(size_t num_channels, size_t samplerate, size_t bits_per_sample);

/* write wav header to stream */
int write_header(FILE* fo, size_t pcm_size, size_t num_channels, size_t sample_rate, size_t bits_per_sample);

/* write stream to strem */
int write_to_stream(FILE* fi, FILE* fo, size_t write_cnt);


/*
* DEBUG
*/
void print_header_info(WaveHeader_t hdr);


#ifdef __cplusplus
}
#endif

#endif // !__WAV_PCM_H__
