#include "wav_pcm.h"

WaveData_t* wavRead(FILE* fp) {

	WaveData_t *wavData = (WaveData_t*)malloc(sizeof(WaveData_t));

	if (wavData == NULL)
	{
		perror("wav data create fail\n");
		return NULL;
	}

	if (fp == NULL) {
		perror("wav file read failed");
		return NULL;
	}
	else
	{
		/* Read header at simple mode */
		WaveHeader_t header;
		fseek(fp, 0, SEEK_SET);
		fread(&header, sizeof(header), 1, fp);

		/* Check if the file is of supported format. */
		if (strncmp(header.chunkId, "RIFF", 4) || strncmp(header.format, "WAVE", 4)) {
			perror("WAV ID not found,please input wav file ");
			return NULL;
		}
		else
		{
			if (strncmp(header.dataId, "data", 4)) {
				/* the wav header is not a standard wav header. */
				perror("DATA id not found in wav file,try to seek DATA id");
				char dataIdTmp[4] = { 'd','a','t','a' };
				int pos = seekId(fp, dataIdTmp, 4, 0, 60);

				if (pos > 0)
				{
					if (fread(&(header.dataSize), sizeof(int32_t), 1, fp) < 1) {
						perror("Read wavData size failed ");
						return NULL;
					}
					else
					{
						/* repaired wav header  */
						memcpy(header.dataId, dataIdTmp, 4);
						wavData->pcmPos = ftell(fp);
					}
				}
				else
				{
					perror("DATA id NOT FOUND, please input correct wav file.");
					return NULL;
				}
			}
			else
			{
				/* Standard wav file,pcm start from 42 */
				wavData->pcmPos = 42;
			}
		}
		/* Initialize the wavData struct. */
		wavData->sampleRate = header.sampleRate;
		wavData->bitsPerSample = header.bitsPerSample;
		wavData->pcmSize = header.dataSize;
		wavData->numChannels = header.numChannels;
		wavData->samplesPerChannel = wavData->pcmSize / (wavData->numChannels * wavData->bitsPerSample / 8.0f);
		wavData->pcmData = NULL;
#ifdef WIN32
		wavData->pcmData = (int16_t*)malloc(header.dataSize * sizeof(int16_t));
		fseek(fp, wavData->pcmPos, SEEK_SET);
		if (wavData->pcmData != NULL)
		{
			fread(wavData->pcmData, 1, wavData->pcmSize, fp);
		}
#endif // WIN32

		return wavData;
	}
	return NULL;
}


int seekId(FILE* fp, char* id, size_t idSize, size_t startIndex, size_t endIndex) {

	if (fp == NULL)
	{
		perror("Input buffer is NULL");
		return -1;
	}
	if (endIndex < startIndex)
	{
		perror("Interval error");
		return -1;
	}
	size_t foundIndex = 0;
	char header[60];
	fseek(fp, 0, startIndex);

	for (size_t n = startIndex; n < endIndex; n++)
	{
		/* read |idSize| bytes from FILE buffer */
		fseek(fp, n, startIndex);
		fread(header, idSize, 1, fp);

		/* Check wheather the id read is correct or not */
		if (!strncmp(id, header, idSize)) {
			foundIndex = ftell(fp);
			break;
		}

	}
	if (ftell(fp) == endIndex - startIndex)
	{
		// need some test
		return -1;
	}


	return foundIndex - idSize;
}

/*
 *	Dump wavData to pcm file
 */
void dumpPcmData(WaveData_t waveData) {
	// Dump wavData into a text file.
	FILE* outputFilePtr = fopen("output.pcm", "wb+");
	if (outputFilePtr == NULL) {
		perror(" output pcm error");
	}
	else {
		if (waveData.pcmData != NULL)
		{
			fwrite(waveData.pcmData, waveData.pcmSize, 1, outputFilePtr);
		}
		else
		{
			perror(" pcm date not found");
		}
		fclose(outputFilePtr);
	}
}

/*
 *  Prints the wave header for debug
 */
void printHeaderInfo(WaveHeader_t hdr) {
	char buf[5];
	printf("Header Info:\n");
	strncpy(buf, hdr.chunkId, 4);
	buf[4] = '\0';
	printf("    Chunk ID: %s\n", buf);
	printf("    Chunk Size: %d\n", hdr.chunkSize);
	strncpy(buf, hdr.format, 4);
	buf[4] = '\0';
	printf("    Format: %s\n", buf);
	strncpy(buf, hdr.subchunkId, 4);
	buf[4] = '\0';
	printf("    Sub-chunk ID: %s\n", buf);
	printf("    Sub-chunk Size: %d\n", hdr.subchunkSize);
	printf("    Audio Format: %d\n", hdr.audioFormat);
	printf("    Channel Count: %d\n", hdr.numChannels);
	printf("    Sample Rate: %d\n", hdr.sampleRate);
	printf("    Bytes per Second: %d\n", hdr.byteRate);
	printf("    Block alignment: %d\n", hdr.blockAlign);
	printf("    Bit depth: %d\n", hdr.bitsPerSample);
	strncpy(buf, hdr.dataId, 4);
	buf[4] = '\0';
	printf("    Data ID: %s\n", buf);
	printf("    Data Size: %d\n", hdr.dataSize);
}

void printWavInfo(WaveData_t hdr) {
	printf("Wav file info:\n");
	printf("    Duration(s):   %f \n", (float)hdr.pcmSize / (hdr.bitsPerSample / 8.0) / hdr.numChannels / hdr.sampleRate);
	printf("    SampleRate(Hz): %d\n", hdr.sampleRate);				// 16k 32k 48k
	printf("    Channels:   %ld\n", hdr.numChannels);				// 1 2 8
	printf("    Bit depth(bit):  %d\n", hdr.bitsPerSample);			// 16
	printf("    number samples per channel:  %ld\n", hdr.samplesPerChannel);
}

WaveHeader_t* createWavHeader(size_t numChannels, size_t samplerate, size_t bitsPerSample) {
	WaveHeader_t *header = NULL;
	header = (WaveHeader_t*)malloc(sizeof(WaveHeader_t));

	if (header == NULL)
	{
		perror("Create wav header error.\n");
		return NULL;
	}

	strncpy(header->chunkId, "RIFF", 4);
	header->chunkSize = 36;					// chunk_size = 4 + (8 + formatchunk.chunkSize) + (8 + pcmSize); 36+pcm size
	strncpy(header->format, "WAVE", 4);
	strncpy(header->subchunkId, "fmt ", 4);
	header->subchunkSize = 16;				// 16 for standard wav file,18 for audition || fmt size
	header->audioFormat = 1;				// pcm
	header->numChannels = numChannels;
	header->sampleRate = samplerate;
	header->bitsPerSample = bitsPerSample;
	header->byteRate = header->sampleRate * header->numChannels * header->bitsPerSample / 8;
	header->blockAlign = header->numChannels * header->bitsPerSample / 8;
	strncpy(header->dataId, "data", 4);
	// header->dataSize =  // pcm size
	return header;
}

int writeHeader(FILE* fo, size_t pcmSize, size_t numChannels, size_t sampleRate, size_t bitsPerSample) {

	if (fo == NULL)
	{
		perror("Output file stream is NULL\n");
		return -1;
	}

	WaveHeader_t* outputWavHeader = NULL;
	outputWavHeader = createWavHeader(numChannels, sampleRate, bitsPerSample);

	if (outputWavHeader == NULL)
	{
		perror("Output wav header create fail\n");
		return -1;
	}

	outputWavHeader->chunkSize = pcmSize + 36;
	outputWavHeader->dataSize = pcmSize;

	if (fwrite(outputWavHeader, 1, sizeof(WaveHeader_t), fo) != sizeof(WaveHeader_t))
	{
		perror("Wave header write error\n");
		free(outputWavHeader);
		outputWavHeader = NULL;
		return -1;
	}

	return 0;
}

int writeToStream(FILE* fi, FILE* fo, size_t writeCount) {
	char dataToWrite[2048];
	size_t count = writeCount / 2048;
	if (fo && fi)
	{
		for (size_t n = 0; n < count; n++)
		{
			fread(dataToWrite, 1, 2048, fi);
			fwrite(dataToWrite, 1, 2048, fo);
		}
		if (writeCount - count * 2048 != 0)
		{
			fread(dataToWrite, 1, writeCount - count * 2048, fi);
			fwrite(dataToWrite, 1, writeCount - count * 2048, fo);
		}
		return 0;
	}
	else
	{
		perror(" Input / output stream NULL\n");
		return -1;
	}
}