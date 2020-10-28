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

#include "octmarkermanager.h"

#include <data_structure/programoptions.h>
#include <data_structure/extraseriesdata.h>

#include <octdata/datastruct/series.h>
#include <octdata/datastruct/bscan.h>
#include <octdata/datastruct/oct.h>
#include <octdata/octfileread.h>
#include <octdata/filereadoptions.h>

#include <boost/exception/exception.hpp>
#include <boost/exception/diagnostic_information.hpp>

#include <iostream>
#include <QMessageBox>

#include <QTime>

#include "octdatamanager.h"

#include <markermodules/bscanmarkerbase.h>
#include <markermodules/slomarkerbase.h>
#include <markermodules/bscanintervalmarker/bscanintervalmarker.h>
#include <markermodules/bscansegmentation/bscansegmentation.h>
#include <markermodules/bscanlayersegmentation/bscanlayersegmentation.h>
#include <markermodules/sloobjects/sloobjectmarker.h>
#include <markermodules/objectsmarker/objectsmarker.h>
#include <markermodules/scanclassifier/scanclassifier.h>
#include <markermodules/distancemeter/distancemeter.h>

#include <helper/ptreehelper.h>

#include <boost/property_tree/ptree.hpp>

namespace bpt = boost::property_tree;


OctMarkerManager::OctMarkerManager()
: QObject()
, extraSeriesData(new ExtraSeriesData)
{
	OctDataManager& dataManager = OctDataManager::getInstance();
	connect(&dataManager, &OctDataManager::seriesChanged     , this, &OctMarkerManager::showSeries         );
	connect(&dataManager, &OctDataManager::saveMarkerState   , this, &OctMarkerManager::saveMarkerStateSlot);
	connect(&dataManager, &OctDataManager::loadMarkerState   , this, &OctMarkerManager::loadMarkerStateSlot);
	connect(&dataManager, &OctDataManager::loadMarkerStateAll, this, &OctMarkerManager::reloadMarkerStateSlot);

	bscanMarkerObj.push_back(new BScanSegmentation(this));
	bscanMarkerObj.push_back(new Objectsmarker(this));
	bscanMarkerObj.push_back(new BScanIntervalMarker(this));
	bscanMarkerObj.push_back(new BScanLayerSegmentation(this));
	bscanMarkerObj.push_back(new ScanClassifier(this));
	bscanMarkerObj.push_back(new DistanceMeter(this));
	
	for(BscanMarkerBase* obj : bscanMarkerObj)
	{
		obj->activate(false);
		connect(obj, &BscanMarkerBase::requestFullUpdate      , this, &OctMarkerManager::udateFromMarkerModul             );
		connect(obj, &BscanMarkerBase::sloViewHasChanged      , this, &OctMarkerManager::handleSloRedrawAfterMarkerChange );
		connect(obj, &BscanMarkerBase::requestSloOverlayUpdate, this, &OctMarkerManager::sloOverlayUpdateFromMarkerModul  );
		connect(obj, &BscanMarkerBase::undoRedoChanged        , this, &OctMarkerManager::updateUndoRedowState             );
		connect(obj, &BscanMarkerBase::requestChangeBscan     , this, &OctMarkerManager::bscanChangeRequestFromMarkerModul);
	}


	sloMarkerObj.push_back(new SloObjectMarker(this));
	
	OctMarkerManager::setBscanMarker(ProgramOptions::bscanMarkerToolId());
	OctMarkerManager::setSloMarker  (ProgramOptions::  sloMarkerToolId());
}


OctMarkerManager::~OctMarkerManager()
{
	for(BscanMarkerBase* obj : bscanMarkerObj)
		delete obj;

	delete extraSeriesData;
}



void OctMarkerManager::chooseBScan(int bscan)
{
	if(bscan == actBScan)
		return;
	if(bscan < 0)
		return;
	if(bscan >= static_cast<int>(series->bscanCount()))
		return;

	actBScan = bscan;

// 	if(actBscanMarker)
// 		actBscanMarker->setActBScan(actBScan);

	// TODO bessere Implementierung (nur aktualisieren was angezeigt wird)
	for(BscanMarkerBase* marker : bscanMarkerObj)
		marker->setActBScan(actBScan);


	emit(newBScanShowed(series->getBScan(actBScan)));
	emit(bscanChanged(actBScan));
}


void OctMarkerManager::showSeries(const std::shared_ptr<const OctData::Series>& s)
{
	series = s;
	bpt::ptree* markerTree = OctDataManager::getInstance().getMarkerTree(s);

	if(!markerTree)
		return;


	if(actBscanMarker && !stateChangedSinceLastSave)
		stateChangedSinceLastSave = actBscanMarker->hasChangedSinceLastSave();

	if(actSloMarker && !stateChangedSinceLastSave)
		stateChangedSinceLastSave = actSloMarker->hasChangedSinceLastSave();


	for(BscanMarkerBase* obj : bscanMarkerObj)
	{
		const QString& markerId = obj->getMarkerId();
		bpt::ptree& subtree = PTreeHelper::get_put(*markerTree, markerId.toStdString());
		obj->newSeriesLoaded(s, subtree);
	}

	for(SloMarkerBase* obj : sloMarkerObj)
	{
		const QString& markerId = obj->getMarkerId();
		bpt::ptree& subtree = PTreeHelper::get_put(*markerTree, markerId.toStdString());
		obj->newSeriesLoaded(s, subtree);
	}

	if(s)
		extraSeriesData->loadExtraData(*s, *markerTree);


// 	emit(newBScanShowed(series->getBScan(actBScan)));
	emit(newSeriesShowed(s));
	actBScan = -1;
	chooseBScan(0);
}

void OctMarkerManager::setBscanMarkerTextID(QString id)
{
	int numId = 0;
	for(BscanMarkerBase* marker : bscanMarkerObj)
	{
		if(marker && marker->getMarkerId() == id)
		{
			setBscanMarker(numId);
			return;
		}
		++numId;
	}
}


void OctMarkerManager::setBscanMarker(int id)
{
	BscanMarkerBase* newMarker = nullptr;
		
	if(id < 0 || static_cast<std::size_t>(id) >= bscanMarkerObj.size())
		newMarker = nullptr;
	else
		newMarker = bscanMarkerObj.at(id);
	
	if(newMarker != actBscanMarker)
	{
		actBscanMarkerId = id;
		if(actBscanMarker)
		{
			actBscanMarker->activate(false);
			stateChangedSinceLastSave &= actBscanMarker->hasChangedSinceLastSave();
		}
		if(newMarker)
		{
// 			newMarker->setActBScan(actBScan);
			newMarker->activate(true);
		}
		
		actBscanMarker = newMarker;
		emit(bscanChanged(actBScan));
		emit(bscanMarkerChanged(actBscanMarker));
		emit(undoRedoStateChange());
		ProgramOptions::bscanMarkerToolId.setValue(actBscanMarkerId);
	}
}

void OctMarkerManager::setSloMarker(int id)
{
	SloMarkerBase* newMarker = nullptr;

	if(id < 0 || static_cast<std::size_t>(id) >= bscanMarkerObj.size())
		newMarker = nullptr;
	else
		newMarker = sloMarkerObj.at(id);

	if(newMarker != actSloMarker)
	{
		actSloMarkerId = id;
		if(actSloMarker)
			actSloMarker->activate(false);
		if(newMarker)
			newMarker->activate(true);

		actSloMarker = newMarker;
		emit(sloMarkerChanged(actSloMarker));
		ProgramOptions::sloMarkerToolId.setValue(actSloMarkerId);
	}
}



void OctMarkerManager::saveMarkerStateSlot(const std::shared_ptr<const OctData::Series>& s)
{
	if(series != s)
		return;

	bpt::ptree* markerTree = OctDataManager::getInstance().getMarkerTree(s);

	if(!markerTree)
		return;


	for(BscanMarkerBase* obj : bscanMarkerObj)
	{
		const QString& markerId = obj->getMarkerId();
		bpt::ptree& subtree = PTreeHelper::get_put(*markerTree, markerId.toStdString());
		obj->saveState(subtree);
	}

	for(SloMarkerBase* obj : sloMarkerObj)
	{
		const QString& markerId = obj->getMarkerId();
		bpt::ptree& subtree = PTreeHelper::get_put(*markerTree, markerId.toStdString());
		obj->saveState(subtree);
	}
}


void OctMarkerManager::loadMarkerStateSlot(const std::shared_ptr<const OctData::Series>& s)
{
	if(series != s)
		return;


	bpt::ptree* markerTree = OctDataManager::getInstance().getMarkerTree(s);

	if(!markerTree)
		return;


	for(BscanMarkerBase* obj : bscanMarkerObj)
	{
		const QString& markerId = obj->getMarkerId();
		bpt::ptree& subtree = PTreeHelper::get_put(*markerTree, markerId.toStdString());
		obj->loadState(subtree);
	}

	for(SloMarkerBase* obj : sloMarkerObj)
	{
		const QString& markerId = obj->getMarkerId();
		bpt::ptree& subtree = PTreeHelper::get_put(*markerTree, markerId.toStdString());
		obj->loadState(subtree);
	}
}


void OctMarkerManager::udateFromMarkerModul()
{
	 QObject* obj = sender();
	 if(obj == actBscanMarker)
	 {
		 emit(bscanChanged(actBScan));
	 }
}

void OctMarkerManager::updateUndoRedowState()
{
	 QObject* obj = sender();
	 if(obj == actBscanMarker)
	 {
		 emit(undoRedoStateChange());
	 }
}

void OctMarkerManager::bscanChangeRequestFromMarkerModul(int bscan)
{
	 QObject* obj = sender();
	 if(obj == actBscanMarker)
	 {
		chooseBScan(bscan);
	 }
}




bool OctMarkerManager::hasActMarkerChanged() const
{
	bool result = false;
	if(actBscanMarker) result |= actBscanMarker->hasChangedSinceLastSave();
	if(actSloMarker)   result |= actSloMarker  ->hasChangedSinceLastSave();
	return result;
}

void OctMarkerManager::handleSloRedrawAfterMarkerChange()
{
	emit(sloViewChanged());
}

void OctMarkerManager::sloOverlayUpdateFromMarkerModul()
{
	emit(sloOverlayChanged());
}


std::shared_ptr<const OctData::BScan> OctMarkerManager::getActBScan() const
{
	if(series)
		return series->getBScan(actBScan);
	return nullptr;
}

const ExtraImageData* OctMarkerManager::getExtraImageData() const
{
	return extraSeriesData->getBScanExtraData(actBScan);
}

std::size_t OctMarkerManager::numRedoSteps() const
{
	if(!actBscanMarker)
		return 0;
	return actBscanMarker->numRedoSteps();
}

std::size_t OctMarkerManager::numUndoSteps() const
{
	if(!actBscanMarker)
		return 0;
	return actBscanMarker->numUndoSteps();
}

void OctMarkerManager::callRedoStep()
{
	if(actBscanMarker)
		actBscanMarker->callRedoStep();
}

void OctMarkerManager::callUndoStep()
{
	if(actBscanMarker)
		actBscanMarker->callUndoStep();
}
