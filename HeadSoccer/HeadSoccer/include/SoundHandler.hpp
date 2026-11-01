#ifndef HEADSOCCER_SOUND_HANDLER_HPP
#define HEADSOCCER_SOUND_HANDLER_HPP

#include <cstddef>
#include <string_view>

enum class SoundEffect : std::size_t {
    Click,
    Dash,
    MatchEnd,
    Goal,
    BallImpact,
    PlayerKick,
    MatchStart,
    KoreaHit,
    AmericaHit,
    ArgentinaHit,
    FranceHit,
    Count
};

class SoundHandler {
public:
    static bool init();
    static void release();

    static void playMenuMusic();
    static void startMatch();
    static void finishMatch();
    static void stopMusic();
    static void playEffect(SoundEffect effect);
    static void playHitSound(std::string_view characterName);
};

#endif
