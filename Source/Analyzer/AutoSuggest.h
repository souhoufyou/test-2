
#pragma once
#include "FeatureExtractor.h"

struct Suggestions
{
    float hpfHz = 90.0f;
    float deEsserHz = 6500.0f;
    float deEsserThresh = 12.0f;
    float deEsserRatio = 4.0f;
    float compThresh = -18.0f;
    float compRatio = 3.0f;
    float compAttackMs = 8.0f;
    float compReleaseMs = 80.0f;
    float satDrive = 4.0f;
    float targetRms = -18.0f;
    float limiterCeil = -0.5f;
};

class AutoSuggest
{
public:
    Suggestions suggest (const Features& f)
    {
        Suggestions s;
        if (f.spectralCentroid < 1500.0f) s.hpfHz = 110.0f;
        if (f.spectralCentroid < 1000.0f) s.hpfHz = 130.0f;

        if (f.sibilanceRatio > 0.18f) { s.deEsserThresh = 16.0f; s.deEsserRatio = 6.0f; }
        if (f.sibilanceRatio > 0.26f) { s.deEsserThresh = 20.0f; s.deEsserRatio = 8.0f; }
        if (f.spectralCentroid > 3000.0f) s.deEsserHz = 7500.0f;

        if (f.crestFactor > 8.0f) { s.compRatio = 4.0f; s.compThresh = -20.0f; }
        if (f.crestFactor > 10.0f) { s.compRatio = 6.0f; s.compThresh = -24.0f; }
        if (f.crestFactor < 5.0f) { s.compRatio = 2.0f; s.compThresh = -14.0f; }

        if (f.spectralCentroid > 3000.0f || f.sibilanceRatio > 0.2f) s.satDrive = 6.0f;
        if (f.spectralCentroid < 1500.0f) s.satDrive = 3.0f;

        s.targetRms = -18.0f;
        if (f.rms < -28.0f) s.targetRms = -16.0f;
        if (f.rms > -14.0f) s.targetRms = -20.0f;
        s.limiterCeil = -0.5f;
        return s;
    }
};
