#ifndef ACTOR_H_
#define ACTOR_H_

#include "Compiler.h"
#include "GraphObject.h"
#include <cassert>
#include <tuple>
#include <utility>

typedef std::tuple<int, int> Coord;

class StudentWorld;

class Actor : public GraphObject {
private:
    StudentWorld& m_sw;
    int m_iid;

protected:
    Actor(StudentWorld& sw, int iid, Coord c, Direction dir, unsigned depth)
      : GraphObject(iid, std::get<0>(c), std::get<1>(c), dir, depth), m_sw(sw), m_iid(iid) {}
    bool canMoveHere(Coord c) const;
    Coord nextLocation() const {
        switch (getDirection()) {
        case Direction::none: assert(false && "Unexpected direction: none");
        case Direction::up: return std::make_tuple(getX(), getY() + 1);
        case Direction::down: return std::make_tuple(getX(), getY() - 1);
        case Direction::left: return std::make_tuple(getX() - 1, getY());
        case Direction::right: return std::make_tuple(getX() + 1, getY());
        }
    }
    void moveTo(Coord c) { GraphObject::moveTo(std::get<0>(c), std::get<1>(c)); }
    int attemptConsumeAtMostFood(int maxEnergy) const;
    void addFoodHere(int howMuch) const;
    void addPheromoneHere(int type) const;
    static auto randomDirection() { return static_cast<Direction>(randInt(up, left)); }
    Coord getCoord() const { return std::make_tuple(getX(), getY()); }
    StudentWorld& sw() const { return m_sw; }

public:
    virtual void doSomething() = 0;
    int iid() const { return m_iid; }
    std::tuple<int, int, int> getKey() const { return std::make_tuple(getX(), getY(), iid()); }
    virtual bool isDead() const { return false; }
    virtual void beStunned() {}
    virtual void bePoisoned() {}
    virtual void beBitten(int) {}
};

class Pebble final : public Actor {
public:
    Pebble(StudentWorld& sw, Coord c) : Actor(sw, IID_ROCK, c, right, 1) {}

private:
    virtual void doSomething() override {}
};

class PoolOfWater final : public Actor {
public:
    PoolOfWater(StudentWorld& sw, Coord c) : Actor(sw, IID_WATER_POOL, c, right, 2) {}

private:
    virtual void doSomething() override;
};

class Poison final : public Actor {
public:
    Poison(StudentWorld& sw, Coord c) : Actor(sw, IID_POISON, c, right, 2) {}

private:
    virtual void doSomething() override;
};

class EnergyHolder : public Actor {
private:
    int m_currentEnergy;

protected:
    int const& currentEnergy() const { return m_currentEnergy; }
    int& currentEnergy() { return m_currentEnergy; }
    template<typename... Args>
    EnergyHolder(int initialEnergy, Args&&... args)
      : Actor(std::forward<Args>(args)...), m_currentEnergy{initialEnergy} {}
    virtual bool isDead() const override {
        assert(m_currentEnergy >= 0);
        return !m_currentEnergy;
    }
};

class Food final : public EnergyHolder {
private:
    virtual void doSomething() override {}

public:
    Food(StudentWorld& sw, Coord c, int energy) : EnergyHolder(energy, sw, IID_FOOD, c, right, 2) {}
    void increaseBy(int howMuch) { currentEnergy() += howMuch; }
    int consumeAtMost(int howMuch) {
        int actualConsumed = std::min(howMuch, currentEnergy());
        currentEnergy() -= actualConsumed;
        return actualConsumed;
    }
};

class Pheromone final : public EnergyHolder {
public:
    Pheromone(StudentWorld& sw, Coord c, int type) : EnergyHolder(256, sw, typeToIID(type), c, right, 2) {}
    void increaseBy(int howMuch) { currentEnergy() = std::max(768, currentEnergy() + howMuch); }

private:
    static int typeToIID(int type) {
        assert(0 <= type && type < 4);
        static_assert(IID_PHEROMONE_TYPE0 + 1 == IID_PHEROMONE_TYPE1, "Unexpected IID_PHEROMONE_TYPE1 index");
        static_assert(IID_PHEROMONE_TYPE0 + 2 == IID_PHEROMONE_TYPE2, "Unexpected IID_PHEROMONE_TYPE2 index");
        static_assert(IID_PHEROMONE_TYPE0 + 3 == IID_PHEROMONE_TYPE3, "Unexpected IID_PHEROMONE_TYPE3 index");
        return IID_PHEROMONE_TYPE0 + type;
    }
    virtual void doSomething() override { --currentEnergy(); }
};

class Anthill final : public EnergyHolder {
public:
    Anthill(StudentWorld& sw, Coord c, int type, Compiler const& comp)
      : EnergyHolder(8999, sw, IID_ANT_HILL, c, right, 2), m_comp(comp), m_type(type) {
        assert(0 <= type && type < 4);
    }
    int getType() const { return m_type; }

private:
    Compiler const& m_comp;
    int m_type;
    virtual void doSomething() override;
};

class Insect : public EnergyHolder {
private:
    int m_sleep;
    bool m_hasBeenStunnedHere;

protected:
    Insect(int initialEnergy, StudentWorld& sw, int iid, Coord c)
      : EnergyHolder(initialEnergy, sw, iid, c, randomDirection(), 1), m_sleep(0), m_hasBeenStunnedHere(false) {}
    bool decrementEnergy(int howMuch) {
        currentEnergy() -= howMuch;
        assert(currentEnergy() >= 0);
        if (!currentEnergy()) addFoodHere(100);
        return currentEnergy();
    }
    bool burnEnergyAndSleep() {
        if (!decrementEnergy(1)) { // Step 1, 2
            return false;
        }
        if (m_sleep) { // Step 3, 4
            --m_sleep;
            return false;
        }
        return true;
    }
    void resetSleep() { m_sleep = 2; }
    void moveTo(Coord c) { // Overload not override. No virtual needed.
        assert(c != getCoord());
        Actor::moveTo(c);
        m_hasBeenStunnedHere = false;
    }
    std::vector<Insect*> findOtherInsectsHere() const;
    virtual void beStunned() override {
        if (!m_hasBeenStunnedHere) {
            m_hasBeenStunnedHere = true;
            m_sleep += 2;
        }
    }
    virtual void bePoisoned() override { decrementEnergy(std::min(150, currentEnergy())); }
    virtual void beBitten(int damage) override { decrementEnergy(std::min(damage, currentEnergy())); }
};

class Ant final : public Insect {
public:
    Ant(StudentWorld& sw, Coord c, int type, Compiler const& comp)
      : Insect(1500, sw, typeToIID(type), c), m_comp(comp), m_ic(0), m_rand(0), m_foodHeld(0), m_isBlocked(false),
        m_isBitten(false) {}

private:
    virtual void doSomething() override;
    Compiler const& m_comp;
    size_t m_ic;
    int m_rand, m_foodHeld;
    bool m_isBlocked, m_isBitten;
    static int typeToIID(int type) {
        assert(0 <= type && type < 4);
        static_assert(IID_ANT_TYPE0 + 1 == IID_ANT_TYPE1, "Unexpected IID_ANT_TYPE1 index");
        static_assert(IID_ANT_TYPE0 + 2 == IID_ANT_TYPE2, "Unexpected IID_ANT_TYPE2 index");
        static_assert(IID_ANT_TYPE0 + 3 == IID_ANT_TYPE3, "Unexpected IID_ANT_TYPE3 index");
        return IID_ANT_TYPE0 + type;
    }
    bool evalInstr();
    bool evalIf(Compiler::Condition cond) const;
    void moveTo(Coord c) { // Overload not override. No virtual needed.
        assert(c != getCoord());
        Insect::moveTo(c);
        m_isBitten = false;
    }
    int getType() const { return iid() - IID_ANT_TYPE0; }
};

class Grasshopper : public Insect {
private:
    int m_distance;

protected:
    template<typename... Args>
    Grasshopper(Args&&... args) : Insect(std::forward<Args>(args)...), m_distance(randInt(2, 10)) {}
    void consumeFoodAndMove();
};

class BabyGrasshopper final : public Grasshopper {
public:
    BabyGrasshopper(StudentWorld& sw, Coord c) : Grasshopper(500, sw, IID_BABY_GRASSHOPPER, c) {}

private:
    virtual void doSomething() override;
};

class AdultGrasshopper final : public Grasshopper {
public:
    AdultGrasshopper(StudentWorld& sw, Coord c) : Grasshopper(1600, sw, IID_ADULT_GRASSHOPPER, c) {}

private:
    virtual void doSomething() override;
    virtual void beStunned() override {}
    virtual void bePoisoned() override {}
    virtual void beBitten(int damage) override {
        Insect::beBitten(damage);
        if (!isDead() && randInt(0, 1)) {
            // Retaliate.
            auto insectsHere = findOtherInsectsHere();
            assert(!insectsHere.empty());
            static_cast<Actor*>(insectsHere[randInt(0, insectsHere.size() - 1)])->beBitten(50);
        }
    }
    std::vector<Coord> findOpenSquaresCenteredHere() const;
};

#endif // ACTOR_H_
