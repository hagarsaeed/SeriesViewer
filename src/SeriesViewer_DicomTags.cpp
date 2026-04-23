#include "SeriesViewer.h"

// Add spaces to tag name for better readability
std::string SeriesViewer::addSpacesToTagName(const std::string& tagName) {
    std::string result;
    bool isPreviousUpperCase = false;

    // Iterate over each character in the tag name
    for (char ch : tagName) {
        // Check if the character is uppercase
        if (isupper(ch)) {
            // Insert space before uppercase letters if the previous character was not uppercase
            if (isPreviousUpperCase) {
                result += ch;
            }
            else {
                result += ' ';
                result += ch;
                isPreviousUpperCase = true;
            }
        }
        else {
            result += ch;
            isPreviousUpperCase = false;
        }
    }

    return result;
}

// Load DICOM tags from a file
DicomTags SeriesViewer::loadTags(const QString& filename) {
    try {
        // Step 1: Load file.
        DcmFileFormat format;
        OFFilename file(filename.toStdString().c_str());
        OFCondition status = format.loadFile(file);

        // Check if the file loading was successful
        if (status.bad()) {
            throw std::runtime_error("Error reading DICOM file: " + filename.toStdString());
        }

        // Step 2: Get data set for all groups except group 2 and get meta info for group 2
        DcmDataset* dataset = format.getDataset();
        DcmMetaInfo* metaInfo = format.getMetaInfo();

        // Step 3: Get All Tags using Dcm data dictionary and iterate over them.
        DcmDataDictionary dict(true, true);
        auto ItrBegin = dict.normalBegin();
        auto ItrEnd = dict.normalEnd();
        unsigned long vm;
        unsigned int length;
        DicomTags TagValues;
        while (ItrBegin != ItrEnd) {
            // Step 3.1: Get the current entry to get the current Tag key.
            const DcmDictEntry* currEntry = *(ItrBegin++);
            DcmTagKey Key = currEntry->getKey();

            // Step 3.3: Get value of dcm element.
            UINT16 G = Key.getGroup();
            DcmElement* currElm;
            G == 2 ? metaInfo->findAndGetElement(Key, currElm) : dataset->findAndGetElement(Key, currElm);
            if (currElm == NULL)
                continue;
            DcmTag tg(Key);
            std::string tagname = tg.getTagName();
            std::string name = addSpacesToTagName(tagname);

            std::string GroupAndElement = Key.toString().data();
            OFString ValueField;
            std::string vr = tg.getVRName();
            vm = currElm->getVM();
            length = currElm->getLength();
            length = (int)length;
            vm = (int)vm;
            currElm->getOFStringArray(ValueField);
            TagValues.push_back(std::make_tuple(GroupAndElement, name, vr, vm, length, ValueField.data()));
        }

        return TagValues;
    }
    catch (const std::exception& e) {
        // Catch any exceptions and log them
        std::cerr << "Error while loading DICOM tags: " << e.what() << std::endl;
        return {};
    }
}

// Load DICOM tags for all files
void SeriesViewer::loadAllTags() {
    try {
        for (int j = 0; j < dicomFiles.size(); j++) {
            if (progressDialogCanceled) {
                break;
            }
            DicomTags tags = loadTags(dicomFiles.at(j));
            allDicomTags.push_back(tags);
            progressDialog.setValue(dicomFiles.count() * 4 + j * 4);
            QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            progressBar->setValue(dicomFiles.count() * 4 + j * 4);
            qApp->processEvents();
        }
    }
    catch (const std::exception& e) {
        // Catch any exceptions and log them
        std::cerr << "Error while loading DICOM tags for all files: " << e.what() << std::endl;
    }
}

// Display DICOM tags in a table widget
void SeriesViewer::viewDicomTags(const DicomTags& DicomTags) {
    try {
        // Clear the table widget
        ui.tableWidget->setRowCount(0);
        ui.tableWidget->setColumnCount(6); // Group Number, Element Number, Tag Description, Tag Value
        ui.tableWidget->setHorizontalHeaderLabels({ "Tags","Description","Value \n Representation","Value \nMultiplicity","Length", "Value" });

        // Iterate over each DICOM tag
        for (size_t i = 0; i < DicomTags.size(); i++) {
            int currentRow = ui.tableWidget->rowCount();
            ui.tableWidget->insertRow(currentRow);
            Tag rowData = DicomTags[i];
            QTableWidgetItem* item0 = new QTableWidgetItem(QString::fromStdString(std::get<0>(rowData)));
            QTableWidgetItem* item1 = new QTableWidgetItem(QString::fromStdString(std::get<1>(rowData)));
            QTableWidgetItem* item2 = new QTableWidgetItem(QString::fromStdString(std::get<2>(rowData)));
            QTableWidgetItem* item3 = new QTableWidgetItem(QString::fromStdString(std::to_string(std::get<3>(rowData))));
            QTableWidgetItem* item4 = new QTableWidgetItem(QString::fromStdString(std::to_string(std::get<4>(rowData))));
            QTableWidgetItem* item5 = new QTableWidgetItem(QString::fromStdString(std::get<5>(rowData)));

            // Set text alignment to center for all items
            item0->setTextAlignment(Qt::AlignCenter);
            item1->setTextAlignment(Qt::AlignCenter);
            item2->setTextAlignment(Qt::AlignCenter);
            item3->setTextAlignment(Qt::AlignCenter);
            item4->setTextAlignment(Qt::AlignCenter);
            item5->setTextAlignment(Qt::AlignCenter);

            // Set items in the table widget
            ui.tableWidget->setItem(currentRow, 0, item0);
            ui.tableWidget->setItem(currentRow, 1, item1);
            ui.tableWidget->setItem(currentRow, 2, item2);
            ui.tableWidget->setItem(currentRow, 3, item3);
            ui.tableWidget->setItem(currentRow, 4, item4);
            ui.tableWidget->setItem(currentRow, 5, item5);
        }

        // Resize columns to fit content
        ui.tableWidget->resizeColumnToContents(1);
        ui.tableWidget->resizeColumnToContents(2);
    }
    catch (const std::exception& e) {
        // Catch any exceptions and log them
        std::cerr << "Error while displaying DICOM tags: " << e.what() << std::endl;
    }
}