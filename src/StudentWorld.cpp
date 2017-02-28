#include "StudentWorld.h"
#include "Actor.h"
#include "Compiler.h"
#include "Field.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>

GameWorld* createStudentWorld(std::string assetDir) { return new StudentWorld(assetDir); }

int StudentWorld::init() {
    StudentWorld::cleanUp();

    auto antFns = getFilenamesOfAntPrograms();
    if (antFns.size() > 4) antFns.resize(4);
    for (auto const& fn : antFns) {
        Compiler c;
        std::string e;
        if (c.compile(fn, e)) {
            antInfo.emplace_back(c.getColonyName(), std::move(c));
        } else {
            setError(fn + " " + e);
            return GWSTATUS_LEVEL_ERROR;
        }
    }

    {
        Field f;
        {
            std::string fieldFileName = this->getFieldFilename();
            auto loadResult = f.loadField(fieldFileName);
            if (loadResult != Field::LoadResult::load_success) { return GWSTATUS_LEVEL_ERROR; }
        }
        for (int x = 0; x < VIEW_WIDTH; ++x) {
            for (int y = 0; y < VIEW_HEIGHT; ++y) {
                auto insertAnthill = [this](Coord c, int t) {
                    if (t < (int) antInfo.size()) insertActor<Anthill>(c, t, antInfo[t].compiler);
                };
                auto c = std::make_tuple(x, y);
                switch (f.getContentsOf(x, y)) {
                case Field::FieldItem::empty: break;
                case Field::FieldItem::water: insertActor<PoolOfWater>(c); break;
                case Field::FieldItem::poison: insertActor<Poison>(c); break;
                case Field::FieldItem::rock: insertActor<Pebble>(c); break;
                case Field::FieldItem::grasshopper: insertActor<BabyGrasshopper>(c); break;
                case Field::FieldItem::food: insertActor<Food>(c, 6000); break;
                case Field::FieldItem::anthill0: insertAnthill(c, 0); break;
                case Field::FieldItem::anthill1: insertAnthill(c, 1); break;
                case Field::FieldItem::anthill2: insertAnthill(c, 2); break;
                case Field::FieldItem::anthill3: insertAnthill(c, 3); break;
                }
            }
        }
    }

    ticks = 0;
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move() {
    ticks++;

    // Save a copy of all actors. It is unsafe to mutate a structure while
    // iterating through it. So we first obtain all iterators (nodes) inside a
    // map. This ensures that: (a) we only perform doSomething() on actors
    // present at the beginning of the tick, not newly created ones; (b) the
    // order of doSomething() is well-defined.
    std::vector<ActorMap::iterator> allCurrentActors;
    allCurrentActors.reserve(actors.size());
    for (auto i = actors.begin(), ie = actors.end(); i != ie; ++i) allCurrentActors.emplace_back(i);

    // Ask actors to doSomething. Immediately after each actor does something,
    // we perform data structure maintenance to make sure the data structure is
    // in sync. This is necessary because actors in their doSomething() can look
    // up other actors by their keys, and it is necessary therefore to do
    // maintenance after every single doSomething().
    for (auto const& i : allCurrentActors) {
        if (!i->second->isDead()) i->second->doSomething();
        if (i->second->isDead()) {
            actors.erase(i);
        } else {
            auto newKey = i->second->getKey();
            if (newKey != i->first) {
                auto p = std::move(i->second);
                actors.erase(i);
                actors.emplace(newKey, std::move(p));
            }
        }
    }

    // Final garbage collection pass. An earlier actor may have become dead
    // through the actions of a later actor.
    for (auto i = actors.begin(); i != actors.end();)
        if (i->second->isDead())
            i = actors.erase(i);
        else
            ++i;

    setGameStatText(makeStatusText());
    if (ticks < 2000)
        return GWSTATUS_CONTINUE_GAME;
    else if (currentWinningAnt > -1) {
        setWinner(antInfo[currentWinningAnt].name);
        return GWSTATUS_PLAYER_WON;
    } else
        return GWSTATUS_NO_WINNER;
}

void StudentWorld::cleanUp() {
    actors.clear();
    antInfo.clear();
    currentWinningAnt = -1;
}
