#include "Timeline.h"

namespace RiverCore {

void Timeline::SetTimeScale(float scale) {
    timeScale = scale;
}

float Timeline::GetTimeScale() const {
    return timeScale.load();
}

void Timeline::SetPaused(bool isPaused) {
    paused = isPaused;
}

bool Timeline::IsPaused() const {
    return paused.load();
}

void Timeline::IncreaseTimeScale() {
    float current = GetTimeScale();
    if (current == 0.5f) SetTimeScale(1.0f);
    else if (current == 1.0f) SetTimeScale(2.0f);
}

void Timeline::DecreaseTimeScale() {
    float current = GetTimeScale();
    if (current == 2.0f) SetTimeScale(1.0f);
    else if (current == 1.0f) SetTimeScale(0.5f);
}

float Timeline::CalculateEffectiveTime(float rawTime) const {
    if (paused.load()) return 0.0f;
    return rawTime * timeScale.load();
}

}