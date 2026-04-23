//#include "SeriesViewer.h"
//
//void SeriesViewer::saveDicomTagsToFile(const DicomTags& dicomTags,const QString &DicomFile)
//{
//    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory to Save File", QDir::homePath());
//    if (dir.isEmpty())
//    {
//        std::cout << "No directory selected. Aborting file saving." << std::endl;
//        return;
//    }
//
//    std::string directory = dir.toStdString();
//    std::string fileName = "dicom_tags.txt";  // Default file name
//
//    // Append the file name
//    std::string file = DicomFile.toStdString();
//    file.pop_back();file.pop_back();file.pop_back();file.pop_back();
//    fileName = file + ".txt";
//
//    std::ofstream outputFile(directory + "/" + fileName);
//    if (!outputFile)
//    {
//        QMessageBox::warning(this, "..", "file not open");
//        return;
//    }
//
//    outputFile << "Groub/Element No\tTag Name\tTag VR\tTag VM\tLength\tValue" << std::endl;
//
//    // Write the tags as rows in the table
//    for (const auto& tag : dicomTags)
//    {
//        std::string tagGroub_Elem_no= std::get<0>(tag);
//        std::string tagName = std::get<1>(tag);
//        std::string tagVR = std::get<2>(tag);
//        unsigned long tagVM = std::get<3>(tag);
//        unsigned int tagLength = std::get<4>(tag);
//        std::string tagValue = std::get<5>(tag);
//
//        outputFile << tagGroub_Elem_no << "\t" << tagName << "\t" << tagVR << "\t" << tagVM << "\t" << tagLength << "\t" << tagValue << std::endl;
//    }
//
//    outputFile.close();
//    QMessageBox::information(this, "..", "File saved Successfully");
//}