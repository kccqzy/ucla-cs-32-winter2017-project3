#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "Compiler.h"
#include "Field.h"
#include "GameWorld.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

typedef std::pair<int, int> Coord;
class Actor;

class StudentWorld final : public GameWorld {
private:
    typedef std::multimap<Coord, std::unique_ptr<Actor>> ActorMap;
    typedef std::pair<ActorMap::iterator, ActorMap::iterator> RawActorRange;

    ActorMap actors;
    int ticks;
    std::vector<std::unique_ptr<Actor>> newActors;

    struct AntColonyInfo {
        std::string name;
        Compiler compiler;
        int antCount;
        AntColonyInfo(std::string const& name, Compiler&& compiler)
          : name(name), compiler(std::move(compiler)), antCount(0) {}
        friend bool operator<(AntColonyInfo const& a, AntColonyInfo const& b) { return a.antCount < b.antCount; }
    };
    std::vector<AntColonyInfo> antInfo;

    template<typename Actor, typename... Args>
    void insertActor(int x, int y, Args&&... args) {
        auto p = std::make_unique<Actor>(*this, std::make_pair(x, y), std::forward<Args>(args)...);
        actors.emplace(p->getCoord(), std::move(p));
    }

    void setStatusText() {
        std::ostringstream oss;
        oss << "Ticks:" << std::right << std::setw(5) << (2000 - ticks);
        if (!antInfo.empty()) {
            auto m = std::max_element(antInfo.begin(), antInfo.end());
            assert(m != antInfo.end());
            auto n = std::max_element(m + 1, antInfo.end());
            int winningAnt = (n == antInfo.end() || *n < *m) ? m - antInfo.begin() : -1;
            for (size_t i = 0; i < antInfo.size(); ++i) {
                oss << (i ? "  " : " - ") << antInfo[i].name;
                if ((int) i == winningAnt) oss << '*';
                oss << ": " << std::setfill('0') << std::setw(2) << antInfo[i].antCount;
            }
        }
        setGameStatText(oss.str());
    }

public:
    StudentWorld(std::string assetDir) : GameWorld(assetDir), actors{}, ticks(0), newActors{}, antInfo{} {}
    virtual int init();
    virtual int move();
    virtual void cleanUp();

    struct ActorRange : private RawActorRange {
        ActorMap::iterator begin() const { return first; }
        ActorMap::iterator end() const { return second; }
        ActorRange(RawActorRange const& p) : RawActorRange(p) {}
    };
    ActorRange getActorsAt(Coord c) { return actors.equal_range(c); }

    template<typename Actor, typename... Args>
    void insertActorAtEndOfTick(Args&&... args) {
        newActors.emplace_back(std::make_unique<Actor>(*this, std::forward<Args>(args)...));
    }
};

#endif // STUDENTWORLD_H_
