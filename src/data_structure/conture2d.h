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


#include<vector>
#include<ostream>

#include<data_structure/point2d.h>

#include<iostream>

/**
 * @ingroup DataStructure
 * @brief Simple connection between to nodes defined by they ids
 *
 */
class Line2D
{
	std::size_t nodeAid;
	std::size_t nodeBid;

	float slope;

public:
	Line2D(const std::size_t pIndex1, const std::size_t pIndex2, float slope)
	: nodeAid(pIndex1)
	, nodeBid(pIndex2)
	, slope  (slope  )
	{}

	std::size_t getNodeAid() const                                  { return nodeAid; }
	std::size_t getNodeBid() const                                  { return nodeBid; }

	float getSlope() const                                          { return slope  ; }
};


/**
 * @ingroup DataStructure
 * @brief Simple point with the id list of connected line
 *
 */
class LineNode
{
	Point2D point;

	std::vector<std::size_t> lineIndices;

public:
	LineNode(const Point2D& point) : point(point)                   {}
	LineNode()                                                      {}

	const Point2D& getPoint() const                                 { return point; }
	void addLineIndex(std::size_t index)                            { lineIndices.push_back(index); }


	const std::vector<std::size_t>& getLineIndices()          const { return lineIndices; }
};


/**
 * @ingroup DataStructure
 * @brief Holds and managed points and they connections
 *
 */
class Conture2D
{
	std::vector<Line2D  > lines;
	std::vector<LineNode> points;

public:
	std::size_t addPoint(const Point2D& p) { std::size_t newIndex = points.size(); points.emplace_back(p); return newIndex; }

	void addLine(const std::size_t pIndex1, const std::size_t pIndex2, float slope = -1)
	{
		if(pIndex1 == pIndex2)
			return;

		const std::size_t numPoints = points.size();
		if(pIndex1 < numPoints && pIndex2 < numPoints)
		{
			const std::size_t newIndex = lines.size();
			lines.emplace_back(pIndex1, pIndex2, slope);
			points[pIndex1].addLineIndex(newIndex);
			points[pIndex2].addLineIndex(newIndex);
		}
		else
			std::cerr << "Invalid pointindex";
	}

	const Point2D& getPoint(std::size_t index)                const { return points[index].getPoint(); }


	const std::vector<Line2D  >& getLines ()                  const { return lines ; }
	const std::vector<LineNode>& getPoints()                  const { return points; }
};


