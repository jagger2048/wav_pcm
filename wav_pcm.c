#include "wav_pcm.h"

WaveData_t* wav_read(FILE* fp) {

	WaveData_t *wav_data = (WaveData_t*)malloc(sizeof(WaveData_t));

	if (wav_data == NULL){
		perror("[wav-pcm] Wav data create fail\n");
		return NULL;
	}

	if (fp == NULL) {
		perror("[wav-pcm] Wav file read failed");
		return NULL;
	}

	/* Read header at simple mode */
	WaveHeader_t header;
	fseek(fp, 0, SEEK_SET);
	fread(&header, sizeof(header), 1, fp);

	/* Check if the file is of supported format. */
	if (strncmp(header.chunk_id, "RIFF", 4) || strncmp(header.format, "WAVE", 4)) {
		perror("[wav-pcm] WAV ID not found,please input wav file ");
		return NULL;
	}

	if (strncmp(header.data_id, "data", 4)) {
		/* the wav header is not a standard wav header. */
		perror("[wav-pcm] DATA id not found in wav file,try to seek DATA id");
		char data_id_tmp[4] = { 'd','a','t','a' };
		int pos = seek_id(fp, data_id_tmp, 4, 0, 60);

		if (pos > 0){
			if (fread(&(header.data_size), sizeof(int32_t), 1, fp) < 1) {
				perror("[wav-pcm] Read wav_data size failed ");
				return NULL;
			}else{
				/* repaired wav header  */
				memcpy(header.data_id, data_id_tmp, 4);
				wav_data->pcm_pos = ftell(fp);
			}
		}else{
			perror("[wav-pcm] DATA id NOT FOUND, please input correct wav file.");
			return NULL;
		}
	}else{
		/* Standard wav file,pcm start from 42 */
		wav_data->pcm_pos = 42;
	}
	/* Initialize the wav_data struct. */
	wav_data->sample_rate = header.sample_rate;
	wav_data->bits_per_sample = header.bits_per_sample;
	wav_data->pcm_size = header.data_size;
	wav_data->num_channels = header.num_channels;
	wav_data->samples_per_ch = wav_data->pcm_size / (wav_data->num_channels * wav_data->bits_per_sample / 8.0f);
	wav_data->pcm_data = NULL;
#ifdef WIN32
	wav_data->pcm_data = (int16_t*)malloc(header.data_size * sizeof(int16_t));
	fseek(fp, wav_data->pcm_pos, SEEK_SET);
	if (wav_data->pcm_data != NULL){
		fread(wav_data->pcm_data, 1, wav_data->pcm_size, fp);
	}
#endif // WIN32

	return wav_data;
	return NULL;
}


int seek_id(FILE* fp, char* id, size_t id_size, size_t start_index, size_t end_index) {

	if (fp == NULL){
		perror("[wav-pcm] Input buffer is NULL");
		return -1;
	}
	if (end_index < start_index){
		perror("[wav-pcm] Interval error");
		return -1;
	}
	size_t found_index = 0;
	char header[60];
	fseek(fp, 0, start_index);
	size_t n = 0;

	for (n = start_index; n < end_index; n++){
		/* read |id_size| bytes from FILE buffer */
		fseek(fp, n, start_index);
		fread(header, id_size, 1, fp);

		/* Check wheather the id read is correct or not */
		if (!strncmp(id, header, id_size)) {
			found_index = ftell(fp);
			break;
		}

	}
	if (ftell(fp) == end_index - start_index){
		// need some test
		return -1;
	}


	return found_index - id_size;
}

/*
 *	Dump wav_data to pcm file
 */
void dump_pcm_data(WaveData_t wav_data) {
	// Dump wav_data into a text file.
	FILE* ofp = fopen("output.pcm", "wb+");
	if (ofp == NULL) {
		perror(" output pcm error");
	}else {
		if (wav_data.pcm_data != NULL){
			fwrite(wav_data.pcm_data, wav_data.pcm_size, 1, ofp);
		}else{
			perror(" pcm date not found");
		}
		fclose(ofp);
	}
}

/*
 *  Prints the wave header for debug
 */
void print_header_info(WaveHeader_t hdr) {
	char buf[5];
	printf("Header Info:\n");
	strncpy(buf, hdr.chunk_id, 4);
	buf[4] = '\0';
	printf("    Chunk ID: %s\n", buf);
	printf("    Chunk Size: %d\n", hdr.chunk_size);
	strncpy(buf, hdr.format, 4);
	buf[4] = '\0';
	printf("    Format: %s\n", buf);
	strncpy(buf, hdr.subchunk_id, 4);
	buf[4] = '\0';
	printf("    Sub-chunk ID: %s\n", buf);
	printf("    Sub-chunk Size: %d\n", hdr.subchunk_size);
	printf("    Audio Format: %d\n", hdr.audio_format);
	printf("    Channel Count: %d\n", hdr.num_channels);
	printf("    Sample Rate: %d\n", hdr.sample_rate);
	printf("    Bytes per Second: %d\n", hdr.byte_rate);
	printf("    Block alignment: %d\n", hdr.block_align);
	printf("    Bit depth: %d\n", hdr.bits_per_sample);
	strncpy(buf, hdr.data_id, 4);
	buf[4] = '\0';
	printf("    Data ID: %s\n", buf);
	printf("    Data Size: %d\n", hdr.data_size);
}

void print_wav_info(WaveData_t hdr) {
	printf("Wav file info:\n");
	printf("    Duration(s):   %f \n", (float)hdr.pcm_size / (hdr.bits_per_sample / 8.0) / hdr.num_channels / hdr.sample_rate);
	printf("    SampleRate(Hz): %d\n", hdr.sample_rate);				// 16k 32k 48k
	printf("    Channels:   %ld\n", hdr.num_channels);				// 1 2 8
	printf("    Bit depth(bit):  %d\n", hdr.bits_per_sample);			// 16
	printf("    number samples per channel:  %ld\n", hdr.samples_per_ch);
}

WaveHeader_t* create_wav_header(size_t num_channels, size_t samplerate, size_t bits_per_sample) {
	WaveHeader_t *header = NULL;
	header = (WaveHeader_t*)malloc(sizeof(WaveHeader_t));

	if (header == NULL){
		perror("Create wav header error.\n");
		return NULL;
	}

	strncpy(header->chunk_id, "RIFF", 4);
	header->chunk_size = 36;					// chunk_size = 4 + (8 + formatchunk.chunk_size) + (8 + pcm_size); 36+pcm size
	strncpy(header->format, "WAVE", 4);
	strncpy(header->subchunk_id, "fmt ", 4);
	header->subchunk_size = 16;				// 16 for standard wav file,18 for audition || fmt size
	header->audio_format = 1;				// pcm
	header->num_channels = num_channels;
	header->sample_rate = samplerate;
	header->bits_per_sample = bits_per_sample;
	header->byte_rate = header->sample_rate * header->num_channels * header->bits_per_sample / 8;
	header->block_align = header->num_channels * header->bits_per_sample / 8;
	strncpy(header->data_id, "data", 4);
	// header->data_size =  // pcm size
	return header;
}

int write_header(FILE* fo, size_t pcm_size, size_t num_channels, size_t sample_rate, size_t bits_per_sample) {
	if (fo == NULL){
		perror("Output file stream is NULL\n");
		return -1;
	}
	WaveHeader_t* out_header = NULL;
	out_header = create_wav_header(num_channels, sample_rate, bits_per_sample);
	if (out_header == NULL){
		perror("Output wav header create fail\n");
		return -1;
	}
	out_header->chunk_size = pcm_size + 36;
	out_header->data_size = pcm_size;
	size_t write_cnt = fwrite(out_header, 1, sizeof(WaveHeader_t), fo);
	if (write_cnt != sizeof(WaveHeader_t)){
		perror("Wave header write error\n");
		return -1;
	}

	return 0;
}

int write_to_stream(FILE* fi, FILE* fo, size_t write_cnt) {
	char data_tmp[2048];	// write buff
	size_t count = write_cnt / 2048;
	size_t write_tmp = 0;
	size_t read_tmp = 0;
	if (fo && fi){
		size_t n = 0;
		for (n = 0; n < count; n++){
			read_tmp = fread(data_tmp, 1, 2048, fi);
			write_tmp = fwrite(data_tmp, 1, 2048, fo);
			if (read_tmp != write_tmp){
				perror("Read size unequals write size.");
				return -1;
			}
		}
		if (write_cnt - count * 2048 != 0){
			fread(data_tmp, 1, write_cnt - count * 2048, fi);
			fwrite(data_tmp, 1, write_cnt - count * 2048, fo);
		}
		return write_cnt;
	}else{
		perror(" Input / output stream NULL\n");
		return -1;
	}
}