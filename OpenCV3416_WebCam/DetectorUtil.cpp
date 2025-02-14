#include "DetectorUtil.h"


DetectorUtil::DetectorUtil()
{
	m_pCapture = new VideoCapture(0);// 0�� ī�޶� ���
	m_pOriginFrame = new Mat(); // ���� ������ ��ü

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
			break;//esc Ű �Է½� ����
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
			 break;//esc Ű �Է½� ����
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

	// ����� �� �ֺ��� �׸� �ڽ� �׸���
	for (const Rect& ball : balls)
	{
		rectangle(outputFrame, ball, Scalar(0, 255, 0), 2);
	}
}

Mat DetectorUtil::Preprocess()
{
	// 1. HSV ��ȯ
	Mat hsv;
	cvtColor(*m_pOriginFrame, hsv, COLOR_BGR2HSV);

	// 2. ���� ���͸� (��� ���� ����)
	Mat mask;
	inRange(hsv, Scalar(0, 0, 180), Scalar(180, 50, 255), mask); // ��� ���� ����

	// 3. �������� ���� (������ ����)
	Mat processed; // �������� ���� �� ��� ������
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5)); // �������� ����� Ŀ�� ������
	morphologyEx(mask, processed, MORPH_OPEN, kernel);
	morphologyEx(processed, processed, MORPH_CLOSE, kernel);

	return processed;
}

vector<Rect> DetectorUtil::DetectBalls(const Mat& mask)
{
	//1. ������ ����
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(mask, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	vector<Rect> ballBoxes;
	for (const vector<Point>& contour : contours)
	{
		double area = contourArea(contour);

		if (area > 500) // ���� ũ�� �̻� ó��
		{
			// ���� �� ã��
			Point2f center;
			float radius;
			minEnclosingCircle(contour, center, radius);

			// ���� ���� Ȯ�� (���� ����� ����)
			double perimeter = arcLength(contour, true);
			double circularity = 4 * M_PI * (area / (perimeter * perimeter));

			if (circularity > 0.7) // �������� ���� ���� ó��
			{
				ballBoxes.emplace_back(center.x - radius, center.y - radius, radius * 2, radius * 2);
			}
		}
	}


	return ballBoxes;
}


bool DetectorUtil::ProcessMorphGaussianHough()
{
	if (!IsCameraOpened())
		return false;

	Mat im1, im2;
	vector<Mat> bgr(3);

	bool bLoop = true;
	do
	{
		(*m_pCapture) >> im1;

		im1.copyTo(im2);
		cvtColor(im1, im1, CV_BGR2HSV);// HSV ��ȯ
		split(im1, bgr);


		vector<Mat> bgr_thresh(3);
		threshold(bgr[0], bgr_thresh[0], 26, 255, THRESH_BINARY);
		threshold(bgr[1], bgr_thresh[1], 60, 255, THRESH_BINARY);
		threshold(bgr[2], bgr_thresh[2], 140, 255, THRESH_BINARY);

		Mat bitwised;
		bitwise_and(bgr_thresh[0], bgr_thresh[1], bitwised);
		bitwise_and(bgr_thresh[2], bitwised, bitwised);


		Mat morphed = MorphOps(bitwised, 5);

		Mat blurred;
		GaussianBlur(morphed, blurred, { 9, 9 }, 1, 1);

		Mat hough = Hough2d(im2, blurred);
		imshow(MAIN_FRAME, hough);

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

	int thresh = 100;


	RNG rng(12345);
	Mat threshold_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	threshold(hough_in, threshold_output, thresh, 255, THRESH_BINARY);

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

		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));

		//circle(img2, center[i], (int)radius[i], color, 5, 8, 0);
	}

	vector<float>::iterator tempo = max_element(radius.begin(), radius.end());
	float maxrad = *max_element(radius.begin(), radius.end());

	int dis = distance(radius.begin(), tempo);

	circle(img2, center[dis], (int)maxrad, { 0, 0, 255 }, 2, 8, 0);
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