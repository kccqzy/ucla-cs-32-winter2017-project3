#include "Actor.h"
#include "StudentWorld.h"
#include <cassert>

bool Actor::canMoveHere(Coord c) const {
    auto actors = m_sw.getActorsAt(c, IID_ROCK);
    if (actors.begin() != actors.end()) return false;
    return true;
}

int Actor::attemptConsumeAtMostFood(int maxEnergy) const {
    auto actorsHere = m_sw.getActorsAt(getCoord(), IID_FOOD);
    for (auto const& actor : actorsHere) return static_cast<Food&>(*actor.second).consumeAtMost(maxEnergy);
    return 0;
}

void Actor::addFoodHere(int howMuch) const {
    auto here = getCoord();
    auto actorsHere = m_sw.getActorsAt(here, IID_FOOD);
    for (auto const& actor : actorsHere) return static_cast<Food&>(*actor.second).increaseBy(howMuch);
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

void GrassHopper::consumeFoodAndMove() {
    if (int consumedFood = attemptConsumeAtMostFood(200)) { // Step 6 (baby) or 7 (adult)
        m_currentEnergy += consumedFood;
        if (randInt(0, 1)) { // Step 7 (baby) or 8 (adult)
            m_sleep = 2;
            return;
        }
    }
    if (!m_distance) { // Step 8 (baby) or 8 (adult)
        setDirection(static_cast<GraphObject::Direction>(randInt(up, left)));
        m_distance = randInt(2, 10);
    }
    auto next = nextLocation();
    if (canMoveHere(next)) { // Step 9 (baby) or 10 (adult)
        moveTo(next);
        --m_distance; // Step 11 (baby) or 12 (adult)
    } else {          // Step 10 (baby) or 11 (adult)
        m_distance = 0;
    }
    m_sleep = 2; // Step 12 (baby) or 13 (adult)
}

void BabyGrassHopper::doSomething() {
    if (!burnEnergyAndSleep()) return; // Step 1--4
    if (m_currentEnergy >= 1600) {     // Step 5
        addFoodHere(100);
        m_sw.insertActorAtEndOfTick<AdultGrassHopper>(getCoord());
        m_currentEnergy = 0;
    }
    consumeFoodAndMove(); // Steps 6--12
}

void AdultGrassHopper::doSomething() {
    if (!burnEnergyAndSleep()) return; // Step 1--4
    // TODO Step 5
    if (!randInt(0, 9)) { // Step 6
        auto openSquares = findOpenSquaresCenteredHere();
        if (!openSquares.empty()) {
            moveTo(openSquares[randInt(0, openSquares.size() - 1)]);
            m_sleep = 2;
            return;
        }
    }
    consumeFoodAndMove(); // Steps 7--13
}

std::vector<Coord> AdultGrassHopper::findOpenSquaresCenteredHere() const {
    int const radius = 10;
    std::vector<Coord> rv;
    int x0 = getX(), y0 = getY();
    int minX = std::max(1, x0 - radius), minY = std::max(1, x0 - radius);
    int maxX = std::min(VIEW_WIDTH - 2, x0 + radius), maxY = std::min(VIEW_HEIGHT - 2, y0 + radius);
    for (int x = minX; x <= maxX; ++x)
        for (int y = minY; y <= maxY; ++y)
            if ((x - x0) * (x - x0) + (y - y0) * (y - y0) <= radius * radius && canMoveHere({x, y}))
                rv.emplace_back(x, y);
    return rv;
}
