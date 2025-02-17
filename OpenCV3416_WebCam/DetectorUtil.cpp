#include "DetectorUtil.h"


DetectorUtil::DetectorUtil()
{
	m_pCapture = new VideoCapture(0);// 0번 카메라 사용
	m_pOriginFrame = new Mat(); // 원본 프레임 객체

	if (!m_pCapture)
	{
		printf("[ERROR] DetectorUtil::m_pCapture is NULL!! \n");
		return;
	}

	m_pCapture->set(CAP_PROP_FRAME_WIDTH, 1920);
	m_pCapture->set(CAP_PROP_FRAME_HEIGHT, 1080);
}

DetectorUtil::~DetectorUtil()
{
	delete m_pCapture;
	delete m_pOriginFrame;

	destroyAllWindows();
}


void DetectorUtil::SimpleShow()
{
	if (!IsCameraOpened())
		return;

	bool bLoop = true;
	do
	{
		(*m_pCapture) >> (*m_pOriginFrame);

		imshow(MAIN_FRAME, *m_pOriginFrame);

		bLoop = ESCKeyUser();
	} while (bLoop);



	/*
	 while (true)
	{
		Mat frame;
		(*m_pCapture) >> frame;

		imshow(MAIN_FRAME, frame);
		if (waitKey(1) == 27)
		{
			printf("ESC key is pressed by user. \n");
			break;//esc 키 입력시 종료
		}
	}
	 */


	 /*
	 Mat frame;
	 while (true)
	 {
		 (*m_pCapture) >> frame;

		 imshow(MAIN_FRAME, frame);

		 if (waitKey(1) == 27)
		 {
			 printf("ESC key is pressed by user. \n");
			 break;//esc 키 입력시 종료
		 }
	 }
	  */
}

bool DetectorUtil::ESCKeyUser()
{
	bool bLoop = true;

	if (waitKey(1) == 27) //wait for 'esc' key press for 1ms. If 'esc' key is pressed, break loop
	{
		printf("[DetectorUtil::ESCKeyUser()] ESC key is pressed by user. \n");
		bLoop = false;//break;
	}

	return bLoop;
}

void DetectorUtil::Run()
{
	if (!IsCameraOpened())
		return;

	bool bLoop = true;
	do
	{
		Mat outputFrame;
		ProcessFrame(outputFrame);
		imshow(MAIN_FRAME, outputFrame);

		bLoop = ESCKeyUser();
	} while (bLoop);
}

const bool DetectorUtil::IsCameraOpened() const
{
	bool bOpen = m_pCapture->isOpened();

	if (!bOpen)
		printf("[ERROR] [DetectorUtil::IsCameraOpened()] Can't open the camera! \n");

	return bOpen;
}

void DetectorUtil::ProcessFrame(Mat& outputFrame)
{
	(*m_pCapture) >> (*m_pOriginFrame);

	if (m_pOriginFrame->empty())
	{
		printf("[ERROR] [DetectorUtil::ProcessFrame(Mat& outputFrame)] m_pOriginFrame is Empty! \n");
		return;
	}

	outputFrame = m_pOriginFrame->clone();
	Mat mask = Preprocess();
	vector<Rect> balls = DetectBalls(mask);

	// 검출된 공 주변에 네모 박스 그리기
	for (const Rect& ball : balls)
	{
		rectangle(outputFrame, ball, Scalar(0, COLOR_MAX, 0), 2);
	}
}

Mat DetectorUtil::Preprocess()
{
	// 1. HSV 변환
	Mat hsv;
	cvtColor(*m_pOriginFrame, hsv, COLOR_BGR2HSV);

	// 2. 색상 필터링 (흰색 영역 검출)
	Mat mask;
	inRange(hsv, Scalar(0, 0, 180), Scalar(180, 50, COLOR_MAX), mask); // 흰색 영역 검출

	// 3. 모폴로지 연산 (노이즈 제거)
	Mat processed; // 모폴로지 연산 후 결과 프레임
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5)); // 모폴로지 연산용 커널 프레임
	morphologyEx(mask, processed, MORPH_OPEN, kernel);
	morphologyEx(processed, processed, MORPH_CLOSE, kernel);

	return processed;
}

vector<Rect> DetectorUtil::DetectBalls(const Mat& mask)
{
	//1. 윤곽선 검출
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(mask, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	vector<Rect> ballBoxes;
	for (const vector<Point>& contour : contours)
	{
		double area = contourArea(contour);

		if (area > 500) // 일정 크기 이상만 처리
		{
			// 외접 원 찾기
			Point2f center;
			float radius;
			minEnclosingCircle(contour, center, radius);

			// 원형 여부 확인 (원에 가까운 정도)
			double perimeter = arcLength(contour, true);
			double circularity = 4 * M_PI * (area / (perimeter * perimeter));

			if (circularity > 0.7) // 원형성이 높을 때만 처리
			{
				ballBoxes.emplace_back(center.x - radius, center.y - radius, radius * 2, radius * 2);
			}
		}
	}


	return ballBoxes;
}

void DetectorUtil::FindCandidateArea()
{
	(*m_pCapture) >> (*m_pOriginFrame);

	if (m_pOriginFrame->empty())
	{
		printf("[ERROR] [DetectorUtil::ProcessFrame(Mat& outputFrame)] m_pOriginFrame is Empty! \n");
		return;
	}

	Mat imgSrc = m_pOriginFrame->clone();
	Rect rtImage(0, 0, imgSrc.cols, imgSrc.rows);
	Rect FindROI = Rect(0, 0, 2048, 660);//Rect(0,0,1650,660);

	if (FindROI != (FindROI & rtImage))
	{
		printf("[ ERROR ] [DetectorUtil::FindCandidateArea()] ROI Bound error. \n");
		return;
	}

	cv::Mat ROIArea = imgSrc(FindROI).clone();

	/*cv::threshold(ROIArea, ROIArea, (double)ReadyDat.nAvr, COLOR_MAX.0, cv::THRESH_BINARY);

	std::vector<std::vector<cv::Point>> contour;
	cv::findContours(ROIArea, contour, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	nBlob = contour.size();*/
}


bool DetectorUtil::ProcessMorphGaussianHough()
{
	if (!IsCameraOpened())
		return false;

	Mat im1, im2;
	vector<Mat> bgr(3);

	/*
	 int iLowH = 170;
	int iHighH = 179;

	int iLowS = 150;
	int iHighS = 255;

	int iLowV = 60;
	int iHighV = 255;
	 */

	bool bLoop = true;
	do
	{
		(*m_pCapture) >> im1;

		/*

		Mat imgOriginal;
		im1.copyTo(imgOriginal);

		Mat imgHSV;
		cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV); //Convert the captured frame from BGR to HSV

		Mat imgThresholded;
		inRange(imgHSV, Scalar(170, 150, 60), Scalar(iHighH, iHighS, iHighV), imgThresholded); //Threshold the image

		 */





		im1.copyTo(im2);
		cvtColor(im1, im1, COLOR_BGR2HSV);// Convert BGR to HSV  (CV_BGR2HSV == COLOR_BGR2HSV)
		split(im1, bgr);


		vector<Mat> bgr_thresh(3);
		threshold(bgr[0], bgr_thresh[0], 40, COLOR_MAX, THRESH_BINARY);
		threshold(bgr[1], bgr_thresh[1], 80, COLOR_MAX, THRESH_BINARY);
		threshold(bgr[2], bgr_thresh[2], 180, COLOR_MAX, THRESH_BINARY);


		Mat bitwised;
		bitwise_and(bgr_thresh[0], bgr_thresh[1], bitwised);
		bitwise_and(bgr_thresh[2], bitwised, bitwised);


		Mat morphed = MorphOps(bitwised, 5);

		Mat blurred;
		GaussianBlur(morphed, blurred, { 9, 9 }, 1, 1);

		Mat hough = Hough2d(im2, blurred);
		imshow(MAIN_FRAME, hough);
		imshow(DEBUG_FRAME, blurred);

		bLoop = ESCKeyUser();
	} while (bLoop);

	return true;
}

Mat DetectorUtil::Hough2d(Mat img, Mat msk)
{
	Mat hough_in = msk;//Mat hough_in = msk.clone();
	Mat img2 = img;//Mat img2 = img.clone();

	vector<Vec3f> circles;
	HoughCircles(hough_in, circles, CV_HOUGH_GRADIENT, 40, 10, 100, 40);

	RNG rng(12345);
	Mat threshold_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	double f64Thresh = 100.f;
	double f64MaxValue = 255.f;
	threshold(hough_in, threshold_output, f64Thresh, f64MaxValue, THRESH_BINARY);

	//threshold(im1, im2, 180, COLOR_MAX, cv::THRESH_BINARY);

	findContours(threshold_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

	vector<vector<Point> > contours_poly(contours.size());
	vector<Rect> boundRect(contours.size());
	vector<Point2f>center(contours.size());
	vector<float>radius(contours.size());


	//	cout << "possible circles " << contours.size() << endl;

	if (contours.size() == 0)
		return(img2);

	for (size_t i = 0; i < contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
		boundRect[i] = boundingRect(Mat(contours_poly[i]));
		minEnclosingCircle(contours_poly[i], center[i], radius[i]);

		Scalar color = Scalar(rng.uniform(0, COLOR_MAX), rng.uniform(0, COLOR_MAX), rng.uniform(0, COLOR_MAX));

		//circle(img2, center[i], (int)radius[i], color, 5, 8, 0);
	}

	vector<float>::iterator tempo = max_element(radius.begin(), radius.end());
	float maxrad = *max_element(radius.begin(), radius.end());

	int dis = distance(radius.begin(), tempo);

	circle(img2, center[dis], (int)maxrad, { 0, 0, COLOR_MAX }, 2, 8, 0);
	//cout << "max radius is " << endl;

	return(img2);
}

Mat DetectorUtil::MorphOps(Mat img, int x)
{
	Mat dst;
	Mat element1 = getStructuringElement(MORPH_ELLIPSE, Size(x, x), Point((x - 1) / 2, (x - 1) / 2));
	Mat element2 = getStructuringElement(MORPH_ELLIPSE, Size(x - 2, x - 2), Point((x - 3) / 2, (x - 3) / 2));

	dilate(img, dst, element1);
	erode(img, dst, element1);

	erode(img, dst, element2);
	dilate(img, dst, element2);

	return(dst);
}

Mat DetectorUtil::RescaleImage(Mat img, double factor)
{
	const int new_width = (int)((float)img.cols * factor);
	const int new_height = (int)((float)img.rows * factor);

	Size s(new_width, new_height);
	Mat img2 = Mat(s, CV_8UC3);
	resize(img, img2, s);

	return(img2);
}