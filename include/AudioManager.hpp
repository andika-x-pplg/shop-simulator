#pragma once
#include "raylib.h"
#include <string>

enum class SoundEvent {
    CLICK,
    PICKUP,
    PUTDOWN,
    PURCHASE,
    CASH_REGISTER,
    NOTIFICATION,
    UPGRADE,
    DOOR
};

class AudioManager {
public:
    static AudioManager& Instance();

    void Init();
    void Close();

    void PlayEvent(SoundEvent event);
    void UpdateAmbience(float deltaTime, int currentHour);

    // Volume controls (0.0f - 1.0f)
    void SetMasterVolume(float vol);
    void SetSfxVolume(float vol);
    void SetAmbientVolume(float vol);

    float GetMasterVolume() const { return masterVolume; }
    float GetSfxVolume() const { return sfxVolume; }
    float GetAmbientVolume() const { return ambientVolume; }

private:
    AudioManager();
    ~AudioManager();

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    bool audioReady;
    float masterVolume;
    float sfxVolume;
    float ambientVolume;

    Sound sndClick;
    Sound sndPickup;
    Sound sndPutdown;
    Sound sndPurchase;
    Sound sndCashRegister;
    Sound sndNotification;
    Sound sndUpgrade;
    Sound sndDoor;

    Sound sndAmbienceMorning;
    Sound sndAmbienceDay;
    Sound sndAmbienceEvening;

    bool soundsGenerated;

    Sound GenerateSynthSound(int type);
    void GenerateProceduralSounds();
    void UnloadGeneratedSounds();
};
