#include "stdafx.h"

int main(int ac, char** av)
{
	int i32Rst = 0;

	VideoCapture vCapture(0);//capture the video from webcam
	vCapture.set(CAP_PROP_FRAME_WIDTH, 1920);
	vCapture.set(CAP_PROP_FRAME_HEIGHT, 1080);

	if (!vCapture.isOpened()) // if not success, exit program
	{
		printf("Can't open the camera");
		i32Rst = -1;
	}
	else
	{
		/*Mat frame;
		while (true) {
			vCapture >> frame;

			imshow("Camera View", frame);

			if (waitKey(1) == 27)
				break;//esc 키 입력시 종료
		}*/

		//Capture a temporary image from the camera
		Mat imgTmp;
		vCapture.read(imgTmp);

		namedWindow("Camera View", CV_WINDOW_AUTOSIZE); //create a window called "Control"

		int iLowH = 170;
		int iHighH = 179;

		int iLowS = 150;
		int iHighS = 255;

		int iLowV = 60;
		int iHighV = 255;

		//Create trackbars in "Control" window
		createTrackbar("LowH", "Control", &iLowH, 255); //Hue (0 - 179)
		createTrackbar("HighH", "Control", &iHighH, 255);

		createTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
		createTrackbar("HighS", "Control", &iHighS, 255);

		createTrackbar("LowV", "Control", &iLowV, 255);//Value (0 - 255)
		createTrackbar("HighV", "Control", &iHighV, 255);

		int iLastX = -1;
		int iLastY = -1;

		//Create a black image with the size as the camera output
		Mat imgLines = Mat::zeros(imgTmp.size(), CV_8UC3);


		while (true)
		{
			Mat imgOriginal;

			bool bSuccess = vCapture.read(imgOriginal); // read a new frame from video



			if (!bSuccess) //if not success, break loop
			{
				cout << "Cannot read a frame from video stream" << endl;
				break;
			}

			Mat imgHSV;
			cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

			Mat imgThresholded;

			inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

			//morphological opening (removes small objects from the foreground)
			erode(imgThresholded, imgThresholded, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)));
			dilate(imgThresholded, imgThresholded, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)));

			//morphological closing (removes small holes from the foreground)
			dilate(imgThresholded, imgThresholded, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)));
			erode(imgThresholded, imgThresholded, getStructuringElement(CV_SHAPE_ELLIPSE, Size(3, 3)));

			//Calculate the moments of the thresholded image
			Moments oMoments = moments(imgThresholded);

			double dM01 = oMoments.m01;
			double dM10 = oMoments.m10;
			double dArea = oMoments.m00;

			// if the area <= 5000, I consider that the there are no object in the image and it's because of the noise, the area is not zero
			if (dArea > 5000)
			{
				printf("DArea Now: %f \n", abs(dArea));

				//calculate the position of the ball
				int posX = dM10 / dArea;
				int posY = dM01 / dArea;

				if (iLastX >= 0 && iLastY >= 0 && posX >= 0 && posY >= 0)
				{
					//Draw a red line from the previous point to the current point
					line(imgLines, Point(posX, posY), Point(iLastX, iLastY), Scalar(0, 0, 255), 2);
				}

				iLastX = posX;
				iLastY = posY;
			}

			imshow("Thresholded Image", imgThresholded); //show the thresholded image

			imgOriginal = imgOriginal + imgLines;
			imshow("Original", imgOriginal); //show the original image

			if (waitKey(30) == 27) //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
			{
				cout << "esc key is pressed by user" << endl;
				break;
			}
		}
	}

	return i32Rst;
}