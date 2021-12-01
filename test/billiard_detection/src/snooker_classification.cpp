#include <gtest/gtest.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <billiard_detection/billiard_detection.hpp>
#include <billiard_snooker/billiard_snooker.hpp>
#include <billiard_capture/billiard_capture.hpp>
#include <librealsense2/rs.hpp>
#include "config.hpp"
#include <chrono>
#include <filesystem>

// TODO: andere metriken als häufigste farbe von hsv ausprobieren, etwa mittlere graustufe von hsv oder andere Zahlen aus Bildverarbeitung
// TODO: evtl. diese anderen metriken mit bestehenden kombinieren

/**
 * Based on https://docs.opencv.org/2.4/doc/tutorials/imgproc/histograms/histogram_calculation/histogram_calculation.html#code
 */
std::vector<billiard::snooker::Histogram> histogram(const cv::Mat& input) {
    std::vector<cv::Mat> planes;
    cv::split(input, planes);

    int histSize = 256;
    float range[] = { 0, 256 } ;
    const float* histRange = { range };
    bool uniform = true;
    bool accumulate = false;

    std::vector<billiard::snooker::Histogram> histograms;
    for (const auto& plane : planes) {
        cv::Mat histogramPlane;
        cv::calcHist(&plane, 1, nullptr, cv::Mat(), histogramPlane, 1, &histSize, &histRange, uniform, accumulate);

        cv::Point minLocation, maxLocation;
        cv::minMaxLoc(histogramPlane, nullptr, nullptr, &minLocation, &maxLocation);

        histograms.emplace_back(billiard::snooker::Histogram{histogramPlane, maxLocation});
    }

    return histograms;
}

int findLabelIndex(const std::vector<std::string>& labels, const std::string& label) {
    for (int i = 0; i < labels.size(); i++) {
        if (labels[i] == label) {
            return i;
        }
    }
    std::cerr << "Unable to find label " << label << std::endl;
    return -1;
};

TEST(SnookerClassificationTests, snooker_classify_from_live_or_from_image) {

    bool live = false;
    std::vector<std::string> imagePaths = {
            "./resources/test_detection/1.png",
            "./resources/test_detection/2.png",
            "./resources/test_detection/3.png",
            "./resources/test_detection/4.png",
            "./resources/test_detection/5.png",
            "./resources/test_detection/6.png",
            "./resources/test_detection/7.png",
            "./resources/test_detection/8.png",
            "./resources/test_detection/9.png",
            "./resources/test_detection/10.png",
            "./resources/test_detection/11.png",
            "./resources/test_detection/12.png",
            "./resources/test_detection/13.png",
            "./resources/test_detection/14.png",
            "./resources/test_detection/15.png",
            "./resources/test_detection/16.png",
            "./resources/test_detection/17.png",
            "./resources/test_detection/18.png",
            "./resources/test_detection/19.png",
            "./resources/test_detection/20.png",
    };

    cv::Size imageSize = getImageSize();
    billiard::detection::Table table = getTable();
    billiard::detection::ArucoMarkers markers = getMarkers();
    billiard::detection::CameraIntrinsics intrinsics = getIntrinsics_realsense_hd();

    unsigned long long imageIndex = 0;
    bool imageChanged = true;

    billiard::capture::CameraCapture capture {};
    if (live) {
        if (!capture.open()) {
            std::cerr << "Unable to open image stream" << std::endl;
            return;
        }
    }

    std::shared_ptr<billiard::detection::DetectionConfig> detectionConfig;

    cv::Mat result;

    while(true) {
        cv::Mat frame;

        if (live) {
            if (imageChanged) {
                billiard::capture::CameraFrames frames = capture.read();
                frames.color.copyTo(frame);
            }
        } else {
            std::string imagePath = imagePaths[imageIndex];
            frame = imread(imagePath, cv::IMREAD_COLOR);
            cv::resize(frame, frame, imageSize);
        }
        frame.copyTo(result);

        if (imageChanged) {
            imageChanged = false;

            detectionConfig = std::make_shared<billiard::detection::DetectionConfig>(billiard::detection::configure(frame, table, markers, intrinsics));
            if (!detectionConfig->valid) {
                std::cout << "Unable to configure detection" << std::endl;
                return;
            }

            if (!billiard::snooker::configure(*detectionConfig)) {
                std::cout << "Unable to configure snooker detection" << std::endl;
                return;
            }

            billiard::detection::State pixelState = billiard::snooker::detect(billiard::detection::State(), frame);

            auto t1 = std::chrono::high_resolution_clock::now();
            billiard::snooker::classify(billiard::detection::State {}, pixelState, frame);
            auto t2 = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double, std::milli> ms_double = t2 - t1;
            std::cout << "Classification took " << ms_double.count() << "ms" << std::endl;

            for (auto& ball : pixelState._balls) {
                cv::Point2d pixelPoint = cv::Point2d(ball._position.x, ball._position.y);
                auto label = ball._type;
                cv::putText(result, label, pixelPoint, cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar{255, 255, 255});
            }
            cv::imshow("Classification", result);
        }

#ifdef NDEBUG
        if (!live) {
            imageIndex++;
            imageChanged = true;
            if (imageIndex == imagePaths.size()) {
                return;
            }
        }
#endif

        char key = (char) cv::waitKey(30);
        if (key == 'q' || key == 27) {
            break;
        } else if (key == 'l') {
            imageChanged = true;
        } else if (key == 97 /* A */) {
            imageIndex = imageIndex == 0 ? imagePaths.size() - 1 : imageIndex - 1;
            imageChanged = true;
        } else if (key == 100 /* D */) {
            imageIndex = (imageIndex + 1) % (imagePaths.size());
            imageChanged = true;
        }
    }

}

TEST(SnookerClassificationTests, snooker_classify_visualization) {

//    std::string classificationFolder = "./resources/test_classification/with_projector_off/";
//    std::string classificationFolder = "./resources/test_classification/with_projector_on/without_text/";
    std::string classificationFolder = "./resources/test_classification/with_projector_on/with_halo/";
//    std::string classificationFolder = "./resources/test_classification/with_projector_on/with_text/";
    std::vector<std::string> labels = {
            "BLACK",
            "BROWN",
            "RED",
            "PINK",
            "YELLOW",
            "WHITE",
            "BLUE",
            "GREEN",
            "UNKNOWN"
    };

    const std::vector<cv::Scalar>& labelColors = {
            // WARNING: these colors are in BGR format, NOT RGB!
            cv::Scalar {0, 0, 0}, // BLACK
            cv::Scalar {0, 102, 204}, // BROWN
            cv::Scalar {0, 51, 255}, // RED
            cv::Scalar {204, 102, 255}, // PINK
            cv::Scalar {0, 255, 255}, // YELLOW
            cv::Scalar {150, 150, 150}, // WHITE
            cv::Scalar {255, 0, 0}, // BLUE
            cv::Scalar {51, 153, 0}, // GREEN
            cv::Scalar {203, 76, 140}, // UNKNOWN
    };

    billiard::snooker::SnookerClassificationConfig config {};

    const auto drawClusterImage = [](
            int imageSize,
            int firstDimensionIndex,
            int secondDimensionIndex,
            const std::vector<std::string>& labels,
            const std::vector<cv::Scalar>& labelColors,
            const std::vector<std::string>& labelsToDraw,
            const std::vector<std::pair<std::string, cv::Vec3i>>& dataPoints) {

        cv::Scalar backgroundColor {255, 255, 255}; // IN BGR
        cv::Scalar conflictColor {251, 255, 0}; // IN BGR

        int borderWidth = 1; // in pixels
        cv::Scalar borderColor {40, 40, 40}; // IN BGR
        cv::Mat image {imageSize, imageSize, CV_8UC3, backgroundColor};

        for (auto& labelToDraw : labelsToDraw) {
            // Draw data sorted by label order in labelsToDraw

            for (auto& dataPoint : dataPoints) {

                auto& label = dataPoint.first;
                if (labelToDraw != label) {
                    continue;
                }

                int labelIndex = findLabelIndex(labels, label);
                cv::Scalar labelColor = labelColors[labelIndex];

                auto& point = dataPoint.second;
                cv::Point2i imagePoint { point[firstDimensionIndex] + borderWidth, point[secondDimensionIndex] + borderWidth };
                int radius = 2;

                auto existingColor = image.at<cv::Vec3b>(imagePoint);
                if (existingColor[0] != labelColor[0] && existingColor[1] != labelColor[1] && existingColor[2] != labelColor[2]) {
//                labelColor = conflictColor;
                }

                cv::circle(image, imagePoint, radius, labelColor, cv::FILLED);
            }
        }

        const cv::Point& topLeft = cv::Point{0, 0};
        const cv::Point& topRight = cv::Point{image.cols - 1, 0};
        const cv::Point& bottomLeft = cv::Point{0, image.rows - 1};
        const cv::Point& bottomRight = cv::Point{image.cols - 1, image.rows - 1};
        cv::line(image, topLeft, topRight, borderColor, borderWidth);
        cv::line(image, topRight, bottomRight, borderColor, borderWidth);
        cv::line(image, bottomRight, bottomLeft, borderColor, borderWidth);
        cv::line(image, bottomLeft, topLeft, borderColor, borderWidth);

        return image;
    };

    const auto drawClusterAnalysis = [drawClusterImage](
            const std::vector<std::string>& labels,
            const std::vector<cv::Scalar>& labelColors,
            const std::vector<std::string>& labelsToDraw,
            const std::vector<std::pair<std::string, cv::Vec3i>>& dataPoints) {

        int singleImageSize = 256 + 2;
        cv::Scalar backgroundColor {255, 255, 255}; // IN BGR
        int dimensions = 3;
        int padding = 10; // in pixels
        int totalImageSize = dimensions * singleImageSize + padding * (dimensions + 1);
        cv::Mat totalImage(totalImageSize, totalImageSize, CV_8UC3, backgroundColor);

        for (int firstDimensionIndex = 0; firstDimensionIndex < dimensions; firstDimensionIndex++) {
            for (int secondDimensionIndex = 0; secondDimensionIndex < dimensions; secondDimensionIndex++) {

                cv::Mat clusterImage = drawClusterImage(singleImageSize, firstDimensionIndex, secondDimensionIndex, labels, labelColors, labelsToDraw, dataPoints);

                int x = secondDimensionIndex * singleImageSize + padding * (secondDimensionIndex + 1);
                int y = firstDimensionIndex * singleImageSize + padding * (firstDimensionIndex + 1);
                cv::Rect roi {cv::Point(x, y), cv::Size {clusterImage.cols, clusterImage.rows}};
                cv::Mat dst = totalImage(roi);
                clusterImage.copyTo(dst);
            }
        }
        return totalImage;
    };

    const auto createDataPoint = [config](const cv::Mat& bgr) {

        cv::Mat blurred;
        cv::GaussianBlur(bgr, blurred, config.blurSize, 0, 0);

        cv::Mat hsv;
        cv::cvtColor(blurred, hsv, cv::COLOR_BGR2HSV);

        std::vector<cv::Mat> planes;
        cv::split(hsv, planes);

//        int hueMean = (int) cv::mean(planes[0])[0];
//        int saturationMean = (int) cv::mean(planes[1])[0];
//        int valueMean = (int) cv::mean(planes[2])[0];
//
//        cv::Vec3i point = {hueMean, saturationMean, valueMean};

        auto histogramByChannels = histogram(hsv);

        billiard::snooker::Histogram hueHist = histogramByChannels[0];
        billiard::snooker::Histogram saturationHist = histogramByChannels[1];
        billiard::snooker::Histogram valueHist = histogramByChannels[2];
        int maxHue = hueHist.maxLocation.y;
        int maxSaturation = saturationHist.maxLocation.y;
        int maxValue = valueHist.maxLocation.y;

        cv::Vec3i point = {maxHue, maxSaturation, maxValue};

        return point;
    };

    std::vector<std::pair<std::string, cv::Vec3i>> dataPoints {};

    for (auto& trueLabel : labels) {

        int trueLabelIndex = findLabelIndex(labels, trueLabel);

        std::string path = classificationFolder + trueLabel + "/";
        for (const auto& fileEntry : std::filesystem::directory_iterator(path)) {
            const auto& imagePath = fileEntry.path().string();

//            std::cout << imagePath << std::endl;

            cv::Mat original = imread(imagePath, cv::IMREAD_COLOR);

            float radius = original.rows / 2 * config.roiRadiusFactor;

            const cv::Rect& roi = cv::Rect{
                    (int) (original.cols / 2 - radius),
                    (int) (original.rows / 2 - radius),
                    (int) (2 * radius),
                    (int) (2 * radius)
            };
            cv::Mat frame = original(roi);

            auto dataPoint = std::make_pair(trueLabel, createDataPoint(frame));
            dataPoints.push_back(dataPoint);
            // Add minimum and maximum points
            dataPoints.push_back(std::make_pair("UNKNOWN", cv::Vec3i {0, 0, 0}));
            dataPoints.push_back(std::make_pair("UNKNOWN", cv::Vec3i {180, 255, 255}));
        }
    }

    std::vector<std::string> labelsToDraw { "BLACK", "BROWN", "RED", "PINK", "YELLOW", "WHITE", "BLUE", "GREEN", "UNKNOWN" };
//    std::vector<std::string> labelsToDraw { "BROWN", "RED", "PINK", "UNKNOWN" };
    cv::Mat clusterAnalysis = drawClusterAnalysis(labels, labelColors, labelsToDraw, dataPoints);
    cv::imshow("Cluster", clusterAnalysis);
    cv::imwrite("Cluster_1.png", clusterAnalysis);

    cv::Mat all = drawClusterAnalysis(labels, labelColors, labels, dataPoints);
    cv::imshow("all", all);
    cv::imwrite("Cluster_all.png", all);

    cv::Mat redOverPink = drawClusterAnalysis(labels, labelColors, { "PINK", "RED", "UNKNOWN" }, dataPoints);
    cv::imwrite("Cluster_RED_over_PINK.png", redOverPink);
    cv::Mat pinkOverRed = drawClusterAnalysis(labels, labelColors, { "RED", "PINK", "UNKNOWN" }, dataPoints);
    cv::imwrite("Cluster_PINK_over_RED.png", pinkOverRed);
    cv::Mat redOverBrown = drawClusterAnalysis(labels, labelColors, { "BROWN", "RED", "UNKNOWN" }, dataPoints);
    cv::imwrite("Cluster_RED_over_BROWN.png", redOverBrown);
    cv::Mat brownOverRed = drawClusterAnalysis(labels, labelColors, { "RED", "BROWN", "UNKNOWN" }, dataPoints);
    cv::imwrite("Cluster_BROWN_over_RED.png", brownOverRed);
    cv::Mat blackOverRed = drawClusterAnalysis(labels, labelColors, { "BLACK", "RED", "UNKNOWN" }, dataPoints);
    cv::imwrite("Cluster_BLACK_over_RED.png", blackOverRed);
    cv::Mat redOverBlack = drawClusterAnalysis(labels, labelColors, { "RED", "BLACK", "UNKNOWN" }, dataPoints);
    cv::imwrite("Cluster_RED_over_BLACK.png", redOverBlack);
    cv::Mat whiteOverYellow = drawClusterAnalysis(labels, labelColors, { "YELLOW", "WHITE", "UNKNOWN" }, dataPoints);
    cv::imwrite("Cluster_WHITE_over_YELLOW.png", whiteOverYellow);
    cv::Mat yellowOverWhite = drawClusterAnalysis(labels, labelColors, { "WHITE", "YELLOW", "UNKNOWN" }, dataPoints);
    cv::imwrite("Cluster_YELLOW_over_WHITE.png", yellowOverWhite);
    cv::Mat blueGreenYellow = drawClusterAnalysis(labels, labelColors, { "BLUE", "GREEN", "YELLOW", "UNKNOWN" }, dataPoints);
    cv::imwrite("Cluster_BLUE_GREEN_WHITE_YELLOW.png", whiteOverYellow);

    cv::Mat some = drawClusterAnalysis(labels, labelColors, { "YELLOW", "WHITE", "BLACK", "UNKNOWN" }, dataPoints);
    cv::imshow("some", some);
    cv::imwrite("Cluster_some.png", some);

    cv::waitKey();
}

TEST(SnookerClassificationTests, snooker_classify_single_balls) {

    std::string configurationImage = "./resources/test_detection/with_projector_on/with_halo/1.png";
//    std::string classificationFolder = "./resources/test_classification/with_projector_off/";
//    std::string classificationFolder = "./resources/test_classification/with_projector_on/without_text/";
    std::string classificationFolder = "./resources/test_classification/with_projector_on/with_halo_2/";
//    std::string classificationFolder = "./resources/test_classification/with_projector_on/with_text/";
    std::vector<std::string> labels = {
            "BROWN",
            "PINK",
            "RED",
            "BLACK",
            "YELLOW",
            "WHITE",
            "BLUE",
            "GREEN",
            "UNKNOWN"
    };
    int numberOfLabels = (int) labels.size();

    // True classes in horizontal, Predicted classes in vertical
    cv::Mat confusionMatrix {numberOfLabels, numberOfLabels, CV_32F, cv::Scalar{0.0}};

    billiard::detection::Table table = getTable();
    billiard::detection::ArucoMarkers markers = getMarkers();
    billiard::detection::CameraIntrinsics intrinsics = getIntrinsics_realsense_hd();
    cv::Mat frame = cv::imread(configurationImage);
    std::shared_ptr<billiard::detection::DetectionConfig> detectionConfig = std::make_shared<billiard::detection::DetectionConfig>(billiard::detection::configure(frame, table, markers, intrinsics));
    billiard::snooker::SnookerClassificationConfig classificationConfig {};
    if (!billiard::snooker::configure(*detectionConfig)) {
        std::cout << "Unable to configure" << std::endl;
        return;
    }

    for (auto& trueLabel : labels) {

        int trueLabelIndex = findLabelIndex(labels, trueLabel);

        std::string path = classificationFolder + trueLabel + "/";
        for (const auto& fileEntry : std::filesystem::directory_iterator(path)) {
            const auto& imagePath = fileEntry.path().string();

//            std::cout << imagePath << std::endl;

            cv::Mat original = imread(imagePath, cv::IMREAD_COLOR);

            double radius = original.rows / 2 * classificationConfig.roiRadiusFactor;

            const cv::Rect& roi = cv::Rect{
                    (int) ((double) original.cols / 2 - radius),
                    (int) ((double) original.rows / 2 - radius),
                    (int) (2 * radius),
                    (int) (2 * radius)
            };
            cv::Mat frame = original(roi);

            billiard::detection::Ball ball {};
            ball._id = imagePath;
            ball._position = glm::vec2{};
            ball._type = "UNKNOWN";
            billiard::snooker::classify(ball, frame);

            std::string predictedLabel = ball._type;
            int predictedLabelIndex = findLabelIndex(labels, predictedLabel);
            confusionMatrix.at<float>(predictedLabelIndex, trueLabelIndex) = confusionMatrix.at<float>(predictedLabelIndex, trueLabelIndex) + 1.0f;

            if (predictedLabelIndex != trueLabelIndex) {
                std::cout << "---> INCORRECT CLASSIFICATION" << std::endl;
            }
        }
    }

    const auto calculateConfusionMatrixStats = [](const cv::Mat& confusionMatrix, std::vector<float>& precisionValues) {

        float diagonalSum = 0.0f;
        float totalSum = 0.0f;

        for (int row = 0; row < confusionMatrix.rows; row++) {

            float sumOfRow = 0.0f;
            for (int col = 0; col < confusionMatrix.cols; col++) {
                float value = confusionMatrix.at<float>(row, col);
                sumOfRow += value;
            }
            totalSum += sumOfRow;

            float predictedValue = confusionMatrix.at<float>(row, row);
            diagonalSum += confusionMatrix.at<float>(row, row);

            // Precision = TP / (TP + FP)
            float precision = predictedValue / sumOfRow;
            precisionValues.push_back(precision);
        }

        // Total accuracy = (TP + TN) / (TP + FP + TN + FN)
        float totalAccuracy = diagonalSum / totalSum;

        return totalAccuracy;
    };

    const auto printConfusionTable = [](const std::vector<std::string>& labels, cv::Mat confusionMatrix) {
        std::string separator = "|";
        int maxLabelLength = 0;
        for (auto& label : labels) {
            if (label.length() > maxLabelLength) {
                maxLabelLength = (int) label.length();
            }
        }
        std::cout << "  ¦-------PREDICTED CLASSES" << std::endl;
        std::cout << "  v       TRUE CLASSES ------>" << std::endl;
        std::cout << "  " << std::setfill(' ') << std::setw(maxLabelLength) << " " << separator << " ";
        for (auto& label : labels) {
            std::cout << std::setfill(' ') << std::setw(maxLabelLength) << label << " " << separator << " ";
        }
        std::cout << std::endl;
        for (int row = 0; row < confusionMatrix.rows; row++) {
            std::cout << " ";
            auto& label = labels[row];
            std::cout << std::setfill(' ') << std::setw(maxLabelLength) << label << " " << separator << " ";
            for (int col = 0; col < confusionMatrix.cols; col++) {
                float value = confusionMatrix.at<float>(row, col);
                std::cout << std::setfill(' ') << std::setw(maxLabelLength) << value << " " << separator << " ";
            }
            std::cout << std::endl;
        }
    };
    std::vector<float> precisionValues;
    float totalAccuracy = calculateConfusionMatrixStats(confusionMatrix, precisionValues);
    printConfusionTable(labels, confusionMatrix);

    std::cout << "Total accuracy: " << totalAccuracy << std::endl;
    std::cout << "Confusion matrix: " << std::endl << confusionMatrix << std::endl;
}

TEST(SnookerClassificationTests, snooker_find_cluster_centers) {

    std::string configurationImage = "./resources/test_detection/with_projector_on/with_halo/1.png";
//    std::string classificationFolder = "./resources/test_classification/with_projector_off/";
    std::string classificationFolder = "./resources/test_classification/with_projector_on/without_text/";
//    std::string classificationFolder = "./resources/test_classification/with_projector_on/with_halo/";
//    std::string classificationFolder = "./resources/test_classification/with_projector_on/with_text/";
    std::vector<std::string> labels = {
            "BROWN",
            "PINK",
            "RED",
            "BLACK",
            "YELLOW",
            "WHITE",
            "BLUE",
            "GREEN",
            "UNKNOWN"
    };
    int numberOfLabels = (int) labels.size();

    billiard::detection::Table table = getTable();
    billiard::detection::ArucoMarkers markers = getMarkers();
    billiard::detection::CameraIntrinsics intrinsics = getIntrinsics_realsense_hd();
    cv::Mat frame = cv::imread(configurationImage);
    std::shared_ptr<billiard::detection::DetectionConfig> detectionConfig = std::make_shared<billiard::detection::DetectionConfig>(billiard::detection::configure(frame, table, markers, intrinsics));
    billiard::snooker::SnookerClassificationConfig classificationConfig {};
    if (!billiard::snooker::configure(*detectionConfig)) {
        std::cout << "Unable to configure" << std::endl;
        return;
    }

    for (auto& trueLabel : labels) {

        int trueLabelIndex = findLabelIndex(labels, trueLabel);

        int totalHue1 = 0;
        int totalHue2 = 0;
        int totalSaturation = 0;
        int totalValue = 0;
        int sampleCounter = 0;
        int hueCounter1 = 0;
        int hueCounter2 = 0;

        std::string path = classificationFolder + trueLabel + "/";
        for (const auto& fileEntry : std::filesystem::directory_iterator(path)) {
            const auto& imagePath = fileEntry.path().string();

            cv::Mat original = imread(imagePath, cv::IMREAD_COLOR);

            double radius = original.rows / 2 * classificationConfig.roiRadiusFactor;

            const cv::Rect& roi = cv::Rect{
                    (int) ((double) original.cols / 2 - radius),
                    (int) ((double) original.rows / 2 - radius),
                    (int) (2 * radius),
                    (int) (2 * radius)
            };
            cv::Mat frame = original(roi);

            cv::Mat blurred;
            cv::GaussianBlur(frame, blurred, classificationConfig.blurSize, 0, 0);

            cv::Mat hsv;
            cv::cvtColor(blurred, hsv, cv::COLOR_BGR2HSV);

            auto histogramByChannels = histogram(hsv);

            billiard::snooker::Histogram hueHist = histogramByChannels[0];
            billiard::snooker::Histogram saturationHist = histogramByChannels[1];
            billiard::snooker::Histogram valueHist = histogramByChannels[2];
            int maxHue = hueHist.maxLocation.y;
            int maxSaturation = saturationHist.maxLocation.y;
            int maxValue = valueHist.maxLocation.y;

            if (maxHue < 128) {
                totalHue1 += maxHue;
                hueCounter1++;
            } else {
                totalHue2 += maxHue;
                hueCounter2++;
            }

            totalSaturation += maxSaturation;
            totalValue += maxValue;

            sampleCounter++;
        }

        double averageHue1 = (double) totalHue1 / (double) hueCounter1;
        double averageHue2 = (double) totalHue2 / (double) hueCounter2;
        double averageSaturation = (double) totalSaturation / (double) sampleCounter;
        double averageValue = (double) totalValue / (double) sampleCounter;

        std::cout << "class: " << trueLabel << " samples: " << sampleCounter
                  << " avg hue1: " << averageHue1 << " (" << hueCounter1 << ")"
                  << " avg hue2: " << averageHue2 << " (" << hueCounter2 << ")"
                  << " avg saturation: " << averageSaturation
                  << " avg value: " << averageValue
                  << std::endl;
    }
}

TEST(SnookerClassificationTests, snooker_write_classification_test_images) {

    bool writeImages = false;
    std::string outputFolder = "to_be_classified_balls/";
    std::vector<std::string> imagePaths = {
//            "./resources/test_detection/1.png",
//            "./resources/test_detection/2.png",
//            "./resources/test_detection/3.png",
//            "./resources/test_detection/4.png",
//            "./resources/test_detection/5.png",
//            "./resources/test_detection/6.png",
//            "./resources/test_detection/7.png",
//            "./resources/test_detection/8.png",
//            "./resources/test_detection/9.png",
//            "./resources/test_detection/10.png",
//            "./resources/test_detection/11.png",
//            "./resources/test_detection/12.png",
//            "./resources/test_detection/13.png",
//            "./resources/test_detection/14.png",
//            "./resources/test_detection/15.png",
//            "./resources/test_detection/16.png",
//            "./resources/test_detection/17.png",
//            "./resources/test_detection/18.png",
//            "./resources/test_detection/19.png",
//            "./resources/test_detection/20.png",
//            "./resources/test_detection_with_projector/1.png",
//            "./resources/test_detection_with_projector/2.png",
//            "./resources/test_detection_with_projector/3.png",
//            "./resources/test_detection_with_projector/4.png",
//            "./resources/test_detection_with_projector/5.png",
//            "./resources/test_detection_with_projector/6.png",
//            "./resources/test_detection_with_projector/7.png",
//            "./resources/test_detection_with_projector/8.png",
//            "./resources/test_detection_with_projector/9.png",
//            "./resources/test_detection_with_projector/10.png",
//            "./resources/test_detection_with_projector/11.png",
//            "./resources/test_detection_with_projector/12.png",
//            "./resources/test_detection_with_projector/13.png",
//            "./resources/test_detection_with_projector/14.png",
//            "./resources/test_detection_with_projector/15.png",
//            "./resources/test_detection_with_projector/16.png",
//            "./resources/test_detection_with_projector/17.png",
//            "./resources/test_detection_with_projector/18.png",
//            "./resources/test_detection_with_projector/19.png",
//            "./resources/test_detection_with_projector/20.png",
//            "./resources/test_detection_with_projector/21.png",
//            "./resources/test_detection_with_projector/22.png",
//            "./resources/test_detection_with_projector/23.png",
//            "./resources/test_detection_with_projector/24.png",
            "./resources/test_detection/with_projector_on/with_halo_and_text/1.png",
            "./resources/test_detection/with_projector_on/with_halo_and_text/2.png",
            "./resources/test_detection/with_projector_on/with_halo_and_text/3.png",
            "./resources/test_detection/with_projector_on/with_halo_and_text/4.png",
            "./resources/test_detection/with_projector_on/with_halo_and_text/5.png",
            "./resources/test_detection/with_projector_on/with_halo_and_text/6.png",
            "./resources/test_detection/with_projector_on/with_halo_and_text/7.png",
            "./resources/test_detection/with_projector_on/with_halo_and_text/8.png",
            "./resources/test_detection/with_projector_on/with_halo_and_text/9.png",
            "./resources/test_detection/with_projector_on/with_halo_and_text/10.png",
            "./resources/test_detection/with_projector_on/with_halo_and_text/11.png",
            "./resources/test_detection/with_projector_on/with_halo_and_text/12.png",
            "./resources/test_detection/with_projector_on/with_halo_and_text/13.png",
            "./resources/test_detection/with_projector_on/with_halo_and_text/14.png",
            "./resources/test_detection/with_projector_on/with_halo_and_text/15.png",
            "./resources/test_detection/with_projector_on/with_halo_and_text/16.png",
            "./resources/test_detection/with_projector_on/with_halo_and_text/17.png",
            "./resources/test_detection/with_projector_on/with_halo_and_text/18.png",
            "./resources/test_detection/with_projector_on/with_halo_and_text/19.png",
            "./resources/test_detection/with_projector_on/with_halo_and_text/20.png",
            "./resources/test_detection/with_projector_on/with_halo_and_text/21.png",
            "./resources/test_detection/with_projector_on/with_halo_and_text/22.png",
            "./resources/test_detection/with_projector_on/with_halo_and_text/23.png",
    };

    cv::Size imageSize = getImageSize();
    billiard::detection::Table table = getTable();
    billiard::detection::ArucoMarkers markers = getMarkers();
    billiard::detection::CameraIntrinsics intrinsics = getIntrinsics_realsense_hd();

    std::shared_ptr<billiard::detection::DetectionConfig> detectionConfig;

    for (int i = 0; i < imagePaths.size(); i++) {
        std::string imagePath = imagePaths[i];
        cv::Mat frame = imread(imagePath, cv::IMREAD_COLOR);
        cv::resize(frame, frame, imageSize);

        detectionConfig = std::make_shared<billiard::detection::DetectionConfig>(billiard::detection::configure(frame, table, markers, intrinsics));
        if (!detectionConfig->valid) {
            std::cout << "Unable to configure detection" << std::endl;
            return;
        }

        if (!billiard::snooker::configure(*detectionConfig)) {
            std::cout << "Unable to configure snooker detection" << std::endl;
            return;
        }

        billiard::detection::State pixelState = billiard::snooker::detect(billiard::detection::State(), frame);
        billiard::detection::State state = billiard::detection::pixelToModelCoordinates(*detectionConfig, pixelState);

        cv::Mat detectedBallsImage = frame.clone();

        std::cout << "Image " << std::to_string(i+1) << " " << std::to_string(pixelState._balls.size()) << " balls detected" << std::endl;
        
        int ballIndex = 0;
        for (auto& ball : pixelState._balls) {

            cv::Point2d pixelPoint = cv::Point2d(ball._position.x, ball._position.y);

            int ballRadius = detectionConfig->ballRadiusInPixel;

            float paddingFactor = 0.5;
            float radius = (1.0f + paddingFactor) * ballRadius;

            const cv::Rect& roi = cv::Rect{
                    (int) (pixelPoint.x - radius),
                    (int) (pixelPoint.y - radius),
                    (int) (2 * radius),
                    (int) (2 * radius)
            };
            cv::Mat ballImage = frame(roi);

//            cv::rectangle(detectedBallsImage, roi, cv::Scalar{255, 0, 0}, 2);
            cv::putText(detectedBallsImage, std::to_string(ballIndex+1), pixelPoint, cv::FONT_HERSHEY_PLAIN, 1.0, cv::Scalar{255, 0, 0});

            cv::Mat ballImageEnlarged;
            float scale = 16.0;
            cv::resize(ballImage, ballImageEnlarged, cv::Size(), scale, scale, cv::INTER_CUBIC);

            {
                std::string filename = outputFolder + std::to_string(i+1) + std::string("_") + std::to_string(ballIndex+1) + std::string(".png");
                if (writeImages) cv::imwrite(filename, ballImage);
                if (!writeImages) cv::imshow(filename, ballImage);
            }

            ballIndex++;
        }
        {
            std::string filename = outputFolder + std::string("Balls_") + std::to_string(i+1) + std::string(".png");
            if (writeImages) cv::imwrite(filename, detectedBallsImage);
            if (!writeImages) cv::imshow(filename, detectedBallsImage);
        }

        if (!writeImages) cv::waitKey();
    }
}
