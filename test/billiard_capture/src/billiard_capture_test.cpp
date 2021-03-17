#include <gtest/gtest.h>
#include <billiard_capture/billiard_capture.hpp>

TEST(ImageCapture, show_images) {
    // Assumption: We assume here that the MAC-address of the camera is known.
    // If this is not the case, the MAC-address can be found by using the eBUS-Player found in the eBUS SDK.
    std::string mac = "00:11:1c:f5:a0:f4";

    billiard::capture::ImageCapture capture {};
    if (capture.open(mac)) {

        cv::Mat frame = capture.read();
        cv::imshow("Frame", frame);
        while (true) {

            int key = cv::waitKey(50);
            if (key != -1) std::cout << "key: " << key << "pressed" << std::endl;
            if (key == 27 /* ESC */) break;
//            if (key == 13 /* Enter */) {
                frame = capture.read();

                frame *= 7;
                std::cout << "Frame: " << frame.cols << " x " << frame.rows << " channels: " << frame.channels() << std::endl;

                cv::imshow("Frame", frame);
//            }
        }
    }
    capture.close();
}