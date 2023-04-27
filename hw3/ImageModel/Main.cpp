#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class QuadtreeNode
{
private:
    Rect _rect;                     // 節點範圍
    int _level;                     // 節點層數
    bool _isLeaf;                   // 是葉節點
    Vec3b _color;                   // 節點顏色
    QuadtreeNode* _childrens[4];    // 子節點

    // 檢查顏色數量是否多於一種
    bool CheakColor(const Mat& image) {
        for (int i = _rect.x; i < _rect.x + _rect.width; i++) 
            for (int j = _rect.y; j < _rect.y + _rect.height; j++) 
                if (image.at<Vec3b>(i, j)[0] != image.at<Vec3b>(_rect.x, _rect.y)[0] ||
                    image.at<Vec3b>(i, j)[1] != image.at<Vec3b>(_rect.x, _rect.y)[1] ||
                    image.at<Vec3b>(i, j)[2] != image.at<Vec3b>(_rect.x, _rect.y)[2])
                    return true;
        return false;
    }

public:
    QuadtreeNode(Rect rect, int level) {
        this->_rect = rect;
        this->_level = level;
        this->_isLeaf = true;
        this->_color = Vec3b(128, 128, 128);
        for (int i = 0; i < 4; i++) {
            this->_childrens[i] = nullptr;
        }
    }

    ~QuadtreeNode() {
        for (int i = 0; i < 4; i++) 
            if (this->_childrens[i]) 
                delete this->_childrens[i];
    }
    
    // 分裂節點
    bool SplitNode(const Mat& image, int maxLevel = INT_MAX) {
        // 不是葉節點或達到最大上限不分裂
        if (_level > maxLevel || !_isLeaf)
            return false;

        bool continueSplit = false;

        // 多於一種顏色繼續分裂
        if (CheakColor(image)) {
            int half_width = _rect.width / 2;
            int half_height = _rect.height / 2;

            _childrens[0] = new QuadtreeNode(Rect(_rect.x, _rect.y, half_width, half_height), _level + 1);
            _childrens[1] = new QuadtreeNode(Rect(_rect.x + half_width, _rect.y, half_width, half_height), _level + 1);
            _childrens[2] = new QuadtreeNode(Rect(_rect.x, _rect.y + half_height, half_width, half_height), _level + 1);
            _childrens[3] = new QuadtreeNode(Rect(_rect.x + half_width, _rect.y + half_height, half_width, half_height), _level + 1);
            
            _isLeaf = false;

            for (int i = 0; i < 4; i++) 
                _childrens[i]->SplitNode(image, maxLevel);
            
            continueSplit = true;
        }
        else // 只有一種顏色
            _color = image.at<Vec3b>(_rect.x, _rect.y);
        
        return continueSplit;
    }

    // 將 node 繪製到 image
    void DrawNode(Mat& image) {
        if (this->_isLeaf) {
            for (int i = _rect.x; i < _rect.x + _rect.width; i++)
                for (int j = _rect.y; j < _rect.y + _rect.height; j++)
                    image.at<Vec3b>(i, j) = _color;
        }
        else {
            for (int i = 0; i < 4; i++)
                this->_childrens[i]->DrawNode(image);
        }
    }
};


class ImageLibrary
{
public:
    ImageLibrary() {
        
    }

    // 轉灰階
    Mat ConvertToGray(const Mat& colorImage) {
        Mat grayImage(colorImage.size(), CV_8UC3);

        for (int i = 0; i < colorImage.rows; i++) {
            for (int j = 0; j < colorImage.cols; j++) {
                Vec3b pixel = colorImage.at<Vec3b>(i, j);
                int grayValue = 0.3 * pixel[2] + 0.59 * pixel[1] + 0.11 * pixel[0];
                grayImage.at<uchar>(i, j) = grayValue;
            }
        }
        return grayImage;
    }

    // 灰階二值化
    Mat ConvertToBinary(const Mat& colorImage, uchar threshold = 128) {
        Mat grayImage = this->ConvertToGray(colorImage);
        Mat binaryImage(grayImage.size(), CV_8UC1);

        for (int i = 0; i < grayImage.rows; i++)
            for (int j = 0; j < grayImage.cols; j++)
                binaryImage.at<uchar>(i, j) = grayImage.at<uchar>(i, j) > threshold ? 255 : 0;
        return binaryImage;
    }

    // 依據 layer 使用 Quadtree 切分圖片
    Mat SplitImageByQuadtree(const Mat& srcImage, int layer = INT_MAX) {
        Mat splitImage = srcImage;
        Mat resultImage(srcImage.size(), CV_8UC3);

        // 如果是 binaryImage 將圖片轉為 3 通道
        if (srcImage.type() != CV_8UC3)
        {
            splitImage = Mat(srcImage.size(), CV_8UC3);
            for (int i = 0; i < srcImage.rows; i++)
                for (int j = 0; j < srcImage.cols; j++)
                    splitImage.at<Vec3b>(i, j) = Vec3b(srcImage.at<uchar>(i, j), srcImage.at<uchar>(i, j), srcImage.at<uchar>(i, j));
        }

        // 建立 Quadtree 並進行切分
        QuadtreeNode root = QuadtreeNode(Rect(0, 0, srcImage.cols, srcImage.rows), 0);
        root.SplitNode(splitImage, layer);
        root.DrawNode(resultImage);
        return resultImage;
    }
};

class ImageInfo 
{
public:
    string _fileName;
    string _extension;
    int _threshold;
    int _layer;

    ImageInfo(string fileName, int threshold, int layer) {
        this->_fileName = fileName;
        this->_threshold = threshold;
        this->_layer = layer;
        this->_extension = ".png";
    };
};

int main() {
    const int IMAGE_NUM = 4; // 圖片數
    const string IMAGE_FOLDER = "..\\image"; // 圖片存放資料夾
    const string IMAGE_PATH_FORMAT = IMAGE_FOLDER + "\\%s.png";
    vector<ImageInfo> images;

    // 圖片檔名、threshold、layer 設定
    images.push_back(ImageInfo("1", 135, 8));
    images.push_back(ImageInfo("2", 245, 9));
    images.push_back(ImageInfo("3", 155, 8));
    images.push_back(ImageInfo("4", 254, 9));

    ImageLibrary library = ImageLibrary();

    for (ImageInfo image : images)
    {
        const string IMAGE_NAME = image._fileName;
        const string IMAGE_PATH = format(IMAGE_PATH_FORMAT.c_str(), IMAGE_NAME.c_str());
        std::cout << IMAGE_PATH << '\n';

        // 讀取圖片、二值化
        Mat colorImage = imread(IMAGE_PATH);
        Mat binaryImage = library.ConvertToBinary(colorImage, image._threshold);

        imshow("true-color " + IMAGE_PATH, colorImage);
        imshow("binary " + IMAGE_PATH, binaryImage);
        
        // Quadtree 分裂圖片
        for (int layer = 1; layer <= image._layer; layer++)
        {
            Mat splittedImage = library.SplitImageByQuadtree(binaryImage, layer);
            imshow("splitted layer" + to_string(layer) + IMAGE_PATH, splittedImage);
            imwrite(format(IMAGE_PATH_FORMAT.c_str(), (IMAGE_NAME + "_splitted layer" + to_string(layer)).c_str()), splittedImage);
        }

        cv::waitKey(0);
        cv::destroyAllWindows();
    }

    return 0;
}