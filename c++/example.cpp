#if 1

#include "opencv2/opencv.hpp"
#include "vector"
#include "module/tracking.hpp"
#include "device/logitech_c920.hpp"
#include "string"
#include "sstream"
#include "iostream"
#include "sys/time.h"

using namespace std;

#define ROITHRESH 20

enum MODE {VIDEO, IMAGE, STREAM};

bool on_track = false;
bool on_drag = false;
bool on_debug = false;
struct timeval start_time, end_time;
int max_scene;
cv::Mat image, next_gray, prev_gray, display;
STracking tracking;
C920 camera;
MODE mode;
int max_number;
string name;
string path;
string type;
ostringstream ss;

void MouseEvent(int event, int x, int y, int flags, void* param);
void SelectMode(bool debug);
void RunStream();
void RunImage();
void RunVideo();
void DestroyWindows();

int main(int argc, char* argv[])
{
    if(argc >= 1)
    {
        cout << "Debug Mode" << endl;
        on_debug = true;
    }

    SelectMode(on_debug);

    if(mode == STREAM)
        RunStream();
    else if(mode == IMAGE)
        RunImage();
    else if(mode == VIDEO)
        RunVideo();

    return 0;
}

void DestroyWindows()
{
    cv::destroyWindow("Debug");
    return;
}

void MouseEvent(int event, int x, int y, int flags, void*)
{
    if(event == cv::EVENT_LBUTTONDOWN)
    {
        on_drag = true;
        on_track = false;
        DestroyWindows();
        tracking.SetTrackingROI(x,y,0,0);
    }
    else if(event == cv::EVENT_MOUSEMOVE && on_drag)
    {
        cv::Rect roi = tracking.GetTrackingROI();
        tracking.SetTrackingROI(roi.x, roi.y, x-roi.x, y-roi.y);
    }
    else if(event == cv::EVENT_LBUTTONUP && on_drag)
    {
        on_drag = false;
        cv::Rect roi = tracking.GetTrackingROI();
        tracking.SetTrackingROI(roi.x, roi.y, x-roi.x, y-roi.y);
        if(tracking.GetTrackingROI().width > ROITHRESH && tracking.GetTrackingROI().height > ROITHRESH)
            on_track = true;
    }
}

void SelectMode(bool debug)
{
    int input;

    if(debug)
    {
        mode = STREAM;
        path = "../video";
        name = "basketball.avi";
        //type = "jpg"
        //max_scene = 100;
    }
    else
    {
        cout << "Select Mode" << endl;
        cout << "1. Live stream" << endl;
        cout << "2. From Image" << endl;
        cout << "3. From Video" << endl;
        cin >> input;

        if(!(input == 1 || input == 2 || input == 3))
        {
            cout << "Input Error" << endl;
            return;
        }
        if(input == 1)
            mode = STREAM;
        else if(input == 2)
            mode = IMAGE;
        else if(input == 3)
            mode = VIDEO;

        if(mode == IMAGE)
        {
            cout << "Image is not supported with out Debug Mode" << endl;
        }
        else if(mode == VIDEO)
        {
            cout << "Path for video folder: ";
            cin >> path;
            cout << "Name for video file: ";
            cin >> name;
        }
    }

    return;
}

void RunStream()
{
    int scene = 0;

    camera.Initialize(0);
    tracking.Initialize(camera.GetProperty(C920::WIDTH), camera.GetProperty(C920::HEIGHT));
    cv::namedWindow("Tracking");
    cv::setMouseCallback("Tracking", MouseEvent);

    camera.Grab(image);
    cv::cvtColor(image, prev_gray, cv::COLOR_BGR2GRAY);

    for(;;)
    {
        scene++;
        gettimeofday(&start_time, NULL);

        camera.Grab(image);
        cv::cvtColor(image, next_gray, cv::COLOR_BGR2GRAY);

        try
        {
            if(on_track)
            {
                display = image.clone();
                cv::Rect next_roi = tracking.FlowTracking(prev_gray, next_gray, display, STracking::AUTOGRID,
                                                          STracking::AFFINE, 1.0, true, on_debug);
                tracking.SetTrackingROI(next_roi);
            }
        }
        catch(cv::Exception ex)
        {
            cout << ex.what() << endl;
        }

        prev_gray = next_gray.clone();
        gettimeofday(&end_time, NULL);
        double elapsed_time = (end_time.tv_sec - start_time.tv_sec)*1000.0 + (end_time.tv_usec - start_time.tv_usec)/1000.0 + 0.5;
        cout << "#" << scene << ": " << elapsed_time << "ms" << endl;

        ss.str("");
        ss << scene << ")" << (1000/elapsed_time) << "fps";
        cv::putText(image, ss.str(), cv::Point(10,20), CV_FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0,255,0), 1);
        cv::rectangle(image, tracking.GetTrackingROI(), cv::Scalar(0,255,0), 2);
        cv::imshow("Tracking", image);

        if(cv::waitKey(10) >= 0)
            break;
    }

    return;
}

void RunImage()
{
    int scene = 0;
    cv::namedWindow("Tracking");
    cv::setMouseCallback("Tracking", MouseEvent);

    ss.str("");
    ss << path << "/" << name << "_" << scene << "." << type;
    image = cv::imread(ss.str());
    cv::cvtColor(image, prev_gray, cv::COLOR_BGR2GRAY);
    tracking.Initialize(image.size().width, image.size().height);

    for(;;)
    {
        gettimeofday(&start_time, NULL);

        ss.str("");
        ss << path << "/" << name << "_" << scene << "." << type;
        image = cv::imread(ss.str());
        cv::cvtColor(image, next_gray, cv::COLOR_BGR2GRAY);

        if(on_track)
        {
            try
            {
                display = image.clone();
                cv::Rect next_roi = tracking.FlowTracking(prev_gray, next_gray, display, STracking::AUTOGRID,
                                                          STracking::AFFINE, 1.0, true, on_debug);
                tracking.SetTrackingROI(next_roi);
                prev_gray = next_gray.clone();
            }
            catch(cv::Exception ex)
            {
                cout << ex.what() << endl;
            }
            if(++scene > max_number)
            {
                on_track = false;
                scene = 0;
            }
        }
        gettimeofday(&end_time, NULL);
        double elapsed_time = (end_time.tv_sec - start_time.tv_sec)*1000.0 + (end_time.tv_usec - start_time.tv_usec)/1000.0 + 0.5;
        cout << "#" << scene << ": " << elapsed_time << "ms" << endl;
        ss.str("");
        ss << scene << ")" << (1000/elapsed_time) << "fps";
        cv::putText(image, ss.str(), cv::Point(10,20), CV_FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0,255,0), 1);
        cv::rectangle(image, tracking.GetTrackingROI(), cv::Scalar(0,255,0), 2);
        cv::imshow("Tracking", image);
        if(cv::waitKey(10) >= 0)
            break;
    }

    return;
}

void RunVideo()
{
    int scene = 0;
    cv::namedWindow("Tracking");
    cv::setMouseCallback("Tracking", MouseEvent);

    ss.str("");
    ss << path << "/" << name;

    cv::VideoCapture capture(ss.str());

    if(capture.isOpened() == false)
    {
        cout << "Cannot open video files!" << endl;
        return;
    }

    capture >> image;
    cv::cvtColor(image, prev_gray, cv::COLOR_BGR2GRAY);
    tracking.Initialize(image.size().width, image.size().height);

    for(;;)
    {
        gettimeofday(&start_time, NULL);
        if(on_track)
        {
            scene++;
            capture >> image;
            if(image.empty())
            {
                on_track = false;
                scene = 0;
                capture.release();
                ss.str("");
                ss << path << "/" << name;
                capture = cv::VideoCapture(ss.str());
                capture >> image;
                cv::cvtColor(image, prev_gray, cv::COLOR_BGR2GRAY);
                continue;
            }
            cv::cvtColor(image, next_gray, cv::COLOR_BGR2GRAY);
            try
            {
                display = image.clone();
                cv::Rect next_roi = tracking.FlowTracking(prev_gray, next_gray, display, STracking::AUTOGRID,
                                                          STracking::AFFINE, 1.0, true, on_debug);
                tracking.SetTrackingROI(next_roi);
                prev_gray = next_gray.clone();
            }
            catch(cv::Exception ex)
            {
                cout << ex.what() << endl;
            }            
        }

        gettimeofday(&end_time, NULL);
        double elapsed_time = (end_time.tv_sec - start_time.tv_sec)*1000.0 + (end_time.tv_usec - start_time.tv_usec)/1000.0 + 0.5;
        cout << "#" << scene << ": " << elapsed_time << "ms" << endl;
        ss.str("");
        ss << scene << ")" << (1000/elapsed_time) << "fps";
        cv::putText(image, ss.str(), cv::Point(10,20), CV_FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0,255,0), 1);
        cv::rectangle(image, tracking.GetTrackingROI(), cv::Scalar(0,255,0), 2);
        cv::imshow("Tracking", image);
        if(cv::waitKey(10) >= 0)
            break;
    }
    return;
}

#endif
