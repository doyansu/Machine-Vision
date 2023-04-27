#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class QuadtreeNode
{
private:
    Rect _rect;                     // �`�I�d��
    int _level;                     // �`�I�h��
    bool _isLeaf;                   // �O���`�I
    Vec3b _color;                   // �`�I�C��
    QuadtreeNode* _childrens[4];    // �l�`�I

    // �ˬd�C��ƶq�O�_�h��@��
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
    
    // �����`�I
    bool SplitNode(const Mat& image, int maxLevel = INT_MAX) {
        // ���O���`�I�ιF��̤j�W��������
        if (_level > maxLevel || !_isLeaf)
            return false;

        bool continueSplit = false;

        // �h��@���C���~�����
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
        else // �u���@���C��
            _color = image.at<Vec3b>(_rect.x, _rect.y);
        
        return continueSplit;
    }

    // �N node ø�s�� image
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

    // ��Ƕ�
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

    // �Ƕ��G�Ȥ�
    Mat ConvertToBinary(const Mat& colorImage, uchar threshold = 128) {
        Mat grayImage = this->ConvertToGray(colorImage);
        Mat binaryImage(grayImage.size(), CV_8UC1);

        for (int i = 0; i < grayImage.rows; i++)
            for (int j = 0; j < grayImage.cols; j++)
                binaryImage.at<uchar>(i, j) = grayImage.at<uchar>(i, j) > threshold ? 255 : 0;
        return binaryImage;
    }

    // �̾� layer �ϥ� Quadtree �����Ϥ�
    Mat SplitImageByQuadtree(const Mat& srcImage, int layer = INT_MAX) {
        Mat splitImage = srcImage;
        Mat resultImage(srcImage.size(), CV_8UC3);

        // �p�G�O binaryImage �N�Ϥ��ର 3 �q�D
        if (srcImage.type() != CV_8UC3)
        {
            splitImage = Mat(srcImage.size(), CV_8UC3);
            for (int i = 0; i < srcImage.rows; i++)
                for (int j = 0; j < srcImage.cols; j++)
                    splitImage.at<Vec3b>(i, j) = Vec3b(srcImage.at<uchar>(i, j), srcImage.at<uchar>(i, j), srcImage.at<uchar>(i, j));
        }

        // �إ� Quadtree �öi�����
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
    const int IMAGE_NUM = 4; // �Ϥ���
    const string IMAGE_FOLDER = "..\\image"; // �Ϥ��s���Ƨ�
    const string IMAGE_PATH_FORMAT = IMAGE_FOLDER + "\\%s.png";
    vector<ImageInfo> images;

    // �Ϥ��ɦW�Bthreshold�Blayer �]�w
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

        // Ū���Ϥ��B�G�Ȥ�
        Mat colorImage = imread(IMAGE_PATH);
        Mat binaryImage = library.ConvertToBinary(colorImage, image._threshold);

        imshow("true-color " + IMAGE_PATH, colorImage);
        imshow("binary " + IMAGE_PATH, binaryImage);
        
        // Quadtree �����Ϥ�
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