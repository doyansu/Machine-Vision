#include <iostream>
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

    const string IMAGE_PATH_FORMAT = "..\\image\\%s.png";

    ImageLibrary library = ImageLibrary();

    for (string name : IMAGE_NAMES)
    {
        const string IMAGE_PATH = format(IMAGE_PATH_FORMAT.c_str(), name.c_str());
        std::cout << IMAGE_PATH << '\n';

        Mat colorImage = imread(IMAGE_PATH);
        Mat binaryImage = library.ConvertToBinary(colorImage, 128);

        cv::imshow("true-color " + IMAGE_PATH, colorImage);
        cv::imshow("binary " + IMAGE_PATH, binaryImage);

        // 4-connected labeling 
        int objNumber = 0;
        Mat connectedFourImage = library.ConvertToLabeling(colorImage, ImageLibrary::Connected::Four, &objNumber);
        cv::imshow("c " + objNumber + IMAGE_PATH, connectedFourImage);
        std::cout << "4-connected object number : " << objNumber << '\n';

        // 8-connected labeling 
        Mat connectedEightImage = library.ConvertToLabeling(colorImage, ImageLibrary::Connected::Eight, &objNumber);
        cv::imshow("labeling 8-connected " + objNumber + IMAGE_PATH, connectedEightImage);
        std::cout << "8-connected object number : " << objNumber << '\n';

        imwrite(format(IMAGE_PATH_FORMAT.c_str(), (name + "_binary").c_str()), binaryImage);
        imwrite(format(IMAGE_PATH_FORMAT.c_str(), (name + "_labeling 4-connected").c_str()), connectedFourImage);
        imwrite(format(IMAGE_PATH_FORMAT.c_str(), (name + "_labeling 8-connected").c_str()), connectedEightImage);

        cv::waitKey(0);
        cv::destroyAllWindows();
    }

    return 0;
}