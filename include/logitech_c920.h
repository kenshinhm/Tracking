#include <opencv2/opencv.hpp>

class Webcam
{
private:
    cv::VideoCapture camera;
    int number = -1;
public:
    enum PROPERTY{OPEN=0, WIDTH=3, HEIGHT=4, BRIGHT=10, CONTRAST=11, COLOR_INT=12, GAIN=14,
                 EXPOSURE=15, WHITE_BALANCE=17, FOCUS=28};
    Webcam();
    ~Webcam();
    bool Initialize(int camera_number);
    bool Grab(cv::Mat& image);
    void GetProperty(PROPERTY property, double& value);
    void SetProperty(PROPERTY property, double value);
};
