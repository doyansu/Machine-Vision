#include <iostream>
#include <filesystem>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

// Filter 類型
enum class Filter {
    Mean,
    Median,
    Gaussian,
};

class ImageLibrary
{
public:
    ImageLibrary() {
        
    }

    Mat FilterBy(const Mat& sourceImage, Filter filter = Filter::Gaussian, unsigned int times = 1, int mask = 3) {
        mask = (mask < 3) ? 3 : (mask|1); // 確保 mask 至少為3，且是奇數
        Mat resultImage = Mat(sourceImage.size(), CV_8UC3);
        while (times--)
        {
            for (int i = 0; i < resultImage.rows; i++)
                for (int j = 0; j < resultImage.cols; j++)
                    resultImage.at<Vec3b>(i, j) = sourceImage.at<Vec3b>(i, j);
        }
        return resultImage;
    }

private:

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

void ShowAndWrite(string name, Mat image) {
    imshow(name, image);
    imwrite(name + ".png", image);
}

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
        Mat sourceImage = imread(image.Path());
        imshow(image.FileName(), sourceImage);

        // Filter
        const int FILTER_TIMES = 7;
        for (int times = 1; times <= FILTER_TIMES; times++) {
            ShowAndWrite(image.FileName() + " Mean mask3 times" + to_string(times), library.FilterBy(sourceImage, Filter::Mean, times, 3));
            ShowAndWrite(image.FileName() + " Mean mask7 times" + to_string(times), library.FilterBy(sourceImage, Filter::Mean, times, 7));
            ShowAndWrite(image.FileName() + " Median mask3 times" + to_string(times), library.FilterBy(sourceImage, Filter::Median, times, 3));
            ShowAndWrite(image.FileName() + " Median mask7 times" + to_string(times), library.FilterBy(sourceImage, Filter::Median, times, 7));
            ShowAndWrite(image.FileName() + " Gaussian mask5 times" + to_string(times), library.FilterBy(sourceImage, Filter::Gaussian, times, 5));
        }

        cv::waitKey(0);
        cv::destroyAllWindows();
    }

    return 0;
}