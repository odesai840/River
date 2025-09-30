#ifndef NETWORKMODE_H
#define NETWORKMODE_H

namespace RiverCore {

// Network mode enumeration
enum class NetworkMode {
    STANDALONE,  // Single-player, no networking
    CLIENT,      // Connected to a server
    SERVER       // Running as server
};

}

#endif
