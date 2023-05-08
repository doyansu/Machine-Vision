#include <iostream>
#include <filesystem>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

class ImageLibrary
{
public:
    ImageLibrary() {
        
    }
};

class ImageInfo 
{
public:
    ImageInfo(string path) {
        this->_path = fs::path(path);
    };

    string Path() {
        return this->_path.relative_path().string();
    }

    string FileName() {
        return this->_path.stem().string();
    }

private:
    fs::path _path;
};

int main() {
    const int IMAGE_NUM = 4; // 圖片數
    const string IMAGE_FOLDER = "..\\image\\"; // 圖片存放資料夾
    vector<ImageInfo> images;

    // 圖片檔名、threshold、layer 設定
    images.push_back(ImageInfo(IMAGE_FOLDER + "House256_noise.png"));
    images.push_back(ImageInfo(IMAGE_FOLDER + "Lena_gray.png"));
    images.push_back(ImageInfo(IMAGE_FOLDER + "Mandrill_gray.png"));
    images.push_back(ImageInfo(IMAGE_FOLDER + "Peppers_noise.png"));

    ImageLibrary library = ImageLibrary();

    for (ImageInfo image : images)
    {
        std::cout << image.Path() << '\n';

        // 讀取圖片
        Mat colorImage = imread(image.Path());

        imshow(image.FileName(), colorImage);

        cv::waitKey(0);
        cv::destroyAllWindows();
    }

    return 0;
}