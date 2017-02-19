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

int Actor::attemptConsumeAtMostFood(int maxEnergy) {
    int totalFoodConsumed = 0;
    auto actorsHere = m_sw.getActorsAt(getCoord());
    for (auto const& actor : actorsHere) {
        if (actor.second->iid() == IID_FOOD) {
            Food& food = static_cast<Food&>(*actor.second);
            totalFoodConsumed += food.consumeAtMost(maxEnergy - totalFoodConsumed);
            assert(totalFoodConsumed <= maxEnergy);
            if (totalFoodConsumed == maxEnergy) break;
        }
    }
    return totalFoodConsumed;
}

void Anthill::doSomething() {
    if (!--m_currentEnergy) {
        m_dead = true;
        return;
    }
    int totalFoodConsumed = attemptConsumeAtMostFood(10000);
    if (totalFoodConsumed) {
        m_currentEnergy += totalFoodConsumed;
        return;
    }
    if (m_currentEnergy >= 2000) {
        // TODO Create new ant.
    }
}

void BabyGrassHopper::doSomething() {
    if (!--m_currentEnergy) { // Step 1, 2
        m_sw.insertActorAtEndOfTick<Food>(getCoord(), 100);
        m_dead = true;
        return;
    }
    if (m_sleep) { // Step 3, 4
        --m_sleep;
        return;
    }
    if (m_currentEnergy >= 1600) { // Step 5
        // TODO become AdultGrasshopper
    }
    if (attemptConsumeAtMostFood(200) && randInt(0, 1)) { // Step 6, 7
        m_sleep = 2;
        return;
    }
    if (!m_distance) { // Step 8
        setDirection(static_cast<GraphObject::Direction>(randInt(up, left)));
        m_distance = randInt(2, 10);
    }
    auto next = nextLocation();
    if (attemptMove(next)) { // Step 9
        moveTo(next);
        --m_distance; // Step 11
    } else {          // Step 10
        m_distance = 0;
    }
    m_sleep = 2; // Step 12
}
