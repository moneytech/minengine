////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "MusicCache.hpp"

////////////////////////////////////////////////////////////////////////////////

sf::Music *MusicCache::load(sf::Music *music, const std::string &name) {
    if (!music->openFromFile(name)) {
        return nullptr;
    }
    return music;
}

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

