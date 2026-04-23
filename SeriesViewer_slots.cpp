#include "SeriesViewer.h"

// PushButton Load DICOM Slot:
void SeriesViewer::on_pushButtonLoadDicom_clicked() {
    try {
        progressDialogCanceled = false; // Initialize progressDialogCanceled to false
        // Select DICOM Files
        QString directoryPath = QFileDialog::getExistingDirectory(this, "Select Directory");

        // Check if No Directory Selected 
        if (directoryPath.isEmpty()) {
            ui.statusBar->showMessage("No directory selected."); // Show message if no directory is selected
            return;
        }

        // If the button is clicked for the second time, or a new directory is selected, clear data
        if (!dicomFiles.isEmpty()) {
            clearData(); // Clear existing data if present
        }

        loadDicomFiles(directoryPath); // Load DICOM files from the selected directory
        
        loadAllDicomData(); // Load all DICOM data

        ui.statusBar->removeWidget(progressBar); // Remove progressBar from the statusBar
        //progressDialog.close();
        if (progressDialogCanceled) { // Check if progressDialog was canceled
            return; // If canceled, return without further processing
        }

        if (allDicomTags.empty() || allDicomImages8bit.empty()) {
            ui.statusBar->showMessage("Failed to load DICOM data."); // Show message if DICOM data loading fails
            return;
        }

        if (progressDialogCanceled) {
            ui.statusBar->showMessage(" "); // Show empty message if progressDialog was canceled
        }

        displayDicomTab(); // Display the DICOM tab with loaded data

        enableDicomTabGUI();

        ui.statusBar->showMessage("Done Loading " + QString::number(dicomFiles.count()) + " DICOM Files"); // Show message indicating successful loading of DICOM files
    }
    catch (const std::exception& e) {
        // Handle exceptions, log the error, or show an appropriate message to the user
        ui.statusBar->showMessage("Error: " + QString(e.what())); // Show error message if an exception occurs
    }
    catch (...) {
        // Handle other types of exceptions
        ui.statusBar->showMessage("An unknown error occurred."); // Show message for unknown errors
    }
}

// Horizontal Slider Slot:
void SeriesViewer::on_horizontalSlider_sliderReleased()
{
    try
    {
        // Check if allDicomTags and allDicomImages8bit are not empty
        if (!allDicomTags.empty() && !allDicomImages8bit.empty())
        {
            dicomIndex = ui.horizontalSlider->value() - 1;

            // Check if dicomIndex is a valid index
            if (dicomIndex >= 0 && dicomIndex < allDicomTags.size() && dicomIndex < allDicomImages8bit.size())
            {
                DicomTags tags = allDicomTags.at(dicomIndex);
                QImage img = allDicomImages8bit.at(dicomIndex);
                set_labels();
                displayDICOM(img);
                DicomDisplayHistogram(dicomIndex);
                ui.label_ImgAvg_DICOM->setText(QString::number(DicomAverage16bit[dicomIndex]));
                viewDicomTags(tags);
                //ui.listWidget->setCurrentRow(dicomIndex);
                ui.spinBox->setValue(ui.horizontalSlider->value());
            }
            else
            {
                ui.statusBar->showMessage("Invalid index when accessing DICOM data.");
            }
        }
    }
    catch (const std::out_of_range& e)
    {
        ui.statusBar->showMessage("Out of range error: " + QString(e.what()));
    }
    catch (const std::exception& e)
    {
        ui.statusBar->showMessage("Error: " + QString(e.what()));
    }
    catch (...)
    {
        ui.statusBar->showMessage("An unknown error occurred.");
    }
}

void SeriesViewer::on_horizontalSlider_valueChanged()
{
    ui.label_DicomInd->setText(QString::number(ui.horizontalSlider->value()));
    on_horizontalSlider_sliderReleased();
}

void SeriesViewer::onProgressCancel()
{
    progressDialogCanceled = true;
    ui.statusBar->showMessage("Loading Cancelled..");
    qApp->processEvents();
    QTimer::singleShot(100000, ui.statusBar,&QLabel::hide);
    qApp->processEvents();
    clearData();
}

void SeriesViewer::on_checkBoxwZeroFreqDICOM_stateChanged() 
{
    DicomDisplayHistogram(ui.horizontalSlider->value()); // Update histogram display
}

void SeriesViewer::on_checkBox_CHistDicom_stateChanged()
{
    DicomDisplayHistogram(ui.horizontalSlider->value()); // Update histogram display
}

void SeriesViewer::on_checkBoxwZeroFreqMHA_stateChanged()
{
    if (ui.checkBoxwZeroFreqMHA->isChecked() && ui.checkBox_CHistMHA->isChecked())
    {
        mhaDisplayTotalHist();
    }
    else if (ui.checkBox_CHistMHA->isChecked())
    {
        mhaDisplayTotalHist();
    }
    else if (ui.checkBoxwZeroFreqMHA->isChecked())
    {
        ui.chartViewMHA->setChart(mhaHistogramsChartsWithoutZero[ui.mhaSlider->value()-1]);
    }
    else
    {
        ui.chartViewMHA->setChart(mhaHistogramsCharts[ui.mhaSlider->value()-1]);
    }
}

void SeriesViewer::on_checkBox_CHistMHA_stateChanged()
{
    if (ui.checkBox_CHistMHA->isChecked())
    {
        mhaDisplayTotalHist();
    }
    else if(ui.checkBoxwZeroFreqMHA->isChecked()&& ui.checkBox_CHistMHA->isChecked())
    {
        mhaDisplayTotalHist();
    }
    else if (ui.checkBoxwZeroFreqMHA->isChecked())
    {
        ui.chartViewMHA->setChart(mhaHistogramsChartsWithoutZero[ui.mhaSlider->value()-1]);
    }
    else
    {
        ui.chartViewMHA->setChart(mhaHistogramsCharts[ui.mhaSlider->value()-1]);
    }
}

void SeriesViewer::mhaDisplayTotalHist()
{

    if (ui.checkBoxwZeroFreqMHA->isChecked())
    {
        createTotalXZeroChartAndSetData(totalHistogramXZero);
    }
    else
    {
        createTotalChartAndSetData(totalHistogram);
    }
}