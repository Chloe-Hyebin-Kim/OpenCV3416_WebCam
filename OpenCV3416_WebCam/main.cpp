#include "stdafx.h"
#include "DetectorUtil.h"

void FindBall_1();
bool FindBall_2();

int main(int ac, char** av)
{
	int i32Rst = 0;

	//FindBall_1();
	//i32Rst = FindBall_2() ? 0 : -1;




	DetectorUtil* pDetector = new DetectorUtil(); // 포인터로 객체 생성

	if (!pDetector)
	{
		printf("[ ERROR ] DetectorUtil is NULL!!! \n");
		return -1;
	}

	//pDetector->Run(); // 실시간 감지 실행
	pDetector->ProcessMorphGaussianHough(); // 실시간 감지 실행

	delete pDetector; // 메모리 해제

	return i32Rst;
}


void FindBall_1()
{
	VideoCapture vCapture(0);//0번 카메라

	//Capture a temporary image from the camera
	Mat imgTmp;
	vCapture.read(imgTmp);

	namedWindow(MAIN_FRAME, CV_WINDOW_AUTOSIZE); //create a window called "Control"

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

		if (waitKey(1) == 27) //wait for 'esc' key press for 1ms. If 'esc' key is pressed, break loop
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}
}


bool FindBall_2()
{
	VideoCapture vCapture(0);//0번 카메라
	vCapture.set(CAP_PROP_FRAME_WIDTH, 1920);
	vCapture.set(CAP_PROP_FRAME_HEIGHT, 1080);

	if (!vCapture.isOpened()) // if not success, exit program
	{
		printf("Can't open the camera");

		vCapture.release();
		destroyAllWindows();

		return false;
	}

	while (true)
	{
		Mat frame, hsv, mask, processed;
		vCapture >> frame;

		if (frame.empty())
			break;

		// 1. 색상 필터링 (HSV 변환 후 흰색 마스크 생성)
		cvtColor(frame, hsv, COLOR_BGR2HSV);
		inRange(hsv, Scalar(0, 0, 180), Scalar(180, 50, 255), mask); // 흰색 범위 설정

		// 2. 모폴로지 연산 (노이즈 제거)
		Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
		morphologyEx(mask, processed, MORPH_OPEN, kernel);
		morphologyEx(processed, processed, MORPH_CLOSE, kernel);

		// 3. 윤곽선 검출
		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;
		findContours(processed, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

		for (const auto& contour : contours) {
			double area = contourArea(contour);
			if (area > 500) { // 일정 크기 이상만 처리
				// 외접 원 찾기
				Point2f center;
				float radius;
				minEnclosingCircle(contour, center, radius);

				// 원형 여부 확인 (원에 가까운 정도)
				double perimeter = arcLength(contour, true);
				double circularity = 4 * M_PI * (area / (perimeter * perimeter));
				if (circularity > 0.7) { // 원형성이 높을 때만 처리
					Rect boundingBox(center.x - radius, center.y - radius, radius * 2, radius * 2);
					rectangle(frame, boundingBox, Scalar(0, 255, 0), 2); // 네모 박스 그리기
				}
			}
		}

		// 결과 출력
		imshow(MAIN_FRAME, frame);

		// 디버깅용 마스크 확인
		imshow(DEBUG_FRAME, mask);

		if (waitKey(1) == 27)
		{
			printf("success, exit program");
			destroyWindow(MAIN_FRAME);
			destroyWindow(DEBUG_FRAME);

			break; // ESC 키로 종료
		}
	}

	//free
	vCapture.release();
	destroyAllWindows();

	return true;
}