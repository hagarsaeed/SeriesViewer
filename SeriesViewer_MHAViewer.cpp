#include "SeriesViewer.h"
#include "vtkLookupTable.h"
#include "vtkImageMapToWindowLevelColors.h"

void SeriesViewer::on_pushButtonLoadmha_clicked()
{
	try
	{
		progressDialogMHACanceled = false; // Initialize progressDialogMHACanceled to false

		// Open a file dialog to choose the Meta Image file
		QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Meta Image Files (*.mha *.mhd)"));

		// Check if a file was selected
		if (fileName.isEmpty())
		{
			throw std::invalid_argument("No Meta Image file selected.");
		}

		// Clear the tab if the button is clicked for the second time
		if (!previousFileName.isEmpty())
		{
			if (previousFileName != fileName)
			{
				clearMHATab();
			}
		}

		previousFileName = fileName;
		ui.statusBar->clearMessage();
		// Set the file name for the reader
		mhaReader->SetFileName(fileName.toStdString().c_str());
		// Update the reader
		mhaReader->Update();

		int dataType = mhaReader->GetOutput()->GetScalarType();

		std::string datatype = vtkImageScalarTypeNameMacro(dataType);

		vtkImageData* imageData = mhaReader->GetOutput();
		double range[2];
		imageData->GetScalarRange(range); // Assuming imageData is your vtkImageData
		double windowWidth = range[1] - range[0]; // Full range
		double windowLevel = (range[1] + range[0]) / 2.0; // Midpoint

		// Create a window level filter
		vtkSmartPointer<vtkImageMapToWindowLevelColors> windowLevelFilter = vtkSmartPointer<vtkImageMapToWindowLevelColors>::New();
		windowLevelFilter->SetWindow(windowWidth);
		windowLevelFilter->SetLevel(windowLevel);
		windowLevelFilter->SetInputConnection(mhaReader->GetOutputPort());

		// Now, instead of directly using mhaReader->GetOutput() for your mapper, use the output of windowLevelFilter
		imageSliceMapper->SetInputConnection(windowLevelFilter->GetOutputPort());
		// Set the mapper for the image slice
		imageSlice->SetMapper(imageSliceMapper);
		//imageSlice->GetProperty()->SetLookupTable(lut);
		imageSlice->Update();
		// Set initial slice number
		imageSliceMapper->SetSliceNumber(0);
		
		if (progressDialogMHACanceled) {
			ui.statusBar->showMessage(" "); // Show empty message if progressDialog was canceled
		}
		if (progressDialogMHACanceled)
		{
			clearMHATab();
			return;
		}
		slices = vtkImageSliceToQVector(mhaReader->GetOutput());
		if (progressDialogMHACanceled)
		{
			clearMHATab();
			return;
		}
		mhaHistogram();
		if (progressDialogMHACanceled)
		{
			clearMHATab();
			return;
		}

		enableMHATabGUI();
		ui.spinBox_2->setValue(1);

		ui.mhaSlider->setRange(1, imageSliceMapper->GetSliceNumberMaxValue() + 1);
		ui.spinBox_2->setRange(1, imageSliceMapper->GetSliceNumberMaxValue() + 1);
		ui.mhaSlider->setValue(1);
		
		ui.label_MHAMax->setText(QString::number(imageSliceMapper->GetSliceNumberMaxValue() + 1));
		ui.label_MHAInd->setText("1");
		

		// Display the Meta Image
		displayMeta();
		getMHA_Info(fileName);

		mhaSliceAverage();
	}
	catch (const std::exception& ex)
	{
		// Handle exceptions, log the error, or show an appropriate message to the user
		ui.statusBar->showMessage("Error: " + QString(ex.what()));
		clearMHATab();  // Clear the tab in case of an exception
	}
	catch (...)
	{
		// Handle other types of exceptions
		ui.statusBar->showMessage("An unknown error occurred.");
		clearMHATab();  // Clear the tab in case of an unknown exception
	}
}



// Function to clear the tab (remove renderer and reset variables)
void SeriesViewer::clearMHATab()
{
	if (imageSliceMapper)
	{
		imageSliceMapper->SetSliceNumber(0);
	}
	sliceNumber = 0;
	QChart* chart = new QChart();
	ui.chartViewMHA->setChart(chart);
	ui.tableWidgetMHA->clearContents();
	renderer->RemoveAllViewProps();
	ui.vtkwidget->renderWindow()->Render();
	ui.label_ImgAvg->setText(" ");
	// Reset labels and status bar
	ui.label_MHAInd->setText(QString::number(0));
	ui.label_MHAMax->setText(QString::number(0));
	ui.statusBar->clearMessage();

	slices.clear();

	// Clear histograms and charts
	mhaHistograms.clear();
	mhaHistogramsCharts.clear();
	mhaHistogramsChartsWithoutZero.clear();


	disconnect(ui.mhaSlider, &QSlider::valueChanged, this, &SeriesViewer::on_mhaSlider_valueChanged);
	ui.mhaSlider->setRange(0, 0);
	ui.mhaSlider->setValue(0);
	ui.spinBox_2->setValue(0);
	connect(ui.mhaSlider, &QSlider::valueChanged, this, &SeriesViewer::on_mhaSlider_valueChanged);

	disableMHATabGUI();

	// Clear the previous file name
	previousFileName.clear();
}


// Slot for handling the value change of the Meta Image slider
void SeriesViewer::on_mhaSlider_valueChanged()
{
	try
	{
		if(ui.mhaSlider->value() != 0 )
		{

			ui.label_MHAInd->setText(QString::number(ui.mhaSlider->value()));
			// Update the slice number based on the slider value
			sliceNumber = ui.mhaSlider->value() - 1;
			// Display the Meta Image with the updated slice number
			displayMeta();
			mhaSliceAverage();

			if (ui.checkBoxwZeroFreqMHA->isChecked() && ui.checkBox_CHistMHA->isChecked())
			{
				mhaDisplayTotalHist();
			}
			else if (ui.checkBoxwZeroFreqMHA->isChecked())
			{
				ui.chartViewMHA->setChart(mhaHistogramsChartsWithoutZero[ui.mhaSlider->value() - 1]);
			}
			else
			{
				ui.chartViewMHA->setChart(mhaHistogramsCharts[ui.mhaSlider->value() - 1]);
			}
		}
	}
	catch (const std::exception& ex)
	{
		// Handle exceptions, log the error, or show an appropriate message to the user
		ui.statusBar->showMessage("Error: " + QString(ex.what()));
	}
	catch (...)
	{
		// Handle other types of exceptions
		ui.statusBar->showMessage("An unknown error occurred.");
	}
}

// Function to display the Meta Image
void SeriesViewer::displayMeta(void)
{
	try
	{
		// Set the slice number for the image slice mapper
		imageSliceMapper->SetSliceNumber(sliceNumber);
		// Update the image slice mapper
		imageSliceMapper->Update();

		// Remove all existing view props from the renderer
		renderer->RemoveAllViewProps();
		// Add the image slice to the renderer
		renderer->AddViewProp(imageSlice);
		//QVector<QImage> imgs = vtkImageDataToQImages(mhaReader->GetOutput());
		//mhaDisplayHistogram(imgs[ui.mhaSlider->value() - 1]);
		ui.vtkwidget->renderWindow()->SetSize(1500, 1500);
		// Reset the camera in the renderer with a specified aspect ratio
		renderer->ResetCamera(imageSlice->GetBounds());
		imageSlice->SetScale(2000, 2000,1);
		// Set the size and render the VTK widget
		ui.vtkwidget->renderWindow()->AddRenderer(renderer);
		ui.vtkwidget->renderWindow()->Render();

		// Render the renderer
		renderer->Render();
	}
	catch (const std::exception& ex)
	{
		// Handle exceptions, log the error, or show an appropriate message to the user
		ui.statusBar->showMessage("Error: " + QString(ex.what()));
	}
	catch (...)
	{
		// Handle other types of exceptions
		ui.statusBar->showMessage("An unknown error occurred.");
	}
}

//void SeriesViewer::getMHA_Info()
//{
//    // Clear existing items in the table widget
//    ui.tableWidgetMHA->clear();
//
//    // Set the column count and headers
//    ui.tableWidgetMHA->setColumnCount(2);
//    ui.tableWidgetMHA->setHorizontalHeaderLabels(QStringList() << "Property" << "Value");
//
//    // Add rows with MHA information
//    int row = 0;
//
//    // ObjectType
//    ui.tableWidgetMHA->insertRow(row);
//    ui.tableWidgetMHA->setItem(row, 0, new QTableWidgetItem("ObjectType"));
//    ui.tableWidgetMHA->setItem(row, 1, new QTableWidgetItem("Image"));
//    row++;
//
//    // NDims
//    ui.tableWidgetMHA->insertRow(row);
//    ui.tableWidgetMHA->setItem(row, 0, new QTableWidgetItem("NDims"));
//    ui.tableWidgetMHA->setItem(row, 1, new QTableWidgetItem(QString::number(mhaReader->GetOutput()->GetDataDimension())));
//    row++;
//
//    // BinaryData
//    ui.tableWidgetMHA->insertRow(row);
//    ui.tableWidgetMHA->setItem(row, 0, new QTableWidgetItem("BinaryData"));
//    ui.tableWidgetMHA->setItem(row, 1, new QTableWidgetItem("True"));
//    row++;
//
//    // BinaryDataByteOrderMSB
//    ui.tableWidgetMHA->insertRow(row);
//    ui.tableWidgetMHA->setItem(row, 0, new QTableWidgetItem("BinaryDataByteOrderMSB"));
//    ui.tableWidgetMHA->setItem(row, 1, new QTableWidgetItem("False"));
//    row++;
//
//    // CompressedData
//    ui.tableWidgetMHA->insertRow(row);
//    ui.tableWidgetMHA->setItem(row, 0, new QTableWidgetItem("CompressedData"));
//    ui.tableWidgetMHA->setItem(row, 1, new QTableWidgetItem("True"));
//    row++;
//
//    // Offset
//    ui.tableWidgetMHA->insertRow(row);
//    ui.tableWidgetMHA->setItem(row, 0, new QTableWidgetItem("Offset"));
//    double offset[3];
//    mhaReader->GetOutput()->GetOrigin(offset);
//    ui.tableWidgetMHA->setItem(row, 1, new QTableWidgetItem(QString("%1 - %2 - %3").arg(offset[0]).arg(offset[1]).arg(offset[2])));
//    row++;
//
//    // CenterOfRotation
//    ui.tableWidgetMHA->insertRow(row);
//    ui.tableWidgetMHA->setItem(row, 0, new QTableWidgetItem("CenterOfRotation"));
//    double centerOfRotation[3] = { 0.0, 0.0, 0.0 }; // Default values if not available
//    // mhaReader->GetCenterOfRotation(centerOfRotation); // Uncomment if available
//    ui.tableWidgetMHA->setItem(row, 1, new QTableWidgetItem(QString("%1 - %2 - %3").arg(centerOfRotation[0]).arg(centerOfRotation[1]).arg(centerOfRotation[2])));
//    row++;
//
//
//    // ElementSpacing
//    ui.tableWidgetMHA->insertRow(row);
//    ui.tableWidgetMHA->setItem(row, 0, new QTableWidgetItem("ElementSpacing"));
//    double spacing[3];
//    mhaReader->GetOutput()->GetSpacing(spacing);
//    ui.tableWidgetMHA->setItem(row, 1, new QTableWidgetItem(QString("%1 - %2 - %3").arg(spacing[0]).arg(spacing[1]).arg(spacing[2])));
//    row++;
//
//    // DimSize
//    ui.tableWidgetMHA->insertRow(row);
//    ui.tableWidgetMHA->setItem(row, 0, new QTableWidgetItem("DimSize"));
//    int dimensions[3];
//    mhaReader->GetOutput()->GetDimensions(dimensions);
//    ui.tableWidgetMHA->setItem(row, 1, new QTableWidgetItem(QString("%1 - %2 - %3").arg(dimensions[0]).arg(dimensions[1]).arg(dimensions[2])));
//    row++;
//
//    // ElementType
//    ui.tableWidgetMHA->insertRow(row);
//    ui.tableWidgetMHA->setItem(row, 0, new QTableWidgetItem("ElementType"));
//    ui.tableWidgetMHA->setItem(row, 1, new QTableWidgetItem(mhaReader->GetOutput()->GetScalarTypeAsString()));
//    row++;
//
//    // Resize to fit the content
//    ui.tableWidgetMHA->resizeRowsToContents();
//    ui.tableWidgetMHA->resizeColumnsToContents();
//}



void SeriesViewer::getMHA_Info(QString filePath)
{
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::critical(nullptr, "Error", "Could not open the file.");
		return;
	}

	QTextStream in(&file);
	QStringList headerLabels;
	QStringList data;

	while (!in.atEnd())
	{
		QString line = in.readLine();

		// Check if the line contains any strange characters
		bool hasStrangeCharacter = false;
		for (QChar c : line)
		{
			if (!c.isLetterOrNumber() && c != '=' && c != ' ')
			{
				hasStrangeCharacter = true;
				break;
			}
		}

		if (hasStrangeCharacter)
		{
			continue; // Skip processing this line
		}

		// Check if the line is not empty and starts with an alphabetical character
		if (!line.trimmed().isEmpty() && line.at(0).isLetter())
		{
			// Define the pattern for allowed characters
			QRegularExpression exp("[A-Za-z=_ ]+");

			// Check if the line matches the pattern
			if (exp.match(line).hasMatch())
			{
				// Extract property and value pairs
				QStringList parts = line.split('=');

				// Make sure the line contains the '=' character
				if (parts.size() == 2)
				{
					headerLabels << parts[0].trimmed();
					data.append(parts[1].trimmed());
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
	}

	file.close();

	ui.tableWidgetMHA->setColumnCount(2);
	ui.tableWidgetMHA->setHorizontalHeaderLabels({ "Property", "Value" });

	ui.label_MHAinfo->setAlignment(Qt::AlignCenter);
	ui.label_MHAinfo->setTextFormat(Qt::RichText);
	ui.label_MHAinfo->setText("Meta Information:");

	ui.tableWidgetMHA->setRowCount(headerLabels.size());
	for (int row = 0; row < headerLabels.size(); ++row)
	{
		QTableWidgetItem* propertyItem = new QTableWidgetItem(headerLabels[row]);
		QTableWidgetItem* valueItem = new QTableWidgetItem(data[row]);
		propertyItem->setTextAlignment(Qt::AlignCenter);
		valueItem->setTextAlignment(Qt::AlignCenter);
		ui.tableWidgetMHA->setItem(row, 0, propertyItem);
		ui.tableWidgetMHA->setItem(row, 1, valueItem);
	}

	ui.tableWidgetMHA->resizeColumnsToContents();
	ui.tableWidgetMHA->sizeIncrement();
}

void SeriesViewer::disableMHATabGUI()
{
	ui.mhaSlider->setDisabled(true);
	ui.spinBox_2->setDisabled(true);
	ui.pushButtonRotateMHA->setEnabled(false);
	ui.checkBoxwZeroFreqMHA->setEnabled(false);
	ui.checkBox_CHistMHA->setEnabled(false);
}

void SeriesViewer::enableMHATabGUI()
{
	ui.mhaSlider->setDisabled(false);
	ui.spinBox_2->setDisabled(false);
	ui.pushButtonRotateMHA->setEnabled(true);
	ui.checkBoxwZeroFreqMHA->setEnabled(true);
	ui.checkBox_CHistMHA->setEnabled(true);
}


