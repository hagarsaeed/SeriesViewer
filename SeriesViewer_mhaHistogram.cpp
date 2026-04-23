#include "SeriesViewer.h"

void SeriesViewer::mhaHistogram()
{
    // Determine the pixel type of mhaReader->getOutput()
    vtkImageData* imageData = mhaReader->GetOutput();
    int vtkScalarType = imageData->GetScalarType();
    switch (vtkScalarType)
    {
    case VTK_SHORT:
        mhaHistograms = calculateHistogramsForAllSlices<short>();
        calculateTotalHistogramMHA<short>();
        break;
    case VTK_UNSIGNED_SHORT:
        mhaHistograms = calculateHistogramsForAllSlices<unsigned short>();
        calculateTotalHistogramMHA<unsigned short>();
        break;
    case VTK_CHAR:
        mhaHistograms = calculateHistogramsForAllSlices<char>();
        calculateTotalHistogramMHA<char>();
        break;
    case VTK_UNSIGNED_CHAR:
        mhaHistograms = calculateHistogramsForAllSlices<unsigned char>();
        calculateTotalHistogramMHA<unsigned char>();
        break;
    case VTK_DOUBLE:
        mhaHistograms = calculateHistogramsForAllSlices<double>();
        calculateTotalHistogramMHA<double>();
        break;
    default:
        ui.statusBar->showMessage("Undefined Pixel Type");
        clearMHATab();
        return;
    }
    if (progressDialogMHACanceled)
    {
        clearMHATab();
        return;
    }

    preloadAllmhaHistograms();
    if (progressDialogMHACanceled)
    {
        clearMHATab();
        return;
    }
    preloadAllmhaHistogramsWithoutZeroIntensity();
    if (progressDialogMHACanceled)
    {
        clearMHATab();
        return;
    }
    if (ui.checkBoxwZeroFreqMHA->isChecked())
    {
        ui.chartViewMHA->setChart(mhaHistogramsChartsWithoutZero[ui.mhaSlider->value()]);
    }
    else
    {
        ui.chartViewMHA->setChart(mhaHistogramsCharts[ui.mhaSlider->value()]);
    }
}

template <typename Iterator>
bool isLess(const Iterator& it, const Iterator& end) {
    return it < end;
}

template <class TImageType>
std::map<double, size_t> SeriesViewer::getUniqueValuesHistogram(typename TImageType::Pointer pImage)
{
    std::map<double, size_t> valuesMap;
    std::map<double, size_t>::iterator it;
    if (!pImage)
    {
        throw itk::ExceptionObject(__FILE__, __LINE__, "Error(3489-7450): Invalid image pointer ...");
    }
    size_t infCount = 0;
    itk::ImageRegionConstIterator<TImageType> imageIterator(pImage, pImage->GetLargestPossibleRegion());
#pragma omp parallel for reduction(+:infCount) private(it) shared(valuesMap)
    for (imageIterator.GoToBegin(); !imageIterator.IsAtEnd(); ++imageIterator)
    {
        typename TImageType::PixelType currentPixel = imageIterator.Get();
        if (isnan<double>(currentPixel))
        {
            continue;
        }
        if (isinf<double>(currentPixel))
        {
#pragma omp atomic
            infCount++;
            continue;
        }
#pragma omp critical
        {
            it = valuesMap.find((double)currentPixel);
            if (it == valuesMap.end()) // if not already included in the output vector
            {
                valuesMap[currentPixel] = 1;
            }
            else
            {
                it->second++;
            }
        }
    }
    if (infCount != 0)
    {
#pragma omp critical
        {
            valuesMap[std::numeric_limits<double>::infinity()] = infCount;
        }
    }
    return valuesMap;
}

QPair<double, double> SeriesViewer::calculateRangeFromHistogram(std::map<double, size_t> freqs)
{
    std::map<double, size_t>::const_iterator i;
    QPair<double, double>range;

    //Check input range is not empty.
    if (freqs.size() == 1)
    {
        i = freqs.cbegin();
        if (std::isinf<double>(i->first))
        {
            range.first = std::numeric_limits<double>::max();
            range.second = std::numeric_limits<double>::min();
        }
        else
        {
            range.first = i->first;
            range.second = i->first;
        }
        return range;
    }
    if (freqs.empty())
    {
        range.first = std::numeric_limits<double>::max();
        range.second = std::numeric_limits<double>::min();
    }
    else
    {
        //Smallest  element is stored at the beginning but have to check it is finite element.
        i = freqs.cbegin();
        if (std::isinf<double>(i->first))
        {
            //if infinite take the next smallest value
            i++;
        }
        range.first = i->first;
        i = freqs.end();
        i--;
        //Largest value is stored at the last position.
        if (std::isinf<double>(i->first))
        {
            //If infinite take the second largest value
            i--;
        }
        range.second = i->first;
    }
    return range;
}

inline QVector<size_t> SeriesViewer::getRangeHistogram(std::map<double, size_t> allFreqs, QPair<double, double> range, int bins)
{
    std::map<double, size_t>::iterator it = allFreqs.begin();
    std::map<double, size_t>::iterator itFirst = allFreqs.begin();
    std::map<double, size_t>::iterator itLast = allFreqs.end();
    itLast--;
    QVector<size_t> outputHistogram;
    //Step 1: calculate bar width.
    double barwidth = (range.second - range.first) / bins;

    //Step 2: start and end point to the start and end of the first bar.
    double start,
        end = range.first;
    size_t binValue = 0;
    for (size_t i = 0; i < bins; i++)
    {
        //update bin start, end and value.
        start = end;
        end = end + barwidth;
        binValue = 0;
        //If current bar range is outside map range make this bin = 0
        if (end < itFirst->first || start > itLast->first)
        {
            outputHistogram << 0;
            continue;
        }
        //Iterate
        while (it != allFreqs.end() && isLess(it->first, end))
        {
            if (it->first >= start && isLess(it->first, end))
            {
                binValue += it->second;
                it++;
            }
            else
            {
                it++;
            }
        }
        outputHistogram << binValue;
    }
    it = allFreqs.find(range.second);
    if (it != allFreqs.end())
    {
        outputHistogram[outputHistogram.size() - 1] += it->second;
    }
    return outputHistogram;
}

QVector<vtkSmartPointer<vtkImageData>> SeriesViewer::vtkImageSliceToQVector(vtkSmartPointer<vtkImageData> imageData) {
    QVector<vtkSmartPointer<vtkImageData>> imageVector;

    int* dims = imageData->GetDimensions();
    progressDialogMHA.setWindowTitle("Loading MHA...");
    progressDialogMHA.setRange(0, (imageSliceMapper->GetSliceNumberMaxValue() + 1)*10);
    progressDialogMHA.setValue(0);
    progressBarMHA->setRange(0, (imageSliceMapper->GetSliceNumberMaxValue() + 1) * 10);
    progressBarMHA->setValue(0);
    QApplication::processEvents();
    progressDialogMHA.show();
    progressBarMHA->show();

    for (int z = 0; z < dims[2]; ++z) {
        vtkSmartPointer<vtkImageData> sliceImage = vtkSmartPointer<vtkImageData>::New();
        sliceImage->SetDimensions(dims[0], dims[1], 1); // Set dimensions for the slice
        sliceImage->AllocateScalars(imageData->GetScalarType(), 1); // Allocate memory for scalar values
        progressDialogMHA.setValue(z * 4);
        progressBarMHA->setValue(z * 4);
        qApp->processEvents();
        // Copy scalar values from the original image to the slice
        for (int y = 0; y < dims[1]; ++y) {
            for (int x = 0; x < dims[0]; ++x) {
                void* src = imageData->GetScalarPointer(x, y, z);
                void* dest = sliceImage->GetScalarPointer(x, y, 0);
                memcpy(dest, src, imageData->GetScalarSize());
                if (progressDialogMHACanceled)
                {
                    break;
                }
            }
            if (progressDialogMHACanceled)
            {
                break;
            }
        }
        imageVector.push_back(sliceImage);
        if (progressDialogMHACanceled)
        {
            break;
        }
    }
    progressDialogMHA.setValue((imageSliceMapper->GetSliceNumberMaxValue() + 1) * 4);
    progressBarMHA->setValue((imageSliceMapper->GetSliceNumberMaxValue() + 1) * 4);
    return imageVector;
}

template <typename PixelType>
QVector<QVector<size_t>> SeriesViewer::calculateHistogramsForAllSlices()
{
    QVector<QVector<size_t>> histograms;
    int current = 0;

    // Resize the ranges vector to accommodate all slices
    ranges.resize(slices.size());

    // Loop over all slices
    for (const auto& slice : slices) {
        // Convert vtkImageData to itk::Image
        constexpr unsigned int Dimension = 3;
        using ImageType = itk::Image<PixelType, Dimension>;

        using FilterType = itk::VTKImageToImageFilter<ImageType>;
        typename FilterType::Pointer filter = FilterType::New();
        filter->SetInput(slice);
        filter->Update();

        // Calculate histogram for the current slice
        std::map<double, size_t> valuesMap = getUniqueValuesHistogram<ImageType>(filter->GetOutput());

        QVector<size_t> outputHistogram;

        ranges[current] = calculateRangeFromHistogram(valuesMap);
        outputHistogram = getRangeHistogram(valuesMap, ranges[current], 256);

        // Add the histogram to the vector of histograms
        histograms.push_back(outputHistogram);

        current++;
        progressDialogMHA.setValue((imageSliceMapper->GetSliceNumberMaxValue() + 1) * 4 + current * 4);
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        progressBarMHA->setValue((imageSliceMapper->GetSliceNumberMaxValue() + 1) * 4 + current * 4);
        qApp->processEvents();

        if (progressDialogMHACanceled)
        {
            break;
        }
    }
    progressDialogMHA.setValue((imageSliceMapper->GetSliceNumberMaxValue() + 1) * 8);
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    progressBarMHA->setValue((imageSliceMapper->GetSliceNumberMaxValue() + 1) * 8);
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    return histograms;
}


template <typename PixelType>
void SeriesViewer::calculateTotalHistogramMHA() 
{
    // Convert vtkImageData to itk::Image
    constexpr unsigned int Dimension = 3;
    using ImageType = itk::Image<PixelType, Dimension>;

    using FilterType = itk::VTKImageToImageFilter<ImageType>;
    typename FilterType::Pointer filter = FilterType::New();
    filter->SetInput(mhaReader->GetOutput());
    filter->Update();

    // Calculate histogram for the current slice
    std::map<double, size_t> valuesMap = getUniqueValuesHistogram<ImageType>(filter->GetOutput());

    totalHistogramRange = calculateRangeFromHistogram(valuesMap);
    totalHistogram = getRangeHistogram(valuesMap, totalHistogramRange, 256);
    totalHistogramXZero = totalHistogram;
    totalHistogramXZero.remove(0);
}

QChart* SeriesViewer::createChartAndSetData(const QVector<size_t>& outputHistogram) {
    QChart* chart = new QChart();
    QBarSeries* series = new QBarSeries();
    QBarSet* set = new QBarSet(" ", series);
    QPen pen(Qt::blue, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    for (int i = 0; i < 256; i++) {
        *set << outputHistogram[i];
    }
    set->setPen(pen);
    series->append(set);

    chart->legend()->setVisible(false);
    chart->addSeries(series);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QValueAxis* axisY = new QValueAxis();
    axisY->setRange(0, findMaxValueMHA(outputHistogram));
    axisY->setTitleText("Frequency");
    chart->addAxis(axisY, Qt::AlignLeft);

    QValueAxis* axisX = new QValueAxis(chart);
    axisX->setRange(ranges[ui.mhaSlider->value()].first, ranges[ui.mhaSlider->value()].second);
    axisX->setTitleText("Intensity");
    chart->addAxis(axisX, Qt::AlignBottom);

    return chart;
}

QChart* SeriesViewer::createChartAndSetDataXZero(const QVector<size_t>& outputHistogram) {
    QChart* chart = new QChart();
    QBarSeries* series = new QBarSeries();
    QBarSet* set = new QBarSet(" ", series);
    QPen pen(Qt::blue, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    for (int i = 0; i < 255; i++) {
        *set << outputHistogram[i];
    }
    set->setPen(pen);
    series->append(set);

    chart->legend()->setVisible(false);
    chart->addSeries(series);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QValueAxis* axisY = new QValueAxis();
    axisY->setRange(0, findMaxValueMHA(outputHistogram));
    axisY->setTitleText("Frequency");
    chart->addAxis(axisY, Qt::AlignLeft);

    QValueAxis* axisX = new QValueAxis(chart);
    axisX->setRange(ranges[ui.mhaSlider->value()].first+1, ranges[ui.mhaSlider->value()].second);
    axisX->setTitleText("Intensity");
    chart->addAxis(axisX, Qt::AlignBottom);

    return chart;
}

void SeriesViewer::createTotalXZeroChartAndSetData(const QVector<size_t>& outputHistogram) {
    QChart* chart = new QChart();
    QBarSeries* series = new QBarSeries();
    QBarSet* set = new QBarSet(" ", series);
    QPen pen(Qt::blue, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    for (int i = 0; i < 255; i++) {
        *set << outputHistogram[i];
    }
    set->setPen(pen);
    series->append(set);

    chart->legend()->setVisible(false);
    chart->addSeries(series);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QValueAxis* axisY = new QValueAxis();
    axisY->setRange(0, findMaxValueMHA(outputHistogram));
    axisY->setTitleText("Frequency");
    chart->addAxis(axisY, Qt::AlignLeft);

    QValueAxis* axisX = new QValueAxis(chart);
    axisX->setRange(totalHistogramRange.first+1 , totalHistogramRange.second);
    axisX->setTitleText("Intensity");
    chart->addAxis(axisX, Qt::AlignBottom);

    ui.chartViewMHA->setChart(chart);
}

void SeriesViewer::createTotalChartAndSetData(const QVector<size_t>& outputHistogram) {
    QChart* chart = new QChart();
    QBarSeries* series = new QBarSeries();
    QBarSet* set = new QBarSet(" ", series);
    QPen pen(Qt::blue, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    for (int i = 0; i < 256; i++) {
        *set << outputHistogram[i];
    }
    set->setPen(pen);
    series->append(set);

    chart->legend()->setVisible(false);
    chart->addSeries(series);
    chart->setAnimationOptions(QChart::SeriesAnimations);

    QValueAxis* axisY = new QValueAxis();
    axisY->setRange(0, findMaxValueMHA(outputHistogram));
    axisY->setTitleText("Frequency");
    chart->addAxis(axisY, Qt::AlignLeft);

    QValueAxis* axisX = new QValueAxis(chart);
    axisX->setRange(totalHistogramRange.first, totalHistogramRange.second);
    axisX->setTitleText("Intensity");
    chart->addAxis(axisX, Qt::AlignBottom);

    ui.chartViewMHA->setChart(chart);
}

void SeriesViewer::preloadAllmhaHistograms() {
    mhaHistogramsCharts.clear();
    //QVector<QVector<size_t>> histograms = calculateHistogramsForAllSlices();

    int sliceCount = mhaHistograms.size();
    int currentSlice = 0;

    for (int i = 0; i < sliceCount;i++) {
        mhaHistogramsCharts.push_back(createChartAndSetData(mhaHistograms[i]));
        //int progress = (currentSlice * 100) / sliceCount;
        progressDialogMHA.setValue((imageSliceMapper->GetSliceNumberMaxValue() + 1)*8+ i);
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        progressBarMHA->setValue((imageSliceMapper->GetSliceNumberMaxValue() + 1) * 8 + i);
        qApp->processEvents();
        if (progressDialogMHACanceled)
        {
            break;
            return;
        }
    }
    progressDialogMHA.setValue((imageSliceMapper->GetSliceNumberMaxValue() + 1) * 9);
    progressBar->setValue((imageSliceMapper->GetSliceNumberMaxValue() + 1) * 9);
    qApp->processEvents();
}

void SeriesViewer::preloadAllmhaHistogramsWithoutZeroIntensity() {

    mhaHistogramsChartsWithoutZero.clear();
    //QVector<QVector<size_t>> mhaHistograms = calculateHistogramsForAllSlices();

    int sliceCount = mhaHistograms.size();
    int currentSlice = 0;

    for (const auto& histogram : mhaHistograms) {
        QVector<size_t> histogramWithoutZero = removeZeroIntensityFromHistogram(histogram);
        mhaHistogramsChartsWithoutZero.push_back(createChartAndSetDataXZero(histogramWithoutZero));
        ++currentSlice;
        //int progress = (currentSlice * 100) / sliceCount;
        progressDialogMHA.setValue((imageSliceMapper->GetSliceNumberMaxValue() + 1)*9 + currentSlice);
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        progressBarMHA->setValue((imageSliceMapper->GetSliceNumberMaxValue() + 1) * 9 + currentSlice);
        qApp->processEvents();
        if (progressDialogMHACanceled)
        {
            break;
            return;
        }
    }
    progressDialogMHA.setValue((imageSliceMapper->GetSliceNumberMaxValue() + 1)*10);
    progressBarMHA->setValue((imageSliceMapper->GetSliceNumberMaxValue() + 1) * 10);
    qApp->processEvents();
    progressBarMHA->close();

}

QVector<size_t> SeriesViewer::removeZeroIntensityFromHistogram(const QVector<size_t>& histogram) {
    QVector<size_t> newHistogram = histogram;
    newHistogram.remove(0); // Remove the bin for intensity 0
    return newHistogram;
}

void SeriesViewer::onProgressMHACancel()
{
    progressDialogMHACanceled = true;
    ui.statusBar->showMessage("Loading Cancelled..");
    qApp->processEvents();
    QTimer::singleShot(100000, ui.statusBar, &QLabel::hide);
    progressBarMHA->close();
    qApp->processEvents();
    clearMHATab();
}