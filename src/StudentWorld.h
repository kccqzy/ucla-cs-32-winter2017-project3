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
    // Induce an ordering on (potentially heterogeneous) tuples of possibly
    // unequal sizes. The standard ordering on tuples require them to be of the
    // same size, which is too restrictive. Instead, we model each element in an
    // tuple as a sort key that determines whether or not any possible actor
    // belongs in this partition. Then tuples of different sizes become coarser
    // or finer partitions of the set of actors. It has the following
    // properties: (a) uniqueness: for all actor, there exists one and only one
    // sort key, and therefore this sort key partitions the set of all actors
    // into disjoint subsets; (b) composability: for all concatenation of tuples
    // $(p,q)$, the partition induced by $(p,q)$ is the same as partitioning by
    // $q$ on each subset produced by partitioning on $p$. Thus we can define a
    // strict weak ordering $<^*$ on these tuples of unequal sizes by taking the
    // longest common prefix and then using the standard strict weak ordering
    // $<$. The proof that this new ordering is in fact irreflexive and
    // transitive is left to whoever grades this CS project ;).
    template<typename... T, typename... U, std::size_t... I>
    bool operator()(std::tuple<T...> const& a, std::tuple<U...> const& b, std::index_sequence<I...>) const {
        return std::make_tuple(std::get<I>(a)...) < std::make_tuple(std::get<I>(b)...);
    }
    template<typename... T, typename... U,
             typename Indices = std::make_index_sequence<std::min(sizeof...(T), sizeof...(U))>>
    bool operator()(std::tuple<T...> const& a, std::tuple<U...> const& b) const {
        static_assert(sizeof...(T) == N || sizeof...(U) == N, "at least one tuple shall be full size");
        static_assert(sizeof...(T) <= N && sizeof...(U) <= N, "both tuples shall be no longer than full size");
        return operator()(a, b, Indices());
    }
    typedef bool is_transparent;
};

class StudentWorld final : public GameWorld {
private:
    typedef std::tuple<int, int, int> ActorKey;
    typedef std::multimap<ActorKey, std::unique_ptr<Actor>, TupleComp<std::tuple_size<ActorKey>::value>> ActorMap;
    typedef std::pair<ActorMap::iterator, ActorMap::iterator> RawActorRange;

    ActorMap actors;
    int ticks;
    std::vector<std::unique_ptr<Actor>> newActors;

    struct AntColonyInfo {
        std::string name;
        Compiler compiler;
        int antCount;
        int antCountTimestamp;
        AntColonyInfo(std::string const& name, Compiler&& compiler)
          : name(name), compiler(std::move(compiler)), antCount(0), antCountTimestamp(0) {}
        friend bool operator<(AntColonyInfo const& a, AntColonyInfo const& b) {
            return std::make_tuple(a.antCount, -a.antCountTimestamp) <
                   std::make_tuple(b.antCount, -b.antCountTimestamp);
        }
    };
    std::vector<AntColonyInfo> antInfo;

    template<typename Actor, typename... Args>
    void insertActor(int x, int y, Args&&... args) {
        auto p = std::make_unique<Actor>(*this, std::make_tuple(x, y), std::forward<Args>(args)...);
        actors.emplace(p->getKey(), std::move(p));
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
                oss << ": " << std::setfill('0') << std::setw(2) << antInfo[i].antCount << " ants";
                // Do not distinguish between plural and singular forms. This is intentional.
            }
        }
        setGameStatText(oss.str());
    }

public:
    StudentWorld(std::string assetDir) : GameWorld(assetDir), actors{}, ticks(0), newActors{}, antInfo{} {}
    virtual int init() override;
    virtual int move() override;
    virtual void cleanUp() override;

    struct ActorRange : private RawActorRange {
        ActorMap::iterator begin() const { return first; }
        ActorMap::iterator end() const { return second; }
        bool empty() const { return first == second; }
        ActorRange(RawActorRange const& p) : RawActorRange(p) {}
    };
    template<typename CoordOrKey>
    ActorRange getActorsAt(CoordOrKey c) {
        return actors.equal_range(c);
    }
    ActorRange getActorsAt(Coord c, int iid) { return actors.equal_range(std::tuple_cat(c, std::make_tuple(iid))); }

    template<typename Actor, typename... Args>
    void insertActorAtEndOfTick(Args&&... args) {
        newActors.emplace_back(std::make_unique<Actor>(*this, std::forward<Args>(args)...));
    }

    void increaseAntCountForColony(int t) {
        // The winner is defined as one that produced more ants than its
        // competitors, or if there is a tie, the colony that produced the most
        // ants first.
        antInfo[t].antCount++;
        antInfo[t].antCountTimestamp = ticks;
    }
};

#endif // STUDENTWORLD_H_
