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

    static Coord extractCoord(GraphObject const& go) { return std::make_pair(go.getX(), go.getY()); }

    template <typename Actor, typename... Args>
    void insertActor(Args&&... args) {
        auto p = std::make_unique<Actor>(std::forward<Args>(args)...);
        actors.emplace(extractCoord(*p), std::move(p));
    }

  public:
    StudentWorld(std::string assetDir) : GameWorld(assetDir), actors{}, ticks(0) {}
    virtual int init();
    virtual int move();
    virtual void cleanUp();

    struct ActorRange : private RawActorRange {
        ActorMap::iterator begin() const { return first; }
        ActorMap::iterator end() const { return second; }
        ActorRange(RawActorRange const& p): RawActorRange(p) {}
    };
    ActorRange getActorsAt(Coord c) {
        return actors.equal_range(c);
    }
};


#endif // STUDENTWORLD_H_
