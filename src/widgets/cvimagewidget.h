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

#ifndef CVIMAGEWIDGET_H
#define CVIMAGEWIDGET_H


#include <QWidget>
#include <QImage>

#include <opencv2/opencv.hpp>

#include<data_structure/scalefactor.h>


// http://develnoter.blogspot.de/2012/05/integrating-opencv-in-qt-gui.html

class QMenu;
class QContextMenuEvent;

class FilterImage;

/**
 * @ingroup Widget
 * @brief
 */
class CVImageWidget : public QWidget
{
	Q_OBJECT

	enum class ScaleMethod { Factor, Size };
	
	double scaleFactorConfig = 1.;
	double aspectRatio       = 1;

	ScaleFactor scaleFactor;

	bool grayCvImage;
	ScaleMethod scaleMethod = ScaleMethod::Factor;
	QSize scaledSize;

	const FilterImage* imageFilter = nullptr;
	
	void addZoomAction(int zoom);

	void updateScaleFactorXY();

public:
	enum class FloatGrayTransform { Auto, Fix, ZeroToOne };

	explicit CVImageWidget(QWidget *parent = 0);
	~CVImageWidget() override;

	QSize sizeHint()        const                      override { return scaledSize; }
	QSize minimumSizeHint() const                      override { return scaledSize/100; }

	virtual void setImageSize(QSize size)                       { imageScale        = size  ; scaleMethod = ScaleMethod::Size  ; updateScaleFactorXY(); cvImage2qtImage(); }
	virtual void setScaleFactor(double factor)                  { scaleFactorConfig = factor; scaleMethod = ScaleMethod::Factor; updateScaleFactorXY(); cvImage2qtImage(); }

	int  imageHight() const                                     { return qtImage.height(); }
	int  imageWidth() const                                     { return qtImage.width (); }

	int  scaledImageHeight() const                              { return scaledSize.height(); }
	int  scaledImageWidth() const                               { return scaledSize.width() ; }

// 	double getImageScaleFactor()                          const { return scaleFactor ; }
	const ScaleFactor& getImageScaleFactor()              const { return scaleFactor; }
	double getScaleFactor()                               const { return scaleFactorConfig; }
// 	double getImageScaleFactorY()                         const { return scaleFactorY; }

	void addZoomItems();

	void setFloatGrayTransform(FloatGrayTransform transform)    { floatGrayTransform = transform; }
	void setGrayTransformValueA(double val)                     { grayTransformA = val; }
	void setGrayTransformValueB(double val)                     { grayTransformB = val; }

	void setImageFilter(const FilterImage* imageFilter);

	static void drawScaled(const QImage& image, QPainter& painter, const QRect* rect, const ScaleFactor& sf);
protected:
	void paintEvent(QPaintEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;
	void contextMenuEvent(QContextMenuEvent* event) override;

	QImage   qtImage;
	cv::Mat  cvImage;
	cv::Mat  outputImage;

	QMenu*   contextMenu;
	QAction* saveAction;
	QSize    imageScale;
	
	
	QString translateFileFormat(const QString& format) const;
	int fileDialog(QString& filename);

	void cvImage2qtImage();
	void updateScaleFactor();
	static void cvImage2qtImage(const cv::Mat& cvImage, QImage& qimage);

	void setZoomInternal(double factor)                          { if(scaleFactorConfig != factor && factor <= 25 && factor > 0) { scaleFactorConfig = factor; updateScaleFactorXY(); zoomChanged(factor); cvImage2qtImage(); } }
	double getFactorFitImage2Parent();

public slots:
	virtual void saveImage();
	virtual void saveBaseImage();
	virtual void showImage(const cv::Mat& image);

	virtual void setZoom(double factor)                          { setZoomInternal(factor); }

	void fitImage2Parent()                                       { setZoom(getFactorFitImage2Parent()); }
	void fitImage2ParentWidth ();
	void fitImage2ParentHeight();

	void zoom_in()                                               { setZoom(scaleFactorConfig+0.5); }
	void zoom_out()                                              { setZoom(scaleFactorConfig-0.5); }

	void setAspectRatio(double v);

private slots:
	void imageParameterChanged();

signals:
	void zoomChanged(double);
	void needScrollTo(int x, int y);
	void sizeChanged();

private:
	FloatGrayTransform floatGrayTransform = FloatGrayTransform::ZeroToOne;
	double grayTransformA = 255;
	double grayTransformB = 0;
};


inline void CVImageWidget::setAspectRatio(double v)
{
	if(aspectRatio != v)
	{
		aspectRatio = v;
		updateScaleFactorXY();
		cvImage2qtImage();
	}
}

#endif // CVIMAGEWIDGET_H
