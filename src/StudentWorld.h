#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "Actor.h"
#include "Field.h"
#include "GameWorld.h"
#include <map>
#include <memory>
#include <string>
#include <utility>

class StudentWorld final : public GameWorld {
  private:
    typedef std::multimap<Coord, std::unique_ptr<Actor>> ActorMap;
    typedef std::pair<ActorMap::iterator, ActorMap::iterator> RawActorRange;
    ActorMap actors;
    int ticks;
    std::vector<std::unique_ptr<Actor>> newActors;

    template <typename Actor, typename... Args>
    void insertActor(int x, int y, Args&&... args) {
        auto p = std::make_unique<Actor>(*this, std::make_pair(x, y), std::forward<Args>(args)...);
        actors.emplace(p->getCoord(), std::move(p));
    }

  public:
    StudentWorld(std::string assetDir) : GameWorld(assetDir), actors{}, ticks(0), newActors{} {}
    virtual int init();
    virtual int move();
    virtual void cleanUp();

    struct ActorRange : private RawActorRange {
        ActorMap::iterator begin() const { return first; }
        ActorMap::iterator end() const { return second; }
        ActorRange(RawActorRange const& p) : RawActorRange(p) {}
    };
    ActorRange getActorsAt(Coord c) { return actors.equal_range(c); }

    template <typename Actor, typename... Args>
    void insertActorAtEndOfTick(Args&&... args) {
        newActors.emplace_back(std::make_unique<Actor>(*this, std::forward<Args>(args)...));
    }
};


#endif // STUDENTWORLD_H_
