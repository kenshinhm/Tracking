#include "tracking.hpp"

void STracking::Initialize(int image_width, int image_height, int max_corners)
{
    this->image_width = image_width;
    this->image_height = image_height;
    this->max_corners = max_corners;

    cv::RNG rng(12345);
    for(int i = 0 ; i < max_corners ; i++)
        colors.push_back(cv::Scalar(rng.uniform(0,255), rng.uniform(0,255), rng.uniform(0,255)));
}

cv::Rect STracking::GetTrackingROI()
{
    return tracking_roi;
}

cv::Rect STracking::GetROI(int x, int y, int width, int height)
{
    cv::Rect ret;
    ret.x = MIN(MAX(x,0), image_width);
    ret.y = MIN(MAX(y,0), image_height);
    ret.width = MIN(width, image_width - ret.x);
    ret.height = MIN(height, image_height - ret.y);

    return ret;
}

cv::Rect STracking::GetROI(cv::Rect roi)
{
    roi.x = MIN(MAX(roi.x,0), image_width);
    roi.y = MIN(MAX(roi.y,0), image_height);
    roi.width = MIN(roi.width, image_width - roi.x);
    roi.height = MIN(roi.height, image_height - roi.y);

    return roi;
}

void STracking::SetTrackingROI(cv::Rect roi)
{
    this->SetTrackingROI(roi.x, roi.y, roi.width, roi.height);
}

void STracking::SetTrackingROI(int x, int y, int width, int height)
{
    tracking_roi.x = MIN(MAX(x,0), image_width);
    tracking_roi.y = MIN(MAX(y,0), image_height);
    tracking_roi.width = MIN(width, image_width - tracking_roi.x);
    tracking_roi.height = MIN(height, image_height - tracking_roi.y);
}

void STracking::Resize(cv::Rect& rect, double scale)
{
    rect.x = cvRound(rect.x*scale);
    rect.y = cvRound(rect.y*scale);
    rect.width = cvRound(rect.width*scale);
    rect.height = cvRound(rect.height*scale);

    return;
}

void STracking::Resize(vector<cv::Point2f>& points, double scale)
{
    for(vector<cv::Point2f>::iterator it = points.begin() ; it != points.end() ; it++)
    {
        it->x *= scale;
        it->y *= scale;
    }

    return;
}

cv::Rect STracking::FlowTracking(cv::Mat& prev_gray, cv::Mat& next_gray, cv::Mat& display, CORNER corner, METHOD method,
                                 double auto_threshold, bool resize, bool debug)
{
    int resize_scale;
    cv::Rect next_roi;
    cv::Rect prev_roi = this->GetTrackingROI();
    cv::Rect optical_roi = this->GetROI(prev_roi.x - image_width/16, prev_roi.y - image_height/16,
                                        prev_roi.width + image_width/8, prev_roi.height + image_width/8);

    if(prev_gray.channels() != 1)
        cv::cvtColor(prev_gray, prev_gray, cv::COLOR_RGB2GRAY);
    if(next_gray.channels() != 1)
        cv::cvtColor(next_gray, next_gray, cv::COLOR_RGB2GRAY);

    cv::Mat prev_image = prev_gray.clone();
    cv::Mat next_image = next_gray.clone();

    if(resize == false)
        resize_scale = 1;
    else
    {
        if(96*prev_roi.width*prev_roi.height < image_height*image_width)
            resize_scale = 1;
        else if(24*prev_roi.width*prev_roi.height < image_height*image_width)
            resize_scale = 2;
        else
            resize_scale = 4;
    }

    cv::resize(prev_image, prev_image, cv::Size(), (double)1/resize_scale, (double)1/resize_scale);
    cv::resize(next_image, next_image, cv::Size(), (double)1/resize_scale, (double)1/resize_scale);

    this->Resize(prev_roi, (double)1/resize_scale);
    this->Resize(optical_roi, (double)1/resize_scale);

    vector<cv::Point2f> prev_corners;
    this->GetCorners(prev_image(prev_roi), prev_corners, corner, 4);

    this->SetPointsGlobal(prev_corners, prev_roi);
    this->SetPointsLocal(prev_corners, optical_roi);

    //Optical Flow
    vector<cv::Point2f> prev_estimate, next_estimate;
    vector<cv::Point2f> prev_matches, next_matches;
    this->OpticalFlowPyLK(prev_image(optical_roi), next_image(optical_roi), prev_corners,
                          prev_estimate, next_estimate, prev_matches, next_matches);

    //Transform
    this->SetPointsGlobal(prev_matches, optical_roi);
    this->SetPointsGlobal(next_matches, optical_roi);

    //Rearrange
    if(corner == AUTOGRID)
        prev_roi = this->Rearrange(prev_matches, next_matches, prev_roi, 4, auto_threshold/resize_scale);

    this->Resize(prev_roi, resize_scale);
    this->Resize(optical_roi, resize_scale);
    this->Resize(prev_matches, resize_scale);
    this->Resize(next_matches, resize_scale);

    next_roi = this->Transform(prev_matches, next_matches, prev_roi, method);

    if(debug)
    {
        for(int i = 0 ; i < prev_matches.size() ; i++)
        {
            if(abs(prev_matches[i].x - next_matches[i].x) >= auto_threshold ||
                    abs(prev_matches[i].y - next_matches[i].y) >= auto_threshold)
                cv::line(display, cv::Point(cvRound(prev_matches[i].x), cvRound(prev_matches[i].y)),
                         cv::Point(cvRound(next_matches[i].x), cvRound(next_matches[i].y)), colors[i], 1, CV_AA);

            int dx = next_roi.x - prev_roi.x;
            int dy = next_roi.y - prev_roi.y;
            double dw = (double)next_roi.width/prev_roi.width;
            double dh = (double)next_roi.height/prev_roi.height;

            string s_resize, s_dxy, s_dwh; ostringstream ss;
            ss << "Resize: x" << resize_scale;
            s_resize = ss.str(); ss.str("");
            ss << "dx: " << dx << "  " << "dy: " << dy;
            s_dxy = ss.str(); ss.str("");
            ss << "dwidth: " << dw << "  " << "dheight: " << dh;
            s_dwh = ss.str(); ss.str("");

            cv::putText(display, s_resize, cv::Point(0,10), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,255,0));
            cv::putText(display, s_dxy, cv::Point(0,30), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,255,0));
            cv::putText(display, s_dwh, cv::Point(0,50), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,255,0));

            cv::rectangle(display, prev_roi, cv::Scalar(255,0,0));
            cv::rectangle(display, next_roi, cv::Scalar(0,255,0));
            cv::rectangle(display, optical_roi, cv::Scalar(0,0,255));

            cv::imshow("Debug", display);
        }
    }


    return next_roi;
}

cv::Rect STracking::Rearrange(vector<cv::Point2f>& prev_matches, vector<cv::Point2f>& next_matches, cv::Rect prev_roi, int step, double threshold)
{
    cv::Rect ret = prev_roi;
    int hmargin = (prev_roi.height % step == 0) ? 4 : 5;
    int wmargin = (prev_roi.width % step == 0) ? 4 : 5;

    cv::Mat blackboard(prev_roi.height/step + hmargin, prev_roi.width/step + wmargin, CV_8U, cv::Scalar(0));

    for(int i = 0 ; i < prev_matches.size() ; i++)
    {
        if(abs(prev_matches[i].x - next_matches[i].x) >= threshold || abs(prev_matches[i].y - next_matches[i].y) >= threshold)
        {
            int x = (int)prev_matches[i].x - prev_roi.x;
            int y = (int)prev_matches[i].y - prev_roi.y;
            blackboard.at<uchar>(y/step + 2, x/step + 2) = 255;
        }
    }
    //cv::imwrite("blackboard_before.jpg, blackboard);

    cv::Mat element(3, 3, CV_8U, cv::Scalar(1));
    cv::morphologyEx(blackboard, blackboard, cv::MORPH_CLOSE, element);
    cv::morphologyEx(blackboard, blackboard, cv::MORPH_OPEN, element);

    //cv::RNG rng(12345);
    vector<vector<cv::Point> > contours;
    //vector<Vec4i> hierarchy;

    cv::findContours(blackboard, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, cv::Point(0,0));

    double area_max = -1; int index_max = -1;
    double area = (double)(prev_roi.width*prev_roi.height)/(step*step);

    if(contours.size() > 0)
    {
        for(int i = 0 ; i < contours.size() ; i++)
        {
            double contour_area = cv::contourArea(contours[i]);
            if(contour_area > area_max && contour_area >= area*0.25 && contour_area <= area*0.75)
            {
                area_max = contour_area;
                index_max = i;
            }
        }

        if(index_max != -1)
        {
            vector<vector<cv::Point> > contours_poly(contours.size());
            cv::approxPolyDP(cv::Mat(contours[index_max]), contours_poly[index_max], 3, true);
            cv::Rect bound = cv::boundingRect(cv::Mat(contours_poly[index_max]));
            bound = cv::Rect((bound.x-2)*step + prev_roi.x, (bound.y-2)*step + prev_roi.y,
                             bound.width * step, bound.height * step);
            ret = cv::Rect((prev_roi.x + bound.x)/2,(prev_roi.y + bound.y)/2,
                           (prev_roi.width + bound.width)/2, (prev_roi.height + bound.height)/2);
        }
    }

    return ret;
}

void STracking::OpticalFlowPyLK(cv::Mat prev, cv::Mat next, vector<cv::Point2f> prev_corners,
                                vector<cv::Point2f> prev_estimate, vector<cv::Point2f> next_estimate,
                                vector<cv::Point2f>& prev_matches, vector<cv::Point2f>& next_matches)
{
    vector<uchar> status_1, status_2;
    vector<float> err_1, err_2;
    int max_level = 2;
    cv::Size window_size(21,21);
    cv::TermCriteria termcrit(cv::TermCriteria::COUNT|cv::TermCriteria::EPS,30,0.03);

    cv::calcOpticalFlowPyrLK(prev, next, prev_corners, next_estimate, status_1, err_1, window_size, max_level, termcrit, 0, 0.0001);
    cv::calcOpticalFlowPyrLK(next, prev, next_estimate, prev_estimate, status_2, err_2, window_size, max_level, termcrit, 0, 0.0001);

    prev_matches.clear();
    next_matches.clear();
    for(int i = 0 ; i < prev_corners.size() ; i++)
    {
        double dist = abs(prev_corners[i].x - prev_estimate[i].x) + abs(prev_corners[i].y - prev_estimate[i].y);
        if(status_1[i] && status_2[i] && (dist < 1.0))
        {
            prev_matches.push_back(prev_corners[i]);
            next_matches.push_back(next_estimate[i]);
        }
    }

    return;
}

void STracking::GetCorners(cv::Mat image, vector<cv::Point2f>& ret, CORNER corner, int step)
{
    ret.clear();

    if(corner == GRID || corner == AUTOGRID)
    {
        for(int y = 0 ; y < image.size().height ; y += step)
            for(int x = 0 ; x < image.size().width ; x += step)
                ret.push_back(cv::Point2f((double)x,(double)y));
    }
    else if(corner == FAST)
    {
//        vector<cv::KeyPoint> key_points;
//        int threshold = 4;
//        bool nonmaxsuppresion = true;
//        cv::Fast(image, key_points, threshold, nonmaxsuppresion);
//        for(vector<cv::KeyPoint>::iterator it = key_points.begin() ; it != key_points.end(); it++)
//            ret.push_back(cv::Point2f((double)it->pt.x, (double)it->pt.y));
    }

    return;
}

cv::Rect STracking::Transform(vector<cv::Point2f>& prev_matches, vector<cv::Point2f>& next_matches, cv::Rect prev_roi, METHOD method)
{
    cv::Rect ret = prev_roi;

    if(method == AFFINE)
    {
        if(prev_matches.size() < 6)
        {
            cout << "Transform: Size of Matches is insufficient" <<endl;
            return ret;
        }
        cv::Mat H = cv::estimateRigidTransform(prev_matches, next_matches, false);
        if(H.cols == 0 || H.rows == 0)
        {
            cout << "Transform: RigidTransform estimate error" <<endl;
            return ret;
        }

        vector<cv::Point2f> from;
        vector<cv::Point2f> to;
        this->RectToPoints(prev_roi, from);
        cv::transform(from, to, H);

        cv::RotatedRect rotated_roi = cv::minAreaRect(to);

        if(abs(prev_roi.width - rotated_roi.size.width) <= abs(prev_roi.width - rotated_roi.size.height))
        {
            ret = cv::Rect(cvRound(rotated_roi.center.x - rotated_roi.size.width/2),
                           cvRound(rotated_roi.center.y - rotated_roi.size.height/2),
                           cvRound(rotated_roi.size.width), cvRound(rotated_roi.size.height));
        }
        else
        {
            ret = cv::Rect(cvRound(rotated_roi.center.x - rotated_roi.size.height/2),
                           cvRound(rotated_roi.center.y - rotated_roi.size.width/2),
                           cvRound(rotated_roi.size.height), cvRound(rotated_roi.size.width));
        }
    }

    return ret;
}

void STracking::RectToPoints(cv::Rect rect, vector<cv::Point2f>& points)
{
    points.clear();
    points.push_back(cv::Point2f(rect.x, rect.y));
    points.push_back(cv::Point2f(rect.x + rect.width, rect.y));
    points.push_back(cv::Point2f(rect.x + rect.width, rect.y + rect.height));
    points.push_back(cv::Point2f(rect.x, rect.y + rect.height));

    return;
}

void STracking::RectToPoints(cv::Rect rect, vector<cv::Point>& points)
{
    points.clear();
    points.push_back(cv::Point(rect.x, rect.y));
    points.push_back(cv::Point(rect.x + rect.width, rect.y));
    points.push_back(cv::Point(rect.x + rect.width, rect.y + rect.height));
    points.push_back(cv::Point(rect.x, rect.y + rect.height));

    return ;
}

void STracking::SetPointsGlobal(vector<cv::Point2f>& points, cv::Rect roi)
{
    for(vector<cv::Point2f>::iterator it = points.begin() ; it != points.end(); it++)
    {
        it->x += roi.x;
        it->y += roi.y;
    }
    return;
}

void STracking::SetPointsLocal(vector<cv::Point2f>& points, cv::Rect roi)
{
    for(vector<cv::Point2f>::iterator it = points.begin() ; it != points.end(); it++)
    {
        it->x -= roi.x;
        it->y -= roi.y;
    }

    return;
}

