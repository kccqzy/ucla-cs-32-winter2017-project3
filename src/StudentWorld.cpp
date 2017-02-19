#include "StudentWorld.h"
#include <string>

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
                }
            }
        }
    }

    return GWSTATUS_CONTINUE_GAME;
}
