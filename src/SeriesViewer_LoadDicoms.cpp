#include "SeriesViewer.h"

bool isDicomFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    // DICOM files have a 128-byte preamble followed by "DICM"
    file.seek(128);
    QByteArray dicomSignature = file.read(4);
    if (dicomSignature == "DICM") {
        return true;
    }

    return false;
}

QStringList SeriesViewer::getDicomFiles(const QString& directoryPath) {
    // Return an empty QStringList if the directoryPath is null or empty
    if (directoryPath.isNull() || directoryPath.isEmpty()) {
        return QStringList();
    }

    QDir dir(directoryPath);
    // Optionally, check if the directory exists and is readable
    if (!dir.exists() || !dir.isReadable()) {
        return QStringList();
    }

    QStringList dicomFiles;
    QDirIterator it(directoryPath, QDirIterator::Subdirectories);

    while (it.hasNext()) {
        QString filePath = it.next();
        QFileInfo fileInfo(filePath);

        // Check if it's a file and not a directory
        if (fileInfo.isFile()) {
            // Check by extension or DICOM signature
            if (fileInfo.suffix().toLower() == "dcm" || isDicomFile(filePath)) {
                dicomFiles << fileInfo.absoluteFilePath();
            }
        }
    }

    return dicomFiles;
}


// Function to load DICOM files from a directory path
void SeriesViewer::loadDicomFiles(const QString& directoryPath) {
    dicomFiles = getDicomFiles(directoryPath);

    // Check if no DICOM files were found
    if (dicomFiles.isEmpty()) {
        ui.statusBar->showMessage("No DICOM files found in the selected directory.");
        return;
    }
    dicomFiles = dicomFiles;

    ui.statusBar->showMessage("Loading Dicom Files");

    ui.horizontalSlider->setRange(1, dicomFiles.count());
    ui.spinBox->setRange(1, dicomFiles.count());

    dicomIndex = 1;

    progressBar->setValue(0);
    progressBar->setRange(0, dicomFiles.size() * 10);
    progressBar->show();

    progressDialog.setWindowTitle("Loading...");
    progressDialog.setRange(0, dicomFiles.size() * 10);
    progressDialog.setValue(0);
    QApplication::processEvents();
    progressDialog.show();
}

// Function to load all DICOM data
void SeriesViewer::loadAllDicomData() {
    set_labels();

    LoadAllDicoms();

    loadAllTags();

    dicomImageAverage();

    loadAllChartsDicom();

    loadAllChartsDicomWithoutZero();

    progressDialog.setValue((dicomFiles.count()) * 10);
    progressBar->setValue((dicomFiles.count() * 10));
}

// Function to display DICOM image
void SeriesViewer::displayDicomTab() {
    DicomTags tags = allDicomTags.at(0);
    const QImage img = allDicomImages8bit.at(0);

    displayDICOM(img);
    viewDicomTags(tags);
    DicomDisplayHistogram(0);
    ui.label_ImgAvg_DICOM->setText(QString::number(DicomAverage16bit[ui.horizontalSlider->value()-1]));
}
