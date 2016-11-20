#if 1

#include "opencv2/opencv.hpp"
#include "vector"
#include "math.h"

using namespace std;
using namespace cv;

#define ROITHRESH 20

cv::Rect roi;
int imageWidth; int imageHeight;
bool onDrag = false;
bool onTrack = false;

void onMouse(int event, int x, int y, int, void*);
void SetROI(int x, int y, int width, int height);

int main(int argc, char *argv[])
{
    cv::Mat prevImage; cv::Mat nextImage;
    cv::Mat prevGray;  cv::Mat nextGray;
    cv::Mat flow;

    cv::VideoCapture cap(0);
    if(!cap.isOpened())
        return -1;

    cv::namedWindow("Tracking");
    cv::setMouseCallback("Tracking", onMouse, 0);

    cap.read(prevImage);
    //Initialize
    imageWidth = prevImage.size().width;
    imageHeight = prevImage.size().height;
    cv::cvtColor(prevImage, prevGray, CV_BGR2GRAY);
    cv::GaussianBlur(prevGray, prevGray, cv::Size(3,3), 0);

    for(;;)
    {
        cap.read(nextImage);
        cv::cvtColor(nextImage, nextGray, CV_BGR2GRAY);
        cv::GaussianBlur(nextGray, nextGray, cv::Size(3,3), 0);
        if(onTrack)
        {
            cv::Mat roiImage = nextImage(roi).clone();
            cv::calcOpticalFlowFarneback(prevGray(roi), nextGray(roi), flow, 0.1, 1, 12, 2, 8, 1.2, 0);
            int total = 0; double dx = 0; double dy = 0;
            for (int y = 0; y < flow.rows; y += 4)
            {
                for (int x = 0; x < flow.cols; x += 4)
                {
                    // get the flow from y, x position * 2 for better visibility
                    const Point2f flowxy = flow.at<Point2f>(y, x);
                    // draw line at flow direction
                    cv::line(roiImage, Point(x, y), Point(cvRound(x + flowxy.x), cvRound(y + flowxy.y)), Scalar(0,0,255));
                    dy += flowxy.y; dx += flowxy.x; total++;
                }
                dy = cvRound(dy/total); dx = cvRound(dx/total);
                SetROI(roi.x+dx, roi.y+dy, roi.width, roi.height);
                cv::imshow("Flow", roiImage);
            }
            prevGray = nextGray.clone();
        }
        cv::rectangle(nextImage, roi, cv::Scalar(0,255,0));
        cv::imshow("Tracking", nextImage);
        if(waitKey(30) >= 0)
            break;
    }

    return 0;
}

void SetROI(int x, int y, int width, int height)
{
    roi.x = std::max(0, x);
    roi.y = std::max(0, y);
    roi.width = std::min(width, imageWidth - x);
    roi.height = std::min(height, imageHeight - y);
}

void onMouse(int event, int x, int y, int, void*)
{
    if(event == EVENT_LBUTTONDOWN)
    {
        onDrag = true;
        onTrack = false;
        SetROI(x,y,0,0);
        cv::destroyWindow("Flow");
    }
    else if(event == EVENT_MOUSEMOVE && onDrag)
    {
        SetROI(roi.x,roi.y,x-roi.x,y-roi.y);
    }
    else if(event == EVENT_LBUTTONUP && onDrag)
    {
        onDrag = false;
        SetROI(roi.x,roi.y,x-roi.x,y-roi.y);
        if(roi.width > ROITHRESH && roi.height > ROITHRESH)
            onTrack = true;
    }
}

#endif
