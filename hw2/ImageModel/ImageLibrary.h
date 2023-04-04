#pragma once
#include <opencv2/opencv.hpp>

using namespace cv;

namespace image_model {
    class ImageLibrary
    {
    public:
        ImageLibrary() {};

        // ��Ƕ�
        Mat ConvertToGray(Mat colorImage);
        // �Ƕ��G�Ȥ�
        Mat ConvertToBinary(Mat colorImage, uchar threshold = 128U);
    };
}
