#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "Actor.h"
#include "Field.h"
#include "GameConstants.h"
#include "GameWorld.h"
#include "GraphObject.h"
#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <utility>

class StudentWorld : public GameWorld {
  private:
    static std::pair<int, int> extractCoord(GraphObject const& go) {
        return std::make_pair(go.getX(), go.getY());
    }
    typedef std::multimap<std::pair<int, int>, std::unique_ptr<Actor>> ActorMap;
    ActorMap actors;

    template <typename Actor, typename... Args>
    void insertActor(Args&&... args) {
        auto p = std::make_unique<Actor>(std::forward<Args>(args)...);
        actors.emplace(extractCoord(*p), std::move(p));
    }

  public:
    StudentWorld(std::string assetDir) : GameWorld(assetDir), actors{} {}

    virtual int init();

    virtual int move() {
        // This code is here merely to allow the game to build, run, and terminate after you hit enter.
        // Notice that the return value GWSTATUS_NO_WINNER will cause our framework to end the simulation.
        return GWSTATUS_NO_WINNER;
    }

    virtual void cleanUp() { actors.clear(); }
};

#endif // STUDENTWORLD_H_
