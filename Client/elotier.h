#ifndef ELOTIER_H
#define ELOTIER_H

enum class EloTier {
    BEGINNER = 0,     // 0-800
    INTERMEDIATE = 1,  // 801-1600
    ADVANCED = 2,     // 1601-2000
    EXPERT = 3,       // 2001-2400
    MASTER = 4        // 2400+
};

inline EloTier getEloTier(int elo) {
    if (elo <= 800) return EloTier::BEGINNER;
    if (elo <= 1600) return EloTier::INTERMEDIATE;
    if (elo <= 2000) return EloTier::ADVANCED;
    if (elo <= 2400) return EloTier::EXPERT;
    return EloTier::MASTER;
}

#endif // ELOTIER_H 