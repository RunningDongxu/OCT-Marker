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


/** \ingroup MarkerModule
 *  \defgroup ClassifierMarkerModule Classifier module
 *  \brief Module for classify volumes and B-scans
 *
 */

#ifndef SCANCLASSIFIER_H
#define SCANCLASSIFIER_H

#include"classifiermarkerstate.h"
#include"classifiermarkerproxy.h"
#include"definedclassifiermarker.h"

#include "../bscanmarkerbase.h"

/**
 *  @ingroup ClassifierMarkerModule
 *  @brief Main class of the classifier module
 *
 */
class ScanClassifier : public BscanMarkerBase
{
	Q_OBJECT

	/**
	 *  @ingroup ClassifierMarkerModule
	 *  @brief Hold on state of all classifier marker
	 *
	 */
	class ClassifierStates
	{
		std::vector<ClassifierMarkerState> states;
	public:
		ClassifierStates(const DefinedClassifierMarker::ClassifierMarkerMap& classifierMap);
		~ClassifierStates();

		ClassifierMarkerState& getState(std::size_t id)                { return states.at(id); }
		std::size_t size()                                       const { return states.size(); }

		void saveState(      boost::property_tree::ptree& markerTree) const;
		void loadState(const boost::property_tree::ptree& markerTree);

		void reset();
	};

public:
	/**
	 *  @ingroup ClassifierMarkerModule
	 *  @brief Hold all proxys for the ClassifierStates
	 *
	 *  On B-scan classifier, it forwards to the state of the concrete B-scan
	 */
	class ClassifierProxys
	{
	public:
		typedef std::vector<ClassifierMarkerProxy*> ProxyList;

		ClassifierProxys(const DefinedClassifierMarker::ClassifierMarkerMap& classifierMap);
		~ClassifierProxys();

		void setClassifierStates(ClassifierStates* states);

		ProxyList::iterator begin()                                    { return proxys.begin(); }
		ProxyList::iterator end()                                      { return proxys.end(); }

		bool hasChanges() const;
		void resetChanges();
	private:
		ProxyList proxys;
	};

	ScanClassifier(OctMarkerManager* markerManager);
	~ScanClassifier() override;

	QWidget* getWidget   ()          override               { return widgetPtr2WGScanClassifier; }

	void saveState(boost::property_tree::ptree& markerTree) override;
	void loadState(boost::property_tree::ptree& markerTree) override;
	void newSeriesLoaded(const OctData::Series*, boost::property_tree::ptree&) override;

	bool hasChangedSinceLastSave() const override           { return scanClassifierProxys.hasChanges() || slideClassifierProxys.hasChanges(); }

	ClassifierProxys& getScanClassifierProxys()  { return scanClassifierProxys; }
	ClassifierProxys& getBScanClassifierProxys() { return slideClassifierProxys; }


	void setActBScan(std::size_t bscan) override;

private:
	void loadBScansState(const boost::property_tree::ptree& markerTree);

	void resetStates();
	void updateStateProxys();

	bool     stateChangedSinceLastSave  = false;
	QWidget* widgetPtr2WGScanClassifier = nullptr;

	ClassifierStates scanClassifierStates;
	ClassifierProxys scanClassifierProxys;

	std::vector<ClassifierStates> slidesClassifierStates;
	ClassifierProxys slideClassifierProxys;
};

#endif // SCANCLASSIFIER_H
