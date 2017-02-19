#include "StudentWorld.h"
#include "Field.h"
#include <cstdio>
#include <string>
#include <vector>
#include "Actor.h"

GameWorld* createStudentWorld(std::string assetDir) { return new StudentWorld(assetDir); }

int StudentWorld::init() {
    actors.clear();
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

    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move() {
    ticks++;
    std::vector<std::pair<Coord, ActorMap::iterator>> movedActors;
    for (auto i = actors.begin(), ie = actors.end(); i != ie; ++i) {
        auto oldLocation = i->second->getCoord();
        i->second->doSomething();
        auto newLocation = i->second->getCoord();
        if (oldLocation != newLocation) {
            movedActors.emplace_back(newLocation, i);
        }
    }
    for (auto const& i : movedActors) {
        auto val = std::move(i.second->second);
        actors.erase(i.second);
        actors.emplace(i.first, std::move(val));
    }
    setGameStatText(std::to_string(ticks));
    return ticks < 2000 ? GWSTATUS_CONTINUE_GAME : GWSTATUS_NO_WINNER;
}

void StudentWorld::cleanUp() { actors.clear(); }
