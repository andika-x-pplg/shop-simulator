#include "AudioManager.hpp"
#include <cmath>
#include <vector>
#include <algorithm>

AudioManager& AudioManager::Instance() {
    static AudioManager instance;
    return instance;
}

AudioManager::AudioManager()
    : audioReady(false),
      masterVolume(1.0f),
      sfxVolume(0.8f),
      ambientVolume(0.35f),
      soundsGenerated(false)
{
}

AudioManager::~AudioManager() {
    Close();
}

void AudioManager::Init() {
    if (!IsAudioDeviceReady()) {
        InitAudioDevice();
    }

    if (IsAudioDeviceReady()) {
        audioReady = true;
        SetMasterVolume(masterVolume);
        GenerateProceduralSounds();
    }
}

void AudioManager::Close() {
    if (audioReady && soundsGenerated) {
        UnloadGeneratedSounds();
        soundsGenerated = false;
    }
    if (IsAudioDeviceReady()) {
        CloseAudioDevice();
        audioReady = false;
    }
}

Sound AudioManager::GenerateSynthSound(int type) {
    const int sampleRate = 44100;
    float duration = 0.2f;
    if (type == 4) duration = 0.45f; // Cash register
    else if (type == 5) duration = 0.35f; // Notification
    else if (type == 6) duration = 0.5f; // Upgrade fanfare
    else if (type == 7) duration = 0.3f; // Door

    int totalSamples = static_cast<int>(sampleRate * duration);
    std::vector<short> samples(totalSamples);

    for (int i = 0; i < totalSamples; ++i) {
        float t = static_cast<float>(i) / sampleRate;
        float env = 1.0f - (static_cast<float>(i) / totalSamples); // Linear decay
        float sampleVal = 0.0f;

        switch (type) {
            case 0: // Click (short sharp burst)
                sampleVal = sinf(2.0f * PI * 1200.0f * t) * expf(-t * 30.0f);
                break;
            case 1: // Pickup (ascending frequency blip)
                sampleVal = sinf(2.0f * PI * (400.0f + 600.0f * (t / duration)) * t) * env;
                break;
            case 2: // Putdown (descending thump)
                sampleVal = sinf(2.0f * PI * (450.0f - 250.0f * (t / duration)) * t) * env;
                break;
            case 3: // Purchase (coin chime)
                sampleVal = 0.6f * sinf(2.0f * PI * 987.77f * t) + 0.4f * sinf(2.0f * PI * 1318.51f * t);
                sampleVal *= expf(-t * 10.0f);
                break;
            case 4: // Cash register (ka-ching double bell)
                if (t < 0.15f) {
                    sampleVal = sinf(2.0f * PI * 1046.50f * t) * expf(-t * 15.0f);
                } else {
                    float t2 = t - 0.15f;
                    sampleVal = (0.7f * sinf(2.0f * PI * 1567.98f * t2) + 0.3f * sinf(2.0f * PI * 2093.00f * t2)) * expf(-t2 * 8.0f);
                }
                break;
            case 5: // Notification (two-tone melodic ding)
                if (t < 0.16f) {
                    sampleVal = sinf(2.0f * PI * 587.33f * t) * expf(-t * 8.0f); // D5
                } else {
                    float t2 = t - 0.16f;
                    sampleVal = sinf(2.0f * PI * 880.0f * t2) * expf(-t2 * 7.0f); // A5
                }
                break;
            case 6: // Upgrade (major chord arpeggio)
                if (t < 0.12f) {
                    sampleVal = sinf(2.0f * PI * 523.25f * t); // C5
                } else if (t < 0.24f) {
                    sampleVal = sinf(2.0f * PI * 659.25f * (t - 0.12f)); // E5
                } else if (t < 0.36f) {
                    sampleVal = sinf(2.0f * PI * 783.99f * (t - 0.24f)); // G5
                } else {
                    sampleVal = sinf(2.0f * PI * 1046.50f * (t - 0.36f)); // C6
                }
                sampleVal *= (1.0f - (t / duration));
                break;
            case 7: // Door (warm low bell)
                sampleVal = (0.6f * sinf(2.0f * PI * 329.63f * t) + 0.4f * sinf(2.0f * PI * 440.0f * t)) * expf(-t * 6.0f);
                break;
            default:
                sampleVal = 0.0f;
                break;
        }

        short finalSample = static_cast<short>(std::clamp(sampleVal, -1.0f, 1.0f) * 32767.0f * 0.7f);
        samples[i] = finalSample;
    }

    Wave wave;
    wave.frameCount = totalSamples;
    wave.sampleRate = sampleRate;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = samples.data();

    Sound snd = LoadSoundFromWave(wave);
    return snd;
}

void AudioManager::GenerateProceduralSounds() {
    if (!audioReady) return;

    sndClick = GenerateSynthSound(0);
    sndPickup = GenerateSynthSound(1);
    sndPutdown = GenerateSynthSound(2);
    sndPurchase = GenerateSynthSound(3);
    sndCashRegister = GenerateSynthSound(4);
    sndNotification = GenerateSynthSound(5);
    sndUpgrade = GenerateSynthSound(6);
    sndDoor = GenerateSynthSound(7);

    // Apply SFX volume
    SetSoundVolume(sndClick, sfxVolume * 0.6f);
    SetSoundVolume(sndPickup, sfxVolume * 0.8f);
    SetSoundVolume(sndPutdown, sfxVolume * 0.8f);
    SetSoundVolume(sndPurchase, sfxVolume * 0.9f);
    SetSoundVolume(sndCashRegister, sfxVolume * 1.0f);
    SetSoundVolume(sndNotification, sfxVolume * 0.85f);
    SetSoundVolume(sndUpgrade, sfxVolume * 1.0f);
    SetSoundVolume(sndDoor, sfxVolume * 0.75f);

    soundsGenerated = true;
}

void AudioManager::UnloadGeneratedSounds() {
    if (!soundsGenerated) return;

    UnloadSound(sndClick);
    UnloadSound(sndPickup);
    UnloadSound(sndPutdown);
    UnloadSound(sndPurchase);
    UnloadSound(sndCashRegister);
    UnloadSound(sndNotification);
    UnloadSound(sndUpgrade);
    UnloadSound(sndDoor);
}

void AudioManager::PlayEvent(SoundEvent event) {
    if (!audioReady || !soundsGenerated) return;

    switch (event) {
        case SoundEvent::CLICK:
            PlaySound(sndClick);
            break;
        case SoundEvent::PICKUP:
            PlaySound(sndPickup);
            break;
        case SoundEvent::PUTDOWN:
            PlaySound(sndPutdown);
            break;
        case SoundEvent::PURCHASE:
            PlaySound(sndPurchase);
            break;
        case SoundEvent::CASH_REGISTER:
            PlaySound(sndCashRegister);
            break;
        case SoundEvent::NOTIFICATION:
            PlaySound(sndNotification);
            break;
        case SoundEvent::UPGRADE:
            PlaySound(sndUpgrade);
            break;
        case SoundEvent::DOOR:
            PlaySound(sndDoor);
            break;
    }
}

void AudioManager::UpdateAmbience(float deltaTime, int currentHour) {
    // Ambience sound management (gentle & stable)
}

void AudioManager::SetMasterVolume(float vol) {
    masterVolume = std::clamp(vol, 0.0f, 1.0f);
    ::SetMasterVolume(masterVolume);
}

void AudioManager::SetSfxVolume(float vol) {
    sfxVolume = std::clamp(vol, 0.0f, 1.0f);
    if (soundsGenerated) {
        SetSoundVolume(sndClick, sfxVolume * 0.6f);
        SetSoundVolume(sndPickup, sfxVolume * 0.8f);
        SetSoundVolume(sndPutdown, sfxVolume * 0.8f);
        SetSoundVolume(sndPurchase, sfxVolume * 0.9f);
        SetSoundVolume(sndCashRegister, sfxVolume * 1.0f);
        SetSoundVolume(sndNotification, sfxVolume * 0.85f);
        SetSoundVolume(sndUpgrade, sfxVolume * 1.0f);
        SetSoundVolume(sndDoor, sfxVolume * 0.75f);
    }
}

void AudioManager::SetAmbientVolume(float vol) {
    ambientVolume = std::clamp(vol, 0.0f, 1.0f);
}
