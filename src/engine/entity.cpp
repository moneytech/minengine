////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "entity.hpp"

////////////////////////////////////////////////////////////////////////////////

EntityID Entity::getID() const {
    return mID;
}

const Position &Entity::getPosition() const {
    return mPosition;
}

const Velocity &Entity::getVelocity() const {
    return mVelocity;
}

const Orientation &Entity::getOrientation() const {
    return mOrientation;
}

void Entity::update() {
    mPosition.x += mVelocity.x;
    mPosition.y += mVelocity.y;
    mPosition.z += mVelocity.z;
}

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

