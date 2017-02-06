#if 1

#include "opencv2/opencv.hpp"
#include "vector"
#include "tracking.h"
#include "logitech_c920.h"
#include "string"
#include "sstream"
#include "iostream"

using namespace std;

#define ROITHRESH 20

enum MODE {VIDEO, IMAGE, STREAM};

bool on_track = false;
bool on_drag = false;
bool on_debug = false;
int max_scene;
cv::Mat image, next_gray, prev_gray, display;
STracking tracking;
C920 camera;
MODE mode;
bool debug;
stringstream ss;
string image_name;
string image_path;
string image_type;
int image_max;
string video_name;
string video_path;
string video_type;

void MouseEvent(int event, int x, int y, int flags, void* param);
void SelectMode();
void RunStream();
void RunImage();
void RunVideo();
void DestroyWindows();

int main()
{
    camera.Initialize(0);
    tracking.Initialize(camera.GetProperty(C920::WIDTH), camera.GetProperty(C920::HEIGHT));

    SelectMode();

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

void SelectMode()
{
    int input;
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
        cout << "Path for image folder: ";
        cin >> image_path;
        cout << "Name for image files: ";
        cin >> image_name;
        cout << "Image Type: ";
        cin >> image_type;
        cout << "How many files would you like to read: ";
        cin >> image_max;
    }
    else if(mode == VIDEO)
    {
        cout << "Path for video folder: ";
        cin >> video_path;
        cout << "Name for video file: ";
        cin >> video_name;
        cout << "Video Type: ";
        cin >> video_type;
    }

    cout << "Run Debug Mode?" << endl;
    cout << "1. Yes" << endl;
    cout << "2. No" << endl;
    cin >> input;

    if(input == 1)
        debug = true;
    else if(input == 2)
        debug = false;

    return;
}

void RunStream()
{
    int scene = 0;
    cv::namedWindow("Tracking");
    cv::setMouseCallback("Tracking", MouseEvent);

    camera.Grab(image);
    cv::cvtColor(image, prev_gray, cv::COLOR_RGB2GRAY);

    for(;;)
    {
        scene++;
        clock_t start = clock();

        camera.Grab(image);
        cv::cvtColor(image, next_gray, cv::COLOR_RGB2GRAY);

        try
        {
            if(on_track)
            {
                display = image.clone();
                cv::Rect next_roi = tracking.FlowTracking(prev_gray, next_gray, display, STracking::AUTOGRID,
                                                          STracking::AFFINE, 1.0, true, debug);
                tracking.SetTrackingROI(next_roi);
            }
        }
        catch(cv::Exception ex)
        {
            cout << ex.what() << endl;
        }

        prev_gray = next_gray.clone();
        clock_t end = clock();
        ss << scene << ")" << (1000/(end-start+1)) << "fps";
        cv::putText(image, ss.str(), cv::Point(10,20), CV_FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0,255,0), 1);
        ss.flush();
        cv::rectangle(image, tracking.GetTrackingROI(), cv::Scalar(0,255,0), 2);
        cv::imshow("Tracking", image);
        cout << "#" << scene << ": " << end-start+1 << "ms" << endl;
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

    ss.flush();
    ss << image_path << "/" << image_name << "_" << scene << "." << image_type;
    image = cv::imread(ss.str());
    cv::cvtColor(image, prev_gray, cv::COLOR_RGB2GRAY);

    for(;;)
    {
        if(on_track)
        {
            scene++;
            clock_t start = clock();

            try
            {
                ss.flush();
                ss << image_path << "/" << image_name << "_" << scene << "." << image_type;
                image = cv::imread(ss.str());

                cv::cvtColor(image, next_gray, cv::COLOR_RGB2GRAY);

                display = image.clone();
                cv::Rect next_roi = tracking.FlowTracking(prev_gray, next_gray, display, STracking::AUTOGRID,
                                                          STracking::AFFINE, 1.0, true, debug);
                tracking.SetTrackingROI(next_roi);
            }
            catch(cv::Exception ex)
            {
                cout << ex.what() << endl;
            }

            prev_gray = next_gray.clone();
            clock_t end = clock();

            ss.flush();
            ss << scene << ")" << (1000/(end-start+1)) << "fps";
            cv::putText(image, ss.str(), cv::Point(10,20), CV_FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0,255,0), 1);
            cv::rectangle(image, tracking.GetTrackingROI(), cv::Scalar(0,255,0), 2);
            cv::imshow("Tracking", image);
            cout << "#" << scene << ": " << end-start+1 << "ms" << endl;
            if(cv::waitKey(10) >= 0)
                break;
            if(scene >= image_max)
            {
                on_track = false;
                scene = 0;
                ss.flush();
                ss << image_path << "/" << image_name << "_" << scene << "." << image_type;
                image = cv::imread(ss.str());
                cv::cvtColor(image, prev_gray, cv::COLOR_RGB2GRAY);
            }
        }
    }

    return;
}

void RunVideo()
{
    int scene = 0;
    cv::namedWindow("Tracking");
    cv::setMouseCallback("Tracking", MouseEvent);

    ss.flush();
    ss << video_path << "/" << video_name << "." << video_type;
    cv::VideoCapture capture(ss.str());

    if(capture.isOpened() == false)
    {
        cout << "Cannot open video files!" << endl;
        return;
    }

    capture >> image;
    cv::cvtColor(image, prev_gray, cv::COLOR_RGB2GRAY);

    for(;;)
    {
        if(on_track)
        {
            scene++;
            clock_t start = clock();

            capture >> image;
            if(image.empty())
            {
                on_track = false;
                scene = 0;
                capture.release();
                ss.flush();
                ss << video_path << "/" << video_name << "." << video_type;
                capture = cv::VideoCapture(ss.str());
                capture >> image;
                cv::cvtColor(image, prev_gray, cv::COLOR_RGB2GRAY);
                continue;
            }
            cv::cvtColor(image, next_gray, cv::COLOR_RGB2GRAY);

            try
            {
                display = image.clone();
                cv::Rect next_roi = tracking.FlowTracking(prev_gray, next_gray, display, STracking::AUTOGRID,
                                                          STracking::AFFINE, 1.0, true, debug);
                tracking.SetTrackingROI(next_roi);
            }
            catch(cv::Exception ex)
            {
                cout << ex.what() << endl;
            }

            prev_gray = next_gray.clone();
            clock_t end = clock();
            ss << scene << ")" << (1000/(end-start+1)) << "fps";
            cv::putText(image, ss.str(), cv::Point(10,20), CV_FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0,255,0), 1);
            ss.flush();
            cv::rectangle(image, tracking.GetTrackingROI(), cv::Scalar(0,255,0), 2);
            cv::imshow("Tracking", image);
            cout << "#" << scene << ": " << end-start+1 << "ms" << endl;
            if(cv::waitKey(10) >= 0)
                break;
        }
    }
    return;
}

#endif
