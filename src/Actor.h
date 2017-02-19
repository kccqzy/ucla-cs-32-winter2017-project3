#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

class Actor : public GraphObject {
protected:
    Actor(Actor const&) = delete;
    Actor& operator=(Actor const&) = delete;
    using GraphObject::GraphObject;
public:
    virtual ~Actor() {}
};

class Pebble : public Actor {
public:
    Pebble(int x, int y) : Actor(IID_ROCK, x, y, right, 1.0, 1) {}
    virtual ~Pebble() {}
};

#endif // ACTOR_H_
