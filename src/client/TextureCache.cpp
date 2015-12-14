////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "TextureCache.hpp"

////////////////////////////////////////////////////////////////////////////////

sf::Texture *TextureCache::load(sf::Texture *texture, const std::string &name) {
    if (!texture->loadFromFile(name)) {
        return nullptr;
    }
    texture->setSmooth(true);
    texture->setRepeated(true);
    texture->generateMipmap();
    return texture;
}

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////
