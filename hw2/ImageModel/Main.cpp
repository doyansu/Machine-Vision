#include <iostream>
#include <opencv2/opencv.hpp>
#include <algorithm>
#include "ImageLibrary.h"

using namespace image_model;

int main() {
    // image file names
    const string IMAGE_NAMES[] = {
        "1",
        "2",
        "3",
        "4",
    };

    // threshold ³]©w
    const int THRESHOLD_SETTRING[] = {
        128,
        210,
        80,
        210,
    };

    const string IMAGE_PATH_FORMAT = "..\\image\\%s.png";

    ImageLibrary library = ImageLibrary();

    for (int i = 0; i < 4; i++)
    {
        const string IMAGE_NAME = IMAGE_NAMES[i];
        const string IMAGE_PATH = format(IMAGE_PATH_FORMAT.c_str(), IMAGE_NAME.c_str());
        std::cout << IMAGE_PATH << '\n';

        // read image
        Mat colorImage = imread(IMAGE_PATH);
        Mat binaryImage = library.ConvertToBinary(colorImage, THRESHOLD_SETTRING[i]);
        cv::imshow("true-color " + IMAGE_PATH, colorImage);
        cv::imshow("binary " + IMAGE_PATH, binaryImage);

        const int SIZE_FILTER = 100;
        // 4-connected labeling 
        int objNumber = 0;
        Mat connectedFourImage = library.ConvertToLabeling(binaryImage, ImageLibrary::Connected::Four, &objNumber, SIZE_FILTER);
        cv::imshow("4-connected " + IMAGE_PATH, connectedFourImage);
        std::cout << "4-connected object number : " << objNumber << '\n';

        // 8-connected labeling 
        Mat connectedEightImage = library.ConvertToLabeling(binaryImage, ImageLibrary::Connected::Eight, &objNumber, SIZE_FILTER);
        cv::imshow("8-connected " + IMAGE_PATH, connectedEightImage);
        std::cout << "8-connected object number : " << objNumber << '\n';

        // save image
        imwrite(format(IMAGE_PATH_FORMAT.c_str(), (IMAGE_NAME + "_binary").c_str()), binaryImage);
        imwrite(format(IMAGE_PATH_FORMAT.c_str(), (IMAGE_NAME + "_labeling 4-connected").c_str()), connectedFourImage);
        imwrite(format(IMAGE_PATH_FORMAT.c_str(), (IMAGE_NAME + "_labeling 8-connected").c_str()), connectedEightImage);

        cv::waitKey(0);
        cv::destroyAllWindows();
    }

    return 0;
}