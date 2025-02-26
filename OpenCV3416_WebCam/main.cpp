#include "stdafx.h"
#include "DetectorUtil.h"


int FindBall_1();
bool FindBall_2();

int FindBall_3();
int FindBall_4();

// ���� ��� ���� �ʱ�ȭ
int DetectorUtil::m_i32KernelSize = 15;
int DetectorUtil::m_i32BlockSize = 11;
int DetectorUtil::m_i32Constant = 2;

Mat DetectorUtil::m_TophatFrame;
Mat DetectorUtil::m_GrayFrame;
Mat DetectorUtil::m_BinaryFrame;

int main(int ac, char** av)
{
	int rst = 0;


	VideoCapture cap(0);
	if (!cap.isOpened()) {
		printf("[ERROR] >>>>>>>>>>>> Can't open the camera! <<<<<<<<<<<< \n");
		return -1;
	}

	Mat frame, hsv, mask, processed;

	while (true)
	{
		cap >> frame;
		if (frame.empty()) break;

		// 1. ��� �� ��� ���� (CLAHE ����)
		Mat lab;
		cvtColor(frame, lab, COLOR_BGR2Lab);
		vector<Mat> lab_channels;
		split(lab, lab_channels);
		Ptr<CLAHE> clahe = createCLAHE(2.0, Size(8, 8));
		clahe->apply(lab_channels[0], lab_channels[0]); // ��� ä���� ����ȭ
		merge(lab_channels, lab);
		cvtColor(lab, frame, COLOR_Lab2BGR);

		// 2. ���� ���͸� (HSV ��ȯ)
		cvtColor(frame, hsv, COLOR_BGR2HSV);

		// ������ ���� ���͸� (���, �����, ���λ� ��)
		Mat redMask1, redMask2, orangeMask, yellowMask, greenMask, blueMask, indigoMask, violetMask, blackMask, whiteMask, combinedMask;
		/*
		inRange(hsv, Scalar(0, 0, 180), Scalar(180, 50, 255), whiteMask);
		inRange(hsv, Scalar(20, 100, 100), Scalar(30, 255, 255), yellowMask);
		inRange(hsv, Scalar(35, 50, 50), Scalar(85, 255, 255), greenMask);
		inRange(hsv, Scalar(0, 180, 55), Scalar(80, 255, 255), blueMask);
		inRange(hsv, Scalar(100, 200, 200), Scalar(140, 255, 255), orangeMask);
		*/

		inRange(hsv, Scalar(0, 100, 100), Scalar(10, 255, 255), redMask1);
		inRange(hsv, Scalar(160, 100, 100), Scalar(179, 255, 255), redMask2);
		inRange(hsv, Scalar(11, 100, 100), Scalar(25, 255, 255), orangeMask);
		inRange(hsv, Scalar(26, 100, 100), Scalar(34, 255, 255), yellowMask);
		inRange(hsv, Scalar(35, 100, 100), Scalar(77, 255, 255), greenMask);
		inRange(hsv, Scalar(78, 100, 100), Scalar(104, 255, 255), blueMask);
		inRange(hsv, Scalar(105, 100, 100), Scalar(131, 255, 255), indigoMask);
		inRange(hsv, Scalar(132, 100, 100), Scalar(159, 255, 255), violetMask);
		inRange(hsv, Scalar(0, 0, 0), Scalar(180, 255, 50), blackMask);
		inRange(hsv, Scalar(0, 0, 180), Scalar(180, 50, 255), whiteMask);

		combinedMask = redMask1 | redMask2 | orangeMask | yellowMask | greenMask | blueMask | indigoMask | violetMask | blackMask | whiteMask;

		// 3. �߰� ���͸� (LAB ���� Ȱ���Ͽ� ���� ��ü ��ȭ)
		Mat labMask;
		Scalar lower_lab = Scalar(200, 120, 120); // ��� Ư�� ���� ����
		Scalar upper_lab = Scalar(255, 140, 140);
		inRange(lab, lower_lab, upper_lab, labMask);
		combinedMask = combinedMask | labMask;

		// 4. ���� ���� ���� (Canny)
		Mat edges;
		Canny(frame, edges, 40, 80, 3);
		Mat edgeMask;
		bitwise_and(edges, combinedMask, edgeMask);

		// 5. ������ ���� (�������� ����)
		Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5));
		morphologyEx(combinedMask, processed, MORPH_OPEN, kernel);
		morphologyEx(processed, processed, MORPH_CLOSE, kernel);

		Mat sumMat = edges | processed;
		//HoughCircles �Լ��� ����Ͽ� ���� ����.
		vector<Vec3f> circles;//���� ������ ����
		HoughCircles(sumMat, circles, CV_HOUGH_GRADIENT, 40, 1, 100, 40);

		// �ٿ�� �ڽ� ��ǥ�� ������ ����
		vector<Rect> boundingBoxes;

		for (size_t i = 0; i < circles.size(); i++)
		{
			// ���� �߽ɰ� �������� ���
			Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
			int radius = cvRound(circles[i][2]);

			// ���� ���δ� �ٿ�� �ڽ��� ��� (�ʷϻ� �׸�ڽ��� ���� ��ǥ)
			Rect boundingBox(center.x - radius, center.y - radius, radius * 2, radius * 2);
			boundingBoxes.push_back(boundingBox);
		}

		// ����� �� �ֺ��� �ʷϻ� �׸� �ڽ� �׸���
		for (const Rect& ball : boundingBoxes)
		{
			rectangle(frame, ball, Scalar(0, COLOR_MAX, 0), 2); \
		}


		//// 6. ������ ���� �� ������ ���͸�
		//vector<vector<Point>> contours;
		//vector<Vec4i> hierarchy;
		//findContours(sumMat, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

		//for (const auto& contour : contours)
		//{
		//	double area = contourArea(contour);
		//	if (area > 500)
		//	{ // ���� ũ�� �̻� ó��
		//		// ���� �� ã��
		//		Point2f center;
		//		float radius;
		//		minEnclosingCircle(contour, center, radius);

		//		// ���� ���� Ȯ�� (���� ����� ����)
		//		double perimeter = arcLength(contour, true);
		//		double circularity = 4 * M_PI * (area / (perimeter * perimeter));
		//		Rect boundingBox = boundingRect(contour);
		//		float aspectRatio = (float)boundingBox.width / boundingBox.height;

		//		if (circularity > 0.6 && aspectRatio > 0.6 && aspectRatio < 1.3)
		//		{
		//			rectangle(frame, boundingBox, Scalar(0, 255, 0), 2); // �׸� �ڽ� �׸���
		//			circle(frame, center, (int)radius, Scalar(0, 0, 255), 2); // ���� �׵θ� �߰�
		//		}
		//	}
		//}

		// ��� ���
		imshow("Golf Ball Detection", frame);
		//imshow("Mask", combinedMask);
		//imshow("Edges", edgeMask);
		//imshow("Canny", edges);
		//imshow("processed", processed);

		imshow("Sum", sumMat);

		if (waitKey(1) == 27) break; // ESC Ű�� ����
	}

	cap.release();
	destroyAllWindows();



	///////////////////////////////////////////////////////////////////////////////////////
	/*

	VideoCapture cap(0);
	if (!cap.isOpened()) {
		printf("[ERROR] >>>>>>>>>>>> Can't open the camera! <<<<<<<<<<<< \n");
		return -1;
	}

	Mat frame, hsv, mask, processed;

	// �پ��� ������ �������� �����ϱ� ���� HSV ���� ������ ����.
	// �� ���� ���� lower�� upper bound�� ����.
	std::vector<std::pair<cv::Scalar, cv::Scalar>> colorRanges;
	// �Ķ���: HSV (100,150,50) ~ (140,255,255)
	colorRanges.push_back({ cv::Scalar(100, 150, 50), cv::Scalar(140, 255, 255) });
	// ���: HSV (40,70,50) ~ (80,255,255)
	colorRanges.push_back({ cv::Scalar(40, 70, 50), cv::Scalar(80, 255, 255) });
	// ������: HSV (0,150,50) ~ (10,255,255) �� (170,150,50) ~ (180,255,255)
	colorRanges.push_back({ cv::Scalar(0, 150, 50), cv::Scalar(10, 255, 255) });
	colorRanges.push_back({ cv::Scalar(170, 150, 50), cv::Scalar(180, 255, 255) });
	// �����: HSV (20,150,50) ~ (30,255,255)
	colorRanges.push_back({ cv::Scalar(20, 150, 50), cv::Scalar(30, 255, 255) });



	while (true)
	{
		cv::Mat frame;
		cap >> frame;
		if (frame.empty()) break;

		// �Է� ������ HSV ���������� ��ȯ.
		cv::Mat hsv;
		cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

		// �� ���� �������� ����ũ ���� ��  ����.
		cv::Mat maskTotal = cv::Mat::zeros(frame.size(), CV_8UC1);
		for (auto& range : colorRanges)
		{
			cv::Mat mask;
			cv::inRange(hsv, range.first, range.second, mask);
			cv::bitwise_or(maskTotal, mask, maskTotal);
		}

		// ������ ���Ÿ� ���� �������� ����(Open & Close) ����.
		cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
		cv::morphologyEx(maskTotal, maskTotal, cv::MORPH_OPEN, kernel);
		cv::morphologyEx(maskTotal, maskTotal, cv::MORPH_CLOSE, kernel);

		// ����ũ���� ������ ����.
		std::vector<std::vector<cv::Point>> contours;
		cv::findContours(maskTotal, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		// �� �������� ������ ���� �Ӱ�ġ �̻��� ��쿡 ���� ��� �簢�� �׸���.
		for (auto& contour : contours)
		{
			if (cv::contourArea(contour) > 100)
			{ // �ʿ信 ���� ���� �Ӱ�ġ�� ����
				cv::Rect boundingBox = cv::boundingRect(contour);
				cv::rectangle(frame, boundingBox, cv::Scalar(0, 255, 0), 2);
			}
		}

		// ��� ������ ȭ�鿡 ����մϴ�.
		cv::imshow("Detected Golf Balls", frame);
		cv::imshow("Mask", maskTotal);

		// Ű �Է��� ������ �����մϴ�.
		if (cv::waitKey(30) >= 0) break;
	}

	cap.release();
	destroyAllWindows();

	 **/


	 ///////////////////////////////////////////////////////////////////////////////////////

/*
	VideoCapture cap(0); // ��ķ ���� (0�� ī�޶�)

	if (!cap.isOpened())
	{
		printf("[ERROR] Can't open the camera! \n");
		return -1;
	}

	int capW = cvRound(cap.get(CAP_PROP_FRAME_WIDTH));
	int capH = cvRound(cap.get(CAP_PROP_FRAME_HEIGHT));

	bool bFinish = false;
	while (!bFinish)
	{
		Mat originFrame;
		cap >> originFrame;

		if (originFrame.empty())
		{
			printf("[ERROR] OriginFrame is Empty! \n");
			return -1;
		}

		// 1. BGR �� Grayscale ��ȯ
		cvtColor(originFrame, DetectorUtil::m_GrayFrame, COLOR_BGR2GRAY);

		// 2. Ʈ���� �����Ͽ� Ŀ�� ũ�� ����
		namedWindow("Top-Hat");
		createTrackbar("Kernel Size", "Top-Hat", &DetectorUtil::m_i32KernelSize, 30, DetectorUtil::OnUpdateTopHat);

		// �ʱ� ���� ����
		DetectorUtil::OnUpdateTopHat(0, 0);





		// Ʈ���� ����
		namedWindow("Adaptive Threshold");
		createTrackbar("Block Size", "Adaptive Threshold", &DetectorUtil::m_i32BlockSize, 50, DetectorUtil::OnUpdateAdaptiveThreshold);
		createTrackbar("C Value", "Adaptive Threshold", &DetectorUtil::m_i32Constant, 20, DetectorUtil::OnUpdateAdaptiveThreshold);

		// �ʱ� ����
		DetectorUtil::OnUpdateAdaptiveThreshold(0, 0);



		bFinish = (waitKey(1) == 27) ? true : false;
	}


	cap.release();
	destroyAllWindows();
 */

 /////////////////////////////////////////////////////////////////////////////////////////////////////

 //FindBall_1();
 //i32Rst = FindBall_2() ? 0 : -1;
 //i32Rst = FindBall_3() ? 0 : -1;

 //DetectorUtil* pDetector = new DetectorUtil(); // �����ͷ� ��ü ����

 //if (!pDetector)
 //{
 //	printf("[ ERROR ] DetectorUtil is NULL!!! \n");
 //	return -1;
 //}

 //pDetector->Run(); //����� ����
 //pDetector->ProcessMorphGaussianHough(); // �׸��� ����
 //pDetector->CustomProcess(); //������׷� ��Ȱȭ
 //pDetector->CustomAdaptiveProcess(); //���Ƽ�� �Ӱ谪 ����
 //pDetector->FindCandidateArea(); //���Ƽ�� �Ӱ谪 ����

 //delete pDetector; // �޸� ����

	return rst;
}

String getDescription(const Point2f& centers)
{
	string temp = static_cast<string>("Ball center: ") + to_string(static_cast<int>(centers.x)) + static_cast<string>(", ") + to_string((static_cast<int>(centers.y)));
	return static_cast<String>(temp);
}

int FindBall_4()
{
	int area;
	Mat src, srcCopy, blured, imageHSV, mask, outputImage;
	vector<vector<Point>> contours;

	const Scalar greenColor = Scalar(0, 255, 0);
	const Scalar turquoiseColor = Scalar(255, 255, 0);
	const int hmin = 0, hmax = 12;
	const int smin = 126, smax = 255;
	const int vmin = 1, vmax = 203;
	const Scalar lower(hmin, smin, vmin);
	const Scalar upper(hmax, smax, vmax);
	const int minBallArea = 50000;


	// Create Window
	String figureName = "Preview";
	namedWindow(figureName, WINDOW_NORMAL);
	resizeWindow(figureName, 600, 600);

	VideoCapture cap(0);
	if (!cap.isOpened()) {
		cerr << "��ķ�� �� �� �����ϴ�!" << endl;
		return -1;
	}

	while (true)
	{
		// ������ ��������
		cap >> src;
		if (src.empty()) break;

		srcCopy = src.clone();

		// Get contours
		GaussianBlur(src, blured, Size(11, 11), 11, 11);
		cvtColor(blured, imageHSV, COLOR_BGR2HSV);
		inRange(imageHSV, lower, upper, mask);
		erode(mask, mask, getStructuringElement(MORPH_RECT, Size(5, 5)));
		dilate(mask, mask, getStructuringElement(MORPH_RECT, Size(10, 10)));
		findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
		vector<Point2f> centers(contours.size()); // inits in each loop but with correct size, no need to recopy the array
		vector<float> radius(contours.size());

		for (int i = 0; i < contours.size(); i++)
		{
			// Filter out too small objects (noise)
			area = contourArea(contours[i]);
			if (area < minBallArea)
				continue;

			// Draw on image
			minEnclosingCircle(contours[i], centers[i], radius[i]);
			circle(srcCopy, centers[i], (int)radius[i], greenColor, 3);
			putText(srcCopy, getDescription(centers[i]), Point(40, 40), FONT_HERSHEY_PLAIN, 3, turquoiseColor, 3);
			line(srcCopy, Point(centers[i].x - 20, centers[i].y), Point(centers[i].x + 20, centers[i].y), turquoiseColor, 3);
			line(srcCopy, Point(centers[i].x, centers[i].y - 20), Point(centers[i].x, centers[i].y + 20), turquoiseColor, 3);
		}
		contours.clear();

		// Combine images and print
		vector<Mat> matrices = { src, srcCopy };
		hconcat(matrices, outputImage);
		imshow(figureName, outputImage);

		// ESC Ű ������ ����
		if (waitKey(1) == 27) break;
	}

	cap.release();
	destroyAllWindows();

	return 0;
}

int FindBall_3()
{
	VideoCapture cap(0); // ��ķ ���� (0�� ī�޶�)

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

		// 1. BGR �� Grayscale
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
	VideoCapture cap(0); // ��ķ ���� (0�� ī�޶�)

	if (!cap.isOpened())
	{
		printf("[ERROR] >>>>>>>>>>>> Can't open the camera! <<<<<<<<<<<< \n");
		return -1;
	}


	while (true)
	{
		Mat frame;
		cap >> frame; // ������ �б�

		if (frame.empty())
		{
			printf("[ERROR] >>>>>>>>>>>> Frame is empty! <<<<<<<<<<<< \n");
			break;
		}

		// ��� ��ȯ
		Mat grayFrame;
		cvtColor(frame, grayFrame, COLOR_BGR2GRAY);
		GaussianBlur(grayFrame, grayFrame, Size(7, 7), 0, 0);

		/*threshold(grayFrame, grayFrame, (double)180.0, 255.0, THRESH_BINARY);
		Mat kernel2 = getStructuringElement(MORPH_RECT, Size(3, 3));
		erode(grayFrame, grayFrame, kernel2);
		dilate(grayFrame, grayFrame, kernel2, Point(-1, -1), 4);

		imshow("SearchTopBinary", grayFrame);*/

		// HoughCircles�� �̿��� �� ����
		vector<Vec3f> circles;
		HoughCircles(grayFrame, circles, HOUGH_GRADIENT, 1, grayFrame.rows / 8, 100, 30, 10, 50);

		// ����� ���� �����ӿ� ǥ��
		for (size_t i = 0; i < circles.size(); i++)
		{
			Vec3i c = circles[i];
			Point center(c[0], c[1]);
			int radius = c[2];

			// �߽� ǥ��
			circle(frame, center, 3, Scalar(0, 255, 0), -1);
			// �� �׵θ� ǥ��
			circle(frame, center, radius, Scalar(0, 0, 255), 3);
		}

		// ��� ���
		imshow("Golf Ball Detection", frame);


		// 'q' Ű�� ������ ����
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
	VideoCapture vCapture(0);//0�� ī�޶�
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

		// 1. Convert BGR to HSV   //���� ���͸� (HSV ��ȯ �� ��� ����ũ ����)
		Mat hsv;
		cvtColor(frame, hsv, COLOR_BGR2HSV);

		//2. ��� ���� ����
		Scalar lower_white(0, 0, 180);
		Scalar upper_white(180, 50, 255);
		Mat mask;
		inRange(hsv, lower_white, upper_white, mask); // ��� ���� ����

		// 2. �������� ���� (������ ����)
		Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));// ���� Ŀ�� (������ �׸��� ����)
		//Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(25, 25));// ū Ŀ�� (���� �׸��� ����, ū ��ü ����)

		Mat processed;
		morphologyEx(mask, processed, MORPH_OPEN, kernel);
		morphologyEx(processed, processed, MORPH_CLOSE, kernel);

		// 3. ������ ����
		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;
		findContours(processed, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

		for (const auto& contour : contours)
		{
			double area = contourArea(contour);
			if (area > 500) { // ���� ũ�� �̻� ó��
				// ���� �� ã��
				Point2f center;
				float radius;
				minEnclosingCircle(contour, center, radius);

				// ���� ���� Ȯ�� (���� ����� ����)
				double perimeter = arcLength(contour, true);
				double circularity = 4 * M_PI * (area / (perimeter * perimeter));
				if (circularity > 0.7) { // �������� ���� ���� ó��
					Rect boundingBox(center.x - radius, center.y - radius, radius * 2, radius * 2);
					rectangle(frame, boundingBox, Scalar(0, 255, 0), 2); // �׸� �ڽ� �׸���
				}
			}
		}

		// ��� ���
		imshow(MAIN_FRAME, frame);

		// ������ ����ũ Ȯ��
		imshow(DEBUG_FRAME, mask);

		if (waitKey(1) == 27)
		{
			printf("success, exit program");
			destroyWindow(MAIN_FRAME);
			destroyWindow(DEBUG_FRAME);

			break; // ESC Ű�� ����
		}
	}

	//free
	vCapture.release();
	destroyAllWindows();

	return true;
}