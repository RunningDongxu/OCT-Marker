#pragma once

#include<vector>

#include <QObject>
#include <QIcon>

#include <boost/property_tree/ptree_fwd.hpp>

class QPainter;
class QMouseEvent;
class BScanMarkerWidget;
class QKeyEvent;
class QEvent;
class QToolBar;
class QContextMenuEvent;

class QGraphicsScene;

class OctMarkerManager;
class SLOImageWidget;
class WidgetOverlayLegend;

class MarkerCommand;


namespace OctData
{
	class Series;
	class BScan;
	class CoordSLOpx;
}

namespace cv
{
	class Mat;
}


class BscanMarkerBase : public QObject
{
	Q_OBJECT
public:
	class RedrawRequest
	{
	public:
		bool redraw = false;
		QRect rect;
	};


	BscanMarkerBase(OctMarkerManager* markerManager) : markerManager(markerManager) {}
	virtual ~BscanMarkerBase()                                      { clearUndoRedo(); }
	
	virtual bool drawingBScanOnSLO() const                          { return false; }
	virtual void drawBScanSLOLine  (QPainter&, std::size_t /*bscanNr*/, const OctData::CoordSLOpx& /*start_px*/, const OctData::CoordSLOpx& /*end_px*/   , SLOImageWidget*) const
	                                                                {}
	virtual void drawBScanSLOCircle(QPainter&, std::size_t /*bscanNr*/, const OctData::CoordSLOpx& /*start_px*/, const OctData::CoordSLOpx& /*center_px*/, bool /*clockwise*/, SLOImageWidget*) const
	                                                                {}
	virtual void drawMarker(QPainter&, BScanMarkerWidget*, const QRect& /*drawrect*/) const    {}
	virtual bool drawBScan() const                                  { return true;  }
	virtual bool drawSLOOverlayImage(const cv::Mat& /*sloImage*/, cv::Mat& /*outSloImage*/, double /*alpha*/) const
	                                                                { return false; }
	
	virtual RedrawRequest mouseMoveEvent   (QMouseEvent*, BScanMarkerWidget*) { return RedrawRequest(); }
	virtual RedrawRequest mousePressEvent  (QMouseEvent*, BScanMarkerWidget*) { return RedrawRequest(); }
	virtual RedrawRequest mouseReleaseEvent(QMouseEvent*, BScanMarkerWidget*) { return RedrawRequest(); }
	virtual void contextMenuEvent (QContextMenuEvent* /*event*/) {}
	
	virtual bool toolTipEvent     (QEvent*     , BScanMarkerWidget*) { return false; }
	virtual bool keyPressEvent    (QKeyEvent*  , BScanMarkerWidget*) { return false; }
	virtual bool leaveWidgetEvent (QEvent*     , BScanMarkerWidget*) { return false; }
	virtual bool setMarkerActive  (bool        , BScanMarkerWidget*);

	virtual void setActBScan(std::size_t /*bscan*/)                 {}
	virtual bool hasChangedSinceLastSave() const                    { return false; }
	
	virtual QToolBar* createToolbar(QObject*)                       { return nullptr; }
	virtual QWidget*  getWidget()                                   { return nullptr; }
	virtual WidgetOverlayLegend* getSloLegendWidget()               { return nullptr; }

	virtual QGraphicsScene* getGraphicsScene()                      { return nullptr; }
	virtual const QGraphicsScene* getGraphicsScene() const          { return nullptr; }
	
	virtual const QString& getName()                         const  { return name; }
	virtual const QString& getMarkerId()                     const  { return id;   }
	virtual const QIcon&   getIcon()                         const  { return icon; }
	
	virtual void activate(bool);
	virtual void saveState(boost::property_tree::ptree&)            {}
	virtual void loadState(boost::property_tree::ptree&)            { clearUndoRedo(); }
	
	virtual void newSeriesLoaded(const OctData::Series*, boost::property_tree::ptree&)
	                                                                { clearUndoRedo(); }

	std::size_t numUndoSteps()                                const { return undoList.size(); }
	std::size_t numRedoSteps()                                const { return redoList.size(); }


	std::size_t getActBScanNr() const;

public slots:
	void callRedoStep();
	void callUndoStep();
signals:
	void enabledToolbar(bool b);
	void requestFullUpdate();
	void requestSloOverlayUpdate();
	void sloViewHasChanged();
	void undoRedoChanged();
	
	
protected:
	OctMarkerManager* const markerManager;
	
	void connectToolBar(QToolBar* toolbar);
	int getBScanWidth() const;
	int getBScanHight() const;
	int getBScanWidth(std::size_t nr) const;
	
	const OctData::Series* getSeries() const;
	const OctData::BScan * getActBScan() const;
	const OctData::BScan * getBScan(std::size_t nr) const;

	bool drawSLOOverlayImage(const cv::Mat& sloImage, cv::Mat& outSloImage, double alpha, const cv::Mat& sloOverlay) const;

	void addUndoCommand(MarkerCommand* command);
	void clearUndoRedo();
	
	QString name;
	QString id;
	QIcon  icon;
	bool isActivated  = false;
	bool markerActive = true;
	
	std::vector<MarkerCommand*> undoList;
	std::vector<MarkerCommand*> redoList;

private:
	void clearRedo();
};

