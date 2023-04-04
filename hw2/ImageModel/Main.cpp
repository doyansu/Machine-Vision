#include <iostream>
#include "ImageLibrary.h"

int main() {
    // image file names
    const string IMAGE_NAMES[] = {
        "1",
        "2",
        "3",
        "4",
    };

    const string IMAGE_PATH_FORMAT = "..\\image\\%s.png";

    image_model::ImageLibrary library = image_model::ImageLibrary();

    for (string name : IMAGE_NAMES)
    {
        const string IMAGE_PATH = format(IMAGE_PATH_FORMAT.c_str(), name.c_str());
        std::cout << IMAGE_PATH << '\n';

        Mat colorImage = imread(IMAGE_PATH);
        Mat grayImage = library.ConvertToGray(colorImage);
        Mat binaryImage = library.ConvertToBinary(colorImage, 128);

        cv::imshow("true-color " + IMAGE_PATH, colorImage);
        cv::imshow("gray " + IMAGE_PATH, grayImage);
        cv::imshow("binary " + IMAGE_PATH, binaryImage);

        // imwrite(format(IMAGE_PATH_FORMAT.c_str(), (name + "_gray").c_str()), grayImage);

        cv::waitKey(0);
        cv::destroyAllWindows();
    }

    return 0;
}