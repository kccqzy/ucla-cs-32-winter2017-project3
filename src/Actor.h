#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"
#include <utility>

class Actor : public GraphObject {
  protected:
    using GraphObject::GraphObject;

  public:
    virtual ~Actor() {}
    virtual void doSomething() = 0;
};

class Pebble : public Actor {
  public:
    Pebble(int x, int y) : Actor(IID_ROCK, x, y, right, 1.0, 1) {}
    virtual void doSomething() {}
    virtual ~Pebble() {}
};

class EnergyHolder : public Actor {
  protected:
    int m_currentEnergy;
    template <typename... Args>
    EnergyHolder(int initialEnergy, Args&&... args)
      : Actor(std::forward<Args>(args)...), m_currentEnergy{initialEnergy} {}
};

class BabyGrassHopper : public EnergyHolder {
    // Need more class hierarchies!
  public:
    BabyGrassHopper(int x, int y)
      : EnergyHolder(500, IID_BABY_GRASSHOPPER, x, y, static_cast<GraphObject::Direction>(randInt(up, left)), 1.0, 1) {}
    virtual void doSomething() {}
    virtual ~BabyGrassHopper() {}
};


#endif // ACTOR_H_
