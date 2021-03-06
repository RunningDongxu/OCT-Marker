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

#include "bscanintervalptree.h"

#include <iostream>

#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>

#include "bscanintervalmarker.h"
#include "definedintervalmarker.h"


namespace bpt = boost::property_tree;


namespace
{
	bool parsePTreeMarkerCollection(const bpt::ptree& ptree, BScanIntervalMarker* markerManager, const std::string& markerCollectionInternalName, const IntervalMarker& markerCollection, BScanIntervalMarker::MarkerCollectionWork& collectionSetterHelper)
	{
		boost::optional<const bpt::ptree&> bscansNode = ptree.get_child_optional(markerCollectionInternalName);
		if(!bscansNode)
			return true; // not a true error


		for(const std::pair<const std::string, const bpt::ptree>& bscanPair : *bscansNode)
		{
			if(bscanPair.first != "BScan")
				continue;

			const bpt::ptree& bscanNode = bscanPair.second;
			boost::optional<const bpt::ptree&> idNodeOpt = bscanNode.get_child_optional("ID");
			if(!idNodeOpt)
				continue;
			int bscanId = idNodeOpt->get_value<int>(-1);
			if(bscanId == -1)
				continue;

			for(const std::pair<const std::string, const bpt::ptree>& intervallNodePair : bscanNode)
			{
				if(intervallNodePair.first != "Intervall")
					continue;

				const bpt::ptree& intervallNode = intervallNodePair.second;

				int         start          = intervallNode.get_child("Start").get_value<int>();
				int         end            = intervallNode.get_child("End"  ).get_value<int>();
				std::string intervallClass = intervallNode.get_child("Class").get_value<std::string>();

				try
				{
					IntervalMarker::Marker marker = markerCollection.getMarkerFromString(intervallClass);
					markerManager->setMarker(start, end, marker, bscanId, collectionSetterHelper);
				}
				catch(std::out_of_range& r)
				{
					std::cerr << "unknown interval class " << intervallClass << " : " << r.what() << std::endl;
				}

			}
		}

		return true;
	}


	void fillPTreeMarkerCollection(bpt::ptree& markerTree, const BScanIntervalMarker* markerManager, const std::string& markerCollectionInternalName)
	{
		markerTree.erase(markerCollectionInternalName);
		bpt::ptree& qualityTree = markerTree.put(markerCollectionInternalName, std::string());

		std::size_t numBscans = markerManager->getNumBScans();
		for(std::size_t bscan = 0; bscan < numBscans; ++bscan)
		{
			const BScanIntervalMarker::MarkerMap& markerMap = markerManager->getMarkers(markerCollectionInternalName, bscan);
			bool bscanEmpty = true;

			for(const BScanIntervalMarker::MarkerMap::interval_mapping_type pair : markerMap)
			{
				if(pair.second.isDefined())
				{
					bscanEmpty = false;
					break;
				}
			}
			if(bscanEmpty)
				continue;

			std::string nodeName = "BScan";
			bpt::ptree& bscanNode = qualityTree.add(nodeName, "");
			bscanNode.add("ID", boost::lexical_cast<std::string>(bscan));

			for(const BScanIntervalMarker::MarkerMap::interval_mapping_type pair : markerMap)
			{

				// std::cout << "paintEvent(QPaintEvent* event) " << pair.second << " - " << pair.first << std::endl;

				IntervalMarker::Marker marker = pair.second;
				if(marker.isDefined())
				{
					boost::icl::discrete_interval<int> itv  = pair.first;

					bpt::ptree& intervallNode = bscanNode.add("Intervall", "");

					intervallNode.add("Start", itv.lower());
					intervallNode.add("End"  , itv.upper());
					intervallNode.add("Class", marker.getInternalName());

					// xmltree.add(name, value);
				}
			}
		}
	}
}


bool BScanIntervalPTree::parsePTree(const bpt::ptree& ptree, BScanIntervalMarker* markerManager)
{
// 	std::string oldMarkerCollection = markerManager->getActMarkerCollectionInternalName();
	bool result = true;
	const DefinedIntervalMarker::IntervallMarkerMap& definedIntervalMarker = DefinedIntervalMarker::getInstance().getIntervallMarkerMap();
	for(auto& obj : definedIntervalMarker)
	{
		const std::string& markerCollectionInternalName = obj.first;
		const IntervalMarker& markerCollection = obj.second;

		BScanIntervalMarker::MarkerCollectionWork collectionSetterHelper = markerManager->getMarkerCollection(markerCollectionInternalName);
// 		markerManager->setMarkerCollection(markerCollectionInternalName);
		result &= parsePTreeMarkerCollection(ptree, markerManager, markerCollectionInternalName, markerCollection, collectionSetterHelper);
	}


	// read old data, remove this and by fillPTree later
	const DefinedIntervalMarker::IntervallMarkerMap::const_iterator signalQuality = definedIntervalMarker.find("signalQuality");
	if(signalQuality != definedIntervalMarker.end())
	{
// // 		markerManager->setMarkerCollection("signalQuality");
		BScanIntervalMarker::MarkerCollectionWork collectionSetterHelper = markerManager->getMarkerCollection("signalQuality");
		result &= parsePTreeMarkerCollection(ptree, markerManager, "Quality", signalQuality->second, collectionSetterHelper);
	}

// 	markerManager->setMarkerCollection(oldMarkerCollection);

	return result;
}

void BScanIntervalPTree::fillPTree(bpt::ptree& markerTree, const BScanIntervalMarker* markerManager)
{

	const DefinedIntervalMarker::IntervallMarkerMap& definedIntervalMarker = DefinedIntervalMarker::getInstance().getIntervallMarkerMap();
	for(auto& obj : definedIntervalMarker)
	{
		const std::string& markerCollectionInternalName = obj.first;
		fillPTreeMarkerCollection(markerTree, markerManager, markerCollectionInternalName);
	}


	// delete old data, remove this and by fillPTree later
	markerTree.erase("Quality");
}

