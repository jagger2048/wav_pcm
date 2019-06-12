
#ifndef WAV_READ
#define WAV_READ

#include "wav_pcm.h"

#define DEBUG 1

/* test the wav write read methon */

int main(int argc, char* argv[]) {

	if (argc == 1) {

#ifdef DEBUG

		char* fileName = "dukou_noReverb.wav";
		//char* fileName = "M1F1-int16-AFsp.wav";
		//char* fileName = "stand-self-record-65dbc_01.wav";
		WaveData_t *data = wavRead(fileName);
		printWavInfo(*data);

		if (data != NULL) {
			//dumpPcmData(*data);
			//return 0;
		}
		else
		{
			return -1;
		}
		FILE* fp = fopen(fileName,"rb+");
		fseek(fp, 0, SEEK_END);
		int32_t totalSize = ftell(fp);
		printf("total size %d\n", totalSize);
		fseek(fp, 40, SEEK_SET);
		int32_t pcmSize;
		fread(&pcmSize, 1, 4, fp);
		fseek(fp, 4, SEEK_SET);
		int32_t chunkSize;
		fread(&chunkSize, 4, 1, fp);
		printf("pcm size:%d \n chunkSize = %d (36 + pcmSize)\n total - 40:%d\n", pcmSize, chunkSize,totalSize - 40);
		fclose(fp);

#endif // DEBUG

	}
    else
	{ 
		printf("Ready to process %s \n",argv[1]);
	}

	WaveData_t *wavData = wavRead(argv[1]);

	printWavInfo(*wavData);

     if (wavData != NULL)
         dumpPcmData(*wavData);

    return EXIT_SUCCESS;

}

WaveData_t* wavRead(char* fileName) {
    //
    FILE* filePtr = fopen(fileName, "r");
	int  dataPos = 0;
    if (filePtr == NULL) {
        perror("fopen() failed:");
		return NULL;
    } 
	else
	{
		/* Read header at simple mode */
		WaveHeader_t header;
		fread(&header, sizeof(header), 1, filePtr);

		if (DEBUG)
		{
			printf("debug\n");
			printHeaderInfo(header);
		}
		// Check if the file is of supported format.
		if (strncmp(header.chunkId, "RIFF", 4)|| strncmp(header.format, "WAVE", 4)) {
			perror("WAV ID not found,please input wav file.");
			return NULL;
		}
		else
		{
			if (strncmp(header.dataId, "data", 4)) {
				// the wav header is not a standard wav header.
				perror("DATA id not found,try to seek DATA id");
					
				// data id not found , turn into catching pcm 

				header.dataSize = getDataSize(filePtr);
				if (header.dataSize > 0)
				{
					// repaired wav header 
					char dataIdTmp[4] = { 'd','a','t','a' };
					memcpy(header.dataId, dataIdTmp, 4);

					dataPos = getDataPos(filePtr) + 4;

				}
				else
				{
					perror("Data size not found,please input wav/pcm file.");
					return NULL;
				}

			}
			else
			{
				/* Standard wav file,pcm start from 42 */
				dataPos = 42;
			}
		}
		// Initialize the data struct.
		WaveData_t *data = (WaveData_t*)malloc(sizeof(WaveData_t));
		data->sampleRate = header.sampleRate;
		data->bitsPerSample = header.bitsPerSample;
		data->size = header.dataSize;
		data->numChannels = header.numChannels;
		data->samplesPerChannel = data->size / (data->numChannels * data->bitsPerSample / 8.0f);

	
		// Read data.
		// ToDo: Add support for 24-32bit files.
		// 24bit samples are best converted to 32bits
		data->pcmData = (int16_t*)malloc(header.dataSize * sizeof(int16_t));
		fseek(filePtr, dataPos, SEEK_SET);
		fread(data->pcmData, header.dataSize, 1, filePtr);
		fclose(filePtr);
		return data;
	}
	return NULL;
}


int seekId(FILE* fp, char* id,size_t idSize, size_t startIndex, size_t endIndex){

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
    fseek(fp,0, startIndex);

    for (size_t n = startIndex; n < endIndex; n++)
    {
		/* read |idSize| bytes from FILE buffer */
        fseek(fp,n, startIndex);
        fread(header, idSize,1,fp);

		/* Check wheather the id read is correct or not */
        if(!strncmp(id,header,idSize)){
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

int  getDataPos(FILE* fp) {
	char id[4] = { 'd','a','t','a' };
	int  pos = seekId(fp, id, 4, 0, 60);
	if (pos > 0)
	{
		return pos;
	}
	else
	{
		return -1;
	}
}

int32_t getDataSize(FILE* fp) {

	int32_t dataSize = 0;				
	int pos = getDataPos(fp);
	if (pos > 0)
	{
		fseek(fp, pos+4, SEEK_SET);
		// read data size
		if (fread(&dataSize, sizeof(int32_t), 1, fp) < 1) {
			perror("read data size failed ");
			return -1;
		}
		return dataSize;
	}
	else
	{
		perror("data id not found");
		return -1;
	}

}
/*
    Dump data to pcm file
*/
void dumpPcmData(WaveData_t waveData) {
    // Dump data into a text file.
    FILE* outputFilePtr = fopen("output.pcm","wb+");
    if (outputFilePtr == NULL) {
        perror(" output pcm error");
    } else {
        fwrite(waveData.pcmData,waveData.size,1,outputFilePtr);
        fclose(outputFilePtr);
    }
}

/*
 *  Prints the wave header
 */
void printHeaderInfo(WaveHeader_t hdr) {
    char buf[5];
    printf("Header Info:\n");
    strncpy(buf, hdr.chunkId, 4);
    buf[4] = '\0';
    printf("    Chunk ID: %s\n",buf);
    printf("    Chunk Size: %d\n", hdr.chunkSize);
    strncpy(buf,hdr.format,4);
    buf[4] = '\0';
    printf("    Format: %s\n", buf);
    strncpy(buf,hdr.subchunkId,4);
    buf[4] = '\0';
    printf("    Sub-chunk ID: %s\n", buf);
    printf("    Sub-chunk Size: %d\n", hdr.subchunkSize);
    printf("    Audio Format: %d\n", hdr.audioFormat);
    printf("    Channel Count: %d\n", hdr.numChannels);
    printf("    Sample Rate: %d\n", hdr.sampleRate);
    printf("    Bytes per Second: %d\n", hdr.byteRate);
    printf("    Block alignment: %d\n", hdr.blockAlign);
    printf("    Bit depth: %d\n", hdr.bitsPerSample);
    strncpy(buf,hdr.dataId, 4);
    buf[4] = '\0';
    printf("    Data ID: %s\n", buf);
    printf("    Data Size: %d\n", hdr.dataSize);
}

void printWavInfo(WaveData_t hdr){
    printf("wav/pcm file info:\n");
    printf("    Duration(s):   %f \n",(float)hdr.size/(hdr.bitsPerSample/8.0)/hdr.numChannels/hdr.sampleRate);
    printf("    SampleRate(Hz): %d\n",hdr.sampleRate);		// 16k 32k 48k
    printf("    Channels:   %ld\n",hdr.numChannels);			// 1 2 8
    printf("    Bit depth(bit):  %d\n",hdr.bitsPerSample);		// 16
    printf("    number samples per channel:  %ld\n",hdr.samplesPerChannel);	
}
#endif
