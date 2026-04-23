#include "SeriesViewer.h"

QImage SeriesViewer::readDICOM(const QString& filePath)
{
        DicomImage* dicomImage = new DicomImage(filePath.toStdString().c_str());


        if (dicomImage != nullptr && dicomImage->getStatus() == EIS_Normal)
        {
            const DiPixel* pixelData = dicomImage->getInterData();
            int width = dicomImage->getWidth();
            int height = dicomImage->getHeight();
            int depth = dicomImage->getDepth();


            if (pixelData && width > 0 && height > 0)
            {
                if (dicomImage->isMonochrome())
                {
                    QImage image;
                    QImage image16;
                    switch (depth)
                    {
                    case 8:
                    {
                        image = QImage(width, height, QImage::Format_Grayscale8);
                        memcpy(image.bits(), pixelData->getData(), width * height);
                        break;
                    }
                    case 16:
                    case 12:
                    {
                        image16 = QImage(width, height, QImage::Format_Grayscale16);
                        memcpy(image16.bits(), pixelData->getData(), width * height * 2);
                        allDicomImages16bit.push_back(image16);

                        const unsigned short* pixeldata = reinterpret_cast<const unsigned short*>(dicomImage->getOutputData(16));
                        if (pixelData)
                        {
                            image = rescale16BitTo8Bit(pixeldata, width, height);
                        }
                        break;
                    }
                    default:
                        // Handle unsupported bit depth (24 and 32 Bit)
                        break;
                    }
                    delete dicomImage;
                    return image;
                }
                else
                {
                    // Handle color images
                    QImage image(width, height, QImage::Format_RGB32);
                    uchar* imageBits = image.bits();
                    switch (depth)
                    {
                    case 8:
                    {
                        const uchar* pixelData = reinterpret_cast<const uchar*>(dicomImage->getOutputData(depth));


                        if (pixelData)
                        {
                            for (int y = 0; y < height; ++y)
                            {
                                for (int x = 0; x < width; ++x)
                                {
                                    int index = (y * width + x) * 3; // 3 channels
                                    // Assuming data is in RGB format
                                    uchar r = pixelData[index];
                                    uchar g = pixelData[index + 1];
                                    uchar b = pixelData[index + 2];


                                    QRgb value = qRgb(r, g, b);
                                    image.setPixel(x, y, value);
                                }
                            }
                        }
                        break;
                    }
                    case 16:
                    {
                        const unsigned short* pixelData = reinterpret_cast<const unsigned short*>(dicomImage->getOutputData(depth));


                        if (pixelData)
                        {
                            for (int y = 0; y < height; ++y)
                            {
                                for (int x = 0; x < width; ++x)
                                {
                                    int index = (y * width + x) * 3; // 3 channels
                                    // Scaling down 16-bit values to 8-bit
                                    uchar r = static_cast<uchar>((pixelData[index] >> 8) & 0xFF);     // Red
                                    uchar g = static_cast<uchar>((pixelData[index + 1] >> 8) & 0xFF); // Green
                                    uchar b = static_cast<uchar>((pixelData[index + 2] >> 8) & 0xFF); // Blue


                                    QRgb value = qRgb(r, g, b);
                                    image.setPixel(x, y, value);
                                }
                            }
                        }
                        break;
                    }
                    default:
                        // Handle unsupported bit depth (24 and 32 Bit)
                        break;
                    }
                    delete dicomImage;
                    return image;
                }
            }
        }


        delete dicomImage;
        return QImage(); // Return an empty image in case of failure
}

QImage SeriesViewer::rescale16BitTo8Bit(const unsigned short* pixelData, int width, int height)
{
    if (pixelData == nullptr)
    {
        return QImage();
    }

    // Find minimum and maximum pixel values
    unsigned short minPixel = USHRT_MAX;
    unsigned short maxPixel = 0;

    for (int i = 0; i < width * height; ++i)
    {
        unsigned short pixel = pixelData[i];
        if (pixel < minPixel) minPixel = pixel;
        if (pixel > maxPixel) maxPixel = pixel;
    }

    // Rescale pixel values
    QImage image(width, height, QImage::Format_Grayscale8);
    double range = maxPixel - minPixel;

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            unsigned short pixelValue = pixelData[y * width + x];
            uchar scaledPixel = static_cast<uchar>(255.0 * (pixelValue - minPixel) / range);
            image.setPixel(x, y, qRgb(scaledPixel, scaledPixel, scaledPixel));
        }
    }

    return image;
}

void SeriesViewer::LoadAllDicoms()
{
    for (int i = 0; i < dicomFiles.size(); i++)
    {
        if (progressDialogCanceled)
        {
            break;
        }
        QImage img = readDICOM(dicomFiles.at(i));
        allDicomImages8bit.push_back(img);
        progressDialog.setValue(i * 4);
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        progressBar->setValue(i * 4);
        qApp->processEvents();
    }
}

void SeriesViewer::displayDICOM(const QImage& DicomImage)
{
    QGraphicsScene* scene = new QGraphicsScene(this);

    // Calculate the target size based on the size of the QGraphicsView
    QSize targetSize = ui.graphicsView->viewport()->size();  // Get the size of the viewport

    // Scale the QImage to fit the target size while maintaining aspect ratio
    QImage scaledImage = DicomImage.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QImage mirroredImage = scaledImage.mirrored(true, false);
    item = scene->addPixmap(QPixmap::fromImage(mirroredImage, Qt::ColorOnly));
    ui.graphicsView->setScene(scene);
    ui.graphicsView->show();
}
