#include "Point.h"

Point::Point()
{
    x_value = 0;
    y_value = 0;
}

Point::Point(const Point& p)
{
    x_value = p.x_value;
    y_value = p.y_value;
}

Point::Point(int _x, int _y)
{
    x_value = _x;
    y_value = _y;
}

void Point::set(int _x, int _y)
{
    x_value = _x;
    y_value = _y;
}

void Point::setX(int value)
{
    x_value = value;
}

void Point::setY(int value)
{
    y_value = value;
}

int Point::getX()
{
    return x_value;
}

int Point::getY()
{
    return y_value;
}

int Point::getSquareDistance(const Point& p)
{
    return ((x_value - p.x_value) * (x_value - p.x_value)
	    + (y_value - p.y_value) * (y_value - p.y_value));
}

bool Point::operator==(const Point& p)
{
    return (x_value == p.x_value) && (y_value == p.y_value);
}

bool Point::operator!=(const Point& p)
{
    return (x_value != p.x_value) || (y_value != p.y_value);
}
