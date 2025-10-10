// clang -lc -framework AudioToolbox NotePlayer.c -o NotePlayer

#include <string.h>
#include <math.h>
#include <unistd.h>
#include <AudioToolbox/AudioToolbox.h>

typedef struct SoundState {
    float toneFreq, volume;
    float sampleRate, frameOffset;
    float squareWaveSign;
}SoundState;



// Calculates and returns target octave frequency
//  root = 1 means down octaves
float frequencyChange(float base, int exp, int root) {
    if (exp <= 0)
        return base;
    
    if (root) {
        return frequencyChange((base / 2.0), (exp - 1), 1);
    }
    else {
        return frequencyChange((base * 2.0), (exp - 1), 0);
    }
}



void audioCallback(void *inUserData, AudioQueueRef queue, AudioQueueBufferRef buffer) {
    SoundState *soundState = (SoundState*)(inUserData);
    
    // fill buffer
    uint32_t framesToGen = buffer->mAudioDataBytesCapacity / 4;
    buffer->mAudioDataByteSize = framesToGen * 4;

    // calculate every up/down portion of each square wave
    float framesPerTransition = soundState->sampleRate / soundState->toneFreq;

    // sample to output at current state
    int16_t sample = 32767.f * soundState->squareWaveSign * soundState->volume;
    
    int16_t *bufferPos =(int16_t*)(buffer->mAudioData);
    float frameOffset = soundState->frameOffset;

    while (framesToGen) {
        // calculate rounded frames
        uint32_t frames;
        uint32_t needFrames = (uint32_t)(round(framesPerTransition - frameOffset));
        frameOffset -= framesPerTransition - needFrames;

        // if end of the buffer, place offset at location in wave and clip
        if (needFrames > framesToGen) {
            frameOffset += framesToGen;
            frames = framesToGen;
        }
        else
            frames = needFrames;
        framesToGen -= frames;

        // put sample in buffer
        for (int x = 0; x < frames; ++x) {
            *bufferPos++ = sample;
            *bufferPos++ = sample;
        }

        // flip sign of wave at end of period
        if (needFrames == frames)
            sample = -sample;
    }

    // save square wave state for next callback
    if (sample > 0)
        soundState->squareWaveSign = 1;
    else
        soundState->squareWaveSign = -1;
    soundState->frameOffset = frameOffset;

    AudioQueueEnqueueBuffer(queue, buffer, 0, 0);
}


int main(int argc, const char * argv[]) {\
    // Input handling
    if (argc < 4 || argc > 5) {
        printf("  Usage: ./NotePlayer <frequency> <octave difference> <up(0)/down(1)> <optional: volume(1-100)>\n");
        printf("Example: ./NotePlayer 261.6 1 0\n");
        return -1;
    }
    float inputFrequency = frequencyChange(atof(argv[1]), atoi(argv[2]), atoi(argv[3]));
    float inputVolume = 0.1; // default value
    float volume = 0.1;
    if (argc == 5) {
        inputVolume = atof(argv[4]);
        volume = 0.0049 * (100.0 / atof(argv[4]));
    }

    // Metadata for formatting audio
    AudioStreamBasicDescription audioStreamDesc =  {};
    audioStreamDesc.mSampleRate = 48000.0f;
    audioStreamDesc.mFormatID = kAudioFormatLinearPCM;
    audioStreamDesc.mFormatFlags = kLinearPCMFormatFlagIsBigEndian | kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
    audioStreamDesc.mBytesPerPacket = 4;
    audioStreamDesc.mFramesPerPacket = 1;
    audioStreamDesc.mBytesPerFrame = 4;
    audioStreamDesc.mChannelsPerFrame = 2;
    audioStreamDesc.mBitsPerChannel = 16;
    
    AudioQueueRef audioQueue = 0;
    AudioQueueBufferRef audioBuffers[2] ={};
    
    // our persistent state for sound playback
    SoundState soundState=  {};
    soundState.toneFreq = inputFrequency;
    soundState.volume = volume;
    soundState.sampleRate = audioStreamDesc.mSampleRate;
    soundState.squareWaveSign = 1;
    
    OSStatus err;

    // Print Metadata
    printf("Sample Rate: %.2f\n", soundState.sampleRate);
    printf("  Frequency: %.2f\n", soundState.toneFreq);
    printf("     Volume: %.2f\n", inputVolume);
    fflush(stdout);


    err = AudioQueueNewOutput(&audioStreamDesc, &audioCallback, &soundState, 0, 0, 0, &audioQueue);    
    if (!err) {
        // generate buffers holding at most 1/16th of a second of data
        uint32_t bufferSize = audioStreamDesc.mBytesPerFrame * (audioStreamDesc.mSampleRate / 16);
        err = AudioQueueAllocateBuffer(audioQueue, bufferSize, &(audioBuffers[0]));

        if (!err)
            err = AudioQueueAllocateBuffer(audioQueue, bufferSize, &(audioBuffers[1]));

        if (!err) {
            // prime buffers
            audioCallback(&soundState, audioQueue, audioBuffers[0]);
            audioCallback(&soundState, audioQueue, audioBuffers[1]);

            // enqueue
            AudioQueueEnqueueBuffer(audioQueue, audioBuffers[0], 0, 0);
            AudioQueueEnqueueBuffer(audioQueue, audioBuffers[1], 0, 0);

            // play indefinitly
            while(1)
                AudioQueueStart(audioQueue, 0);
        }
    }
}