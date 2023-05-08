#include <iostream>
#include <filesystem> // ISO C++17 標準 (/std:c++17)
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
namespace fs = std::filesystem;

class Filter 
{
public:
    Filter() {}

    void SetMask(int mask) {
        this->_mask = (mask < 3) ? 3 : (mask | 1); // 確保 mask 至少為3，且是奇數
    }

    virtual Mat FilterImage(const Mat& sourceImage) = 0;

private:
    int _mask = 3;
};

class MeanFilter : public Filter
{
public:
    MeanFilter() {

    }

    Mat FilterImage(const Mat& sourceImage) override {

    }
};

class MedianFilter : public Filter
{
public:
    MedianFilter() {

    }

    Mat FilterImage(const Mat& sourceImage) override {

    }
};

class GaussianFilter : public Filter
{
public:
    GaussianFilter() {

    }

    Mat FilterImage(const Mat& sourceImage) override {

    }
};

class ImageLibrary
{
public:
    // Filter 類型
    enum class FilterType {
        Mean,
        Median,
        Gaussian,
    };

    ImageLibrary() {
        
    }

    // Filter
    Mat FilterBy(const Mat& sourceImage, FilterType filterType = FilterType::Gaussian, unsigned int times = 1, int mask = 3) {
        
        Mat resultImage = Mat(sourceImage.size(), CV_8UC3);
        Filter *filter = this->CreateFilter(filterType);
        filter->SetMask(mask);
        while (times--)
            resultImage = filter->FilterImage(resultImage);
        delete filter;
        return resultImage;
    }

private:
    Filter* CreateFilter(FilterType filterType) {
        Filter* filter = nullptr;
        switch (filterType)
        {
        case ImageLibrary::FilterType::Mean:
            filter = new MeanFilter();
            break;
        case ImageLibrary::FilterType::Median:
            filter = new MedianFilter();
            break;
        case ImageLibrary::FilterType::Gaussian:
            filter = new GaussianFilter();
            break;
        default:
            throw "error";
            break;
        }
        return filter;
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

    string Extension() {
        return this->_path.extension().string();
    }

private:
    fs::path _path;
};

int main() {
    const string IMAGE_FOLDER = "..\\image\\"; // 圖片存放資料夾
    vector<ImageInfo> images;

    // 圖片檔名設定
    images.push_back(ImageInfo(IMAGE_FOLDER + "House256_noise.png"));
    images.push_back(ImageInfo(IMAGE_FOLDER + "Lena_gray.png"));
    images.push_back(ImageInfo(IMAGE_FOLDER + "Mandrill_gray.png"));
    images.push_back(ImageInfo(IMAGE_FOLDER + "Peppers_noise.png"));

    ImageLibrary library = ImageLibrary();

    for (ImageInfo imageInfo : images)
    {
        std::cout << imageInfo.Path() << '\n';

        // 讀取圖片
        Mat sourceImage = imread(imageInfo.Path());
        imshow(imageInfo.FileName(), sourceImage);

        // 顯示並儲存圖片
        auto showWriteImage = [extension = imageInfo.Extension()](string name, Mat image) {
            imshow(name, image);
            imwrite(name + extension, image);
        };

        // Filter
        const int FILTER_TIMES = 7;
        for (int times = 1; times <= FILTER_TIMES; times++) {
            showWriteImage(imageInfo.FileName() + " Mean mask3 times" + to_string(times), library.FilterBy(sourceImage, ImageLibrary::FilterType::Mean, times, 3));
            showWriteImage(imageInfo.FileName() + " Mean mask7 times" + to_string(times), library.FilterBy(sourceImage, ImageLibrary::FilterType::Mean, times, 7));
            showWriteImage(imageInfo.FileName() + " Median mask3 times" + to_string(times), library.FilterBy(sourceImage, ImageLibrary::FilterType::Median, times, 3));
            showWriteImage(imageInfo.FileName() + " Median mask7 times" + to_string(times), library.FilterBy(sourceImage, ImageLibrary::FilterType::Median, times, 7));
            showWriteImage(imageInfo.FileName() + " Gaussian mask5 times" + to_string(times), library.FilterBy(sourceImage, ImageLibrary::FilterType::Gaussian, times, 5));
        }

        cv::waitKey(0);
        cv::destroyAllWindows();
    }

    return 0;
}