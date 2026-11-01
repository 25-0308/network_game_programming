#include "../include/SoundHandler.hpp"
#include "../include/common.hpp"
#include "../third_party/miniaudio/miniaudio.h"

namespace {
    constexpr std::size_t voiceCount = 4;
    constexpr std::size_t effectCount = static_cast<std::size_t>(SoundEffect::Count);
    constexpr std::size_t noMusic = 2;
    constexpr float musicVolume = 0.5f;
    constexpr float hitSoundVolume = 0.75f;

    struct SoundVoice {
        ma_sound sound{};
        bool loaded = false;
    };

    struct EffectBank {
        std::array<SoundVoice, voiceCount> voices{};
        std::size_t nextVoice = 0;
    };

    struct AudioState {
        ma_engine engine{};
        bool initialized = false;
        std::array<SoundVoice, 2> music{};
        std::array<EffectBank, effectCount> effects{};
        std::size_t currentMusic = noMusic;
    };

    AudioState audio;

    constexpr std::array<const wchar_t*, 2> musicFiles = {
        L"bgm/Title5.wav", L"bgm/Play.wav"
    };

    // ENUM 순서랑 같에 배치
    constexpr std::array<const wchar_t*, effectCount> effectFiles = {
        L"sfx/Click.wav", L"sfx/Dash.wav", L"sfx/End2.wav", L"sfx/Goal1.wav",
        L"sfx/Kick.wav", L"sfx/Kick3.wav", L"sfx/Start3.wav",
        L"sfx/screams/Scream1.wav", L"sfx/screams/Scream4.wav",
        L"sfx/screams/Scream7.wav", L"sfx/screams/Scream12.wav"
    };

    void reportFailure(const wchar_t* message, const fs::path& path = {}) {
        std::wstring text = L"[SoundHandler] ";
        text += message;
        if (!path.empty()) {
            text += L": ";
            text += path.wstring();
        }
        text += L"\n";
        OutputDebugStringW(text.c_str());
    }

    fs::path soundDirectory() {
        const fs::path relative = fs::path(L"assets") / L"sounds";
        std::error_code error;
        if (fs::is_directory(relative, error)) {
            return relative;
        }

        std::array<wchar_t, 32768> executable{};
        const DWORD length = GetModuleFileNameW(nullptr, executable.data(),
            static_cast<DWORD>(executable.size()));
        if (length > 0 && length < executable.size()) {
            return fs::path(executable.data()).parent_path() / relative;
        }
        return relative;
    }

    bool loadSound(SoundVoice& voice, const fs::path& path) {
        const auto flags = MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION;
        if (ma_sound_init_from_file_w(&audio.engine, path.c_str(), flags,
            nullptr, nullptr, &voice.sound) != MA_SUCCESS) {
            reportFailure(L"Could not load WAV", path);
            return false;
        }
        voice.loaded = true;
        return true;
    }

    void stopEffect(SoundEffect effect) {
        for (auto& voice : audio.effects[static_cast<std::size_t>(effect)].voices) {
            if (voice.loaded) {
                ma_sound_stop(&voice.sound);
            }
        }
    }

    void playMusic(std::size_t index) {
        if (!audio.initialized || index >= audio.music.size()) {
            return;
        }
        auto& voice = audio.music[index];
        if (audio.currentMusic == index && voice.loaded && ma_sound_is_playing(&voice.sound)) {
            return; // 메뉴 화면이 바뀌어도 Title5 그대로
        }

        SoundHandler::stopMusic();
        if (!voice.loaded) {
            return;
        }
        ma_sound_seek_to_pcm_frame(&voice.sound, 0);
        if (ma_sound_start(&voice.sound) == MA_SUCCESS) {
            audio.currentMusic = index;
        }
    }
}

bool SoundHandler::init() {
    if (audio.initialized) {
        return true;
    }
    if (ma_engine_init(nullptr, &audio.engine) != MA_SUCCESS) {
        reportFailure(L"Could not initialize audio; continuing without sound");
        return false;
    }
    audio.initialized = true;
    const auto directory = soundDirectory();

    for (std::size_t i = 0; i < musicFiles.size(); ++i) {
        auto& voice = audio.music[i];
        if (loadSound(voice, directory / musicFiles[i])) {
            ma_sound_set_looping(&voice.sound, MA_TRUE);
            ma_sound_set_volume(&voice.sound, musicVolume);
        }
    }

    for (std::size_t i = 0; i < effectFiles.size(); ++i) {
        auto& bank = audio.effects[i];
        bank.nextVoice = 0;
        if (!loadSound(bank.voices[0], directory / effectFiles[i])) {
            continue;
        }
        for (std::size_t j = 1; j < bank.voices.size(); ++j) {
            auto& voice = bank.voices[j];
            voice.loaded = ma_sound_init_copy(&audio.engine, &bank.voices[0].sound,
                MA_SOUND_FLAG_NO_SPATIALIZATION, nullptr, &voice.sound) == MA_SUCCESS;
            if (!voice.loaded) {
                reportFailure(L"Could not allocate an effect voice", directory / effectFiles[i]);
            }
        }
    }
    return true;
}

void SoundHandler::release() {
    if (!audio.initialized) {
        return;
    }
    for (auto& bank : audio.effects) {
        for (std::size_t i = bank.voices.size(); i > 0; --i) {
            auto& voice = bank.voices[i - 1];
            if (voice.loaded) {
                ma_sound_uninit(&voice.sound);
                voice.loaded = false;
            }
        }
    }
    for (auto& voice : audio.music) {
        if (voice.loaded) {
            ma_sound_uninit(&voice.sound);
            voice.loaded = false;
        }
    }
    ma_engine_uninit(&audio.engine);
    audio.initialized = false;
    audio.currentMusic = noMusic;
}

void SoundHandler::playMenuMusic() {
    stopEffect(SoundEffect::MatchEnd);
    playMusic(0);
}

void SoundHandler::startMatch() {
    stopEffect(SoundEffect::MatchEnd);
    playMusic(1); // 경기 BGM 시작 전 Title5 중지
    playEffect(SoundEffect::MatchStart);
}

void SoundHandler::finishMatch() {
    stopMusic();
    for (auto effect : {SoundEffect::Dash, SoundEffect::Goal, SoundEffect::BallImpact,
        SoundEffect::PlayerKick, SoundEffect::MatchStart, SoundEffect::KoreaHit,
        SoundEffect::AmericaHit, SoundEffect::ArgentinaHit, SoundEffect::FranceHit}) {
        stopEffect(effect);
    }
    playEffect(SoundEffect::MatchEnd);
}

void SoundHandler::stopMusic() {
    if (!audio.initialized) {
        return;
    }
    for (auto& voice : audio.music) {
        if (voice.loaded) {
            ma_sound_stop(&voice.sound);
        }
    }
    audio.currentMusic = noMusic;
}

void SoundHandler::playEffect(SoundEffect effect) {
    const auto index = static_cast<std::size_t>(effect);
    if (!audio.initialized || index >= audio.effects.size()) {
        return;
    }
    auto& bank = audio.effects[index];
    for (std::size_t offset = 0; offset < bank.voices.size(); ++offset) {
        const auto voiceIndex = (bank.nextVoice + offset) % bank.voices.size();
        auto& voice = bank.voices[voiceIndex];
        if (!voice.loaded || ma_sound_is_playing(&voice.sound)) {
            continue;
        }
        if (effect == SoundEffect::KoreaHit || effect == SoundEffect::AmericaHit
            || effect == SoundEffect::ArgentinaHit || effect == SoundEffect::FranceHit) {
            ma_sound_set_volume(&voice.sound, hitSoundVolume);
        }
        ma_sound_seek_to_pcm_frame(&voice.sound, 0);
        ma_sound_start(&voice.sound);
        bank.nextVoice = (voiceIndex + 1) % bank.voices.size();
        return;
    }
}

void SoundHandler::playHitSound(std::string_view characterName) {
    if (characterName == "korea") {
        playEffect(SoundEffect::KoreaHit);
    }
    else if (characterName == "america") {
        playEffect(SoundEffect::AmericaHit);
    }
    else if (characterName == "argentina") {
        playEffect(SoundEffect::ArgentinaHit);
    }
    else if (characterName == "france") {
        playEffect(SoundEffect::FranceHit);
    }
}
