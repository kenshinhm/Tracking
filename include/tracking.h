#include "math.h"
#include "string"
#include "sstream"
#include "iostream"
#include "vector"
#include <opencv2/opencv.hpp>

using namespace std;

class STracking
{
public:
    enum METHOD {AFFINE};
    enum CORNER {FAST, GRID, AUTOGRID};
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
    cv::Rect FlowTracking(cv::Mat& prev_gray, cv::Mat& next_gray, cv::Mat& display, CORNER corner, METHOD method,
                                     double auto_threshold, bool resize, bool debug);
private:
    cv::Rect GetROI(int x, int y, int width, int height);
    cv::Rect GetROI(cv::Rect roi);
    void SetPointsGlobal(vector<cv::Point2d>& points, cv::Rect roi);
    void SetPointsLocal(vector<cv::Point2d>& points, cv::Rect roi);
    void Resize(cv::Rect& rect, double scale);
    void Resize(vector<cv::Point2d>& points, double scale);
    void RectToPoints(cv::Rect rect, vector<cv::Point>& points);
    void RectToPoints(cv::Rect rect, vector<cv::Point2d>& points);
    void GetCorners(cv::Mat image, vector<cv::Point2d>& ret ,CORNER corner, int step);
    cv::Rect Rearrange(vector<cv::Point2d>& prev_matches, vector<cv::Point2d>& next_matches, cv::Rect prev_roi, int step, double threshold);

    cv::Rect Transform(vector<cv::Point2d>& prev_matches, vector<cv::Point2d>& next_matches, cv::Rect prev_roi, METHOD method);
    void OpticalFlowPyLK(cv::Mat prev, cv::Mat next, vector<cv::Point2d> prev_corners,
                         vector<cv::Point2d>& prev_matches, vector<cv::Point2d>& next_matches);
};
