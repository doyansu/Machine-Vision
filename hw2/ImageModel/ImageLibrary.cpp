#include "ImageLibrary.h"

namespace image_model {

    // Âà¦Ç¶¥
    Mat ImageLibrary::ConvertToGray(Mat colorImage) {
        Mat grayImage(colorImage.size(), CV_8UC1);

        for (int row = 0; row < colorImage.rows; row++) {
            for (int col = 0; col < colorImage.cols; col++) {
                Vec3b pixel = colorImage.at<Vec3b>(row, col);
                int grayValue = (int)(0.3 * pixel[2] + 0.59 * pixel[1] + 0.11 * pixel[0]);
                grayImage.at<uchar>(row, col) = grayValue;
            }
        }
        return grayImage;
    }

    // ¦Ç¶¥¤G­È¤Æ
    Mat ImageLibrary::ConvertToBinary(Mat colorImage, uchar threshold) {
        Mat grayImage = this->ConvertToGray(colorImage);
        Mat binaryImage(grayImage.size(), CV_8UC1);

        for (int row = 0; row < grayImage.rows; row++)
            for (int col = 0; col < grayImage.cols; col++)
                binaryImage.at<uchar>(row, col) = grayImage.at<uchar>(row, col) > threshold ? 255 : 0;
        return binaryImage;
    }
}