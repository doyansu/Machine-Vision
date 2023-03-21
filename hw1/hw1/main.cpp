#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace cv;

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
Mat ConvertToBinary(Mat grayImage, uchar threshold=128) {
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

// 轉 index-color image
Mat ConvertToIndexColor(Mat colorImage) {
    Mat colorMap(1, 256, CV_8UC3);
    for (int i = 0; i < 256; i++) 
    {
        colorMap.at<Vec3b>(0, i) = Vec3b(i, i, i);
    }

    Mat indexColorImage(colorImage.size(), CV_8UC1);
    for (int i = 0; i < colorImage.rows; i++) {
        for (int j = 0; j < colorImage.cols; j++) {
            Vec3b color = colorImage.at<Vec3b>(i, j);
            int index = (color[0] + color[1] + color[2]) / 3;
            indexColorImage.at<Vec3b>(i, j) = colorMap.at<Vec3b>(0, index);
        }
    }
    return indexColorImage;
}


int main() {
    const std::string PATHS[] = {
        "..\\image\\House256.png",
        "..\\image\\House512.png",
        "..\\image\\JellyBeans.png",
        "..\\image\\Lena.png",
        "..\\image\\Mandrill.png",
        "..\\image\\Peppers.png",
    };

    for (std::string path : PATHS)
    {
        Mat colorImage = imread(path);
        Mat grayImage = ConvertToGray(colorImage);
        Mat binaryImage = ConvertToBinary(grayImage, 128);
        Mat indexColorImage = ConvertToIndexColor(colorImage);

        imshow("true-color " + path, colorImage);
        imshow("gray " + path, grayImage);
        imshow("binary " + path, binaryImage);
        imshow("index-color " + path, indexColorImage);
        waitKey(0);
    }
    destroyAllWindows();
    return 0;
}