#include <iostream>
#include <filesystem> // ISO C++17 標準 (/std:c++17)
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

class ImageLibrary
{
public:
    ImageLibrary() {
        
    }

private:
    
};

// 存圖片路徑資訊
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

    string Extension() {
        return this->_path.extension().string();
    }

private:
    fs::path _path;
};

int main() {
    const string IMAGE_FOLDER = "..\\image\\"; // 圖片存放資料夾
    const string IMAGE_OUTPUT_FOLDER = "..\\image\\result"; // 圖片結果存放資料夾

    if (!fs::exists(IMAGE_FOLDER))
    {

        return 1;
    }

    if (!fs::exists(IMAGE_OUTPUT_FOLDER))
        std::cout << (fs::create_directory(IMAGE_OUTPUT_FOLDER) ? "Output folder :" + IMAGE_OUTPUT_FOLDER + " created successfully." : "Failed to create Output folder.") << std::endl;

    vector<ImageInfo> images;
    // 圖片檔名設定
    images.push_back(ImageInfo(IMAGE_FOLDER + "House512.png"));
    images.push_back(ImageInfo(IMAGE_FOLDER + "Lena.png"));
    images.push_back(ImageInfo(IMAGE_FOLDER + "Mandrill.png"));

    ImageLibrary library = ImageLibrary();

    for (ImageInfo imageInfo : images)
    {
        std::cout << imageInfo.Path() << '\n';

        // 讀取圖片
        Mat sourceImage = imread(imageInfo.Path());
        imshow(imageInfo.FileName(), sourceImage);

        cv::waitKey(0);
        cv::destroyAllWindows();
    }

    return 0;
}