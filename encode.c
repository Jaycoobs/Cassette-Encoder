/**
	Convert files to an audio file which can be played into the cassette
	input of an Apple ][+ for transferring data.

	Output format is a single channel pcm file with U8 samples at a
	sample rate defined by SAMPLE_RATE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <math.h>

// Sample rate of the output
const size_t SAMPLE_RATE = 44100;

// Duration of the entry tone to write to the output in samples.
const size_t ENTRY_TONE_DURATION = SAMPLE_RATE * 106 / 100; // 10.6 secs

// Apple 2 recordings consist of sine waves playing at four different
// frequencies.
//
// These values are the period of each tone in samples.
const size_t ENTRY_TONE_PERIOD = SAMPLE_RATE * 1300 / 1E6; // 1300 usec
const size_t TAPE_IN_PERIOD = SAMPLE_RATE * 400 / 1E6;     //  400 usec
const size_t ZERO_PERIOD = SAMPLE_RATE * 500 / 1E6;        //  500 usec
const size_t ONE_PERIOD = SAMPLE_RATE * 1000 / 1E6;        // 1000 usec

// These are the change in the angle passed to sine per sample for each tone.
const double ENTRY_TONE_DELTA_THETA = M_PI * 2.0 / (double)ENTRY_TONE_PERIOD;
const double TAPE_IN_DELTA_THETA = M_PI * 2.0 / (double)TAPE_IN_PERIOD;
const double ZERO_DELTA_THETA = M_PI * 2.0 / (double)ZERO_PERIOD;
const double ONE_DELTA_THETA = M_PI * 2.0 / (double)ONE_PERIOD;

/**
	Writes samples to the given output file.
	Samples are formatted as unsigned bytes.

	out     - the output file
	samples - number of samples to write
	dt      - change in the angle passed to sine per sample
	t       - angle to start the sine wave at
*/
static inline void write_wave(FILE* out, size_t samples, double dt, double t) {
	for (size_t i = 0; i < samples; i++) {
		int sample = (int)(127.0 * sin(t) + 127.0);
		fputc(sample, out);
		t += dt;
	}
}

/**
	Writes the audio corresponding to the given byte to the output file.

	out  - the output file
	data - the byte
*/
static inline void write_byte(FILE* out, char data) {
	for (size_t i = 0; i < 8; i++) {
		// higher order bits are written first
		if (data & 0x80) {
			write_wave(out, ONE_PERIOD, ONE_DELTA_THETA, 0.0);
		} else {
			write_wave(out, ZERO_PERIOD, ZERO_DELTA_THETA, 0.0);
		}
		data <<= 1;
	}
}

/**
	Writes the audio to indicate the beginning of the data.

	out - the output file
*/
static inline void write_tape_in_tone(FILE* out) {
	write_wave(out, TAPE_IN_PERIOD / 2, TAPE_IN_DELTA_THETA, 0.0);
	write_wave(out, ZERO_PERIOD / 2, ZERO_DELTA_THETA, M_PI);
}

/**
	Writes the entry tone to the output.

	out - the output file
*/
static inline void write_entry_tone(FILE* out) {
	write_wave(out, ENTRY_TONE_DURATION, ENTRY_TONE_DELTA_THETA, 0.0);
}

int main(int argc, char** argv) {

	// Default to stdin and stdout
	FILE* infile = stdin;
	FILE* outfile = stdout;

	// ============== Argument parsing ======================

	for (size_t i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-o")) {
			i++;
			if (i > argc) {
				fprintf(stderr, "-o requires an argument");
				return 1;
			}
			outfile = fopen(argv[i], "w");
		} else {
			infile = fopen(argv[i], "r");
		}
	}

	if (!infile) {
		err(1, "Failed to open input file");
	}

	if (!outfile) {
		err(1, "Failed to open output file");
	}

	// ======================================================

	write_entry_tone(outfile);
	write_tape_in_tone(outfile);

	// the final byte of data is always an xor checksum starting at 0xFF
	char checksum = 0xFF;
	while (1) {
		char data = (char)fgetc(infile);

		if (feof(infile)) {
			break;
		}

		if (ferror(infile)) {
			err(1, "Failed to read input file");
		}

		checksum ^= data;

		write_byte(outfile, data);

		if (ferror(outfile)) {
			err(1, "Failed to write output file");
		}
	}
	write_byte(outfile, checksum);

	fclose(infile);
	fclose(outfile);

	return 0;
}
