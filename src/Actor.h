#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include <cassert>
#include <utility>

typedef std::pair<int, int> Coord;

class StudentWorld;

class Actor : public GraphObject {
  protected:
    bool m_dead;
    StudentWorld& m_sw;
    Actor(StudentWorld& sw, int iid, Coord c, Direction dir, unsigned depth)
      : GraphObject(iid, c.first, c.second, dir, depth), m_dead(false), m_sw(sw) {}
    bool attemptMove(Coord c) const;
    Coord nextLocation() const {
        switch (getDirection()) {
        case Direction::none: assert(false && "Unexpected direction: none");
        case Direction::up: return std::make_pair(getX(), getY() + 1);
        case Direction::down: return std::make_pair(getX(), getY() - 1);
        case Direction::left: return std::make_pair(getX() - 1, getY());
        case Direction::right: return std::make_pair(getX() + 1, getY());
        }
    }
    void moveTo(Coord c) { GraphObject::moveTo(c.first, c.second); }
    int attemptConsumeAtMostFood(int maxEnergy);

  public:
    virtual ~Actor() {}
    virtual void doSomething() {}
    virtual int iid() const = 0;
    Coord getCoord() const { return std::make_pair(getX(), getY()); }
    bool isDead() const { return m_dead; }
};

class Pebble final : public Actor {
  public:
    Pebble(StudentWorld& sw, Coord c) : Actor(sw, IID_ROCK, c, right, 1) {}
    virtual int iid() const override { return IID_ROCK; }
};

class EnergyHolder : public Actor {
  protected:
    int m_currentEnergy;
    template <typename... Args>
    EnergyHolder(int initialEnergy, Args&&... args)
      : Actor(std::forward<Args>(args)...), m_currentEnergy{initialEnergy} {}
};

class Food final : public EnergyHolder {
  public:
    Food(StudentWorld& sw, Coord c, int energy) : EnergyHolder(energy, sw, IID_FOOD, c, right, 2) {}
    virtual int iid() const override { return IID_FOOD; }
    int consumeAtMost(int howMuch) {
        if (howMuch < m_currentEnergy) {
            m_currentEnergy -= howMuch;
            return howMuch;
        } else {
            int rv = m_currentEnergy;
            m_currentEnergy = 0;
            m_dead = true;
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
    static int typeToIID(int type) { return IID_PHEROMONE_TYPE0 + type; }
    virtual int iid() const override { return typeToIID(m_type); }
    virtual void doSomething() override {
        if (!--m_currentEnergy) m_dead = true;
    }
};

class Anthill final : public EnergyHolder {
  public:
    Anthill(StudentWorld& sw, Coord c, int type) : EnergyHolder(8999, sw, IID_ANT_HILL, c, right, 2), m_type(type) {}

  private:
    int m_type;
    virtual int iid() const override { return IID_ANT_HILL; }
    virtual void doSomething() override;
};

class BabyGrassHopper final : public EnergyHolder {
  public:
    BabyGrassHopper(StudentWorld& sw, Coord c)
      : EnergyHolder(500, sw, IID_BABY_GRASSHOPPER, c, static_cast<GraphObject::Direction>(randInt(up, left)), 1),
        m_distance(randInt(2, 10)), m_sleep(0) {}
    virtual void doSomething() override;
    virtual int iid() const override { return IID_BABY_GRASSHOPPER; }

  private:
    int m_distance, m_sleep;
};


#endif // ACTOR_H_
