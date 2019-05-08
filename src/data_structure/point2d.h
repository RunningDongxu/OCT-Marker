/*
 * Copyright (c) 2018 Kay Gawlik <kaydev@amarunet.de> <kay.gawlik@beuth-hochschule.de> <kay.gawlik@charite.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include<ostream>

class ScaleFactor;

/**
 * @ingroup DataStructure
 * @brief Generic point with some calculations functions
 *
 */
template<typename T>
class Point2DBase
{
	T x = T();
	T y = T();
public:
	typedef T value_type;

	Point2DBase(T x, T y) : x(x), y(y) {}
	Point2DBase() = default;
	Point2DBase(const Point2DBase<T>& other) = default;


	const T& getX() const                                          { return x; }
	const T& getY() const                                          { return y; }

	void setX(const T& v) { x = v; }
	void setY(const T& v) { y = v; }

	double euklidDist(const Point2DBase<T>& p) const;
	double euklidDist(const Point2DBase<T>& p, const ScaleFactor& scale) const;


	Point2DBase operator-(const Point2DBase& o) const              { return Point2DBase(x-o.x, y-o.y); }
	Point2DBase operator+(const Point2DBase& o) const              { return Point2DBase(x+o.x, y+o.y); }
	Point2DBase operator*(T factor)             const              { return Point2DBase(x*factor, y*factor); }
	Point2DBase& operator+=(const Point2DBase& o)                  { x += o.x; y += o.y; return *this; }

	double length() const;
	void normize()                                                 { T len = length(); if(len > 1e-8) *this /= len; }

	Point2DBase& operator*=(T factor)                              { x *= factor; y *= factor; return *this; }
	Point2DBase& operator/=(T factor)                              { return operator*=(1/factor); }

	virtual void print(std::ostream& stream) const                 { stream << '(' << x << "; " << y << ')'; }

	bool operator==(const Point2DBase& other) const                { return x == other.x && y == other.y; }
	bool operator!=(const Point2DBase& other) const                { return !operator==(other); }


	double normquadrat()                      const                { return x*x + y*y; }
	double operator*(const Point2DBase& v)    const                { return x*v.x + y*v.y; }
};


template<typename T>
inline std::ostream& operator<<(std::ostream& stream, const Point2DBase<T>& p) { p.print(stream); return stream; }

typedef Point2DBase<double> Point2D;
typedef Point2DBase<int> Point2DInt;
typedef Point2DBase<std::size_t> PixtureElement;

