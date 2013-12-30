#pragma once

#ifndef POINT_H
#define POINT_H

class Point
{
public:
    Point();
    Point(const Point& p);
    Point(int _x, int _y);
    void set(int _x, int _y);
    void setX(int value);
    void setY(int value);
    int getX();
    int getY();
    int getSquareDistance(const Point& p);
    bool operator==(const Point& p);
    bool operator!=(const Point& p);
private:
    int x_value, y_value;
};

#endif
