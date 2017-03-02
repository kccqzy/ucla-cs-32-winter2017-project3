#include "Actor.h"
#include "StudentWorld.h"
#include <algorithm>
#include <cassert>
#include <string>

bool Actor::canMoveHere(Coord c) const { return sw().getActorsAt(c, IID_ROCK).empty(); }

int Actor::attemptConsumeAtMostFood(int maxEnergy) const {
    for (auto const& actor : sw().getActorsAt(getCoord(), IID_FOOD))
        return static_cast<Food&>(*actor.second).consumeAtMost(maxEnergy);
    return 0;
}

void Actor::addFoodHere(int howMuch) const {
    auto here = getCoord();
    for (auto const& actor : sw().getActorsAt(here, IID_FOOD))
        return static_cast<Food&>(*actor.second).increaseBy(howMuch);
    sw().insertActor<Food>(here, howMuch);
}

void Actor::addPheromoneHere(int type) const {
    auto here = getCoord();
    for (auto const& actor : sw().getActorsAt(here, IID_PHEROMONE_TYPE0 + type))
        return static_cast<Pheromone&>(*actor.second).increaseBy(256);
    sw().insertActor<Pheromone>(here, type);
}

void PoolOfWater::doSomething() {
    for (auto const& actor : sw().getActorsAt(getCoord())) actor.second->beStunned();
}

void Poison::doSomething() {
    for (auto const& actor : sw().getActorsAt(getCoord())) actor.second->bePoisoned();
}

void Anthill::doSomething() {
    if (!--currentEnergy()) return;
    if (int consumedFood = attemptConsumeAtMostFood(10000)) {
        currentEnergy() += consumedFood;
        return;
    }
    if (currentEnergy() >= 2000) {
        sw().insertActor<Ant>(getCoord(), m_type, m_comp);
        currentEnergy() -= 1500;
        sw().increaseAntCountForColony(m_type);
    }
}

void Grasshopper::consumeFoodAndMove() {
    if (int consumedFood = attemptConsumeAtMostFood(200)) { // Step 6 (baby) or 7 (adult)
        currentEnergy() += consumedFood;
        if (randInt(0, 1)) { // Step 7 (baby) or 8 (adult)
            return resetSleep();
        }
    }
    if (!m_distance) { // Step 8 (baby) or 8 (adult)
        setDirection(randomDirection());
        m_distance = randInt(2, 10);
    }
    auto next = nextLocation();
    if (canMoveHere(next)) { // Step 9 (baby) or 10 (adult)
        moveTo(next);
        --m_distance; // Step 11 (baby) or 12 (adult)
    } else {          // Step 10 (baby) or 11 (adult)
        m_distance = 0;
    }
    resetSleep(); // Step 12 (baby) or 13 (adult)
}

void BabyGrasshopper::doSomething() {
    if (!burnEnergyAndSleep()) return; // Step 1--4
    if (currentEnergy() >= 1600) {     // Step 5
        sw().insertActor<AdultGrasshopper>(getCoord());
        decrementEnergy(currentEnergy());
    }
    consumeFoodAndMove(); // Steps 6--12
}

void AdultGrasshopper::doSomething() {
    if (!burnEnergyAndSleep()) return; // Step 1--4
    if (!randInt(0, 2)) {              // Step 5
        auto insectsHere = findOtherInsectsHere();
        if (!insectsHere.empty()) {
            static_cast<Actor*>(insectsHere[randInt(0, insectsHere.size() - 1)])->beBitten(50);
            return resetSleep();
        }
    }
    if (!randInt(0, 9)) { // Step 6
        auto openSquares = findOpenSquaresCenteredHere();
        if (!openSquares.empty()) {
            moveTo(openSquares[randInt(0, openSquares.size() - 1)]);
            return resetSleep();
        }
    }
    consumeFoodAndMove(); // Steps 7--13
}

std::vector<Coord> AdultGrasshopper::findOpenSquaresCenteredHere() const {
    int const radius = 10;
    std::vector<Coord> rv;
    int x0 = getX(), y0 = getY();
    int minX = std::max(1, x0 - radius), minY = std::max(1, y0 - radius);
    int maxX = std::min(VIEW_WIDTH - 2, x0 + radius), maxY = std::min(VIEW_HEIGHT - 2, y0 + radius);
    for (int x = minX; x <= maxX; ++x)
        for (int y = minY; y <= maxY; ++y)
            if ((x != x0 || y != y0) && (x - x0) * (x - x0) + (y - y0) * (y - y0) <= radius * radius &&
                canMoveHere({x, y}))
                rv.emplace_back(x, y);
    return rv;
}

std::vector<Insect*> Insect::findOtherInsectsHere() const {
    auto actorsHere = sw().getActorsAt(getCoord());
    std::vector<Insect*> insectsHere; // TODO use better search
    for (auto const& actor : actorsHere) {
        int iid = actor.second->iid();
        if ((iid == IID_ADULT_GRASSHOPPER || iid == IID_BABY_GRASSHOPPER ||
             (iid >= IID_ANT_TYPE0 && iid <= IID_ANT_TYPE3)) &&
            actor.second.get() != this && !actor.second->isDead())
            insectsHere.emplace_back(static_cast<Insect*>(actor.second.get()));
    }
    return insectsHere;
}

void Ant::doSomething() {
    if (!burnEnergyAndSleep()) return; // Step 1--3
    for (int i = 0; i < 10; ++i)
        if (!evalInstr()) return; // Step 4
}

bool Ant::evalIf(Compiler::Condition cond) const {
    switch (cond) {
    case Compiler::Condition::last_random_number_was_zero: return m_rand == 0;
    case Compiler::Condition::i_am_carrying_food: return m_foodHeld > 0;
    case Compiler::Condition::i_am_hungry: return currentEnergy() <= 25;
    case Compiler::Condition::i_am_standing_with_an_enemy:
        for (auto const& actor : sw().getActorsAt(getCoord())) {
            int iid = actor.second->iid();
            if (iid == IID_ADULT_GRASSHOPPER || iid == IID_BABY_GRASSHOPPER ||
                (iid >= IID_ANT_TYPE0 && iid <= IID_ANT_TYPE3 && this->iid() != iid))
                return true;
        }
        return false;
    case Compiler::Condition::i_am_standing_on_my_anthill:
        for (auto const& actor : sw().getActorsAt(getCoord(), IID_ANT_HILL))
            if (static_cast<Anthill&>(*actor.second).getType() == this->getType()) return true;
        return false;
    case Compiler::Condition::i_am_standing_on_food: return !sw().getActorsAt(getCoord(), IID_FOOD).empty();
    case Compiler::Condition::i_smell_pheromone_in_front_of_me:
        for (auto const& actor : sw().getActorsAt(nextLocation())) {
            int iid = actor.second->iid();
            if (iid >= IID_PHEROMONE_TYPE0 && iid <= IID_PHEROMONE_TYPE3) return true;
        }
        return false;
    case Compiler::Condition::i_smell_danger_in_front_of_me:
        for (auto const& actor : sw().getActorsAt(nextLocation())) {
            int iid = actor.second->iid();
            if (iid == IID_POISON || iid == IID_ADULT_GRASSHOPPER || iid == IID_BABY_GRASSHOPPER ||
                (iid >= IID_ANT_TYPE0 && iid <= IID_ANT_TYPE3 && this->iid() != iid))
                return true;
        }
        return false;
    case Compiler::Condition::i_was_bit: return m_isBitten;
    case Compiler::Condition::i_was_blocked_from_moving: return m_isBlocked;
    case Compiler::Condition::invalid_if: assert(false && "invalid if condition in compiled Ant instructions");
    }
    assert(false && "unknown if condition in compiled Ant instructions");
}

bool Ant::evalInstr() {
    Compiler::Command cmd;
    if (!m_comp.getCommand(m_ic++, cmd)) {
        decrementEnergy(currentEnergy());
        return false;
    }
    switch (cmd.opcode) {
    case Compiler::Opcode::moveForward: {
        auto next = nextLocation();
        if (canMoveHere(next)) {
            moveTo(next);
            m_isBlocked = false;
        } else {
            m_isBlocked = true;
        }
        return false;
    }
    case Compiler::Opcode::eatFood: {
        int toEat = std::min(100, m_foodHeld);
        m_foodHeld -= toEat;
        currentEnergy() += toEat;
        return false;
    }
    case Compiler::Opcode::dropFood:
        if (m_foodHeld) {
            addFoodHere(m_foodHeld);
            m_foodHeld = 0;
        }
        return false;
    case Compiler::Opcode::bite: {
        auto insectsHere = findOtherInsectsHere();
        insectsHere.erase(
          std::remove_if(insectsHere.begin(), insectsHere.end(), [this](Insect* i) { return i->iid() == iid(); }),
          insectsHere.end());
        if (!insectsHere.empty()) static_cast<Actor*>(insectsHere[randInt(0, insectsHere.size() - 1)])->beBitten(15);
        return false;
    }
    case Compiler::Opcode::pickupFood:
        m_foodHeld += attemptConsumeAtMostFood(std::min(400, 1800 - m_foodHeld));
        return false;
    case Compiler::Opcode::emitPheromone: addPheromoneHere(getType()); return false;
    case Compiler::Opcode::faceRandomDirection: setDirection(randomDirection()); return false;
    case Compiler::Opcode::generateRandomNumber: {
        int operand1 = std::stoi(cmd.operand1);
        assert(operand1 >= 0);
        m_rand = operand1 ? randInt(0, operand1 - 1) : 0;
        return true;
    }
    case Compiler::Opcode::goto_command: m_ic = std::stoi(cmd.operand1); return true;
    case Compiler::Opcode::if_command:
        if (evalIf(static_cast<Compiler::Condition>(std::stoi(cmd.operand1)))) m_ic = std::stoi(cmd.operand2);
        return true;
    case Compiler::Opcode::rotateClockwise:
        setDirection(static_cast<Direction>((getDirection() - up + 1) % 4 + up));
        return false;
    case Compiler::Opcode::rotateCounterClockwise:
        setDirection(static_cast<Direction>((getDirection() - up + 3) % 4 + up));
        return false;
    case Compiler::Opcode::label: assert(false && "unresolved label in compiled Ant instructions");
    case Compiler::Opcode::invalid: assert(false && "invalid instruction in compiled Ant instructions");
    }
    assert(false && "unknown instruction in compiled Ant instructions");
}
