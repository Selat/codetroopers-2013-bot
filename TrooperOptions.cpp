#include "TrooperOptions.h"

TrooperOptions::TrooperOptions() : start_point(-1, -1), local_target_point(-1, -1),
				   target_point(-1, -1)
{
    alive = true;
}

void TrooperOptions::setStartPoint(Point& value)
{
    start_point = value;
}

void TrooperOptions::setLastPoint(Point& value)
{
    last_point = value;
}

void TrooperOptions::setLocalTargetPoint(Point& value)
{
    local_target_point = value;
}

void TrooperOptions::setTargetPoint(Point& value)
{
    target_point = value;
}

Point& TrooperOptions::getStartPoint()
{
    return start_point;
}

Point& TrooperOptions::getLastPoint()
{
    return last_point;
}

Point& TrooperOptions::getLocalTargetPoint()
{
    return local_target_point;
}

Point& TrooperOptions::getTargetPoint()
{
    return target_point;
}

bool TrooperOptions::isAlive()
{
    return alive;
}

void TrooperOptions::setAlive(bool value)
{
    alive = value;
}
