#pragma once
#include <opencv2/opencv.hpp>

using namespace cv;

namespace image_model {
    class ImageLibrary
    {
    public:
        ImageLibrary() {};

        // Âà¦Ç¶¥
        Mat ConvertToGray(Mat colorImage);
        // ¦Ç¶¥¤G­È¤Æ
        Mat ConvertToBinary(Mat colorImage, uchar threshold = 128U);
    };
}
