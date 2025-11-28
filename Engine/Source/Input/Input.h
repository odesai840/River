#ifndef INPUT_H
#define INPUT_H

#include <SDL3/SDL_scancode.h>
#include <vector>
#include <deque>
#include <set>
#include <string>

namespace RiverCore {

// Chord type enumeration
enum class ChordType {
    SIMULTANEOUS,  // Multiple keys held simultaneously
    SEQUENCE       // Timed sequence of keypresses
};

// Chord definition structure
struct ChordDefinition {
    std::string name;                           // Chord identifier
    ChordType type;                             // Type of chord
    std::vector<SDL_Scancode> keys;             // Required keys in order
    float maxTimeBetweenPresses = 0.3f;         // Max time between keys (sequences)
    float simultaneousWindow = 0.05f;           // Tolerance for simultaneous detection
};

// Key press event for history tracking
struct KeyPressEvent {
    SDL_Scancode key;
    float timestamp;
    bool isPressed;
};

class Input {
public:
    Input();

    // Returns if a key was pressed
    bool IsKeyPressed(SDL_Scancode scancode);

    // Register a chord for detection
    void RegisterChord(const ChordDefinition& chord);

    // Update chord detector with current key states and timeline
    void UpdateChords(const std::set<SDL_Scancode>& pressedKeys, float currentTime);

    // Get list of chords detected this frame
    std::vector<std::string> GetDetectedChords() const;

    // Clear detected chords (call after processing)
    void ClearDetectedChords();

    // Check if a specific chord is currently active
    bool IsChordActive(const std::string& chordName) const;

private:
    // Chord detection state
    std::vector<ChordDefinition> registeredChords;    // Registered chord definitions
    std::deque<KeyPressEvent> keyHistory;             // Recent key events for sequence detection
    std::set<SDL_Scancode> currentlyPressed;          // Keys currently held down
    std::vector<std::string> detectedThisFrame;       // Chords detected this frame
    std::set<std::string> activeChords;               // Active chords (still being held)
    float maxHistoryDuration = 1.0f;                  // How long to keep history

    // Detection methods
    void DetectSimultaneousChords(float currentTime);
    void DetectSequenceChords(float currentTime);
    void UpdateKeyHistory(float currentTime);
    bool AreKeysSimultaneous(const std::vector<SDL_Scancode>& keys, float window) const;
    bool IsSequenceInHistory(const std::vector<SDL_Scancode>& keys, float maxTime, float currentTime) const;
};

}

#endif
