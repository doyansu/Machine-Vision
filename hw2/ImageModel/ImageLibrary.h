#pragma once
#include <opencv2/opencv.hpp>

using namespace cv;

namespace image_model {
    class ImageLibrary
    {
    public:
        ImageLibrary() {};

        // �s�q�� 4-connected 8-connected
        enum class Connected
        {
            Four = 4,
            Eight = 8
        };

        // ��Ƕ�
        Mat ConvertToGray(Mat colorImage);
        // �G�Ȥ�
        Mat ConvertToBinary(Mat colorImage, uchar threshold = 128);
        // �� Labeling Image
        Mat ConvertToLabeling(Mat colorImage, Connected connected = Connected::Four, int* objNumber = nullptr);

    private:

    };
}
