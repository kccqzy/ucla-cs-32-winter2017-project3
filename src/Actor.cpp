#include "Actor.h"
#include "StudentWorld.h"
#include <cassert>

bool Actor::attemptMove(Coord c) const {
    auto actors = m_sw.getActorsAt(c);
    for (auto const& actor : actors) {
        if (actor.second->iid() == IID_ROCK) return false;
    }
    return true;
}

void Anthill::doSomething() {
    if (!--m_currentEnergy) {
        m_dead = true;
        return;
    }
    auto actorsHere = m_sw.getActorsAt(getCoord());
    int totalFoodConsumed = 0;
    for (auto const& actor : actorsHere) {
        if (actor.second->iid() == IID_FOOD) {
            Food& food = static_cast<Food&>(*actor.second);
            totalFoodConsumed += food.consumeAtMost(10000 - totalFoodConsumed);
            assert(totalFoodConsumed <= 10000);
            if (totalFoodConsumed == 10000) break;
        }
    }
    if (totalFoodConsumed) {
        m_currentEnergy += totalFoodConsumed;
        return;
    }
    if (m_currentEnergy >= 2000) {
        // TODO Create new ant.
    }
}
