#ifndef CVIMAGEWIDGET_H
#define CVIMAGEWIDGET_H


#include <QWidget>
#include <QImage>

#include <opencv2/opencv.hpp>


// http://develnoter.blogspot.de/2012/05/integrating-opencv-in-qt-gui.html

class QMenu;
class QContextMenuEvent;
class CVImageWidget : public QWidget
{
	Q_OBJECT
	virtual void contextMenuEvent(QContextMenuEvent* event);

	enum class ScaleMethod { Factor, Size };
	
	double scaleFactor = 1.;
	bool grayCvImage;
	ScaleMethod scaleMethod = ScaleMethod::Factor;
	
public:
	explicit CVImageWidget(QWidget *parent = 0);
	virtual ~CVImageWidget();

	QSize sizeHint()        const                               { return qtImage.size(); }
	QSize minimumSizeHint() const                               { return qtImage.size(); }

	virtual void setImageSize(QSize size)                       { imageScale  = size  ; scaleMethod = ScaleMethod::Size  ; cvImage2qtImage(); }
	virtual void setScaleFactor(double factor)                  { scaleFactor = factor; scaleMethod = ScaleMethod::Factor; cvImage2qtImage(); }

	int  imageHight() const;
	int  imageWidth() const;

	int  scaledImageHeight() const                              { return qtImage.height(); }
	int  scaledImageWidth() const                               { return qtImage.width() ; }

	double getImageScaleFactor()                                { return scaleFactor; }

	void addZoomItems();

protected:
	void paintEvent(QPaintEvent* event);

	QImage   qtImage;
	cv::Mat  cvImage;
	QMenu*   contextMenu;
	QAction* saveAction;
	QSize    imageScale;
	
	
	QString translateFileFormat(const QString& format) const;
	int fileDialog(QString& filename);

	void cvImage2qtImage();
	void updateScaleFactor();
	
public slots:
	virtual void saveImage();
	void showImage(const cv::Mat& image);

	void setZoom(double factor)                                  { if(scaleFactor != factor && factor <= 5 && factor > 0) { scaleFactor = factor; cvImage2qtImage(); zoomChanged(factor); } }

	void setZoom1()                                              { setZoom(1); }
	void setZoom2()                                              { setZoom(2); }
	void setZoom3()                                              { setZoom(3); }
	void setZoom4()                                              { setZoom(4); }
	void setZoom5()                                              { setZoom(5); }

	void zoom_in()                                               { setZoom(scaleFactor+1); }
	void zoom_out()                                              { setZoom(scaleFactor-1); }

signals:
	void zoomChanged(double);
};

#endif // CVIMAGEWIDGET_H
