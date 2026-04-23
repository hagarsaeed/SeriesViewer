#include "SeriesViewer.h"

// Add the following slot to handle the "Generate" button click
void SeriesViewer::on_pushButtonRotateMHA_clicked()
{
    rotateMHASlice();
}

template <typename T>
double calculateAverage(const T* pixelData, int size)
{
    double sum = 0.0;
    for (int i = 0; i < size; ++i)
    {
        sum += static_cast<double>(pixelData[i]);
    }
    return sum / size;
}

void SeriesViewer::mhaSliceAverage()
{
    try
    {
        vtkImageData* imageData;
        // Get the current slice data
        if (ui.mhaSlider->value() != 0)
        {

            imageData = slices[sliceNumber];

            int* dims = imageData->GetDimensions();
            int sliceSize = dims[0] * dims[1];

            // Access the pixel data
            void* scalarPointer = imageData->GetScalarPointer();
            double average = 0.0;

            // Calculate the average intensity based on the pixel data type
            if (imageData->GetScalarType() == VTK_CHAR)
            {
                average = calculateAverage(static_cast<char*>(scalarPointer), sliceSize);
            }
            else if (imageData->GetScalarType() == VTK_UNSIGNED_CHAR)
            {
                average = calculateAverage(static_cast<unsigned char*>(scalarPointer), sliceSize);
            }
            else if (imageData->GetScalarType() == VTK_SHORT)
            {
                average = calculateAverage(static_cast<short*>(scalarPointer), sliceSize);
            }
            else if (imageData->GetScalarType() == VTK_UNSIGNED_SHORT)
            {
                average = calculateAverage(static_cast<unsigned short*>(scalarPointer), sliceSize);
            }
            else if (imageData->GetScalarType() == VTK_DOUBLE)
            {
                average = calculateAverage(static_cast<double*>(scalarPointer), sliceSize);
            }

            ui.label_ImgAvg->setText(QString::number(average));
        }
        else
        {
            return;
        }
    }
    catch (const std::exception& ex)
    {
        // Handle exceptions, log the error, or show an appropriate message to the user
        ui.statusBar->showMessage("Error calculating average: " + QString(ex.what()));
    }
}

void SeriesViewer::rotateMHASlice()
{
    try
    {
        int remainder = 0;
        if (cnt < 4)
        {
            remainder = 10;
        }
        else
        {
            remainder = cnt % 4;
        }
		if (remainder == 0 || cnt == 0)
        {
            ui.pushButtonRotateDicom->setIcon(QIcon("C:/Users/hp/Desktop/SeriesViewer/rotate icons/180.png"));
			imageSlice->SetOrientation(0, 0, -90);
			displayMeta();
		}
		else if (remainder == 1 || cnt == 1) 
        {
            ui.pushButtonRotateDicom->setIcon(QIcon("C:/Users/hp/Desktop/SeriesViewer/rotate icons/270.png"));
			imageSlice->SetOrientation(0, 0, -180);
			displayMeta();
		}
		else if (remainder == 2 || cnt == 2)
        {
            ui.pushButtonRotateDicom->setIcon(QIcon("C:/Users/hp/Desktop/SeriesViewer/rotate icons/360.png"));
			imageSlice->SetOrientation(0, 0, -270);
			displayMeta();
		}
		else if (remainder == 3 || cnt == 3) 
        {
            ui.pushButtonRotateDicom->setIcon(QIcon("C:/Users/hp/Desktop/SeriesViewer/rotate icons/90.png"));
			imageSlice->SetOrientation(0, 0, 0);
			displayMeta();
		}
        cnt++;
    }
    catch (const std::exception& ex)
    {
        ui.statusBar->showMessage("Error: " + QString(ex.what()));
    }
    catch (...)
    {
        // Handle other types of exceptions
        ui.statusBar->showMessage("An unknown error occurred.");
    }
}
