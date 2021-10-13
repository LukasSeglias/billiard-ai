#include <billiard_snooker/billiard_snooker.hpp>

#ifndef BILLIARD_SNOOKER_DEBUG_OUTPUT
    #ifdef NDEBUG
        #undef BILLIARD_SNOOKER_DEBUG_OUTPUT
    #endif
    #ifndef NDEBUG
        #define BILLIARD_SNOOKER_DEBUG_OUTPUT 1
    #endif
#endif
#ifndef BILLIARD_SNOOKER_CLASSIFICATION_DEBUG_OUTPUT
    #ifdef NDEBUG
        #undef BILLIARD_SNOOKER_CLASSIFICATION_DEBUG_OUTPUT
    #endif
    #ifndef NDEBUG
        #define BILLIARD_SNOOKER_CLASSIFICATION_DEBUG_OUTPUT 1
    #endif
#endif

// TODO: remove this
//#undef BILLIARD_SNOOKER_DEBUG_OUTPUT
#undef BILLIARD_SNOOKER_CLASSIFICATION_DEBUG_OUTPUT

namespace billiard::snooker {

    SnookerDetectionConfig config;
    SnookerClassificationConfig classificationConfig;

    bool configure(const billiard::detection::DetectionConfig& detectionConfig) {

        if (!detectionConfig.valid) {
            return false;
        }

        int radiusInPixel = detectionConfig.ballRadiusInPixel;
        int errorRadiusLow = ceil(radiusInPixel * (config.radiusErrorLow / 100.0));
        int errorRadiusHigh = ceil(radiusInPixel * (config.radiusErrorHigh / 100.0));
        config.minRadius = radiusInPixel - errorRadiusLow;
        config.maxRadius = radiusInPixel + errorRadiusHigh;
        config.houghMinDistance = config.minRadius * 2;

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
        std::cout << "radius in pixels: " << std::to_string(radiusInPixel) << " min radius: " << std::to_string(config.minRadius) << " max radius: " << std::to_string(config.maxRadius) << " error radius range: " << std::to_string(errorRadiusLow) << ", " << std::to_string(errorRadiusHigh) << std::endl;
#endif
        config.morphElementRect3x3 = getStructuringElement(cv::MORPH_RECT, cv::Size(3,3), cv::Point(1, 1));
        config.innerTableMask = detectionConfig.innerTableMask;

        config.valid = true;

        classificationConfig.roiRadius = detectionConfig.ballRadiusInPixel * classificationConfig.roiRadiusFactor;
        classificationConfig.valid = true;
        return true;
    }

    void hsvFromBgr(cv::Mat& bgr, cv::Mat& hue, cv::Mat& saturation, cv::Mat& value) {
        cv::Mat hsv;
        cv::cvtColor(bgr, hsv, cv::COLOR_BGR2HSV);
        std::vector<cv::Mat> channels;
        cv::split(hsv, channels);
        hue = channels[0];
        saturation = channels[1];
        value = channels[2];
    }

    inline std::vector<cv::Vec3f> filterCircles(const std::vector<cv::Vec3f>& circles, const cv::Rect& rect) {
        std::vector<cv::Vec3f> filteredCircles;
        for(auto& circle : circles) {
            auto center = cv::Point2i{(int)circle[0], (int)circle[1]};
            if (rect.contains(center)) {
                filteredCircles.push_back(circle);
            }
        }
        return filteredCircles;
    }

    inline std::vector<cv::Vec3f> filterCircles(const std::vector<cv::Vec3f>& circles, const cv::Mat& mask, bool expectedValue) {
        std::vector<cv::Vec3f> filteredCircles;
        for(auto& circle : circles) {
            auto center = cv::Point2i{(int)circle[0], (int)circle[1]};
            if ((bool)mask.at<char>(center.y, center.x) == expectedValue) {
                filteredCircles.push_back(circle);
            }
        }
        return filteredCircles;
    }

    inline std::vector<cv::Point2d> circleCenters(const std::vector<cv::Vec3f>& circles) {
        std::vector<cv::Point2d> imagePoints;
        for (auto& circle : circles) {
            imagePoints.emplace_back(circle[0], circle[1]);
        }
        return imagePoints;
    }


    void drawHoughResult(cv::Mat& image, std::vector<cv::Vec3f>& circles) {
        for(auto c : circles) {
            cv::Point center = cv::Point(c[0], c[1]);
            uint8_t radius = c[2];
            cv::Rect roi(center.x - radius, center.y - radius, radius * 2, radius * 2);
            if (roi.x >= 0 && roi.y >= 0 && roi.width <= image.cols && roi.height <= image.rows) {

                // circle center
                cv::circle(image, center, 1, cv::Scalar(0, 100, 100), 3, cv::LINE_AA);
                // circle outline
                cv::circle(image, center, radius, cv::Scalar(255, 0, 255), 1, cv::LINE_AA);
            }
        }
    }

    billiard::detection::State detect(const billiard::detection::State& previousState, const cv::Mat& original) {

        cv::Mat input;
        cv::resize(original, input, cv::Size(), config.scale, config.scale);

        // Apply blur
        cv::Mat blurred;
        cv::GaussianBlur(input, blurred, cv::Size(5, 5), 0, 0);

        cv::Mat grayscaleInput;
        cv::cvtColor(blurred, grayscaleInput, cv::COLOR_BGR2GRAY);

        // Convert image into HSV and retrieve separate channels
        cv::Mat hue, saturation, value;
        hsvFromBgr(blurred, hue, saturation, value);

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
        cv::imshow("input", input);
        cv::imshow("blurred", blurred);
        cv::imshow("hue", hue);
        cv::imshow("saturation", saturation);
        cv::imshow("value", value);
#endif

        // Saturated balls mask
        cv::Mat saturationMask;
        cv::inRange(saturation, config.saturationFilter.x, config.saturationFilter.y, saturationMask);

        cv::Mat openedSaturationMask;
        cv::morphologyEx(saturationMask, openedSaturationMask, cv::MORPH_OPEN, config.morphElementRect3x3, cv::Point(-1, -1),
                         1);

        cv::Mat closedSaturationMask;
        cv::morphologyEx(openedSaturationMask, closedSaturationMask, cv::MORPH_CLOSE, config.morphElementRect3x3,cv::Point(-1, -1), 2);

        cv::Mat saturatedBallMask = closedSaturationMask;

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
        cv::imshow("saturationMask", saturationMask);
        cv::imshow("openedSaturationMask", openedSaturationMask);
        cv::imshow("closedSaturationMask", closedSaturationMask);
        cv::imshow("saturatedBallMask", saturatedBallMask);
#endif

        cv::Mat saturationMaskedGrayscale;
        cv::bitwise_and(grayscaleInput, grayscaleInput, saturationMaskedGrayscale, saturatedBallMask);

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
        cv::Mat saturationMaskedInput;
        cv::bitwise_and(input, input, saturationMaskedInput, saturatedBallMask);

        cv::Mat invMask;
        cv::bitwise_not(saturatedBallMask, invMask);
        cv::Mat maskedDelta;
        cv::bitwise_and(input, input, maskedDelta, invMask);

        cv::imshow("saturationMask - masked color", saturationMaskedInput);
        cv::imshow("saturationMask - masked grayscale", saturationMaskedGrayscale);
        cv::imshow("saturationMask - masked delta", maskedDelta);
#endif

        // Hough on saturation masked input
        std::vector<cv::Vec3f> saturationCircles;
        HoughCircles(saturationMaskedGrayscale, saturationCircles, cv::HOUGH_GRADIENT, 1,
                     config.houghMinDistance, // minimal distance between centers
                     config.houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                     config.houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                     config.minRadius, config.maxRadius
        );

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
        cv::Mat edges;
        cv::Canny(saturationMaskedGrayscale, edges, config.houghCannyHigherThreshold / 2, config.houghCannyHigherThreshold);

        cv::imshow("saturationMask - maskedInput grayscale edges", edges);
#endif

        // Black ball mask
        cv::Mat blackMask;
        {
            cv::Mat valueMask;
            cv::inRange(value, config.blackValueFilter.x, config.blackValueFilter.y, valueMask);

            cv::Mat valueMaskAndedWithTableMask;
            cv::bitwise_and(valueMask, config.innerTableMask, valueMaskAndedWithTableMask);

            cv::Mat closedValueMask;
            cv::morphologyEx(valueMaskAndedWithTableMask, closedValueMask, cv::MORPH_CLOSE, config.morphElementRect3x3,cv::Point(-1, -1), 4);

            cv::Mat openedValueMask;
            cv::morphologyEx(closedValueMask, openedValueMask, cv::MORPH_OPEN, config.morphElementRect3x3, cv::Point(-1, -1),3);

            blackMask = openedValueMask;

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
            cv::imshow("black - valueMask", valueMask);
            cv::imshow("black - valueMask anded with innerTableMask", valueMaskAndedWithTableMask);
            cv::imshow("black - closedValueMask", closedValueMask);
            cv::imshow("black - openedValueMask", openedValueMask);
            cv::imshow("black - mask", blackMask);
#endif

        }

        cv::Mat blackMaskedGrayscale;
        cv::bitwise_and(grayscaleInput, grayscaleInput, blackMaskedGrayscale, blackMask);

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
        {
            cv::Mat blackMaskedInput;
            cv::bitwise_and(input, input, blackMaskedInput, blackMask);
            cv::Mat invMask;
            cv::bitwise_not(blackMask, invMask);
            cv::Mat maskedDelta;
            cv::bitwise_and(input, input, maskedDelta, invMask);

            cv::imshow("black - masked color", blackMaskedInput);
            cv::imshow("black - masked grayscale", blackMaskedGrayscale);
            cv::imshow("black - masked delta", maskedDelta);
        };
#endif

        // Hough on black masked input
        std::vector<cv::Vec3f> blackCircles;
        HoughCircles(blackMaskedGrayscale, blackCircles, cv::HOUGH_GRADIENT, 1,
                     config.houghMinDistance, // minimal distance between centers
                     config.houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                     config.houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                     config.minRadius, config.maxRadius
        );

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
        {
            cv::Mat edges;
            cv::Canny(blackMaskedGrayscale, edges, config.houghCannyHigherThreshold / 2, config.houghCannyHigherThreshold);

            cv::imshow("black - maskedInput grayscale edges", edges);
        };
#endif

        // White & Pink Mask
        cv::Mat whitePinkMask;
        {
            cv::Mat valueMask;
            cv::inRange(value, config.whitePinkValueFilter.x, config.whitePinkValueFilter.y, valueMask);

            cv::Mat openedMask;
            cv::morphologyEx(valueMask, openedMask, cv::MORPH_OPEN, config.morphElementRect3x3, cv::Point(-1, -1), 1);

            cv::Mat closedMask;
            cv::morphologyEx(openedMask, closedMask, cv::MORPH_CLOSE, config.morphElementRect3x3, cv::Point(-1, -1), 1);

            whitePinkMask = closedMask;

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
            cv::imshow("white&pink - valueMask", valueMask);
            cv::imshow("white&pink - openedMask", openedMask);
            cv::imshow("white&pink - closedMask", closedMask);
            cv::imshow("white&pink - mask", whitePinkMask);
#endif
        }

        cv::Mat whitePinkMaskedGrayscale;
        cv::bitwise_and(grayscaleInput, grayscaleInput, whitePinkMaskedGrayscale, whitePinkMask);

        cv::Mat closedWhitePinkMaskedGrayscale;
        cv::morphologyEx(whitePinkMaskedGrayscale, closedWhitePinkMaskedGrayscale, cv::MORPH_CLOSE, config.morphElementRect3x3, cv::Point(-1, -1), 2);

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
        {
            cv::Mat whitePinkMaskedInput;
            cv::bitwise_and(input, input, whitePinkMaskedInput, whitePinkMask);

            cv::Mat invMask;
            cv::bitwise_not(whitePinkMask, invMask);
            cv::Mat maskedDelta;
            cv::bitwise_and(input, input, maskedDelta, invMask);

            cv::imshow("white&pink - masked color", whitePinkMaskedInput);
            cv::imshow("white&pink - masked grayscale", whitePinkMaskedGrayscale);
            cv::imshow("white&pink - masked and closed grayscale", closedWhitePinkMaskedGrayscale);
            cv::imshow("white&pink - masked delta", maskedDelta);
        };
#endif

        // Hough on white&pink masked input
        std::vector<cv::Vec3f> whitePinkCircles;
        HoughCircles(closedWhitePinkMaskedGrayscale, whitePinkCircles, cv::HOUGH_GRADIENT, 1,
                     config.houghMinDistance, // minimal distance between centers
                     config.houghCannyHigherThreshold, // higher threshold of the two passed to the Canny edge detector (lower threshold is twice smaller)
                     config.houghAccumulatorThreshold, // accumulator threshold for the circle centers (if the value is smaller, the more "circles" are detected)
                     config.minRadius, config.maxRadius
        );

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
        {
            cv::Mat edges;
            cv::Canny(whitePinkMaskedGrayscale, edges, config.houghCannyHigherThreshold / 2, config.houghCannyHigherThreshold);
            cv::imshow("white&pink - maskedInput grayscale edges", edges);
        };
#endif

        std::vector<cv::Vec3f> allCircles;

        {
            std::vector<cv::Vec3f> circles = saturationCircles;
            std::vector<cv::Vec3f> filteredCircles1 = filterCircles(circles, config.innerTableMask, true);
            std::vector<cv::Vec3f> filteredCircles2 = filterCircles(filteredCircles1, saturatedBallMask, true);
            std::vector<cv::Vec3f> filteredCircles3 = filterCircles(filteredCircles2, blackMask, false);
            for (auto& circle : filteredCircles3) allCircles.push_back(circle);

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
            {
                cv::Mat houghOnMaskedInput = input.clone();
                cv::Mat houghOnMaskedInputAfterCircleFilter1 = input.clone();
                cv::Mat houghOnMaskedInputAfterCircleFilter2 = input.clone();
                cv::Mat houghOnMaskedInputAfterCircleFilter3 = input.clone();

                drawHoughResult(houghOnMaskedInput, circles);
                drawHoughResult(houghOnMaskedInputAfterCircleFilter1, filteredCircles1);
                drawHoughResult(houghOnMaskedInputAfterCircleFilter2, filteredCircles2);
                drawHoughResult(houghOnMaskedInputAfterCircleFilter3, filteredCircles3);

                cv::imshow("saturated - hough on masked input", houghOnMaskedInput);
                cv::imshow("saturated - hough on masked input after circle filter 1", houghOnMaskedInputAfterCircleFilter1);
                cv::imshow("saturated - hough on masked input after circle filter 2", houghOnMaskedInputAfterCircleFilter2);
                cv::imshow("saturated - hough on masked input after circle filter 3", houghOnMaskedInputAfterCircleFilter3);
            };
#endif
        }

        {
            std::vector<cv::Vec3f> circles = blackCircles;
            std::vector<cv::Vec3f> filteredCircles1 = filterCircles(circles, config.innerTableMask, true);
            std::vector<cv::Vec3f> filteredCircles2 = filterCircles(filteredCircles1, blackMask, true);
            for (auto& circle : filteredCircles2) allCircles.push_back(circle);

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
            cv::Mat houghOnMaskedInput = input.clone();
            cv::Mat houghOnMaskedInputAfterCircleFilter1 = input.clone();
            cv::Mat houghOnMaskedInputAfterCircleFilter2 = input.clone();

            drawHoughResult(houghOnMaskedInput, circles);
            drawHoughResult(houghOnMaskedInputAfterCircleFilter1, filteredCircles1);
            drawHoughResult(houghOnMaskedInputAfterCircleFilter2, filteredCircles2);

            cv::imshow("black - hough on masked input", houghOnMaskedInput);
            cv::imshow("black - hough on masked input after circle filter 1", houghOnMaskedInputAfterCircleFilter1);
            cv::imshow("black - hough on masked input after circle filter 2", houghOnMaskedInputAfterCircleFilter2);
#endif
        }

        {
            std::vector<cv::Vec3f> circles = whitePinkCircles;
            std::vector<cv::Vec3f> filteredCircles1 = filterCircles(circles, config.innerTableMask, true);
            std::vector<cv::Vec3f> filteredCircles2 = filterCircles(filteredCircles1, whitePinkMask, true);

            // Filter circles by saturated ball mask in order to remove 'duplicate' balls
            std::vector<cv::Vec3f> filteredCircles3 = filterCircles(filteredCircles2, saturatedBallMask, false);

            for (auto& circle : filteredCircles3) allCircles.push_back(circle);

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
            cv::Mat houghOnMaskedInput = input.clone();
            cv::Mat houghOnMaskedInputAfterCircleFilter1 = input.clone();
            cv::Mat houghOnMaskedInputAfterCircleFilter2 = input.clone();
            cv::Mat houghOnMaskedInputAfterCircleFilter3 = input.clone();

            drawHoughResult(houghOnMaskedInput, whitePinkCircles);
            drawHoughResult(houghOnMaskedInputAfterCircleFilter1, filteredCircles1);
            drawHoughResult(houghOnMaskedInputAfterCircleFilter2, filteredCircles2);
            drawHoughResult(houghOnMaskedInputAfterCircleFilter3, filteredCircles3);

            cv::imshow("white&pink - hough on masked input", houghOnMaskedInput);
            cv::imshow("white&pink - hough on masked input after circle filter 1", houghOnMaskedInputAfterCircleFilter1);
            cv::imshow("white&pink - hough on masked input after circle filter 2", houghOnMaskedInputAfterCircleFilter2);
            cv::imshow("white&pink - hough on masked input after circle filter 3", houghOnMaskedInputAfterCircleFilter3);
#endif
        }

#ifdef BILLIARD_SNOOKER_DEBUG_OUTPUT
        cv::Mat hough = input.clone();
        drawHoughResult(hough, allCircles);
        cv::imshow("hough", hough);
        cv::waitKey(1);
#endif

        std::vector<cv::Point2d> imagePoints = circleCenters(allCircles);

        billiard::detection::State state;
        for (int i = 0; i < imagePoints.size(); i++) {
            auto& point = imagePoints[i];
            billiard::detection::Ball ball;
            ball._type = "UNKNOWN";
            ball._id = std::to_string(i);
            ball._position = glm::vec2 {point.x, point.y};
            state._balls.push_back(ball);
        }

        return state;
    }

    /**
     * Based on https://docs.opencv.org/2.4/doc/tutorials/imgproc/histograms/histogram_calculation/histogram_calculation.html#code
     */
    std::vector<Histogram> histogram(const cv::Mat& input) {
        std::vector<cv::Mat> planes;
        cv::split(input, planes);

        int histSize = 256;
        float range[] = { 0, 256 } ;
        const float* histRange = { range };
        bool uniform = true;
        bool accumulate = false;

        std::vector<Histogram> histograms;
        for (const auto& plane : planes) {
            cv::Mat histogramPlane;
            cv::calcHist(&plane, 1, nullptr, cv::Mat(), histogramPlane, 1, &histSize, &histRange, uniform, accumulate);

            double min, max;
            cv::Point minLocation, maxLocation;
            cv::minMaxLoc(histogramPlane, &min, &max, &minLocation, &maxLocation);

            histograms.emplace_back(Histogram{histogramPlane, maxLocation});
        }

        return histograms;
    }


    /**
     * Based on https://docs.opencv.org/2.4/doc/tutorials/imgproc/histograms/histogram_calculation/histogram_calculation.html#code
     */
    void showHistograms(const std::vector<Histogram>& histograms) {
        int bins = 256;
        int hist_w = 512;
        int hist_h = 400;
        int bin_w = cvRound( (double) hist_w/bins );

        cv::Mat histImage( hist_h, hist_w, CV_8UC3, cv::Scalar( 0,0,0) );

        for (int i = 0; i < histograms.size(); i++) {
            auto histogram = histograms[i].histogram;
            cv::normalize(histogram, histogram, 0, histImage.rows, cv::NORM_MINMAX, -1, cv::Mat());

            int blue = i == 0 || i > 2 ? 255 : 0;
            int green = i == 1 || i > 2 ? 255 : 0;
            int red = i == 2 || i > 2 ? 255 : 0;

            for (int bin = 1; bin < bins; bin++) {
                cv::line(histImage, cv::Point(bin_w * (bin - 1), hist_h - cvRound(histogram.at<float>(bin - 1))),
                         cv::Point(bin_w * (bin), hist_h - cvRound(histogram.at<float>(bin))),
                         cv::Scalar(blue, green, red), 2, 8, 0);
            }
        }

        std::cout << "Max Hue" << ": " << std::setfill(' ') << std::setw(3) << histograms[0].maxLocation.y << " "
                    << "Max Saturation" << ": " << std::setfill(' ') << std::setw(3) << histograms[1].maxLocation.y << " "
                    << "Max Value" << ": " << std::setfill(' ') << std::setw(3) << histograms[2].maxLocation.y
                    << std::endl;

        cv::imshow("Histogram", histImage);
    }

    void classify(const billiard::detection::State& previousState,
                  billiard::detection::State& currentState,
                  const cv::Mat& image) {

        double radius = classificationConfig.roiRadius;

        for (auto& ball : currentState._balls) {

            const glm::vec2& pixelPosition = ball._position;
            const cv::Rect& roi = cv::Rect{
                    (int) (pixelPosition.x - radius),
                    (int) (pixelPosition.y - radius),
                    (int) (2 * radius),
                    (int) (2 * radius)
            };
            cv::Mat ballBgr = image(roi);

            classify(ball, ballBgr);
        }

    }

    void classify(billiard::detection::Ball& ball, const cv::Mat& original) {

        cv::Mat blurred;
        cv::GaussianBlur(original, blurred, classificationConfig.blurSize, 0, 0);

        cv::Mat hsv;
        cv::cvtColor(blurred, hsv, cv::COLOR_BGR2HSV);

        auto histogramByChannels = histogram(hsv);

        Histogram hueHist = histogramByChannels[0];
        Histogram saturationHist = histogramByChannels[1];
        Histogram valueHist = histogramByChannels[2];
        int maxHue = hueHist.maxLocation.y;
        int maxSaturation = saturationHist.maxLocation.y;
        int maxValue = valueHist.maxLocation.y;

        std::string label = "UNKNOWN";

        const auto between = [](int value, int min, int max)  {
            return value >= min && value <= max;
        };

        cv::Vec2f redPinkSeparatorLine = cv::Point2f { 1.0, 1.0 }; // Separate by (saturation, value)

        if (between(maxHue, 10, 35) && between(maxSaturation, 230, 255) && between(maxValue, 245, 255)) {
            label = "YELLOW";
        }
        else if (between(maxHue, 10, 40) && between(maxSaturation, 0, 80) && between(maxValue, 245, 255)) {
            label = "WHITE";
        }
        else if (between(maxValue, 0, 60)) {
            label = "BLACK";
        }
        else if (between(maxHue, 100, 120)) {
            label = "BLUE";
        }
        else if (between(maxHue, 80, 99)) {
            label = "GREEN";
        }
        else if(between(maxHue, 0, 10) || between(maxHue, 170, 180)) {
            // BROWN or RED or PINK

            if (between(maxHue, 0, 10) && between(maxSaturation, 200, 240) && between(maxValue, 150, 255)) {
                label = "BROWN";
            }
            else {
                cv::Vec2f point { (float) maxSaturation / 255, (float) maxValue / 255 }; // Separate by (saturation, value)

                float perpProduct = redPinkSeparatorLine[0] * point[1] - redPinkSeparatorLine[1] * point[0];
                std::cout << "redPinkSeparatorLine: " << redPinkSeparatorLine <<  " point: " << point << " perp product: " << perpProduct << std::endl;
                if (perpProduct <= 0) {
                    label = "RED";
                } else {
                    label = "PINK";
                }
            }
        }
//        else if (between(maxHue, 0, 10) && between(maxSaturation, 230, 255) && between(maxValue, 150, 255)) {
//            label = "RED";
//        }
        else if (between(maxHue, 0, 10) && between(maxSaturation, 200, 255) && between(maxValue, 150, 255)) {
            label = "BROWN";
        }
//        else if (between(maxHue, 0, 10) && between(maxSaturation, 60, 255) && between(maxValue, 230, 255)) {
//            label = "PINK";
//        }
//        else if (between(maxHue, 170, 180) && between(maxSaturation, 60, 255) && between(maxValue, 230, 255)) {
//            label = "PINK";
//        }
//        else if (between(maxHue, 0, 5)) {
//            label = "RED";
//        }
//        else if (between(maxHue, 170, 180)) {
//            label = "RED";
//        }

        ball._type = label;

#ifdef BILLIARD_SNOOKER_CLASSIFICATION_DEBUG_OUTPUT
        std::cout << "Ball " << ball._id << " at (" << ball._position.x << ", " << ball._position.y << ")" << " classified as " << label << std::endl;

        cv::Mat rgbDebug {original.rows + 100, 3*original.cols + 100, CV_8UC3, cv::Scalar{120, 120, 120}};
        original.copyTo(rgbDebug(cv::Rect{cv::Point(0, 0), cv::Size{original.cols, original.rows}}));
        blurred.copyTo(rgbDebug(cv::Rect {cv::Point(original.cols, 0), cv::Size {blurred.cols, blurred.rows}}));
        cv::imshow("original, blurred", rgbDebug);

        cv::Mat hue, saturation, value;
        {
            std::vector<cv::Mat> channels;
            cv::split(hsv, channels);
            hue = channels[0];
            saturation = channels[1];
            value = channels[2];
        }

        cv::Mat hsvDebug {hue.rows + 100, 3*hue.cols + 100, CV_8UC1, cv::Scalar{120, 120, 120}};
        hue.copyTo(hsvDebug(cv::Rect{cv::Point(0, 0), cv::Size{hue.cols, hue.rows}}));
        saturation.copyTo(hsvDebug(cv::Rect {cv::Point(hsv.cols, 0), cv::Size {saturation.cols, saturation.rows}}));
        value.copyTo(hsvDebug(cv::Rect {cv::Point(2*hsv.cols, 0), cv::Size {value.cols, value.rows}}));
        cv::imshow("hsv", hsvDebug);

        showHistograms(histogramByChannels);

//        cv::waitKey();
#endif

    }

    billiard::search::Search nextSearch(const billiard::search::Search& previousSearch,
                                        const std::vector<std::string>& previousTypes) {
        return previousSearch; // TODO: Switch between search types -> From RED to color and vice versa
    }

    billiard::search::node::Layer stateAfterBreak(const billiard::search::node::Layer& layer) {
        return layer; // TODO: Replace colorized balls at their positions. (Pass config and append configuration)
    }

    bool validEndState(const std::vector<std::string>& expectedTypes, const billiard::search::node::Layer& layer) {
        for(auto& node : layer._nodes) {
            if (node.second._ballType == "WHITE" && node.second._type == billiard::search::node::NodeType::BALL_POTTING) {
                return false;
            }
        }
        return true; // TODO: Implement check if potted ball(s) is/are of the expected type
    }
}