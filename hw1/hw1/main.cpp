#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class ImageLibrary 
{
private:
    Vec3b _colorMap[256];

    void CreateColorMap() {
        for (int i = 0; i < 256; i++)
            _colorMap[i] = Vec3b(rand() % 256, rand() % 256, rand() % 256);
    }

public:
    ImageLibrary() {
        this->CreateColorMap();
    }

    // 轉灰階
    Mat ConvertToGray(Mat colorImage) {
        Mat grayImage(colorImage.size(), CV_8UC1);

        for (int i = 0; i < colorImage.rows; i++)
        {
            for (int j = 0; j < colorImage.cols; j++)
            {
                Vec3b pixel = colorImage.at<Vec3b>(i, j);
                int grayValue = 0.3 * pixel[2] + 0.59 * pixel[1] + 0.11 * pixel[0];
                grayImage.at<uchar>(i, j) = grayValue;
            }
        }
        return grayImage;
    }

    // 灰階二值化
    Mat ConvertToBinary(Mat colorImage, uchar threshold = 128) {
        Mat grayImage = this->ConvertToGray(colorImage);
        Mat binaryImage(grayImage.size(), CV_8UC1);

        for (int i = 0; i < grayImage.rows; i++)
        {
            for (int j = 0; j < grayImage.cols; j++)
            {
                binaryImage.at<uchar>(i, j) = grayImage.at<uchar>(i, j) > threshold ? 255 : 0;
            }
        }
        return binaryImage;
    }

    // 轉 indexed image
    Mat ConvertToIndexedColor(Mat colorImage, Vec3b colorMap[256] = nullptr) {
        if (colorMap == nullptr)
            colorMap = this->_colorMap;

        Mat indexImage(colorImage.size(), CV_8UC1);
        Mat mappingImage(colorImage.size(), CV_8UC3);
        const int MAX_DIST = 255.0 * 255.0 * 3.0;
        for (int i = 0; i < colorImage.rows; i++) {
            for (int j = 0; j < colorImage.cols; j++) {
                Vec3b color = colorImage.at<Vec3b>(i, j);
                uchar index = 0;
                int minDist = MAX_DIST;
                for (int k = 0; k < 256; k++) {
                    Vec3b mapColor = colorMap[k];
                    int dist =
                        (color[0] - mapColor[0]) * (color[0] - mapColor[0]) +
                        (color[1] - mapColor[1]) * (color[1] - mapColor[1]) +
                        (color[2] - mapColor[2]) * (color[2] - mapColor[2]);
                    if (dist < minDist) {
                        minDist = dist;
                        index = k;
                    }
                }
                indexImage.at<uchar>(i, j) = index;
                mappingImage.at<Vec3b>(i, j) = colorMap[index];
            }
        }
        return mappingImage;
    }
};

int main() {
    const string IMAGE_NAMES[] = {
        "House256",
        "House512",
        "JellyBeans",
        "Lena",
        "Mandrill",
        "Peppers",
    };
    
    const string IMAGE_PATH_FORMAT = "..\\image\\%s.png";
    for (string name : IMAGE_NAMES)
    {
        const string IMAGE_PATH = format(IMAGE_PATH_FORMAT.c_str(), name.c_str());
        cout << IMAGE_PATH << '\n';

        ImageLibrary library = ImageLibrary();
        Mat colorImage = imread(IMAGE_PATH);
        Mat grayImage = library.ConvertToGray(colorImage);
        Mat binaryImage = library.ConvertToBinary(colorImage, 128);
        Mat indexColorImage = library.ConvertToIndexedColor(colorImage);

        imshow("true-color " + IMAGE_PATH, colorImage);
        imshow("gray " + IMAGE_PATH, grayImage);
        imshow("binary " + IMAGE_PATH, binaryImage);
        imshow("indexed " + IMAGE_PATH, indexColorImage);

        imwrite(format(IMAGE_PATH_FORMAT.c_str(), (name + "_gray").c_str()), grayImage);
        imwrite(format(IMAGE_PATH_FORMAT.c_str(), (name + "_binary").c_str()), binaryImage);
        imwrite(format(IMAGE_PATH_FORMAT.c_str(), (name + "_indexed").c_str()), indexColorImage);
        
        waitKey(0);
        destroyAllWindows();
    }
    return 0;
}