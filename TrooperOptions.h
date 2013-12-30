#ifndef TROOPER_OPTIONS_H
#define TROOPER_OPTIONS_H

#include "Point.h"

class TrooperOptions
{
public:
    TrooperOptions();
    void setStartPoint(Point& value);
    void setLastPoint(Point& value);
    void setLocalTargetPoint(Point& value);
    void setTargetPoint(Point& value);

    Point& getStartPoint();
    Point& getLastPoint();
    Point& getLocalTargetPoint();
    Point& getTargetPoint();

    bool isAlive();
    void setAlive(bool value);

private:
    Point start_point;
    Point last_point;
    Point local_target_point;
    Point target_point;
    bool alive;
};

#endif
