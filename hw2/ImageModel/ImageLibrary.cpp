#include "ImageLibrary.h"

namespace image_model {
    // 轉灰階
    Mat ImageLibrary::ConvertToGray(Mat colorImage) {
        Mat grayImage(colorImage.size(), CV_8UC1);

        for (int row = 0; row < grayImage.rows; row++) {
            for (int col = 0; col < grayImage.cols; col++) {
                Vec3b pixel = colorImage.at<Vec3b>(row, col);
                int grayValue = (int)(0.34 * pixel[2] + 0.33 * pixel[1] + 0.33 * pixel[0]);
                grayImage.at<uchar>(row, col) = grayValue;
            }
        }
        return grayImage;
    }

    // 二值化
    Mat ImageLibrary::ConvertToBinary(Mat colorImage, uchar threshold) {
        Mat grayImage = this->ConvertToGray(colorImage);
        Mat binaryImage(grayImage.size(), CV_8UC1);

        for (int row = 0; row < binaryImage.rows; row++)
            for (int col = 0; col < binaryImage.cols; col++)
                binaryImage.at<uchar>(row, col) = grayImage.at<uchar>(row, col) > threshold ? 255 : 0;
        return binaryImage;
    }

    // 轉 Labeling Image
    Mat ImageLibrary::ConvertToLabeling(Mat colorImage, Connected connected, int* objNumber){
        int count = 0;
        Mat binaryImage = this->ConvertToBinary(colorImage, 160);
        Mat labelingImage(binaryImage.size(), CV_8UC3);

        for (int row = 0; row < labelingImage.rows; row++)
            for (int col = 0; col < labelingImage.cols; col++) {
                int value = binaryImage.at<uchar>(row, col) ^ 255 ^ 0;
                labelingImage.at<Vec3b>(row, col) = Vec3b(value, value, value);
            }

        if (objNumber != nullptr)
            *objNumber = count;

        return labelingImage;
    }
}