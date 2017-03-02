#ifndef GRAPHOBJ_H_
#define GRAPHOBJ_H_

#include "GameConstants.h"
#include <cstdio>
#include <cassert>

class GraphObject {
public:
    enum Direction { none, up, right, down, left };
    GraphObject(int imageID, int startX, int startY, Direction dir = right, int depth = 0, double size = 0.25)
      : m_imageID(imageID), m_x(startX), m_y(startY), m_direction(dir) {
        printf("GraphObject created with (imageID=%s, startX=%d, startY=%d, dir=%s, depth=%d, size=%.2f)\n",
               describeIID(imageID), startX, startY, describeDirection(dir), depth, size);
    }
    virtual ~GraphObject() = default;
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    void moveTo(int x, int y) {
        printf("GraphObject (imageID=%s, x=%d, y=%d, dir=%s) moving to (x=%d, y=%d)\n", describeIID(m_imageID), m_x,
               m_y, describeDirection(m_direction), x, y);
        assert(0 <= x);
        assert(0 <= y);
        assert(x < VIEW_WIDTH);
        assert(y < VIEW_HEIGHT);
        m_x = x;
        m_y = y;
    }
    Direction getDirection() const { return m_direction; }
    void setDirection(Direction d) {
        printf("GraphObject (imageID=%s, x=%d, y=%d, dir=%s) changing direction to %s\n", describeIID(m_imageID), m_x,
               m_y, describeDirection(m_direction), describeDirection(d));
        assert(d != none);
        m_direction = d;
    }

private:
    GraphObject(const GraphObject&) = delete;
    GraphObject& operator=(const GraphObject&) = delete;
    int m_imageID;
    int m_x;
    int m_y;
    Direction m_direction;

#define label(x)                                                                                                       \
    case x: return #x

    static const char* describeDirection(Direction dir) {
        switch (dir) {
            label(none);
            label(up);
            label(right);
            label(down);
            label(left);
        default: assert(false && "unknown direction");
        }
    }

    static const char* describeIID(int iid) {
        switch (iid) {
            label(IID_ANT_TYPE0);
            label(IID_ANT_TYPE1);
            label(IID_ANT_TYPE2);
            label(IID_ANT_TYPE3);
            label(IID_ANT_HILL);
            label(IID_POISON);
            label(IID_FOOD);
            label(IID_WATER_POOL);
            label(IID_ROCK);
            label(IID_BABY_GRASSHOPPER);
            label(IID_ADULT_GRASSHOPPER);
            label(IID_PHEROMONE_TYPE0);
            label(IID_PHEROMONE_TYPE1);
            label(IID_PHEROMONE_TYPE2);
            label(IID_PHEROMONE_TYPE3);
        default: assert(false && "unknown IID");
        }
    }
};

#endif // GRAPHOBJ_H_
