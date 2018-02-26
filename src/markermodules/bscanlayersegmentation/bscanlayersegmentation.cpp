#include "bscanlayersegmentation.h"

#define _USE_MATH_DEFINES
#include <cmath>


#include<opencv/cv.hpp>

#include <widgets/bscanmarkerwidget.h>
#include <QPainter>
#include <QMouseEvent>

#include <octdata/datastruct/series.h>
#include <octdata/datastruct/bscan.h>
#include <data_structure/programoptions.h>

#include "wglayerseg.h"


#include"editbase.h"
#include"editpen.h"
#include"editspline.h"

#include"bscanlayersegptree.h"
#include <manager/octmarkermanager.h>

#include <cpp_framework/cvmat/cvmattreestruct.h>
#include <cpp_framework/cvmat/treestructbin.h>
#include "layersegmentationio.h"

#include"thicknessmap.h"
#include <qelapsedtimer.h>


#include"thicknessmaplegend.h"
#include<markermodules/widgetoverlaylegend.h>
#include <manager/octdatamanager.h>
#include "colormaphsv.h"
#include "layersegcommand.h"


const std::array<OctData::Segmentationlines::SegmentlineType, 10> BScanLayerSegmentation::keySeglines = {{
	  OctData::Segmentationlines::SegmentlineType::RPE
	, OctData::Segmentationlines::SegmentlineType::ILM
	, OctData::Segmentationlines::SegmentlineType::BM
	, OctData::Segmentationlines::SegmentlineType::RNFL
	, OctData::Segmentationlines::SegmentlineType::GCL
	, OctData::Segmentationlines::SegmentlineType::IPL
	, OctData::Segmentationlines::SegmentlineType::INL
	, OctData::Segmentationlines::SegmentlineType::OPL
	, OctData::Segmentationlines::SegmentlineType::ELM
	, OctData::Segmentationlines::SegmentlineType::PR1
}};

BScanLayerSegmentation::BScanLayerSegmentation(OctMarkerManager* markerManager)
: BscanMarkerBase(markerManager)
, editMethodSpline(new EditSpline(this))
, editMethodPen   (new EditPen   (this))
, thicknesMapImage(new cv::Mat)
{
	name = tr("Layer Segmentation");
	id   = "LayerSegmentation";
	icon = QIcon(":/icons/typicons_mod/layer_seg.svg");

	setSegMethod(SegMethod::Pen);

	thicknessMapLegend   = new ThicknessmapLegend;
	legendWG             = new WidgetOverlayLegend(thicknessMapLegend);
	widgetPtr2WGLayerSeg = new WGLayerSeg(this);

	connect(&ProgramOptions::layerSegThicknessmapBlend, &OptionBool::valueChanged, this, &BScanLayerSegmentation::generateThicknessmap);
}

BScanLayerSegmentation::~BScanLayerSegmentation()
{
	delete editMethodSpline;
	delete editMethodPen   ;

	delete thicknesMapImage;
// 	delete thicknessMapLegend; // TODO
	delete legendWG;
}



void BScanLayerSegmentation::drawMarker(QPainter& painter, BScanMarkerWidget* widget, const QRect& rec) const
{
	if(!showSegmentationlines)
		return;

	int bScanHeight = getBScanHight();
	const ScaleFactor& scaleFactor = widget->getImageScaleFactor();

	QPen penEdit;
	penEdit.setColor(ProgramOptions::layerSegActiveLineColor());
	penEdit.setWidth(ProgramOptions::layerSegActiveLineSize ());

	QPen penNormal;
	penNormal.setColor(ProgramOptions::layerSegPassivLineColor());
	penNormal.setWidth(ProgramOptions::layerSegPassivLineSize ());


	painter.setPen(penNormal);
	for(OctData::Segmentationlines::SegmentlineType type : OctData::Segmentationlines::getSegmentlineTypes())
	{
		if(type == actEditType)
			continue;

		BScanMarkerWidget::paintSegmentationLine(painter, bScanHeight, lines[getActBScanNr()].lines.getSegmentLine(type), scaleFactor);
	}

	painter.setPen(penEdit);
	BScanMarkerWidget::paintSegmentationLine(painter, bScanHeight, tempLine, scaleFactor);


	if(actEditMethod)
		actEditMethod->drawMarker(painter, widget, rec, scaleFactor);
}



BscanMarkerBase::RedrawRequest BScanLayerSegmentation::mouseMoveEvent(QMouseEvent* event, BScanMarkerWidget* widget)
{
	if(!showSegmentationlines)
		return BscanMarkerBase::RedrawRequest();
	if(actEditMethod)
		return actEditMethod->mouseMoveEvent(event, widget);
	return BscanMarkerBase::RedrawRequest();
}

BscanMarkerBase::RedrawRequest BScanLayerSegmentation::mousePressEvent(QMouseEvent* event, BScanMarkerWidget* widget)
{
	if(!showSegmentationlines)
		return BscanMarkerBase::RedrawRequest();
// 	paintSegLine = true;
// 	double scaleFactor = widget->getImageScaleFactor();
// 	lastPoint = calcPoint(event->x(), event->y(), scaleFactor, getBScanWidth());
	if(actEditMethod)
		return actEditMethod->mousePressEvent(event, widget);
	return BscanMarkerBase::RedrawRequest();
}

BscanMarkerBase::RedrawRequest BScanLayerSegmentation::mouseReleaseEvent(QMouseEvent* event, BScanMarkerWidget* widget)
{
	if(actEditMethod)
		return actEditMethod->mouseReleaseEvent(event, widget);
// 	paintSegLine = false;
	return BscanMarkerBase::RedrawRequest();
}

void BScanLayerSegmentation::contextMenuEvent(QContextMenuEvent* event)
{
	if(!showSegmentationlines)
		return;
	if(actEditMethod)
		actEditMethod->contextMenuEvent(event);
}


void BScanLayerSegmentation::newSeriesLoaded(const OctData::Series* series, boost::property_tree::ptree& ptree)
{
	BscanMarkerBase::newSeriesLoaded(series, ptree);
	*thicknesMapImage = cv::Mat();
	resetMarkers(series);
	loadState(ptree);
}


void BScanLayerSegmentation::resetMarkers(const OctData::Series* series)
{
	if(!series)
		return;

	const std::size_t numBscans = series->bscanCount();

	lines.clear();
	lines.resize(numBscans);

	for(std::size_t bscanNr = 0; bscanNr<numBscans; ++bscanNr)
		resetMarkers(bscanNr);

	updateEditLine();
}

void BScanLayerSegmentation::resetMarkers(std::size_t bscanNr)
{
	BScanSegData& segData = lines.at(bscanNr);
	const OctData::BScan* bscan = getBScan(bscanNr);
	if(!bscan)
		return;

	const std::size_t bscanWidth = static_cast<std::size_t>(bscan->getWidth());

	segData.lines  = bscan->getSegmentLines();
	segData.filled = true;

	for(OctData::Segmentationlines::SegmentlineType type : OctData::Segmentationlines::getSegmentlineTypes())
	{
		std::size_t typeId = static_cast<std::size_t>(type);
		segData.lineModified[typeId] = false;
		segData.lineLoaded  [typeId] = false;

		OctData::Segmentationlines::Segmentline& segline = segData.lines.getSegmentLine(type);
		std::size_t seglineSize = segline.size();
		segline.resize(bscanWidth);
		for(std::size_t i = seglineSize; i < bscanWidth; ++i)
			segline[i] = std::numeric_limits<double>::quiet_NaN();
	}
}



void BScanLayerSegmentation::setActEditLinetype(OctData::Segmentationlines::SegmentlineType type)
{
	if(type == actEditType)
		return;

	actEditType = type;

	updateEditLine();
	if(actEditMethod)
		emit(segLineIdChanged(static_cast<int>(type)));

	requestFullUpdate();
}


std::vector<double> BScanLayerSegmentation::getSegPart(const std::vector<double>& segLine, std::size_t ascanBegin, std::size_t ascanEnd)
{
	if(ascanBegin >= ascanEnd)
		return std::vector<double>();

	if(ascanEnd >= segLine.size())
		return std::vector<double>();

	auto it1 = segLine.begin() + ascanBegin;
	auto it2 = segLine.begin() + ascanEnd;

	return std::vector<double>(it1, it2);
}


void BScanLayerSegmentation::rangeModified(std::size_t ascanBegin, std::size_t ascanEnd)
{
	const std::size_t bscanNr = getActBScanNr();
	if(lines.size() <= bscanNr)
		return;


	OctData::Segmentationlines::Segmentline& line = lines[bscanNr].lines.getSegmentLine(actEditType);

// 	LayerSegCommand(BScanLayerSegmentation* parent, std::size_t start, std::vector<double>&& newPart, std::vector<double>&& oldPart);
	LayerSegCommand* command = new LayerSegCommand(this, ascanBegin, getSegPart(tempLine, ascanBegin, ascanEnd), getSegPart(line, ascanBegin, ascanEnd));
	addUndoCommand(command);

	modifiedSegPart(bscanNr, actEditType, ascanBegin, ascanEnd-ascanBegin, tempLine.data() + ascanBegin, false);



	/*
	lines[bscanNr].lineModified[static_cast<std::size_t>(actEditType)] = true;

	OctData::Segmentationlines::Segmentline& line = lines[bscanNr].lines.getSegmentLine(actEditType);

	const std::size_t endPos = std::min(std::min(ascanEnd, line.size()), line.size());

	for(std::size_t i = ascanBegin; i < endPos; ++i)
		line[i] = tempLine[i];
	*/
}

void BScanLayerSegmentation::modifiedSegPart(std::size_t bscan, OctData::Segmentationlines::SegmentlineType segLine, std::size_t start, std::size_t length, const double* segPart, bool updateMethode)
{
	if(lines.size() <= bscan)
		return;

	lines[bscan].lineModified[static_cast<std::size_t>(segLine)] = true;
	OctData::Segmentationlines::Segmentline& line = lines[bscan].lines.getSegmentLine(segLine);

	const std::size_t endPos = std::min(start+length, line.size());

	for(std::size_t i = start; i < endPos; ++i)
	{
		line[i] = *segPart;
		++segPart;
	}

	if(updateMethode)
	{
		if(bscan == getActBScanNr())
		{
			if(actEditType == segLine)
				updateEditLine();
			requestFullUpdate();
		}
	}
}




bool BScanLayerSegmentation::keyPressEvent(QKeyEvent* event, BScanMarkerWidget* widget)
{
	int key = event->key();
	if(key >= Qt::Key_0 && key <= Qt::Key_9)
	{
		setActEditLinetype(keySeglines[key-Qt::Key_0]);
	}
	else
	{
		switch(key)
		{
	#ifndef NDEBUG
			case Qt::Key_C:
				if(event->modifiers() == Qt::ShiftModifier)
					copyAllSegLinesFromOctData();
				else
					copySegLinesFromOctData();
				return true;
	#endif
			case Qt::Key_Q:
				setSegMethod(BScanLayerSegmentation::SegMethod::Pen);
				return true;

			case Qt::Key_W:
				setSegMethod(BScanLayerSegmentation::SegMethod::Spline);
				return true;

			case Qt::Key_E:
				setSegmentationLinesVisible(!showSegmentationlines);
				return true;

			case Qt::Key_T:
				generateThicknessmap();
				return true;
		}
	}

	if(actEditMethod)
		return actEditMethod->keyPressEvent(event, widget);

	return false;
}

void BScanLayerSegmentation::generateThicknessmap()
{
	if(thicknessmapConfig.colormap)
	{
// 		QElapsedTimer timer;
// 		timer.start();

		const OctData::BScan* bscan = getActBScan();
		OctDataManager& manager = OctDataManager::getInstance();
		const SloBScanDistanceMap* distMap = manager.getSeriesSLODistanceMap();
		if(bscan && distMap)
		{
			double factor = bscan->getScaleFactor().getZ()*1000; // milli meter -> micro meter

			ThicknessMap tm;
			tm.createMap(*distMap, lines, thicknessmapConfig.upperLayer, thicknessmapConfig.lowerLayer, factor, *thicknessmapConfig.colormap);
			*thicknesMapImage = tm.getThicknessMap();
			showThicknessmap = true;
			requestSloOverlayUpdate();

// 			std::cout << "Creating thickness map took " << timer.elapsed() << " milliseconds" << std::endl;
		}
	}

	thicknessMapLegend->setColormap(thicknessmapConfig.colormap);
}


bool BScanLayerSegmentation::drawSLOOverlayImage(const cv::Mat& sloImage, cv::Mat& outSloImage, double alpha) const
{
	if(thicknesMapImage && showThicknessmap)
		return BscanMarkerBase::drawSLOOverlayImage(sloImage, outSloImage, alpha, *thicknesMapImage);
	return false;
}



void BScanLayerSegmentation::copyAllSegLinesFromOctData()
{
	for(std::size_t i = 0; i<lines.size(); ++i)
		copySegLinesFromOctData(i);
}


void BScanLayerSegmentation::copySegLinesFromOctData() { copySegLinesFromOctData(getActBScanNr()); }

void BScanLayerSegmentation::copySegLinesFromOctData(const std::size_t bScanNr)
{
	const OctData::BScan* bscan = getBScan(bScanNr);
	if(!bscan)
		return;

	resetMarkers(bScanNr);

	if(bScanNr == getActBScanNr())
	{
		updateEditLine();

		requestFullUpdate();
	}
}

void BScanLayerSegmentation::setSegMethod(BScanLayerSegmentation::SegMethod method)
{
	if(getSegMethod() == method)
		return;

	switch(method)
	{
		case BScanLayerSegmentation::SegMethod::None:
			actEditMethod = nullptr;
			break;
		case BScanLayerSegmentation::SegMethod::Pen:
			actEditMethod = editMethodPen;
			break;
		case BScanLayerSegmentation::SegMethod::Spline:
			actEditMethod = editMethodSpline;
			break;
	}

	updateEditLine();

	emit(segMethodChanged());

	requestFullUpdate();
}


BScanLayerSegmentation::SegMethod BScanLayerSegmentation::getSegMethod() const
{
	if(actEditMethod == editMethodPen   ) return BScanLayerSegmentation::SegMethod::Pen;
	if(actEditMethod == editMethodSpline) return BScanLayerSegmentation::SegMethod::Spline;

	return BScanLayerSegmentation::SegMethod::None;
}


void BScanLayerSegmentation::copySegLinesFromOctDataWhenNotFilled() { copySegLinesFromOctDataWhenNotFilled(getActBScanNr()); }

void BScanLayerSegmentation::copySegLinesFromOctDataWhenNotFilled(std::size_t bscan)
{
	if(bscan < lines.size())
	{
		BScanSegData& segData = lines[bscan];
		if(!segData.filled)
			copySegLinesFromOctData(bscan);
	}
}



void BScanLayerSegmentation::setActBScan(std::size_t bscan)
{
	BscanMarkerBase::setActBScan(bscan);
	updateEditLine();
}

void BScanLayerSegmentation::loadState(boost::property_tree::ptree& markerTree)
{
	BscanMarkerBase::loadState(markerTree);
	BScanLayerSegPTree::parsePTree(markerTree, this);
}

void BScanLayerSegmentation::saveState(boost::property_tree::ptree& markerTree)
{
	BscanMarkerBase::saveState(markerTree);
	BScanLayerSegPTree::fillPTree(markerTree, this);
}

bool BScanLayerSegmentation::saveSegmentation2Bin(const std::string& filename)
{
	return LayerSegmentationIO::saveSegmentation2Bin(*this, filename);
}



std::size_t BScanLayerSegmentation::getMaxBscanWidth() const // TODO: Codedopplung mit IntervalMarker
{
	const OctData::Series* series = getSeries();
	if(!series)
		return 0;

	std::size_t maxBscanWidth = 0;
	for(const OctData::BScan* bscan : series->getBScans())
	{
		if(bscan)
		{
			if(maxBscanWidth < bscan->getWidth())
				maxBscanWidth = static_cast<std::size_t>(bscan->getWidth());
		}
	}
	return maxBscanWidth;
}

void BScanLayerSegmentation::setIconsToSimple(int size)
{
	WGLayerSeg* widget = dynamic_cast<WGLayerSeg*>(widgetPtr2WGLayerSeg);
	if(widget)
		widget->setIconsToSimple(size);
}

void BScanLayerSegmentation::setSegmentationLinesVisible(bool visible)
{
	if(visible != showSegmentationlines)
	{
		showSegmentationlines = visible;
		emit(segLineVisibleChanged(visible));
		requestFullUpdate();
	}
}

void BScanLayerSegmentation::setThicknessmapVisible(bool visible)
{
	if(visible != showThicknessmap)
	{
		showThicknessmap = visible;
		requestSloOverlayUpdate();
	}
}



WidgetOverlayLegend* BScanLayerSegmentation::getSloLegendWidget()
{
	if(thicknesMapImage->empty() || !showThicknessmap)
		return nullptr;
	return legendWG;
}


void BScanLayerSegmentation::ThicknessmapConfig::setHSVColor()
{
	static ColormapHSV hsvMap;
	colormap = &hsvMap;
}

void BScanLayerSegmentation::ThicknessmapConfig::setYellowColor()
{
	static ColormapYellow mapYellow;
	colormap = &mapYellow;
}

void BScanLayerSegmentation::ThicknessmapConfig::setUpperColorLimit(double thickness)
{
	if(colormap)
		colormap->setMaxValue(thickness);
}

void BScanLayerSegmentation::updateEditLine()
{
	const std::size_t bscanNr = getActBScanNr();
	if(lines.size() <= bscanNr)
		return;

	OctData::Segmentationlines::Segmentline& line = lines[bscanNr].lines.getSegmentLine(actEditType);
	tempLine = line;

	if(actEditMethod)
		actEditMethod->segLineChanged(&tempLine);
}


bool BScanLayerSegmentation::hasChangedSinceLastSave() const
{
	for(const BScanSegData& data : lines)
	{
		for(bool modified : data.lineModified)
			if(modified)
				return true;
	}
	return false;
}
