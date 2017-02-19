#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include <cassert>
#include <utility>

typedef std::pair<int, int> Coord;

class StudentWorld;

class Actor : public GraphObject {
  protected:
    bool m_dead = false;
    StudentWorld& m_sw;
    template <typename... Args>
    Actor(StudentWorld& sw, Args&&... args) : GraphObject(std::forward<Args>(args)...), m_dead(false), m_sw(sw) {}
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

  public:
    virtual ~Actor() {}
    virtual void doSomething() = 0;
    virtual int iid() const = 0;
    Coord getCoord() const { return std::make_pair(getX(), getY()); }
};

class Pebble final : public Actor {
  public:
    Pebble(StudentWorld& sw, int x, int y) : Actor(sw, IID_ROCK, x, y, right, 1.0, 1) {}
    virtual void doSomething() override {}
    virtual int iid() const override { return IID_ROCK; }
};

class EnergyHolder : public Actor {
  protected:
    int m_currentEnergy;
    template <typename... Args>
    EnergyHolder(int initialEnergy, Args&&... args)
      : Actor(std::forward<Args>(args)...), m_currentEnergy{initialEnergy} {}
};

class BabyGrassHopper final : public EnergyHolder {
  public:
    BabyGrassHopper(StudentWorld& sw, int x, int y)
      : EnergyHolder(500, sw, IID_BABY_GRASSHOPPER, x, y, static_cast<GraphObject::Direction>(randInt(up, left)), 1.0,
                     1),
        m_distance(randInt(2, 10)), m_sleep(0) {}
    virtual void doSomething() override {
        --m_currentEnergy;
        if (m_sleep) {
            --m_sleep;
            return;
        }
        if (!m_distance) {
            setDirection(static_cast<GraphObject::Direction>(randInt(up, left)));
            m_distance = randInt(2, 10);
        }
        auto next = nextLocation();
        if (attemptMove(next)) {
            moveTo(next);
            --m_distance;
        } else {
            m_distance = 0;
        }
        m_sleep = 2;
    }
    virtual int iid() const override { return IID_BABY_GRASSHOPPER; }

  private:
    int m_distance, m_sleep;
};


#endif // ACTOR_H_
