#include "StudentWorld.h"
#include "Actor.h"
#include "Field.h"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>

GameWorld* createStudentWorld(std::string assetDir) { return new StudentWorld(assetDir); }

int StudentWorld::init() {
    actors.clear();
    newActors.clear();
    antInfo.clear();
    {
        Field f;
        {
            std::string fieldFileName = this->getFieldFilename();
            fprintf(stderr, "Attempting to load file %s as field data.\n", fieldFileName.c_str());
            auto loadResult = f.loadField(fieldFileName);
            if (loadResult != Field::LoadResult::load_success) { return GWSTATUS_LEVEL_ERROR; }
        }
        for (int x = 0; x < VIEW_WIDTH; ++x) {
            for (int y = 0; y < VIEW_HEIGHT; ++y) {
                switch (f.getContentsOf(x, y)) {
                case Field::FieldItem::empty: break;
                case Field::FieldItem::rock: insertActor<Pebble>(x, y); break;
                case Field::FieldItem::grasshopper: insertActor<BabyGrassHopper>(x, y); break;
                case Field::FieldItem::food: insertActor<Food>(x, y, 6000); break;
                }
            }
        }
    }

    {
        auto antFns = getFilenamesOfAntPrograms();
        if (antFns.size() > 4) antFns.resize(4);
        for (size_t i = 0; i < antFns.size(); ++i) antInfo.emplace_back(antFns[i]);
    }

    ticks = 0;
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move() {
    ticks++;

    // Ask actors to doSomething.
    std::vector<std::pair<Coord, ActorMap::iterator>> movedActors;
    std::vector<ActorMap::iterator> deadActors;
    for (auto i = actors.begin(), ie = actors.end(); i != ie; ++i) {
        auto oldLocation = i->second->getCoord();
        if (!i->second->isDead()) i->second->doSomething();
        auto newLocation = i->second->getCoord();
        if (i->second->isDead()) {
            deadActors.emplace_back(i);
        } else if (oldLocation != newLocation) {
            movedActors.emplace_back(newLocation, i);
        }
    }
    // Remove dead, move moved, and add new actors.
    for (auto const& i : deadActors) actors.erase(i);
    for (auto const& i : movedActors) {
        auto val = std::move(i.second->second);
        actors.erase(i.second);
        actors.emplace(i.first, std::move(val));
    }
    for (auto& i : newActors) { actors.emplace(i->getCoord(), std::move(i)); }
    newActors.clear();

    setStatusText();
    return ticks < 2000 ? GWSTATUS_CONTINUE_GAME : GWSTATUS_NO_WINNER;
}

void StudentWorld::cleanUp() {
    actors.clear();
    newActors.clear();
}
