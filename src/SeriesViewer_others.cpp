#include "SeriesViewer.h"

/*void SeriesViewer::viewDicomFileNames()
{

    for (int i = 0; i < dicomFiles.count();i++)
    {
        QFile f(dicomFiles.at(i));
        QFileInfo fileInfo(f.fileName());
        QString filename(fileInfo.fileName());
        dicomFileNames.insert(i, filename);
    }
    ui.listWidget->addItems(dicomFileNames);
}*/

void SeriesViewer::setColors()
{
    QPalette palete = ui.tab_DicomViewer->palette();
    palete.setColor(ui.tab_DicomViewer->backgroundRole(), QColor::QColor(170, 170, 255, 255));
    ui.tab_DicomViewer->setAutoFillBackground(true);
    ui.tab_DicomViewer->setPalette(palete);


    ui.pushButtonLoadDicom->setStyleSheet("QPushButton { color: white;background-color: grey; }\n"
        "QPushButton:enabled { background-color:  rgb(110, 110, 165); }\n");

    ui.graphicsView->setStyleSheet("QGraphicsView { background-color: grey; }\n"
        "QGraphicsView:enabled { background-color:  rgb(170,170 ,255); }\n");

    ui.tableWidget->setStyleSheet("QTableWidget { background-color: grey; }\n"
        "QTableWidget:enabled { background-color:  rgb(170,170 ,255); }\n");

}

void SeriesViewer::set_labels()
{
    ui.label_DicomInd->setText(QString::number(ui.horizontalSlider->value()));
    ui.label_DicomMax->setText(QString::number(dicomFiles.count()));
}


void SeriesViewer::clearData()
{
    ui.statusBar->removeWidget(progressBar);
    dicomFiles.clear();
    allDicomTags.clear();
    allDicomImages8bit.clear();
    chartsDicom.clear();
    chartsDicomXZero.clear();
    QChart* chart = new QChart();
    ui.chartViewDICOM->setChart(chart);
    QGraphicsScene* scene = new QGraphicsScene();
    ui.graphicsView->setScene(scene);
    ui.graphicsView->show();

    disableDicomTabGUI();

    // Clear the table widget
    ui.tableWidget->clearContents();
    ui.tableWidget->setRowCount(0);
    ui.tableWidget->setColumnCount(0);
    ui.label_ImgAvg_DICOM->setText(" ");
    ui.statusBar->clearMessage();
    ui.horizontalSlider->setRange(0, 0);
    ui.label_DicomInd->setText(QString::number(0));
    ui.label_DicomMax->setText(QString::number(0));
    ui.spinBox->setRange(0, 0);
}

void SeriesViewer::disableDicomTabGUI()
{
    ui.horizontalSlider->setDisabled(true);
    ui.spinBox->setDisabled(true);
    ui.pushButtonRotateDicom->setEnabled(false);
    ui.checkBoxwZeroFreqDICOM->setEnabled(false);
    ui.checkBox_CHistDicom->setEnabled(false);
}

void SeriesViewer::enableDicomTabGUI()
{
    ui.horizontalSlider->setDisabled(false);
    ui.spinBox->setDisabled(false);
    ui.pushButtonRotateDicom->setEnabled(true);
    ui.checkBoxwZeroFreqDICOM->setEnabled(true);
    ui.checkBox_CHistDicom->setEnabled(true);
}


