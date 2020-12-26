#include <cstdlib>
#include <iostream>
#include <iterator>
#include <opencv2/core.hpp>
#include <opencv2/core/base.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/flann/logger.h>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv4/opencv2/opencv.hpp>

enum class case_id
{
    READ_IMAGE = 0,
    READ_VIDEO,
    READ_WEBCAM,
    HISTOGRAM_EQUALIZATION,
    OBJECT_DETECTION,
    OBJECT_TRACKING
};

void demos(case_id demo_id = case_id::READ_IMAGE)
{
    switch (demo_id)
    {
    case case_id::OBJECT_TRACKING: {
        cv::VideoCapture cap("/home/caner/Desktop/pend0001-0122.mp4"); // capture the video from
                                                                       // webcam

        if (!cap.isOpened()) // if not success, exit program
        {
            std::cout << "Cannot open the web cam" << std::endl;
            exit(EXIT_FAILURE);
        }

        cv::namedWindow("Control",
                        cv::WINDOW_NORMAL); // create a window called "Control"

        cv::namedWindow("Original",
                        cv::WINDOW_NORMAL); // create a window called "Control"

        cv::namedWindow("Thresholded Image",
                        cv::WINDOW_AUTOSIZE); // create a window called "Control"

        int iLowH = 25;
        int iHighH = 79;

        int iLowS = 35;
        int iHighS = 255;

        int iLowV = 60;
        int iHighV = 255;

        // Create trackbars in "Control" window
        cv::createTrackbar("LowH", "Control", &iLowH, 179); // Hue (0 - 179)
        cv::createTrackbar("HighH", "Control", &iHighH, 179);

        cv::createTrackbar("LowS", "Control", &iLowS, 255); // Saturation (0 -
                                                            // 255)
        cv::createTrackbar("HighS", "Control", &iHighS, 255);

        cv::createTrackbar("LowV", "Control", &iLowV, 255); // Value (0 - 255)
        cv::createTrackbar("HighV", "Control", &iHighV, 255);

        int iLastX = -1;
        int iLastY = -1;

        // Capture a temporary image from the camera
        cv::Mat imgTmp;
        cap.read(imgTmp);

        // Create a black image with the size as the camera output
        cv::Mat imgLines = cv::Mat::zeros(imgTmp.size(), CV_8UC3);

        while (true)
        {
            cv::Mat imgOriginal;
        loop:
            bool bSuccess = cap.read(imgOriginal); // read a new frame from video

            if (!bSuccess) // if not success, break loop
            {
                cap.set(cv::CAP_PROP_POS_FRAMES, 0);
                goto loop;
            }

            cv::Mat imgHSV;

            cv::cvtColor(imgOriginal, imgHSV,
                         cv::COLOR_BGR2HSV); // Convert the captured frame
                                             // from BGR to HSV

            cv::Mat imgThresholded;

            cv::inRange(imgHSV, cv::Scalar(iLowH, iLowS, iLowV), cv::Scalar(iHighH, iHighS, iHighV),
                        imgThresholded); // Threshold the image

            // morphological opening (removes small objects from the
            // foreground)
            cv::erode(imgThresholded, imgThresholded, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
            cv::dilate(imgThresholded, imgThresholded, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));

            // morphological closing (removes small holes from the foreground)
            cv::dilate(imgThresholded, imgThresholded, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
            cv::erode(imgThresholded, imgThresholded, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));

            // Calculate the moments of the thresholded image
            cv::Moments oMoments = cv::moments(imgThresholded);

            double M01 = oMoments.m01;
            double M10 = oMoments.m10;
            double Area = oMoments.m00;

            // if the area <= 10000, I consider that the there are no object in
            // the image and it's because of the noise, the area is not zero
            if (Area > 10000)
            {
                // calculate the position of the ball
                int posX = M10 / Area;
                int posY = M01 / Area;

                if (iLastX >= 0 && iLastY >= 0 && posX >= 0 && posY >= 0)
                {
                    // Draw a red line from the previous point to the current
                    // point
                    cv::line(imgLines, cv::Point(posX, posY), cv::Point(iLastX, iLastY), cv::Scalar(0, 0, 255), 2);
                }

                iLastX = posX;
                iLastY = posY;
            }

            imgOriginal = imgOriginal + imgLines;
            cv::imshow("Original", imgOriginal); // show the original image

            cv::imshow("Thresholded Image",
                       imgThresholded); // show the thresholded image

            if (cv::waitKey(30) == 27) // wait for 'esc' key press for 30ms.
                                       // If 'esc' key is pressed, break loop
            {
                std::cout << "esc key is pressed by user" << std::endl;
                break;
            }
        }
        cv::destroyAllWindows();
        break;
    }
    case case_id::OBJECT_DETECTION: {
        cv::Mat image_cpy = cv::imread("/home/caner/Downloads/2_kopya.png");
        cv::Mat image = cv::imread("/home/caner/Downloads/2.jpg");

        // Check for failure
        if (image.empty())
        {
            std::cout << "Could not open or find the image" << std::endl;
            std::cin.get(); // wait for any key press
            return exit(EXIT_SUCCESS);
        }

        cv::namedWindow("Control",
                        cv::WINDOW_NORMAL); // create a window called "Control"

        cv::namedWindow("Thresholded Image",
                        cv::WINDOW_NORMAL); // show the thresholded image
        cv::namedWindow("Thresholded Image Copy",
                        cv::WINDOW_NORMAL); // show the original image

        int iLowH = 11;
        int iHighH = 33;

        int iLowS = 14;
        int iHighS = 255;

        int iLowV = 4;
        int iHighV = 255;
#if 0
        // Create trackbars in "Control" window
        cv::createTrackbar("LowH", "Control", &iLowH, 179); // Hue (0 - 179)
        cv::createTrackbar("HighH", "Control", &iHighH, 179);

        cv::createTrackbar("LowS", "Control", &iLowS,
                           255); // Saturation (0 - 255)
        cv::createTrackbar("HighS", "Control", &iHighS, 255);

        cv::createTrackbar("LowV", "Control", &iLowV, 255); // Value (0 - 255)
        cv::createTrackbar("HighV", "Control", &iHighV, 255);
#endif
        while (true)
        {
            cv::Mat imgHSV, imgHSV_cpy;

            cv::cvtColor(image_cpy, imgHSV_cpy,
                         cv::COLOR_BGR2HSV); // Convert the captured frame
                                             // from BGR to HSV

            cv::cvtColor(image, imgHSV,
                         cv::COLOR_BGR2HSV); // Convert the captured frame
                                             // from BGR to HSV

            cv::Mat imgThresholded, imgThresholded_cpy;

            cv::inRange(imgHSV_cpy, cv::Scalar(iLowH, iLowS, iLowV), cv::Scalar(iHighH, iHighS, iHighV),
                        imgThresholded_cpy); // Threshold the image

            // morphological opening (remove small objects from the foreground)
            cv::erode(imgThresholded_cpy, imgThresholded_cpy,
                      cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));

            cv::dilate(imgThresholded_cpy, imgThresholded_cpy,
                       cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));

            // morphological closing (fill small holes in the foreground)
            cv::dilate(imgThresholded_cpy, imgThresholded_cpy,
                       cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));

            cv::erode(imgThresholded_cpy, imgThresholded_cpy,
                      cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
            //----------------------------------------------------------------------------------------------------------------------------------------------
            cv::inRange(imgHSV, cv::Scalar(iLowH, iLowS, iLowV), cv::Scalar(iHighH, iHighS, iHighV),
                        imgThresholded); // Threshold the image

            // morphological opening (remove small objects from the foreground)
            cv::erode(imgThresholded, imgThresholded, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));

            cv::dilate(imgThresholded, imgThresholded, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));

            // morphological closing (fill small holes in the foreground)
            cv::dilate(imgThresholded, imgThresholded, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));

            cv::erode(imgThresholded, imgThresholded, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
            //----------------------------------------------------------------------------------------------------------------------------------------------
            cv::imshow("Thresholded Image",
                       imgThresholded);                               // show the thresholded image
            cv::imshow("Thresholded Image Copy", imgThresholded_cpy); // show the original image

            // cv::Mat img_result;
            // cv::compare(imgThresholded_cpy, imgThresholded, img_result, cv::CMP_EQ);

            // uint8_t *pixelPtr = (uint8_t *)img_result.data;
            // int cn = img_result.channels();
            // cv::Scalar_<uint8_t> bgrPixel;

            for (int i = 0; i < imgThresholded.rows; i++)
            {
                for (int j = 0; j < imgThresholded.cols; j++)
                {
                    std::cout << (int)imgThresholded.at<uint8_t>(i, j) << "  ";
                }
                std::cout << std::endl;
                std::cout << "<------------------------ROW END------------------------------->" << std::endl;
            }
            std::cout << imgThresholded.size().height << ", " << imgThresholded.size().width << std::endl;
            goto EXIT;

            // for (int i = 0; i < img_result.rows; i++)
            // {
            //     for (int j = 0; j < img_result.cols; j++)
            //     {
            //         if((int)img_result.at<uint8_t> (i, j) != 255){
            //             std::cout << "Images are not same!" << std::endl;
            //             goto EXIT;
            //         }
            //     }
            // }

            // for (size_t i = 0; i < img_result.size().width * img_result.size().height; ++i)
            // {
            // }

            if (cv::waitKey(30) == 27) // wait for 'esc' key press for 30ms.
            {
                std::cout << "esc key is pressed by user" << std::endl;
                break;
            }
        }
    EXIT:
        cv::destroyWindow("Control");                // destroy the created window
        cv::destroyWindow("Thresholded Image Copy"); // destroy the created window
        cv::destroyWindow("Thresholded Image");      // destroy the created window
        break;
    }
    case case_id::READ_IMAGE: {
        // Read the image file
        cv::Mat image = cv::imread("/home/caner/Pictures/boostNote.png");

        // Check for failure
        if (image.empty())
        {
            std::cout << "Could not open or find the image" << std::endl;
            std::cin.get(); // wait for any key press
            return exit(EXIT_SUCCESS);
        }

        // Create trackbar to change brightness

        cv::namedWindow("OpenCV", cv::WINDOW_AUTOSIZE); // Create a window
        int iSliderValue1 = 50;
        cv::createTrackbar("Brightness", "OpenCV", &iSliderValue1, 100);

        while (true)
        {
            cv::Mat dst;
            int iBrightness = iSliderValue1 - 50;
            image.convertTo(dst, -1, 1, iBrightness);

            // show the brightness and contrast adjusted image
            cv::imshow("OpenCV", dst);

            // if user press 'ESC' key
            if (cv::waitKey(50) == 27)
            {
                break;
            }
        }

        cv::destroyWindow("OpenCV"); // destroy the created window

        break;
    }
    case case_id::READ_VIDEO: {
        // open the video file for reading
        cv::VideoCapture cap("/home/caner/Misc/non_lin/nonlinear_week2.mp4");

        // if not success, exit program
        if (cap.isOpened() == false)
        {
            std::cout << "Cannot open the video file" << std::endl;
            std::cin.get(); // wait for any key press
            return exit(EXIT_FAILURE);
        }

        // Uncomment the following line if you want to start the video in the
        // middle cap.set(CAP_PROP_POS_MSEC, 300);

        // get the frames rate of the video
        double fps = cap.get(cv::CAP_PROP_FPS);
        std::cout << "Frames per seconds : " << fps << std::endl;

        cv::String window_name = "My First Video";

        cv::namedWindow(window_name, cv::WINDOW_NORMAL); // create a window

        while (true)
        {
            cv::Mat frame;
            bool bSuccess = cap.read(frame); // read a new frame from video

            // Breaking the while loop at the end of the video
            if (bSuccess == false)
            {
                std::cout << "Found the end of the video" << std::endl;
                break;
            }

            // show the frame in the created window
            cv::imshow(window_name, frame);

            // wait for for 10 ms until any key is pressed.
            // If the 'Esc' key is pressed, break the while loop.
            // If the any other key is pressed, continue the loop
            // If any key is not pressed withing 10 ms, continue the loop
            if (cv::waitKey(10) == 27)
            {
                std::cout << "Esc key is pressed by user. Stoppig the video" << std::endl;
                break;
            }
        }
        break;
    }
    case case_id::READ_WEBCAM: {
        // open the video file for reading
        cv::VideoCapture cap(0);

        // if not success, exit program
        if (cap.isOpened() == false)
        {
            std::cout << "Cannot open the video file" << std::endl;
            std::cin.get(); // wait for any key press
            return exit(EXIT_FAILURE);
        }

        double dWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);   // get the width of frames of the video
        double dHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT); // get the height of frames
                                                             // of the video

        std::cout << "Resolution of the video : " << dWidth << " x " << dHeight << std::endl;

        // Uncomment the following line if you want to start the video in the
        // middle cap.set(CAP_PROP_POS_MSEC, 300);

        // get the frames rate of the video
        double fps = cap.get(cv::CAP_PROP_FPS);
        std::cout << "Frames per seconds : " << fps << std::endl;

        cv::String window_name = "My First Video";

        cv::namedWindow(window_name, cv::WINDOW_NORMAL); // create a window

        while (true)
        {
            cv::Mat frame;
            bool bSuccess = cap.read(frame); // read a new frame from video

            // Breaking the while loop at the end of the video
            if (bSuccess == false)
            {
                std::cout << "Found the end of the video" << std::endl;
                break;
            }

            // show the frame in the created window
            cv::imshow(window_name, frame);

            // wait for for 10 ms until any key is pressed.
            // If the 'Esc' key is pressed, break the while loop.
            // If the any other key is pressed, continue the loop
            // If any key is not pressed withing 10 ms, continue the loop
            if (cv::waitKey(10) == 27)
            {
                std::cout << "Esc key is pressed by user. Stoppig the video" << std::endl;
                break;
            }
        }
        break;
    }
    case case_id::HISTOGRAM_EQUALIZATION: {
        // Read the image file
        cv::Mat image = cv::imread("/home/caner/Pictures/stockByte.png");

        // Check for failure
        if (image.empty())
        {
            std::cout << "Could not open or find the image" << std::endl;
            std::cin.get(); // wait for any key press
            return exit(EXIT_SUCCESS);
        }

        // change the color image to grayscale image
        cvtColor(image, image, cv::COLOR_BGR2GRAY);

        // equalize the histogram
        cv::Mat hist_equalized_image;
        equalizeHist(image, hist_equalized_image);

        cv::String windowName = "AFTER"; // Name of the window

        cv::namedWindow(windowName, cv::WINDOW_NORMAL); // Create a window
        cv::namedWindow("BEFORE", cv::WINDOW_NORMAL);

        cv::imshow(windowName,
                   hist_equalized_image); // Show our image inside the created window.
        cv::imshow("BEFORE", image);

        cv::waitKey(0); // Wait for any keystroke in the window

        cv::destroyWindow(windowName); // destroy the created window
        cv::destroyWindow("AFTER");    // destroy the created window
    }
    }
}

int main(int argc, char **argv)
{
    demos(case_id::OBJECT_DETECTION);
    return 0;
}
