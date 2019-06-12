#include <assert.h>
#include "wav_pcm.h"
#include <string.h>

int wav2pcm(FILE* ifp, FILE* ofp) {

	if (ifp == NULL || ofp == NULL){
		perror("[wav-pcm] Input / output stream is NULL\n");
		return -1;
	}

	fseek(ifp, 0, SEEK_SET);
	WaveData_t* in_wav = wav_read(ifp);

	if (in_wav != NULL ){
		fseek(ifp, in_wav->pcm_pos, SEEK_SET);
		write_to_stream(ifp, ofp, in_wav->pcm_size);

		if (in_wav->pcm_data != NULL){
			free(in_wav->pcm_data);
		}

		free(in_wav);
		fclose(ifp);
		fclose(ofp);
		return 0;

	}else{
		perror("[wav-pcm] Cannot open WAV file\n");
		return -1;
	}
}

int pcm2wav(FILE* ifp, FILE* ofp,size_t pcm_size,size_t sample_rate,size_t chs,size_t bit_depth) {
	// bit_depth : bits per samples
	if (ifp == NULL || ofp == NULL){
		perror("[wav-pcm] Input / output stream is NULL\n");
		return -1;
	}

	size_t cur_pos = ftell(ifp);
	fseek(ifp, 0, SEEK_END);
	size_t end_pos = ftell(ifp);

	if (end_pos - cur_pos != pcm_size){
		perror("[wav-pcm] Input stream size unequals to |pcm_size|.\n");
		pcm_size = end_pos - cur_pos;
	}

	fseek(ifp, cur_pos, SEEK_SET);
	write_header(ofp, pcm_size, chs, sample_rate, bit_depth);
	write_to_stream(ifp, ofp, pcm_size);
	fclose(ifp);
	fclose(ofp);
	return 0;

}

int wav2pcm_unittest(char* file_in, char* file_out) {
	FILE* ifp = NULL;
	FILE* ofp = NULL;
	ifp = fopen(file_in, "rb+");
	ofp = fopen(file_out, "wb+");

	if (ifp == NULL || ofp == NULL) {
		perror("Cannot open input / output files.");
		return -1;
	}

	wav2pcm(ifp, ofp);
	// TODO: Check the pcm data between in_wav and out_pcm

	fclose(ifp);
	fclose(ofp);
	return 0;
}
int pcm2wav_unittest(char* file_in, char* file_out) {
	// input wav output wav
	FILE* ifp = NULL;
	FILE* ofp = NULL;
	ifp = fopen(file_in, "rb+");
	ofp = fopen(file_out, "wb+");

	if (ifp == NULL || ofp == NULL){
		perror("Cannot open input / output files.");
		return -1;
	}

	WaveData_t* in_wav = wav_read(ifp);

	if (in_wav == NULL){
		perror("Cannot read wav file.");
		fclose(ifp);
		fclose(ofp);
		return -1;
	}

	fseek(ifp, in_wav->pcm_pos, SEEK_SET);
	pcm2wav(ifp, ofp, in_wav->pcm_size, in_wav->num_channels, in_wav->sample_rate, in_wav->bits_per_sample);
	// TODO: Check whether the output wav file is correct or not.

	if (in_wav->pcm_data != NULL) {
		free(in_wav->pcm_data);
		in_wav->pcm_data = NULL;
	}

	free(in_wav);
	fclose(ifp);
	fclose(ofp);
	return 0;
}
void usage() {
	printf("Wav pcm usage:\n");
	printf("\t./wavpcm wavFile pcmFile\n");
	printf("\t./wavpcm pcmFile wavFile sample_rate num_channels bits_per_sample\n");
}
int main(int argc, char* argv[]) {
	//argv[1]
	char file_in[256];
	char file_out[256];
	FILE* ifp = NULL; 
	FILE* ofp = NULL;

	if (argc == 1) {
		usage();
#ifdef WIN32
		// debug mode in win32
		strcpy(file_in, "dukou_noReverb.wav");
		strcpy(file_out, "convert.pcm");
		wav2pcm_unittest(file_in, file_out);

#endif // WIN32

	}else{
		if (argc == 3){
			printf("wav2pcm: %s to %s\n", argv[1], argv[2]);
			wav2pcm_unittest(argv[1], argv[2]);
			return 0;

		}else{

			printf("Pcm2wav:\n");
			size_t sample_rate = atoi(argv[3]);
			size_t num_channels = atoi(argv[4]);
			size_t bits_per_sample = atoi(argv[5]);
			ifp = fopen(argv[1], "rb+");
			ofp = fopen(argv[2], "wb+");
			fseek(ifp, 0, SEEK_END);
			size_t pcm_size = ftell(ifp);
			fseek(ifp, 0, SEEK_SET);
			printf("Pcm in: %s \nWav out: %s\nfs:%ld channels:%ld bitDepth:%ld\n", argv[1], argv[2], sample_rate, num_channels, bits_per_sample);
			pcm2wav(ifp, ofp, pcm_size, sample_rate, num_channels, bits_per_sample);
			return 0;
		}
	}
	return 0;
}