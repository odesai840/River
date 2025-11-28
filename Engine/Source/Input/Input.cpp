#include "Input.h"
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_scancode.h>
#include <algorithm>
#include <iostream>

namespace RiverCore {

Input::Input() {}

bool Input::IsKeyPressed(SDL_Scancode scancode) {
    // Get the current state of the keyboard
    int numKeys = 0;
    const bool* keybordState = SDL_GetKeyboardState(&numKeys);

    // Check that the scancode is between 0 and the highest-value keyboard key
    if (0 < scancode && scancode < numKeys) {
        // If the scancode is valid, return whether that key is pressed
        return keybordState[scancode];
    }

    // If the scancode is invalid, return false
    return false;
}

void Input::RegisterChord(const ChordDefinition& chord) {
    registeredChords.push_back(chord);
}

void Input::UpdateChords(const std::set<SDL_Scancode>& pressedKeys, float currentTime) {
    // Clear previous frame's detections
    detectedThisFrame.clear();

    // Track changes to key states
    for (SDL_Scancode key : pressedKeys) {
        if (currentlyPressed.find(key) == currentlyPressed.end()) {
            keyHistory.push_back({key, currentTime, true});
            currentlyPressed.insert(key);
        }
    }

    // Track key releases
    std::vector<SDL_Scancode> releasedKeys;
    for (SDL_Scancode key : currentlyPressed) {
        if (pressedKeys.find(key) == pressedKeys.end()) {
            releasedKeys.push_back(key);
        }
    }
    for (SDL_Scancode key : releasedKeys) {
        keyHistory.push_back({key, currentTime, false});
        currentlyPressed.erase(key);
    }

    // Clean old history
    UpdateKeyHistory(currentTime);

    // Detect chords
    DetectSimultaneousChords(currentTime);
    DetectSequenceChords(currentTime);
}

void Input::DetectSimultaneousChords(float currentTime) {
    for (const ChordDefinition& chord : registeredChords) {
        if (chord.type != ChordType::SIMULTANEOUS) continue;

        // Check if all required keys are currently pressed
        bool allPressed = true;
        for (SDL_Scancode key : chord.keys) {
            if (currentlyPressed.find(key) == currentlyPressed.end()) {
                allPressed = false;
                break;
            }
        }

        if (allPressed) {
            // Verify they were pressed within the simultaneous window
            bool simultaneous = AreKeysSimultaneous(chord.keys, chord.simultaneousWindow);
            if (simultaneous) {
                if (activeChords.find(chord.name) == activeChords.end()) {
                    detectedThisFrame.push_back(chord.name);
                    activeChords.insert(chord.name);
                }
            }
        } else {
            // Chord no longer active
            activeChords.erase(chord.name);
        }
    }
}

void Input::DetectSequenceChords(float currentTime) {
    for (const ChordDefinition& chord : registeredChords) {
        if (chord.type != ChordType::SEQUENCE) continue;

        if (IsSequenceInHistory(chord.keys, chord.maxTimeBetweenPresses, currentTime)) {
            // Only trigger once per sequence
            if (activeChords.find(chord.name) == activeChords.end()) {
                detectedThisFrame.push_back(chord.name);
                activeChords.insert(chord.name);
            }
        } else {
            activeChords.erase(chord.name);
        }
    }
}

void Input::UpdateKeyHistory(float currentTime) {
    // Remove old key history
    while (!keyHistory.empty() &&
           (currentTime - keyHistory.front().timestamp) > maxHistoryDuration) {
        keyHistory.pop_front();
    }
}

bool Input::AreKeysSimultaneous(const std::vector<SDL_Scancode>& keys, float window) const {
    if (keys.empty()) return false;

    // Find the press events for these keys
    std::vector<float> pressTimes;
    for (SDL_Scancode key : keys) {
        for (auto it = keyHistory.rbegin(); it != keyHistory.rend(); ++it) {
            if (it->key == key && it->isPressed) {
                pressTimes.push_back(it->timestamp);
                break;
            }
        }
    }

    if (pressTimes.size() != keys.size()) return false;
    
    float minTime = *std::min_element(pressTimes.begin(), pressTimes.end());
    float maxTime = *std::max_element(pressTimes.begin(), pressTimes.end());

    return (maxTime - minTime) <= window;
}

bool Input::IsSequenceInHistory(const std::vector<SDL_Scancode>& keys,
                                  float maxTime, float currentTime) const {
    if (keys.empty()) return false;

    // Find the sequence of key presses in history
    std::vector<float> pressTimes;
    size_t keyIndex = 0;

    for (const KeyPressEvent& event : keyHistory) {
        if (keyIndex >= keys.size()) break;

        if (event.isPressed && event.key == keys[keyIndex]) {
            pressTimes.push_back(event.timestamp);
            keyIndex++;
        }
    }
    
    if (pressTimes.size() != keys.size()) return false;

    // Check timing between presses
    for (size_t i = 1; i < pressTimes.size(); i++) {
        float timeBetween = pressTimes[i] - pressTimes[i-1];
        if (timeBetween > maxTime) return false;
    }

    // Check that the last key was pressed recently
    return (currentTime - pressTimes.back()) <= maxTime;
}

std::vector<std::string> Input::GetDetectedChords() const {
    return detectedThisFrame;
}

void Input::ClearDetectedChords() {
    detectedThisFrame.clear();
}

bool Input::IsChordActive(const std::string& chordName) const {
    return activeChords.find(chordName) != activeChords.end();
}

}
