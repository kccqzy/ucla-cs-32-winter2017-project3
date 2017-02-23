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
protected:
    StudentWorld& m_sw;
    Actor(StudentWorld& sw, int iid, Coord c, Direction dir, unsigned depth)
      : GraphObject(iid, std::get<0>(c), std::get<1>(c), dir, depth), m_sw(sw) {}
    bool canMoveHere(Coord c) const;
    Coord nextLocation() const {
        switch (getDirection()) {
        case Direction::none: assert(false && "Unexpected direction: none");
        case Direction::up: return std::make_pair(getX(), getY() + 1);
        case Direction::down: return std::make_pair(getX(), getY() - 1);
        case Direction::left: return std::make_pair(getX() - 1, getY());
        case Direction::right: return std::make_pair(getX() + 1, getY());
        }
    }
    void moveTo(Coord c) { GraphObject::moveTo(std::get<0>(c), std::get<1>(c)); }
    int attemptConsumeAtMostFood(int maxEnergy) const;
    void addFoodHere(int howMuch) const;

public:
    virtual ~Actor() {}
    virtual void doSomething() {}
    virtual int iid() const = 0;
    Coord getCoord() const { return std::make_tuple(getX(), getY()); }
    std::tuple<int, int, int> getKey() const { return std::make_tuple(getX(), getY(), iid()); }
    virtual bool isDead() const { return false; } // TODO are non energyholders always not dead?
};

class Pebble final : public Actor {
public:
    Pebble(StudentWorld& sw, Coord c) : Actor(sw, IID_ROCK, c, right, 1) {}
    virtual int iid() const override { return IID_ROCK; }
};

class EnergyHolder : public Actor {
protected:
    int m_currentEnergy;
    template<typename... Args>
    EnergyHolder(int initialEnergy, Args&&... args)
      : Actor(std::forward<Args>(args)...), m_currentEnergy{initialEnergy} {}

public:
    virtual bool isDead() const override { return !m_currentEnergy; }
};

class Food final : public EnergyHolder {
public:
    Food(StudentWorld& sw, Coord c, int energy) : EnergyHolder(energy, sw, IID_FOOD, c, right, 2) {}
    virtual int iid() const override { return IID_FOOD; }
    void increaseBy(int howMuch) { m_currentEnergy += howMuch; }
    int consumeAtMost(int howMuch) {
        if (howMuch < m_currentEnergy) {
            m_currentEnergy -= howMuch;
            return howMuch;
        } else {
            int rv = m_currentEnergy;
            m_currentEnergy = 0;
            return rv;
        }
    }
};

class Pheremone final : public EnergyHolder {
public:
    Pheremone(StudentWorld& sw, Coord c, int type)
      : EnergyHolder(256, sw, typeToIID(type), c, right, 2), m_type(type) {}

private:
    int m_type;
    static int typeToIID(int type) {
        static_assert(IID_PHEROMONE_TYPE0 + 1 == IID_PHEROMONE_TYPE1, "Unexpected IID_PHEROMONE_TYPE1 index");
        static_assert(IID_PHEROMONE_TYPE0 + 2 == IID_PHEROMONE_TYPE2, "Unexpected IID_PHEROMONE_TYPE2 index");
        static_assert(IID_PHEROMONE_TYPE0 + 3 == IID_PHEROMONE_TYPE3, "Unexpected IID_PHEROMONE_TYPE3 index");
        return IID_PHEROMONE_TYPE0 + type;
    }
    virtual int iid() const override { return typeToIID(m_type); }
    virtual void doSomething() override { --m_currentEnergy; }
};

class Anthill final : public EnergyHolder {
public:
    Anthill(StudentWorld& sw, Coord c, int type, Compiler const& comp)
      : EnergyHolder(8999, sw, IID_ANT_HILL, c, right, 2), m_comp(comp), m_type(type) {}
    int getType() const { return m_type; }

private:
    Compiler const& m_comp;
    int m_type;
    virtual int iid() const override { return IID_ANT_HILL; }
    virtual void doSomething() override;
};

class Insect : public EnergyHolder {
protected:
    Insect(int initialEnergy, StudentWorld& sw, int iid, Coord c)
      : EnergyHolder(initialEnergy, sw, iid, c, static_cast<GraphObject::Direction>(randInt(up, left)), 1), m_sleep(0) {
    }
    int m_sleep;
    bool burnEnergyAndSleep() {
        if (!--m_currentEnergy) { // Step 1, 2
            addFoodHere(100);
            return false;
        }
        if (m_sleep) { // Step 3, 4
            --m_sleep;
            return false;
        }
        return true;
    }
};

class Ant final : public Insect {
public:
    Ant(StudentWorld& sw, Coord c, int type, Compiler const& comp)
        : Insect(1500, sw, typeToIID(type), c), m_comp(comp), m_ic(0), m_type(type), m_rand(0), m_foodHeld(0), m_isBlocked(false) {}
    virtual int iid() const override { return typeToIID(m_type); }
    virtual void doSomething() override;

private:
    Compiler const& m_comp;
    size_t m_ic;
    int m_type, m_rand, m_foodHeld;
    bool m_isBlocked;
    static int typeToIID(int type) {
        static_assert(IID_ANT_TYPE0 + 1 == IID_ANT_TYPE1, "Unexpected IID_ANT_TYPE1 index");
        static_assert(IID_ANT_TYPE0 + 2 == IID_ANT_TYPE2, "Unexpected IID_ANT_TYPE2 index");
        static_assert(IID_ANT_TYPE0 + 3 == IID_ANT_TYPE3, "Unexpected IID_ANT_TYPE3 index");
        return IID_ANT_TYPE0 + type;
    }
    bool eval_instr();
    bool eval_if(Compiler::Condition cond) const;
};

class GrassHopper : public Insect {
protected:
    template<typename... Args>
    GrassHopper(Args&&... args) : Insect(std::forward<Args>(args)...), m_distance(randInt(2, 10)) {}
    int m_distance;
    void consumeFoodAndMove();
};

class BabyGrassHopper final : public GrassHopper {
public:
    BabyGrassHopper(StudentWorld& sw, Coord c) : GrassHopper(500, sw, IID_BABY_GRASSHOPPER, c) {}
    virtual void doSomething() override;
    virtual int iid() const override { return IID_BABY_GRASSHOPPER; }
};

class AdultGrassHopper final : public GrassHopper {
public:
    AdultGrassHopper(StudentWorld& sw, Coord c) : GrassHopper(1600, sw, IID_ADULT_GRASSHOPPER, c) {}
    virtual void doSomething() override;
    virtual int iid() const override { return IID_ADULT_GRASSHOPPER; }

private:
    std::vector<Coord> findOpenSquaresCenteredHere() const;
};

#endif // ACTOR_H_
