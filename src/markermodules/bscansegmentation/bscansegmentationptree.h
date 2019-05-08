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

#ifndef BSCANSEGMENTATIONPTREE_H
#define BSCANSEGMENTATIONPTREE_H


#include <boost/property_tree/ptree_fwd.hpp>

class BScanSegmentation;


/**
 *  @ingroup FreeFormSegmentation
 *  @brief Free form segmentation IO class
 *
 */
class BScanSegmentationPtree
{
public:
	static bool parsePTree(const boost::property_tree::ptree& ptree,       BScanSegmentation* markerManager);
	static void fillPTree (      boost::property_tree::ptree& ptree, const BScanSegmentation* markerManager);
};


#endif // BSCANSEGMENTATIONPTREE_H
