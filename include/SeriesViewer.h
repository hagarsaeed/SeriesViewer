#pragma once

#include "ui_SeriesViewer.h"

#include <QtWidgets/QMainWindow>

#include <QtCore>
#include <QtGui>
#include <iostream>

//DCMTK Library
#include <dcmtk/dcmimgle/dcmimage.h>
#include <dcmtk/dcmdata/dcdatset.h>
#include <dcmtk/dcmdata/dctk.h>
#include "dcmtk/config/osconfig.h"

//GUI
#include <QTableWidget>
#include <qmessagebox.h>
#include <qprogressbar.h>
#include <QProgressDialog.h>
#include <QVTKOpenGLNativeWidget.h>
#include <qfiledialog.h>
#include <qpixmap.h>
#include <qfilesystemmodel.h>

#include <QVector>
#include <qlist.h>
#include <limits>

//VTK Library
#include <vtkImageData.h>
#include <vtkMetaImageReader.h>
#include <vtkSmartPointer.h>
#include <vtkImageProperty.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderer.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageSliceMapper.h>
#include <vtkImageAccumulate.h>
#include <vtkImageViewer2.h>
#include <vtkImageHistogram.h>
#include <vtkChartXY.h>
#include <vtkContextView.h>
#include <vtkPlot.h>
#include <vtkAxis.h>
#include <vtkPlotBar.h>
#include <vtkContextView.h>
#include <vtkTable.h>
#include <vtkIntArray.h>
#include <vtkContextView.h>
#include <vtkPointData.h>
#include <vtkAutoInit.h>
#include <vtkImageMagnitude.h>
#include <vtkSmartPointer.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkDoubleArray.h>
#include <vtkIntArray.h>
#include <vtkDataArray.h>
#include <vtkImageAccumulate.h>
#include <qregularexpression.h>
#include <vtkMatrix4x4.h>
#include <QTransform>
#include <qchartview.h>
#include <qchart.h>
#include <qbarseries.h>
#include <qbarset.h>
#include <QPointF>
#include <qvector.h>
#include <QMap>
#include <QPair>
#include <qvalueaxis.h>
#include <QBarCategoryAxis.h>
#include <qlineseries.h>
#include <qStackedBarSeries.h>
#include <vtkUnsignedCharArray.h>

#include "itkImageFileReader.h"
#include "vtkImageMagnitude.h"
#include "itkVTKImageToImageFilter.h"
#include "itkImageToVTKImageFilter.h"
#include "itkImageFileWriter.h"

#include <omp.h>

#include <opencv2/opencv.hpp>
#include <vector>
#include <algorithm>
#include <gtest/gtest.h>

// Define Tag of Dicom File (tuple type) 
typedef std::tuple<std::string, std::string, std::string, unsigned long, unsigned int, std::string> Tag;

// Define Dicom Tags for Every DICOM file (vector of tuples) 
typedef std::vector<Tag> DicomTags;

// Define the all Dicom Files Tags (QList that holds vectors of tuples)
typedef std::vector<DicomTags> AllDicomTags;

typedef std::vector <QImage> AllDicoms;

class SeriesViewer : public QMainWindow
{
    Q_OBJECT

public:
    SeriesViewer(QWidget *parent = nullptr);
    ~SeriesViewer();

    //Variables
    QStringList dicomFiles;
    int dicomIndex = 0;
    QStringList dicomFileNames;
    AllDicoms allDicomImages8bit;
    AllDicoms allDicomImages16bit;
    AllDicomTags allDicomTags;
    int sliceNumber;
    QString previousFileName;
    QProgressDialog progressDialog;
    QProgressDialog progressDialogMHA;
    bool progressDialogCanceled;
    bool progressDialogMHACanceled;
    int cnt = 0;
    int cntRotateDicom = 0;
    QGraphicsPixmapItem* item;
    int previousDicom = 0;
    QVector<QChart*> chartsDicom;
    QVector<QChart*> chartsDicomXZero;
    QVector<QChart*> mhaHistogramsCharts;
    QVector<QChart*> mhaHistogramsChartsWithoutZero;
    QVector <double> DicomAverage16bit;
 
    vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
    vtkSmartPointer<vtkMetaImageReader> mhaReader;
    vtkSmartPointer<vtkImageSlice> imageSlice;
    vtkSmartPointer<vtkImageSliceMapper>  imageSliceMapper;
    QVector<QPair<double, double>> ranges;
    QVector<QPair<double, double>> rangesDicom;
    QPair<double, double> totalHistogramRange;
    QVector<size_t> totalHistogram;
    QVector<size_t> totalHistogramXZero;


    //DICOM 
    QImage readDICOM(const QString& filePath);
    QImage rescale16BitTo8Bit(const unsigned short* pixelData, int width, int height);
    void LoadAllDicoms();
    void displayDICOM(const QImage& DicomImage);
    void loadDicomFiles(const QString& directoryPath);
    void loadAllDicomData();
    void displayDicomTab();
    
    void disableDicomTabGUI();
    void enableDicomTabGUI();


    std::string addSpacesToTagName(const std::string& tagName);

    //Tags
    DicomTags loadTags(const QString &filename);
    void loadAllTags();
    void viewDicomTags(const DicomTags& DicomTags);
    void saveDicomTagsToFile(const DicomTags& dicomTags, const QString &DicomFile);

    QStringList getDicomFiles(const QString& directoryPath);


    //MHA Viewer
    void displayMeta();
    void getMHA_Info(QString filePath);
    void mhaSliceAverage();
    void rotateMHASlice();
    void clearMHATab();
    template<class TImageType>
    std::map<double, size_t> getUniqueValuesHistogram(typename TImageType::Pointer pImage);
    int findMaxValueMHA(const QVector<size_t>& histogramData);
    void preloadAllmhaHistograms();
    void preloadAllmhaHistogramsWithoutZeroIntensity();
    QVector<size_t> removeZeroIntensityFromHistogram(const QVector<size_t>& histogram);
    QChart* createChartAndSetDataXZero(const QVector<size_t>& outputHistogram);

    void disableMHATabGUI();
    void enableMHATabGUI();

    //QImage vtkImageSliceToQImage(vtkImageSlice* imageSlice);
    void mhaDisplayHistogram(const QImage& image);
    QVector<QImage> vtkImageDataToQImages(vtkSmartPointer<vtkImageData> imageData);
    QVector<QPointF> mhaCalculateHistogram(const QImage& image);
    void mhaHistogram();
    QPair<double, double> calculateRangeFromHistogram(std::map<double, size_t> freqs);
    QVector<size_t> getRangeHistogram(std::map<double, size_t> allFreqs, QPair<double, double> range, int bins);
    QVector<vtkSmartPointer<vtkImageData>> vtkImageSliceToQVector(vtkSmartPointer<vtkImageData> imageData);
    void mhaDisplayHistogram();
    void mhaHistogramWithoutZeroIntensity();
    void mhaDisplayTotalHist();
    template <typename PixelType>
    QVector<QVector<size_t>> calculateHistogramsForAllSlices();
    template <typename PixelType>
    void calculateTotalHistogramMHA();
    void createChartAndSetData(const QVector<QPointF>& outputHistogram);
    QChart* createChartAndSetData(const QVector<size_t>& outputHistogram);
    void createTotalXZeroChartAndSetData(const QVector<size_t>& outputHistogram);

    void createTotalChartAndSetData(const QVector<size_t>& outputHistogram);

    //Generate for DICOM
    void dicomImageAverage();
    void rotateDicomImage();
    QVector<QPointF> dicomCalculateHistogram(const QImage& image);
    QVector<QPointF> calculatetotalHistogram();
    void DicomDisplayHistogram(int index);
    void loadAllChartsDicom();
    void loadAllChartsDicomWithoutZero();
    //QChart* DicomtotalHistogramChart(QVector<QPointF> histogramData);
    int findMaxValue(const QVector<QPointF>& histogramData);
    QChart* createHistogramChart(const QVector<QPointF>& histogramData);

    //Added features
    //void viewDicomFileNames();
    void set_labels();
    void setColors();

    void clearData();


private:
    Ui::SeriesViewerClass ui;
    QStringList m_fileList;
    void resizeEvent(QResizeEvent* event);
    vtkSmartPointer<vtkImageHistogram> histogram;
    bool generateHistogram;
    QProgressBar* progressBar;
    QProgressBar* progressBarMHA;
    QVector<vtkSmartPointer<vtkImageData>> slices;
    QVector<QVector<size_t>> mhaHistograms;

public slots:
   void on_pushButtonLoadDicom_clicked();
   void on_horizontalSlider_sliderReleased();
   //void on_actionSave_triggered();
   //void on_spinBox_valueChanged();
   void on_horizontalSlider_valueChanged();
   void on_mhaSlider_valueChanged();
   void on_pushButtonLoadmha_clicked();
   void onProgressCancel();
   void onProgressMHACancel();

   void on_pushButtonRotateDicom_clicked();
   void on_pushButtonRotateMHA_clicked();

   void on_checkBoxwZeroFreqDICOM_stateChanged();
   void on_checkBox_CHistDicom_stateChanged();
   void on_checkBoxwZeroFreqMHA_stateChanged();
   void on_checkBox_CHistMHA_stateChanged();
};