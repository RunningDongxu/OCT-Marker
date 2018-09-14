#ifndef COORDSLO_H
#define COORDSLO_H

#include <cmath>
#include <ostream>

class ScaleFactor
{
	double x;
	double y;

public:
	ScaleFactor(double x, double y) : x(x), y(y)                {}
	explicit ScaleFactor(double factor) : x(factor), y(factor)  {}
	ScaleFactor() : x(1), y(1)                                  {}

	double getX() const                                         { return x; }
	double getY() const                                         { return y; }

	ScaleFactor  operator* (double factor) const                { return ScaleFactor(x*factor, y*factor); }
	ScaleFactor& operator*=(double factor)                      { x *= factor; y *= factor; return *this; }
};

class CoordSLOpx
{
	double x;
	double y;

public:
	CoordSLOpx(double x, double y) : x(x), y(y)                 {}
	CoordSLOpx() : x(0), y(0)                                   {}

	int getX() const                                            { return std::round(x); }
	int getY() const                                            { return std::round(y); }
	
	
	double getXf() const                                        { return x; }
	double getYf() const                                        { return y; }
	
	CoordSLOpx operator+(const CoordSLOpx& o) const             { return CoordSLOpx(x+o.x   , y+o.y   ); }
	CoordSLOpx operator*(double factor)       const             { return CoordSLOpx(x*factor, y*factor); }
	
	double abs(const CoordSLOpx& o) const                       { return std::sqrt((x-o.x)*(x-o.x) + (y-o.y)*(y-o.y)); }
	
	void print(std::ostream& stream) const                      { stream << "(" << x << " | " << y << ")"; }
};

inline std::ostream& operator<<(std::ostream& stream, const CoordSLOpx& obj) { obj.print(stream); return stream; }


class CoordSLOmm
{
	double x;
	double y;
	
	bool set = false;

public:
	CoordSLOmm(double x, double y) : x(x), y(y), set(true)      {}
	CoordSLOmm() : x(0), y(0)                                   {}

	double getX() const                                         { return x; }
	double getY() const                                         { return y; }

	CoordSLOpx operator*(const ScaleFactor& factor) const       { return CoordSLOpx(x/factor.getX(), y/factor.getY()); }
	
	operator bool() const                                       { return set; }
};




#endif // COORDSLO_H
