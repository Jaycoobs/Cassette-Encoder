# Apple ][ Cassette Encoder

This is a simple program to convert regular files into audio files which
can be played into the cassette input on an Apple ][ to transfer data to it.

The output format is a single channel pcm file with U8 samples and a sample
rate of 44100 kHz.

## Compiling

```
cc -lm encode.c -o encode
```

## Using

```
./encode file -o file.pcm
```

If no input or output file is given, they default to `stdin` and `stdout`
respectively.

## Playing the Output

Since the output is raw data, you have to specify the format yourself.
You could convert to some other format with a program like audacity,
but I've been using `aplay`...

```
aplay -c1 -fU8 -r44100 file.pcm
```