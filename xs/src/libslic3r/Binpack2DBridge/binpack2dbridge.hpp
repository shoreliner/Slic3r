#ifndef BINPACK2DBRIDGE_HPP
#define BINPACK2DBRIDGE_HPP

#include <sstream>

#include <Binpack2D/binpack2d.hpp>

#include <Polygon.hpp>
#include <Point.hpp>

namespace binpack2d {

// Aliases for convinience
using SlcrPoint = Slic3r::Point;
using SlcrPolygon = Slic3r::Polygon;

// Type of coordinate units used by Slic3r::Point
template<> struct CoordType<SlcrPoint> {
    using Type = coord_t;
};

// Type of point used by Slic3r::Polygon
template<> struct PointType<SlcrPolygon> {
    using Type = SlcrPoint;
};

// Tell binpack2d how to extract the X coord from a SlcrPoint object
template<> coord_t PointLike::x<SlcrPoint>(const SlcrPoint& p) { return p.x; }

// Tell binpack2d how to extract the Y coord from a SlcrPoint object
template<> coord_t PointLike::y<SlcrPoint>(const SlcrPoint& p) { return p.y; }

// Tell binpack2d how to make string out of a SlcrPolygon object
template<> std::string ShapeLike::toString<SlcrPolygon>(const SlcrPolygon& sh) {
    std::stringstream ss;

    for(auto p : sh.points) {
        ss << p.x << " " << p.y << "\n";
    }

    return ss.str();
}

}

#endif // BINPACK2DBRIDGE_HPP
