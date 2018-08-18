/* 
Part of this code is taken from the RtAudio examples by thestk:
https://github.com/thestk/rtaudio
 */
#include "RtAudio.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <math.h>

// Platform-dependent sleep routines.
#if defined(__WINDOWS_ASIO__) || defined(__WINDOWS_DS__) || defined(__WINDOWS_WASAPI__)
#include <windows.h>
#define SLEEP(milliseconds) Sleep((DWORD)milliseconds)
#else // Unix variants
#include <unistd.h>
#define SLEEP(milliseconds) usleep((unsigned long)(milliseconds * 1000.0))
#endif

#define PI 3.14159265

#define BASE_RATE 0.005
#define TIME 1.0

RtAudio dac;
RtAudio adac;
unsigned int channels, bufferFrames = 512, fs, oDevice = 0, iDevice = 0, iOffset = 0, oOffset = 0;
float *data = (float *)calloc(channels, sizeof(float));
float bufferBytes;
float *mybuffer = (float *)calloc(channels, sizeof(float));

void usage(void)
{
    // Error function in case of incorrect command-line
    // argument specifications
    std::cout << "\nuseage: testall N fs <iDevice> <oDevice> <iChannelOffset> <oChannelOffset>\n";
    std::cout << "    where N = number of channels,\n";
    std::cout << "    fs = the sample rate,\n";
    std::cout << "    iDevice = optional input device to use (default = 0),\n";
    std::cout << "    oDevice = optional output device to use (default = 0),\n";
    std::cout << "    iChannelOffset = an optional input channel offset (default = 0),\n";
    std::cout << "    and oChannelOffset = optional output channel offset (default = 0).\n\n";
    exit(0);
}

// Interleaved buffers
int sawi(void *outputBuffer, void * /*inputBuffer*/, unsigned int nBufferFrames,
         double /*streamTime*/, RtAudioStreamStatus status, void * /*userData*/)
{
    unsigned int i, j;
    extern unsigned int channels;
    float *buffer = (float *)outputBuffer;
    float *lastValues = (float *)data;
    if (status)
        std::cout << "Stream underflow detected!" << std::endl;

    for (i = 0; i < nBufferFrames; i++)
    {
        for (j = 0; j < channels; j++)
        {
            *buffer++ = (float)lastValues[j];
            lastValues[j] += BASE_RATE * (j + 1 + (j * 0.1));
            if (lastValues[j] >= 1.0)
                lastValues[j] -= 2.0;
        }
    }
    mybuffer = (float *)outputBuffer;
    return 0;
}

// Non-interleaved buffers (not used)
int sawni(void *outputBuffer, void * /*inputBuffer*/, unsigned int nBufferFrames,
          double /*streamTime*/, RtAudioStreamStatus status, void * /*userData*/)
{
    unsigned int i, j;
    extern unsigned int channels;
    double *buffer = (double *)outputBuffer;
    double *lastValues = (double *)data;

    if (status)
        std::cout << "Stream underflow detected!" << std::endl;

    double increment;
    for (j = 0; j < channels; j++)
    {
        increment = BASE_RATE * (j + 1 + (j * 0.1));
        for (i = 0; i < nBufferFrames; i++)
        {
            *buffer++ = (double)lastValues[j];
            lastValues[j] += increment;
            if (lastValues[j] >= 1.0)
                lastValues[j] -= 2.0;
        }
    }
    return 0;
}

int inout(void *outputBuffer, void *inputBuffer, unsigned int /*nBufferFrames*/,
          double /*streamTime*/, RtAudioStreamStatus status, void *data)
{
    // Since the number of input and output channels is equal, we can do
    // a simple buffer copy operation here.
    if (status)
        std::cout << "Stream over/underflow detected." << std::endl;

    float *bytes = (float *)data;
    memcpy(outputBuffer, inputBuffer, *bytes);
    mybuffer = (float *)inputBuffer;
    return 0;
}

// map a value proportionally
float map(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void cleanUp()
{
    if (dac.isStreamOpen())
        dac.closeStream();
    if (adac.isStreamOpen())
        adac.closeStream();
}

bool playSaw(int argc, char *argv[])
{
    // minimal command-line checking
    if (argc < 3 || argc > 7)
        usage();

    if (dac.getDeviceCount() < 1)
    {
        std::cout << "\nNo audio devices found!\n";
        exit(1);
    }

    channels = (unsigned int)atoi(argv[1]);
    fs = (unsigned int)atoi(argv[2]);
    if (argc > 4)
        oDevice = (unsigned int)atoi(argv[4]);
    if (argc > 6)
        oOffset = (unsigned int)atoi(argv[6]);

    // Let RtAudio print messages to stderr.
    dac.showWarnings(true);

    // Set our stream parameters for output only.
    RtAudio::StreamParameters oParams, iParams;
    oParams.deviceId = oDevice;
    oParams.nChannels = channels;
    oParams.firstChannel = oOffset;

    if (oDevice == 0)
        oParams.deviceId = dac.getDefaultOutputDevice();

    RtAudio::StreamOptions options;
    options.flags = RTAUDIO_HOG_DEVICE;
    options.flags = RTAUDIO_NONINTERLEAVED;

    // Test non-interleaved functionality
    try
    {
        dac.openStream(&oParams, NULL, RTAUDIO_FLOAT32, fs, &bufferFrames, &sawi, (void *)&bufferBytes, &options);
        std::cout << "\nStream latency = " << dac.getStreamLatency() << std::endl;

        // Start the stream
        dac.startStream();
        std::cout << "\nPlaying Saw Wave ...\n";
    }
    catch (RtAudioError &e)
    {
        e.printMessage();
        return false;
    }

    return true;
}

bool playDuplex(int argc, char *argv[])
{
    // minimal command-line checking
    if (argc < 3 || argc > 7)
        usage();

    if (adac.getDeviceCount() < 1)
    {
        std::cout << "\nNo audio devices found!\n";
        exit(1);
    }

    channels = (unsigned int)atoi(argv[1]);
    fs = (unsigned int)atoi(argv[2]);
    if (argc > 3)
        iDevice = (unsigned int)atoi(argv[3]);
    if (argc > 4)
        oDevice = (unsigned int)atoi(argv[4]);
    if (argc > 5)
        iOffset = (unsigned int)atoi(argv[5]);
    if (argc > 6)
        oOffset = (unsigned int)atoi(argv[6]);

    // Let RtAudio print messages to stderr.
    adac.showWarnings(true);

    // Set our stream parameters for output and input.
    bufferFrames = 512;
    RtAudio::StreamParameters oParams, iParams;
    oParams.deviceId = oDevice;
    oParams.nChannels = channels;
    oParams.firstChannel = oOffset;
    iParams.deviceId = iDevice;
    iParams.nChannels = channels;
    iParams.firstChannel = iOffset;
    if (oDevice == 0)
        oParams.deviceId = adac.getDefaultOutputDevice();
    if (iDevice == 0)
        iParams.deviceId = adac.getDefaultInputDevice();

    RtAudio::StreamOptions options;
    options.flags = RTAUDIO_HOG_DEVICE;
    options.flags = RTAUDIO_NONINTERLEAVED;

    try
    {
        adac.openStream(&oParams, &iParams, RTAUDIO_FLOAT32, fs, &bufferFrames, &inout, (void *)&bufferBytes, &options);
        std::cout << "\nStream latency = " << adac.getStreamLatency() << std::endl;

        bufferBytes = bufferFrames * channels * 4;

        // Start the stream
        adac.startStream();
        std::cout << "\nRunning Audio In -> Out ...\n";
    }
    catch (RtAudioError &e)
    {
        e.printMessage();
        return false;
    }

    return true;
}