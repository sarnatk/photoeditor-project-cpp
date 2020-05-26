#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QMainWindow>
#include <QDebug>
#include <QImage>
#include <QLineEdit>
#include <QSlider>
#include <QtGlobal>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/types_c.h"
#include "imgur.h"
#include "controller.h"
#include "sliders.h"

#if defined(QT_PRINTSUPPORT_LIB)

#  include <QtPrintSupport/qtprintsupportglobal.h>

#  if QT_CONFIG(printer)

#    include <QPrinter>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>

#  endif
#endif

QT_BEGIN_NAMESPACE
class QAction;

class QLabel;

class QMenu;

class QScrollArea;

class QScrollBar;

QT_END_NAMESPACE


class ImageViewer : public QMainWindow {
Q_OBJECT

public:
    ImageViewer(QWidget* parent = nullptr);

    bool loadFile(const QString&);

private slots:

    void scalingTime(qreal x);

    void animFinished();

    void open();

    void undo();

    void redo();

    void saveAs();

    void uploadToImgur();

    void print();

    void copy();

    void paste();

    void rotate();

    void color();

    void applyTint();

    void applySaturation();

    void applyBright();

    void applyLight();

    void applyHue();

    void applyContrast();

    void applyTemperature();

    void applySharp();

    void zoomIn();

    void zoomOut();

    void normalSize();

    void fitToWindow();

    void about();

private:
    QToolBar* createToolBar();

    void createActions();

    void updateActions();

    bool saveFile(const QString& fileName);

    void setImage(const cv::Mat& new_image);

    void scaleImage(double factor);

    void adjustScrollBar(QScrollBar* scrollBar, double factor);

    void wheelEvent(QWheelEvent* event) override;

    cv::Mat image;
    controller::Controller controller;
    QLabel* imageLabel;
    QScrollArea* scrollArea;
    double scaleFactor = 1;
    double tmpFactor = 1;
    int _numScheduledScalings = 1;

#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printer)
    QPrinter printer;
#endif

    QAction* toolUndoAct;
    QAction* toolRedoAct;
    QAction* toolRotateAct;
    QAction* toolColorAct;
    QAction* toolLightenAct;

    QAction* undoAct;
    QAction* redoAct;
    QAction* saveAsAct;
    QAction* uploadToImgurAct;
    QAction* printAct;
    QAction* copyAct;
    QAction* rotateAct;
    QAction* colorAct;
    QAction* tintAct;
    QAction* saturationAct;
    QAction* brightenAct;
    QAction* lightenAct;
    QAction* contrastAct;
    QAction* hueAct;
    QAction* temperatureAct;
    QAction* sharpenAct;
    QAction* zoomInAct;
    QAction* zoomOutAct;
    QAction* normalSizeAct;
    QAction* fitToWindowAct;

private:
    QLineEdit* edit = nullptr;
    SliderWindow* window = nullptr;

};

#endif

inline QImage cvMatToQImage(const cv::Mat& inMat) {
    switch (inMat.type()) {
        // 8-bit, 4 channel
        case CV_8UC4: {
            QImage image(inMat.data,
                         inMat.cols, inMat.rows,
                         static_cast<int>(inMat.step),
                         QImage::Format_ARGB32);

            return image;
        }

            // 8-bit, 3 channel
        case CV_8UC3: {
            QImage image(inMat.data,
                         inMat.cols, inMat.rows,
                         static_cast<int>(inMat.step),
                         QImage::Format_RGB888);

            return image.rgbSwapped();
        }

            // 8-bit, 1 channel
        case CV_8UC1: {
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
            QImage image(inMat.data,
                         inMat.cols, inMat.rows,
                         static_cast<int>(inMat.step),
                         QImage::Format_Grayscale8);
#else
            static QVector<QRgb>  sColorTable;

            // only create our color table the first time
            if ( sColorTable.isEmpty() )
            {
               sColorTable.resize( 256 );

               for ( int i = 0; i < 256; ++i )
               {
                  sColorTable[i] = qRgb( i, i, i );
               }
            }

            QImage image( inMat.data,
                          inMat.cols, inMat.rows,
                          static_cast<int>(inMat.step),
                          QImage::Format_Indexed8 );

            image.setColorTable( sColorTable );
#endif

            return image;
        }

        default:
            qWarning() << "ASM::cvMatToQImage() - cv::Mat image type not handled in switch:" << inMat.type();
            break;
    }

    return QImage();
}

inline QPixmap cvMatToQPixmap(const cv::Mat& inMat) {
    return QPixmap::fromImage(cvMatToQImage(inMat));
}