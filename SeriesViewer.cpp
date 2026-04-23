#include "SeriesViewer.h"
#include <qgraphicsview.h>

SeriesViewer::SeriesViewer(QWidget* parent)
    : QMainWindow(parent), // Initialize parent class
    progressDialog("Loading in Progress...", "Cancel", 0, 0, this),// Initialize progress dialog in the member initializer list
    progressDialogMHA("Loading in Progress...", "Cancel", 0, 0, this),
    chartsDicom(), // Initialize chartsDicom
    chartsDicomXZero() ,// Initialize chartsDicomXZero
    mhaHistogramsCharts(),
    mhaHistogramsChartsWithoutZero(),
    ranges(),
    allDicomImages16bit()
{
    // Set up the UI
    ui.setupUi(this);

    // Initialize member variables
    slices = QVector<vtkSmartPointer<vtkImageData>>();
    histogram = vtkSmartPointer<vtkImageHistogram>::New();
    generateHistogram = false; // Initialize to false

    QPair<double, double> totalHistogramRange = qMakePair(0.0, 0.0);

    // Set alignment for labels
    ui.label_DicomHistogram->setAlignment(Qt::AlignCenter);
    ui.label_mhaHistogram->setAlignment(Qt::AlignCenter);

    // Set icon for the push button
    ui.pushButtonRotateDicom->setIcon(QIcon("C:/Users/hp/Desktop/SeriesViewer/rotate icons/90.png"));


    // Set up progress dialog
    progressDialog.setWindowTitle("Loading...");
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.close(); // Close progress dialog initially

    // Set up progress dialog
    progressDialogMHA.setWindowTitle("Loading...");
    progressDialogMHA.setWindowModality(Qt::WindowModal);
    progressDialogMHA.close(); // Close progress dialog initially

    //Disable GUI components in both tabs
    disableMHATabGUI();
    disableDicomTabGUI();

    //Set up progress Bar 
    progressBar = new QProgressBar();
    progressBar->setAlignment(Qt::AlignRight);
    ui.statusBar->addPermanentWidget(progressBar);
    progressBar->close(); // Close progress Bar initially

    //Set up progress Bar 
    progressBarMHA = new QProgressBar();
    progressBarMHA->setAlignment(Qt::AlignRight);
    ui.statusBar->addPermanentWidget(progressBarMHA);
    progressBarMHA->close(); // Close progress Bar initially

    // Connect the canceled signal to your slot
    connect(&progressDialog, &QProgressDialog::canceled, this, &SeriesViewer::onProgressCancel);

    // Connect the canceled signal to your slot
    connect(&progressDialogMHA, &QProgressDialog::canceled, this, &SeriesViewer::onProgressMHACancel);

    // Initialize renderer and add it to the render window
    renderer = vtkSmartPointer<vtkRenderer>::New();
    ui.vtkwidget->renderWindow()->AddRenderer(renderer);

    // Initialize image slice mapper and image slice
    imageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
    imageSlice = vtkSmartPointer<vtkImageSlice>::New();

    // Initialize meta image reader
    mhaReader = vtkSmartPointer<vtkMetaImageReader>::New();

    // Initialize slice number and set slider values
    sliceNumber = 0;
    ui.horizontalSlider->setValue(ui.spinBox->value());
    ui.horizontalSlider->setRange(0, 0);
    ui.spinBox->setRange(0, 0);

    // Set text for labels
    ui.label_DicomInd->setText("0");
    ui.label_DicomMax->setText("0");
    ui.label_MHAInd->setText(QString::number(0));
    ui.label_MHAMax->setText(QString::number(0));

    // Set up table widget
    ui.tableWidget->setColumnCount(6);
    ui.tableWidget->setHorizontalHeaderLabels({ "Tags", "Description", "Value \n Representation", "Value \nMultiplicity", "Length", "Value" });

}

SeriesViewer::~SeriesViewer()
{}


void SeriesViewer::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event); // Avoid unused parameter warning

    // Check if the list of DICOM images is not empty
    if (!allDicomImages8bit.empty())
    {
        // Create a new QGraphicsScene
        QGraphicsScene* scene1 = new QGraphicsScene(this);

        // Get the target size for the graphics view
        QSize targetSize1 = ui.graphicsView->viewport()->size();

        // Scale the DICOM image to fit the target size while maintaining aspect ratio
        QImage scaledImage1 = allDicomImages8bit[dicomIndex].scaled(targetSize1, Qt::KeepAspectRatio);

        // Add the scaled image to the scene as a pixmap item
        QGraphicsPixmapItem* item1 = scene1->addPixmap(QPixmap::fromImage(scaledImage1));

        // Set the scene for the graphics view
        ui.graphicsView->setScene(scene1);

        // Resize the tableWidgetMHA to fit its contents
        ui.tableWidgetMHA->resizeColumnsToContents();
    }
}

