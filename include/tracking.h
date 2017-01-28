#include "math.h"
#include <opencv2/opencv.hpp>

using namespace std;

class STracking
{
private:
    int image_width;
    int image_height;
    int max_corners;
    cv::Rect tracking_roi;
    vector<cv::Scalar> colors;
public:
    void Initialize(int image_width, int image_height, int max_corners = 300);
    cv::Rect GetTrackingROI();
    void SetTrackingROI(int x, int y, int width, int height);
    void SetTrackingROI(cv::Rect roi);

};
