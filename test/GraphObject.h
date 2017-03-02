#ifndef GRAPHOBJ_H_
#define GRAPHOBJ_H_

#include "GameConstants.h"
#include <cstdio>

class GraphObject {
public:
    enum Direction { none, up, right, down, left };
    GraphObject(int imageID, int startX, int startY, Direction dir = right, int depth = 0, double size = 0.25)
      : m_imageID(imageID), m_x(startX), m_y(startY), m_direction(dir) {
        printf("GraphObject created with (imageID=%d, startX=%d, startY=%d, dir=%d, depth=%d, size=%.2f)\n", imageID,
               startX, startY, dir, depth, size);
    }
    virtual ~GraphObject() = default;
    int getX() const { return m_x; }
    int getY() const { return m_y; }
    void moveTo(int x, int y) {
        printf("GraphObject (imageID=%d, x=%d, y=%d, dir=%d) moving to (x=%d, y=%d)\n", m_imageID, m_x, m_y,
               m_direction, x, y);
        m_x = x;
        m_y = y;
    }
    Direction getDirection() const { return m_direction; }
    void setDirection(Direction d) {
        printf("GraphObject (imageID=%d, x=%d, y=%d, dir=%d) changing direction to %d\n", m_imageID, m_x, m_y,
               m_direction, d);
        m_direction = d;
    }

private:
    GraphObject(const GraphObject&) = delete;
    GraphObject& operator=(const GraphObject&) = delete;
    int m_imageID;
    int m_x;
    int m_y;
    Direction m_direction;
};

#endif // GRAPHOBJ_H_
