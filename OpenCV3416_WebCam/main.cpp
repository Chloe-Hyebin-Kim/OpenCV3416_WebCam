#include "stdafx.h"
#include "DetectorUtil.h"


int FindBall_1();
bool FindBall_2();

int FindBall_3();

int main(int ac, char** av)
{
	int i32Rst = 0;

	//CString a;
	//	CString b;
	//	LOG_INFO(a, b);


	//FindBall_1();
	//i32Rst = FindBall_2() ? 0 : -1;
	i32Rst = FindBall_3() ? 0 : -1;

	//DetectorUtil* pDetector = new DetectorUtil(); // 포인터로 객체 생성

	//if (!pDetector)
	//{
	//	printf("[ ERROR ] DetectorUtil is NULL!!! \n");
	//	return -1;
	//}

	//pDetector->Run(); //운곽선 검출
	//pDetector->ProcessMorphGaussianHough(); // 그림자 제거
	//pDetector->CustomProcess(); //히스토그램 평활화
	//pDetector->CustomAdaptiveProcess(); //어댑티브 임계값 적용
	//pDetector->FindCandidateArea(); //어댑티브 임계값 적용

	//delete pDetector; // 메모리 해제

	return i32Rst;
}


int FindBall_3()
{
	VideoCapture cap(0); // 웹캠 열기 (0번 카메라)

	if (!cap.isOpened())
	{
		printf("[ERROR] Can't open the camera! \n");
		return -1;
	}

	bool bFinish = false;
	while (!bFinish)
	{
		int capW = cvRound(cap.get(CAP_PROP_FRAME_WIDTH));
		int capH = cvRound(cap.get(CAP_PROP_FRAME_HEIGHT));

		Mat src, srcGray, blurred, canny, thresh, hsv, filtered, cont, done;
		vector< vector<Point> > contours;
		vector< vector<Point> > ThreshContours;
		vector<Vec4i> heirarchy;

		cap >> src;

		resize(src, src, Size(640, 480));

		// 1. BGR → Grayscale
		hsv = src.clone();
		cvtColor(src, srcGray, CV_BGR2GRAY);

		//Uses HSV for filtering
		cvtColor(src, hsv, CV_BGR2HSV);
		blur(hsv, blurred, Size(13, 13));

		inRange(hsv, Scalar(34, 23, 181), Scalar(48, 218, 255), filtered);

		threshold(srcGray, thresh, 221, 255, CV_THRESH_BINARY);

		done = src.clone();

		findContours(thresh, ThreshContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

		drawContours(done, ThreshContours, -1, Scalar(0, 0, 0), -1);


		vector<Point> largest;


		findContours(filtered, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
		cont = Mat::zeros(filtered.size(), CV_8UC1);
		drawContours(cont, contours, -1, Scalar(255, 255, 255), 1);


		vector< vector<Point> > contours_poly(contours.size());
		vector<Rect> boundRect(contours.size());
		vector<Point2f> center(contours.size());
		vector<float> radius(contours.size());


		for (int i = 0; i < contours.size(); i++) {
			approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
			boundRect[i] = boundingRect(Mat(contours_poly[i]));
			minEnclosingCircle((Mat)contours_poly[i], center[i], radius[i]);
		}


		string text;

		for (int i = 0; i < contours.size(); i++) {
			if (contourArea(contours[i]) > 650)
				rectangle(done, boundRect[i].tl(), boundRect[i].br(), Scalar(255, 255, 255), 2);
			text = to_string(i);
		}


		int circles = contours.size() - 1;


		//show original
		namedWindow("Original");
		imshow("Original", src);

		//show HSV
		namedWindow("HSV");
		imshow("HSV", hsv);

		//show blurred
		namedWindow("blurred");
		imshow("blurred", blurred);

		//show filtered
		namedWindow("filtered");
		imshow("filtered", filtered);

		//show contours
		namedWindow("contours");
		imshow("contours", cont);

		//show end img
		namedWindow("end");
		imshow("end", done);

		namedWindow("thresh");
		imshow("thresh", thresh);


		bFinish = (waitKey(1) == 27) ? true : false;
	}



	cap.release();
	destroyAllWindows();

	return 0;
}

int FindBall_1()
{
	VideoCapture cap(0); // 웹캠 열기 (0번 카메라)

	if (!cap.isOpened())
	{
		printf("[ERROR] >>>>>>>>>>>> Can't open the camera! <<<<<<<<<<<< \n");
		return -1;
	}


	while (true)
	{
		Mat frame;
		cap >> frame; // 프레임 읽기

		if (frame.empty())
		{
			printf("[ERROR] >>>>>>>>>>>> Frame is empty! <<<<<<<<<<<< \n");
			break;
		}

		// 흑백 변환
		Mat grayFrame;
		cvtColor(frame, grayFrame, COLOR_BGR2GRAY);
		GaussianBlur(grayFrame, grayFrame, Size(7, 7), 0, 0);

		/*threshold(grayFrame, grayFrame, (double)180.0, 255.0, THRESH_BINARY);
		Mat kernel2 = getStructuringElement(MORPH_RECT, Size(3, 3));
		erode(grayFrame, grayFrame, kernel2);
		dilate(grayFrame, grayFrame, kernel2, Point(-1, -1), 4);

		imshow("SearchTopBinary", grayFrame);*/

		// HoughCircles를 이용한 원 검출
		vector<Vec3f> circles;
		HoughCircles(grayFrame, circles, HOUGH_GRADIENT, 1, grayFrame.rows / 8, 100, 30, 10, 50);

		// 검출된 원을 프레임에 표시
		for (size_t i = 0; i < circles.size(); i++)
		{
			Vec3i c = circles[i];
			Point center(c[0], c[1]);
			int radius = c[2];

			// 중심 표시
			circle(frame, center, 3, Scalar(0, 255, 0), -1);
			// 원 테두리 표시
			circle(frame, center, radius, Scalar(0, 0, 255), 3);
		}

		// 결과 출력
		imshow("Golf Ball Detection", frame);


		// 'q' 키를 누르면 종료
		if (waitKey(10) == 'q')
		{
			printf("********************** EXIT PROGRAM ********************** \n");
			break;
		}
	}

	cap.release();
	destroyAllWindows();

	return 0;
}


bool FindBall_2()
{
	VideoCapture vCapture(0);//0번 카메라
	vCapture.set(CAP_PROP_FRAME_WIDTH, 1920);
	vCapture.set(CAP_PROP_FRAME_HEIGHT, 1080);

	if (!vCapture.isOpened()) // if not success, exit program
	{
		printf("[ERROR] >>>>>>>>>>>> Can't open the camera! <<<<<<<<<<<< \n");

		vCapture.release();
		destroyAllWindows();

		return false;
	}

	while (true)
	{
		Mat frame;
		vCapture >> frame;

		if (frame.empty())
		{
			printf("[ERROR] >>>>>>>>>>>> frame is empty! <<<<<<<<<<<< \n");
			break;
		}

		// 1. Convert BGR to HSV   //색상 필터링 (HSV 변환 후 흰색 마스크 생성)
		Mat hsv;
		cvtColor(frame, hsv, COLOR_BGR2HSV);

		//2. 흰색 범위 설정
		Scalar lower_white(0, 0, 180);
		Scalar upper_white(180, 50, 255);
		Mat mask;
		inRange(hsv, lower_white, upper_white, mask); // 흰색 범위 설정

		// 2. 모폴로지 연산 (노이즈 제거)
		Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));// 작은 커널 (세밀한 그림자 제거)
		//Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(25, 25));// 큰 커널 (넓은 그림자 제거, 큰 객체 강조)

		Mat processed;
		morphologyEx(mask, processed, MORPH_OPEN, kernel);
		morphologyEx(processed, processed, MORPH_CLOSE, kernel);

		// 3. 윤곽선 검출
		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;
		findContours(processed, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

		for (const auto& contour : contours)
		{
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