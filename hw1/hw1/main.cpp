#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace cv;

// Âà¦Ç¶¥
Mat ConvertToGray(Mat original) {
    Mat grayImage(original.size(), CV_8UC1);

    for (int i = 0; i < original.rows; i++)
    {
        for (int j = 0; j < original.cols; j++)
        {
            Vec3b pixel = original.at<Vec3b>(i, j);
            int grayValue = 0.3 * pixel[2] + 0.59 * pixel[1] + 0.11 * pixel[0];
            grayImage.at<uchar>(i, j) = grayValue;
        }
    }
    return grayImage;
}

// ¦Ç¶¥¤G­È¤Æ
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
        Mat image = imread(path);
        Mat grayImage = ConvertToGray(image);
        Mat binaryImage = ConvertToBinary(grayImage, 128);
        imshow("Gray " + path, grayImage);
        imshow("Binary " + path, binaryImage);
        waitKey(0);
    }
    destroyAllWindows();
    return 0;
}