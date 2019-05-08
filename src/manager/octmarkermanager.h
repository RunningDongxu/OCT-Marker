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

#ifndef MARKERMANAGER_H
#define MARKERMANAGER_H

#include <QObject>
#include <vector>

// #include <boost/property_tree/ptree_fwd.hpp>

namespace OctData
{
	class Series;
	class BScan;
}

class BscanMarkerBase;
class SloMarkerBase;
class ExtraSeriesData;
class ExtraImageData;
class QPaintEvent;

class BScanMarkerWidget;

/**
 * @ingroup Manager
 * @brief
 *
 */
class OctMarkerManager : public QObject
{
public:
	static OctMarkerManager& getInstance()                          { static OctMarkerManager instance; return instance; }

	int getActBScanNum() const                                      { return actBScan; }
	const OctData::Series* getSeries() const                        { return series;   }
	const OctData::BScan * getActBScan () const;


	BscanMarkerBase* getActBscanMarker()                            { return actBscanMarker; }
	int getActBscanMarkerId() const                                 { return actBscanMarkerId; }
	const std::vector<BscanMarkerBase*>& getBscanMarker() const     { return bscanMarkerObj; }

	SloMarkerBase* getActSloMarker()                                { return actSloMarker; }
	int getActSloMarkerId() const                                   { return actSloMarkerId; }
	const std::vector<SloMarkerBase*>& getSloMarker() const         { return sloMarkerObj; }

	bool hasChangedSinceLastSave() const                            { if(stateChangedSinceLastSave) return true; return hasActMarkerChanged(); }
	void resetChangedSinceLastSaveState()                           { stateChangedSinceLastSave = false; }

	const ExtraImageData* getExtraImageData() const;

	std::size_t numUndoSteps() const;
	std::size_t numRedoSteps() const;

	void callUndoStep();
	void callRedoStep();

private:
	OctMarkerManager();
	virtual ~OctMarkerManager();

	int                    actBScan = 0;
	const OctData::Series* series   = nullptr;
	
	std::vector<BscanMarkerBase*> bscanMarkerObj;
	BscanMarkerBase* actBscanMarker = nullptr;
	int actBscanMarkerId = -1;

	std::vector<SloMarkerBase*> sloMarkerObj;
	SloMarkerBase* actSloMarker = nullptr;
	int actSloMarkerId = -1;
	bool stateChangedSinceLastSave = false;

	bool hasActMarkerChanged() const;

	ExtraSeriesData* extraSeriesData = nullptr;


private slots:
	virtual void saveMarkerStateSlot(const OctData::Series* series);
	virtual void loadMarkerStateSlot(const OctData::Series* series);
	virtual void reloadMarkerStateSlot()                            { loadMarkerStateSlot(series); }

	virtual void udateFromMarkerModul();
	void bscanChangeRequestFromMarkerModul(int bscan);
	void sloOverlayUpdateFromMarkerModul();

	void handleSloRedrawAfterMarkerChange();

	void updateUndoRedowState();

public slots:
	virtual void chooseBScan(int bscan);
	virtual void inkrementBScan(int inkrement)                      { chooseBScan(actBScan + inkrement); }

	virtual void nextBScan()                                        { inkrementBScan(+1); }
	virtual void previousBScan()                                    { inkrementBScan(-1); }

	virtual void showSeries(const OctData::Series* series);
	
	virtual void setBscanMarker(int id);
	virtual void setBscanMarkerTextID(QString id);
	virtual void setSloMarker  (int id);

signals:
	void bscanChanged      (int bscan);
	void sloViewChanged    ();
	void newSeriesShowed   (const OctData::Series* series);
	void newBScanShowed    (const OctData::BScan * series);
	void bscanMarkerChanged(BscanMarkerBase* marker);
	void sloMarkerChanged  (SloMarkerBase  * marker);
	void sloOverlayChanged ();
	void undoRedoStateChange();

private:
	Q_OBJECT
};

#endif // MARKERMANAGER_H
