#include <stdio.h>
#include <stdlib.h>
#include <lame/lame.h>
int main()
{
    const char *input_file="file.wav";
    const char *output_file="file.mp3";
    FILE *pcm = fopen(input_file, "rb");
	if (!pcm)
		exit(255);
    FILE *mp3 = fopen(output_file, "wb");
	if (!mp3)
		exit(255);
    size_t nread;
    int ret, nwrite;

// 1. Get lame version (OPTIONAL)
    printf("Using LAME v%s\n", get_lame_version());

    const int PCM_SIZE = 8192;
    const int MP3_SIZE = 8192;

    short pcm_buffer[PCM_SIZE * 2];
    unsigned char mp3_buffer[MP3_SIZE];

// 2. Initializing
    lame_t lame = lame_init();

// 3. Do some settings (OPTIONAL)
// lame_set_in_samplerate(lame, 48000);
    lame_set_VBR(lame, vbr_default);
// lame_set_VBR_quality(lame, 2);

// 4. Initialize parameters
    ret = lame_init_params(lame);
    if (ret < 0) {
        printf("Error occurred during parameters initializing. Code = %d\n",
               ret);
        return 1;
    }

    do {
        // Read PCM_SIZE of array
        nread = fread(pcm_buffer, 2 * sizeof(short), PCM_SIZE, pcm);
        if (nread != 0) {
            // 5. Encode
            int nsamples = nread;
            short buffer_l[nsamples];
            short buffer_r[nsamples];

            printf("nread = %zu\n", nread);
            printf("pcm_buffer.length = %zu\n", sizeof(pcm_buffer)/sizeof(short));

            int j = 0;
            int i = 0;
            for (i = 0; i < nsamples; i++) {
                buffer_l[i] = pcm_buffer[j++];
                buffer_r[i] = pcm_buffer[j++];

            }

            nwrite = lame_encode_buffer(lame, buffer_l, buffer_r, nread,
                                        mp3_buffer, MP3_SIZE);
        } else {
            // 6. Flush and give some final frames
            nwrite = lame_encode_flush(lame, mp3_buffer, MP3_SIZE);
        }

        if (nwrite < 0) {
            printf("Error occurred during encoding. Code = %d\n", nwrite);
            return 1;
        }

        fwrite(mp3_buffer, nwrite, 1, mp3);
    } while (nread != 0);

// 7. Write INFO tag (OPTIONAL)
// lame_mp3_tags_fid(lame, mp3);

// 8. Free internal data structures
    lame_close(lame);

    fclose(mp3);
    fclose(pcm);
    return 0;
}
