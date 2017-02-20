#include "Actor.h"
#include "StudentWorld.h"
#include <cassert>

bool Actor::attemptMove(Coord c) const {
    auto actors = m_sw.getActorsAt(c, IID_ROCK);
    if (actors.begin() != actors.end()) return false;
    return true;
}

int Actor::attemptConsumeAtMostFood(int maxEnergy) const {
    int totalFoodConsumed = 0;
    auto actorsHere = m_sw.getActorsAt(getCoord(), IID_FOOD);
    for (auto const& actor : actorsHere) {
        Food& food = static_cast<Food&>(*actor.second);
        totalFoodConsumed += food.consumeAtMost(maxEnergy - totalFoodConsumed);
        assert(totalFoodConsumed <= maxEnergy);
        break;
    }
    return totalFoodConsumed;
}

void Actor::addFoodHere(int howMuch) const {
    auto here = getCoord();
    auto actorsHere = m_sw.getActorsAt(here, IID_FOOD);
    for (auto const& actor : actorsHere) {
        Food& food = static_cast<Food&>(*actor.second);
        food.increaseBy(howMuch);
        return;
    }
    m_sw.insertActorAtEndOfTick<Food>(here, howMuch);
}

void Anthill::doSomething() {
    if (!--m_currentEnergy) return;

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
        addFoodHere(100);
        return;
    }
    if (m_sleep) { // Step 3, 4
        --m_sleep;
        return;
    }
    if (m_currentEnergy >= 1600) { // Step 5
        addFoodHere(100);
        m_sw.insertActorAtEndOfTick<AdultGrassHopper>(getCoord());
        m_currentEnergy = 0;
    }
    if (int consumedFood = attemptConsumeAtMostFood(200)) { // Step 6
        m_currentEnergy += consumedFood;
        if (randInt(0, 1)) {    // Step 7
            m_sleep = 2;
            return;
        }
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

void AdultGrassHopper::doSomething() {
    if (!--m_currentEnergy) { // Step 1, 2
        addFoodHere(100);
        return;
    }
    if (m_sleep) { // Step 3, 4
        --m_sleep;
        return;
    }
    // TODO Step 5
    // TODO Step 6
    if (int consumedFood = attemptConsumeAtMostFood(200)) { // Step 6
        m_currentEnergy += consumedFood;
        if (randInt(0, 1)) {    // Step 7
            m_sleep = 2;
            return;
        }
    }
    if (!m_distance) { // Step 9
        setDirection(static_cast<GraphObject::Direction>(randInt(up, left)));
        m_distance = randInt(2, 10);
    }
    auto next = nextLocation();
    if (attemptMove(next)) { // Step 10
        moveTo(next);
        --m_distance; // Step 12
    } else {          // Step 11
        m_distance = 0;
    }
    m_sleep = 2; // Step 13

}
