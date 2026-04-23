#include "SeriesViewer.h"

void SeriesViewer::on_pushButtonRotateDicom_clicked()
{
    try
    {
        // Reset counter if the current index changes
        if (!(previousDicom == ui.horizontalSlider->value() - 1))
        {
            cntRotateDicom = 0;
            previousDicom = ui.horizontalSlider->value() - 1;
        }

        // Calculate remainder to determine rotation angle
        int remainder = (cntRotateDicom < 4) ? 10 : (cntRotateDicom % 4);

        // Set the rotation angle and icon based on the remainder
        int rotationAngle = 0;

        if (remainder == 0 || cntRotateDicom == 0)
        {
            rotationAngle = 90;

        }
        else if (remainder == 1 || cntRotateDicom == 1)
        {
            rotationAngle = 180;

        }
        else if (remainder == 2 || cntRotateDicom == 2)
        {
            rotationAngle = 270;

        }
        else if (remainder == 3 || cntRotateDicom == 3)
        {
            rotationAngle = 360;

        }

        // Apply rotation to all images in allDicomImages
        for (int i = 0; i < allDicomImages8bit.size(); ++i)
        {
            QImage rotatedImage = allDicomImages8bit[i].transformed(QTransform().rotate(rotationAngle));
            allDicomImages8bit[i] = rotatedImage;
        }

        // Display the first image
        displayDICOM(allDicomImages8bit[0]);

        cntRotateDicom++;
    }
    catch (const std::exception& ex)
    {
        ui.statusBar->showMessage("Error: " + QString(ex.what()));
    }
    catch (...)
    {
        ui.statusBar->showMessage("An unknown error occurred.");
    }
}


void SeriesViewer::dicomImageAverage()
{
    // Clear the existing ranges
    rangesDicom.clear();

    // Loop over all images
    for (int i = 0; i < allDicomImages16bit.size(); ++i) {
        QImage image = allDicomImages16bit[i];

        double sum = 0;
        int width = image.width();
        int height = image.height();
        int pixelCount = width * height;

        // Initialize min and max values for each image
        quint16 minValue = std::numeric_limits<quint16>::max();
        quint16 maxValue = std::numeric_limits<quint16>::min();

        for (int y = 0; y < height; ++y) {
            const quint16* row = reinterpret_cast<const quint16*>(image.constScanLine(y));
            for (int x = 0; x < width; ++x) {
                quint16 value16 = row[x];
                sum += value16;

                // Update min and max values
                if (value16 < minValue) {
                    minValue = value16;
                }
                if (value16 > maxValue) {
                    maxValue = value16;
                }
            }
        }

        double average = 0.0;
        if (pixelCount > 0)
        {
            average = sum / (width * height);
        }
        DicomAverage16bit.append(average);
        // Store the range in the rangesDicom vector
        rangesDicom.append(QPair<double, double>(minValue, maxValue));
    }

    // You can display or use the rangesDicom vector as needed
}



QVector<QPointF> SeriesViewer::dicomCalculateHistogram(const QImage& image)
{
    QVector<QPointF> histogramData(256, QPointF(0, 0)); // Initialize histogram with 256 points

    if (image.format() != QImage::Format_Grayscale8) {
        qWarning("Input image is not in Grayscale8 format.");
        return histogramData;
    }

    const uchar* imageData = image.bits(); // Get the image data
    int imageSize = image.width() * image.height();

    // Iterate through the image data and update the histogram
    for (int i = 0; i < imageSize; ++i) {
        int intensity = imageData[i];
        histogramData[intensity].setY(histogramData[intensity].y() + 1); // Increment frequency
    }

    // Set x values for each intensity level
    for (int i = 0; i < histogramData.size(); ++i) {
        histogramData[i].setX(i);
    }

    return histogramData;
}

// Function to calculate cumulative histogram for all images
QVector<QPointF> SeriesViewer::calculatetotalHistogram() {
    QVector<QPointF> totalHistogram(256, QPointF(0, 0)); // Initialize cumulative histogram with 256 points
    for (const auto& image : allDicomImages8bit) {
        QVector<QPointF> histogramData = dicomCalculateHistogram(image);
        for (int i = 0; i < histogramData.size(); ++i) {
            totalHistogram[i].setY(totalHistogram[i].y() + histogramData[i].y());
        }
    }
    return totalHistogram;
}
// Function to display DICOM histogram in a chart
void SeriesViewer::DicomDisplayHistogram(int index) {
    try {
        // Check if the charts vector is not empty
        if (!chartsDicom.empty()) {
            QVector<QPointF> cumulativeHist = calculatetotalHistogram();
            QChart* CChart = createHistogramChart(cumulativeHist);
            // Check if index is within the valid range
            if (index >= 0 && static_cast<size_t>(index) < chartsDicom.size()) {
                ui.chartViewDICOM->setRenderHint(QPainter::Antialiasing);
                if (ui.checkBox_CHistDicom->isChecked())
                {
                    if (ui.checkBoxwZeroFreqDICOM->isChecked())
                    {
                        cumulativeHist.remove(0);
                        CChart = createHistogramChart(cumulativeHist);
                        ui.chartViewDICOM->setChart(CChart);
                    }
                    else
                    {
                        ui.chartViewDICOM->setChart(CChart);
                    }
                }
                else
                {
                    if (ui.checkBoxwZeroFreqDICOM->isChecked())
                    {
                        ui.chartViewDICOM->setChart(chartsDicomXZero.at(index));
                    }
                    else
                    {
                        ui.chartViewDICOM->setChart(chartsDicom.at(index));
                    }
                }
            }
            else {
                ui.statusBar->showMessage("Invalid index when accessing histogram data.");
            }
        }
        else {
            ui.statusBar->showMessage("No histograms available.");
        }
    }
    catch (const std::out_of_range& e) {
        ui.statusBar->showMessage("Out of range error: " + QString(e.what()));
    }
    catch (const std::exception& e) {
        ui.statusBar->showMessage("Error: " + QString(e.what()));
    }
    catch (...) {
        ui.statusBar->showMessage("An unknown error occurred.");
    }
}

void SeriesViewer::loadAllChartsDicom()
{
    for (int k = 0; k < allDicomImages8bit.size();k++)
    {
        QVector<QPointF> histogramData = dicomCalculateHistogram(allDicomImages8bit[k]);

        QChart* chart = createHistogramChart(histogramData);

        this->chartsDicom.push_back(chart);

        progressDialog.setValue(dicomFiles.count() * 8 + k);
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        progressBar->setValue(dicomFiles.count() * 8 + k);
        qApp->processEvents();
    }
}

void SeriesViewer::loadAllChartsDicomWithoutZero()
{
    for (int m = 0; m < allDicomImages8bit.size(); m++) {
        QVector<QPointF> histogramDataXZero = dicomCalculateHistogram(allDicomImages8bit[m]);
        histogramDataXZero.remove(0);

        QChart* chart = createHistogramChart(histogramDataXZero);
        chartsDicomXZero.push_back(chart);
        progressDialog.setValue(dicomFiles.count() * 9 + m);
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        progressBar->setValue(dicomFiles.count() * 9 + m);
        qApp->processEvents();
    }
}

// Function to find the maximum value in histogram data for QPointF type
int SeriesViewer::findMaxValue(const QVector<QPointF>& histogramData) {
    // Initialize the maximum value
    int maxVal = 0;

    // Iterate through each point in the histogram data
    for (const auto& point : histogramData) {
        // Get the frequency from the y-coordinate of the point
        int frequency = static_cast<int>(point.y());

        // Update the maximum value if the current frequency is greater
        if (frequency > maxVal) {
            maxVal = frequency;
        }
    }

    // Check if the maximum value needs rounding up to the nearest thousand
    int remainder = maxVal % 1000;
    if (remainder == 0) {
        return maxVal;  // No rounding needed if already a multiple of 1000
    }
    else {
        return maxVal + (1000 - remainder);  // Round up to the nearest thousand
    }
}

// Function to find the maximum value in histogram data for size_t type (used for MHA format)
int SeriesViewer::findMaxValueMHA(const QVector<size_t>& histogramData) {
    // Initialize the maximum value
    int maxVal = 0;

    // Iterate through each size in the histogram data
    for (const auto& size : histogramData) {
        // Convert the size to int (assuming it represents frequency)
        int frequency = static_cast<int>(size);

        // Update the maximum value if the current frequency is greater
        if (frequency > maxVal) {
            maxVal = frequency;
        }
    }

    // Check if the maximum value needs rounding up to the nearest thousand
    int remainder = maxVal % 1000;
    if (remainder == 0) {
        return maxVal;  // No rounding needed if already a multiple of 1000
    }
    else {
        return maxVal + (1000 - remainder);  // Round up to the nearest thousand
    }
}

QChart* SeriesViewer::createHistogramChart(const QVector<QPointF>& histogramData) {
    // Create a new chart
    QChart* chart = new QChart();

    // Create a bar series to represent the histogram
    QBarSeries* barSeries = new QBarSeries();

    // Set pen for the bar series
    QPen pen(Qt::blue, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    // Create a bar set and populate it with histogram data
    QBarSet* barSet = new QBarSet("");
    for (int i = 0; i < histogramData.size(); i++) {
        *barSet << histogramData[i].y();
    }
    barSet->setPen(pen);

    // Append the bar set to the bar series
    barSeries->append(barSet);

    // Add the bar series to the chart
    chart->addSeries(barSeries);

    // Create and configure the X-axis (Intensity)
    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    QStringList categories;
    for (int i = 0; i < 256; ++i) {
        categories << QString::number(i);
    }
    axisX->setCategories(categories);
    axisX->setVisible(false);
    chart->addAxis(axisX, Qt::AlignBottom);

    // Create and configure the Y-axis (Frequency)
    QValueAxis* axisY = new QValueAxis();
    axisY->setRange(0, findMaxValue(histogramData));
    chart->addAxis(axisY, Qt::AlignLeft);

    // Create and configure the secondary X-axis
    QValueAxis* Xaxis = new QValueAxis();
    Xaxis->setRange(rangesDicom[static_cast<qsizetype>(ui.horizontalSlider->value()) - 1].first, rangesDicom[ui.horizontalSlider->value() - 1].second);
    Xaxis->setTitleText("Intensity");
    chart->addAxis(Xaxis, Qt::AlignBottom);

    // Hide the legend and set Y-axis title
    chart->legend()->setVisible(false);
    chart->axisY()->setTitleText("Frequency");

    return chart;
}

