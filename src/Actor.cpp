#include "Actor.h"
#include "StudentWorld.h"

bool Actor::attemptMove(Coord c) const {
    auto actors = m_sw.getActorsAt(c);
    for (auto const& actor : actors) {
        if (actor.second->iid() == IID_ROCK) return false;
    }
    return true;
}
