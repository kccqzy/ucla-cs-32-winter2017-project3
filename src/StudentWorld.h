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
#include <tuple>
#include <utility>
#include <vector>

#if __cplusplus < 201402L
#error "This file requires C++14."
#endif

typedef std::tuple<int, int> Coord;
class Actor;

template<std::size_t N>
struct TupleComp {
    template<typename T, std::size_t... I>
    static auto select_from_tuple(T const& t, std::index_sequence<I...>) {
        return std::make_tuple(std::get<I>(t)...);
    }
    template<typename... T, typename... U>
    bool operator()(std::tuple<T...> const& a, std::tuple<U...> const& b) const {
        static_assert(sizeof...(T) == N || sizeof...(U) == N, "at least one tuple shall be full size");
        static_assert(sizeof...(T) <= N && sizeof...(U) <= N, "both tuples shall be no longer than full size");
        auto seq = std::make_index_sequence<std::min(sizeof...(T), sizeof...(U))>();
        return select_from_tuple(a, seq) < select_from_tuple(b, seq);
    }
    typedef bool is_transparent;
};

class StudentWorld final : public GameWorld {
private:
    typedef std::tuple<int, int, int> ActorKey;
    typedef std::multimap<ActorKey, std::unique_ptr<Actor>, TupleComp<std::tuple_size<ActorKey>::value>> ActorMap;
    typedef std::pair<ActorMap::const_iterator, ActorMap::const_iterator> RawActorRange;

    ActorMap actors;
    int ticks;

    struct AntColonyInfo {
        std::string name;
        Compiler compiler;
        int antCount;
        AntColonyInfo(std::string const& name, Compiler&& compiler)
          : name(name), compiler(std::move(compiler)), antCount(0) {}
    };
    std::vector<AntColonyInfo> antInfo;
    int currentWinningAnt;

    std::string makeStatusText() const {
        std::ostringstream oss;
        oss << "Ticks:" << std::right << std::setw(5) << (2000 - ticks);
        for (size_t i = 0; i < antInfo.size(); ++i) {
            oss << (i ? "  " : " - ") << antInfo[i].name;
            if ((int) i == currentWinningAnt) oss << '*';
            oss << ": " << std::setfill('0') << std::setw(2) << antInfo[i].antCount << " ants";
            // Do not distinguish between plural and singular forms. This is intentional.
        }
        return oss.str();
    }

public:
    StudentWorld(std::string assetDir) : GameWorld(assetDir), actors{}, ticks(0), antInfo{}, currentWinningAnt{-1} {}
    virtual int init() override;
    virtual int move() override;
    virtual void cleanUp() override;

    struct ActorRange : private RawActorRange {
        auto begin() const { return first; }
        auto end() const { return second; }
        bool empty() const { return first == second; }
        ActorRange(RawActorRange const& p) : RawActorRange(p) {}
    };
    template<typename K, typename... T>
    ActorRange getActorsAt(K k, T... t) const {
        return actors.equal_range(std::tuple_cat(k, std::make_tuple(t...)));
    }

    template<typename Actor, typename... Args>
    void insertActor(Args&&... args) {
        auto p = std::make_unique<Actor>(*this, std::forward<Args>(args)...);
        actors.emplace(p->getKey(), std::move(p));
    }

    void increaseAntCountForColony(int t) {
        // The winner is defined as one that produced more ants than its
        // competitors, or if there is a tie, the colony that produced the most
        // ants first.
        if (++antInfo[t].antCount >= 6 &&
            (currentWinningAnt == -1 || antInfo[t].antCount > antInfo[currentWinningAnt].antCount))
            // Current ant colony has produced enough ants, and either there is no winning ant or this colony has more
            // winning ants.
            currentWinningAnt = t;
    }
};

#endif // STUDENTWORLD_H_
