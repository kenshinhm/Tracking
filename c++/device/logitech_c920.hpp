#include <opencv2/opencv.hpp>

class C920
{
private:
    cv::VideoCapture camera;
    int number;
public:
    enum PROPERTY{OPEN=0, WIDTH=3, HEIGHT=4, BRIGHT=10, CONTRAST=11, COLOR_INT=12, GAIN=14,
                 EXPOSURE=15, WHITE_BALANCE=17, FOCUS=28};
    C920();
    ~C920();
    bool Initialize(int camera_number, int width = 640, int height = 480);
    bool Grab(cv::Mat& image);
    double GetProperty(PROPERTY property);
    void SetProperty(PROPERTY property, double value);
};
