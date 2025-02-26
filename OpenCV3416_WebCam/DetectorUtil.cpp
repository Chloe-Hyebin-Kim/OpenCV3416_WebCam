#include "DetectorUtil.h"
//#include <omp.h>

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

void DetectorUtil::OnUpdateAdaptiveThreshold(int, void*)
{
	// blockSize가 짝수 일 때 홀수로 설정
	if (m_i32BlockSize % 2 == 0)
		m_i32BlockSize += 1;

	//blockSize너무 작으면 3으로 설정
	if (m_i32BlockSize < 3)
		m_i32BlockSize = 3;

	//  blockSize가 입력 이미지 크기보다 크면 조정
	int maxBlockSize = min(m_GrayFrame.rows, m_GrayFrame.cols);
	if (m_i32BlockSize > maxBlockSize)
		m_i32BlockSize = maxBlockSize | 1; // 홀수 유지

	adaptiveThreshold(m_GrayFrame, m_BinaryFrame, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, m_i32BlockSize, m_i32Constant);
	imshow("Adaptive Threshold", m_BinaryFrame);
}

void DetectorUtil::OnUpdateTopHat(int, void*)
{
	Mat kernel = getStructuringElement(MORPH_RECT, Size(m_i32KernelSize, m_i32KernelSize));
	morphologyEx(m_GrayFrame, m_TophatFrame, MORPH_TOPHAT, kernel);
	imshow("Top-Hat", m_TophatFrame);
}

void DetectorUtil::SimpleShow()
{
	if (!IsCameraOpened())
		return;



	bool bFinish = false;
	while (!bFinish)
	{
		(*m_pCapture) >> (*m_pOriginFrame);

		imshow(MAIN_FRAME, *m_pOriginFrame);

		bFinish = (waitKey(1) == 27) ? true : false;
	}



	//bool bLoop = true;
	//do
	//{
	//	(*m_pCapture) >> (*m_pOriginFrame);

	//	imshow(MAIN_FRAME, *m_pOriginFrame);

	//	bLoop = ESCKeyUser();
	//} while (bLoop);



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
	bool bFinish = false;


	//bFinish = (waitKey(1) == 27) ? true : false;
	if (waitKey(1) == 27)
	{
		printf("***************************** [ EXIT PROGRAM ] ***************************** \n");
		bFinish = true;
	}
	else
	{
		bFinish = false;
	}
	return bFinish;


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
	Mat processFrame = m_pOriginFrame->clone();

	// 1. BGR → Grayscale 변환 (HSV 변환 X)
	Mat grayFrame;
	cvtColor(processFrame, grayFrame, COLOR_BGR2GRAY);


	// 2. 그림자 제거 (Top-Hat 변환)
	Mat tophatFrame;
	Mat kernel = getStructuringElement(MORPH_RECT, Size(15, 15)); // 커널 크기 조정 가능
	morphologyEx(grayFrame, tophatFrame, MORPH_TOPHAT, kernel);// 배경보다 더 밝은 객체 강조




	// 3. 어댑티브 임계값 적용
	Mat binaryFrame;
	adaptiveThreshold(grayFrame, binaryFrame, COLOR_MAX, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 11, 2);//임계값을 계산할 블록 크기(11 : 값이 크면 넓은 영역을 고려), 계산된 임계값에서 조정하는 보정값(2 : 값이 크면 어두워짐)

	// 4. 노이즈 제거 (Morphological 연산)
	kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
	morphologyEx(binaryFrame, binaryFrame, MORPH_CLOSE, kernel);  // 작은 구멍을 메워 골프공이 깨지는 것을 방지
	morphologyEx(binaryFrame, binaryFrame, MORPH_OPEN, kernel);   // 작은 노이즈 제거
	return binaryFrame;



	/////////////////////////////////////////////////////

		//// 1. HSV 변환
		//Mat hsv;
		//cvtColor(*m_pOriginFrame, hsv, COLOR_BGR2HSV);

		//// 2. 색상 필터링 (흰색 영역 검출)
		//Mat mask;
		//inRange(hsv, Scalar(0, 0, 180), Scalar(180, 50, COLOR_MAX), mask); // 흰색 영역 검출

		//// 3. 모폴로지 연산 (노이즈 제거)
		//Mat processed; // 모폴로지 연산 후 결과 프레임
		//Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5)); // 모폴로지 연산용 커널 프레임
		//morphologyEx(mask, processed, MORPH_OPEN, kernel);
		//morphologyEx(processed, processed, MORPH_CLOSE, kernel);

		//return processed;
}

vector<Rect> DetectorUtil::DetectBalls(const Mat& mask)
{
	//1. 윤곽선 검출
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(mask, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	vector<Rect> ballBoxes;	// 바운딩 박스 좌표를 저장할 벡터
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
	/*Mat imgSrc = m_pOriginFrame->clone();
	Rect rtImage(0, 0, imgSrc.cols, imgSrc.rows);
	Rect FindROI = Rect(0, 0, 2048, 660);//Rect(0,0,1650,660);

	if (FindROI != (FindROI & rtImage))
	{
		printf("[ ERROR ] [DetectorUtil::FindCandidateArea()] ROI Bound error. \n");
		return;
	}

	Mat ROIArea = imgSrc(FindROI).clone();

	threshold(ROIArea, ROIArea, (double)ReadyDat.nAvr, COLOR_MAX.0, THRESH_BINARY);

	vector<vector<Point>> contour;
	findContours(ROIArea, contour, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
	nBlob = contour.size();*/


	if (!IsCameraOpened())
		return;
	bool bLoop = true;
	do
	{
		(*m_pCapture) >> (*m_pOriginFrame);

		if (m_pOriginFrame->empty())
		{
			printf("[ERROR] [DetectorUtil::FindCandidateArea()] m_pOriginFrame is Empty! \n");
			return;
		}
		Mat BallROI = m_pOriginFrame->clone();

		CirInfo circle;
		FindCircle(*m_pOriginFrame, 235, circle);


		bLoop = ESCKeyUser();
	} while (bLoop);
}



bool DetectorUtil::HistogramStretch(Mat& imgSrc, Mat& imgDst)
{
	if (imgSrc.empty())
	{
		printf("[ERROR] [DetectorUtil::HistogramStretch()] m_pOriginFrame is Empty! \n");
		return false;
	}

	imgSrc = (*m_pOriginFrame).clone();

	int nHeight = imgSrc.rows;
	int nWidth = imgSrc.cols;

	double dMax, dMin;
	double dSub;

	unsigned char* pPos;

	int nHist[256];
	memset(nHist, 0, sizeof(int) * 256);

	for (int i = 0; i < nHeight; ++i)
	{
		pPos = (unsigned char*)(imgSrc.row(i).data);
		for (int j = 0; j < nWidth; ++j)
		{
			nHist[pPos[j]] += 1;
		}
	}




	double dBottomR = 15;

	int nNumBottom = (int)((double)(nHeight * nWidth) * (dBottomR / 100.0));
	int nBoAccum = 0;
	int nBoIdx = 0;
	for (int i = 0; i < 256; ++i)
	{
		if (nBoAccum < nNumBottom)
		{
			nBoAccum += nHist[i];
			nBoIdx = i;
		}
	}

	if (nBoIdx == 0)
		nBoIdx = 1;

	nBoAccum = 0;
	int nNumBo = 0;
	for (int i = 0; i < nBoIdx; ++i)
	{
		nBoAccum += nHist[i] * i;
		nNumBo += nHist[i];
	}

	dMin = (double)nBoAccum / (double)nNumBo;

	dMax = dMin + 20.0;
	dSub = dMax - dMin;

	int nVal;
	for (int i = 0; i < nHeight; ++i)
	{
		unsigned char* pData1 = (unsigned char*)imgSrc.row(i).data;
		unsigned char* pData2 = (unsigned char*)imgDst.row(i).data;
		for (int j = 0; j < nWidth; ++j)
		{
			nVal = (int)pData1[j];
			nVal = (int)((((double)nVal - (double)dMin) / (double)dSub) * 255);

			if (nVal > 255) nVal = 255;
			else if (nVal < 0) nVal = 0;

			pData2[j] = (unsigned char)nVal;
		}
	}

	return true;
}

void DetectorUtil::FindCircle(Mat imgSrc, unsigned char Thres, CirInfo& circle)
{
	Mat EdgeImg = m_pOriginFrame->clone();
	Mat CloneImg;
	Mat EdgeImgClone;
	Mat Contour;


	Canny(EdgeImg, EdgeImgClone, 40, 80, 3);
	threshold(*m_pOriginFrame, Contour, (double)235, 255.0, THRESH_BINARY);

	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));

	erode(Contour, CloneImg, kernel);
	dilate(CloneImg, Contour, kernel, Point(-1, -1), 4);

	bitwise_and(EdgeImgClone, Contour, EdgeImg);

	int nEdgeCnt = 0;
	unsigned char* pPos;
	for (int i = 0; i < EdgeImg.rows; ++i)
	{
		pPos = (unsigned char*)(EdgeImg.row(i).data);
		for (int j = 0; j < EdgeImg.cols; ++j)
		{
			if (pPos[j] == 255)
			{
				++nEdgeCnt;
			}
		}
	}

	if (nEdgeCnt == 0)
	{
		printf("[ERROR] [DetectorUtil::ProcessFrame(Mat& outputFrame)] nEdgeCnt is Zero! \n");
		return;
	}

	vector<Point> pEdgePos(nEdgeCnt);
	memset(pEdgePos.data(), 0x0, sizeof(Point) * nEdgeCnt);

	nEdgeCnt = 0;
	for (int i = 0; i < EdgeImg.rows; ++i)
	{
		pPos = (unsigned char*)(EdgeImg.row(i).data);
		for (int j = 0; j < EdgeImg.cols; ++j)
		{
			if (pPos[j] == 255)
			{
				pEdgePos[nEdgeCnt] = Point(j, i);
				++nEdgeCnt;
			}
		}
	}


	int nHeightTmp = (m_pOriginFrame->rows - 2 * MINBALLRADIUS);
	int nWidthTmp = (m_pOriginFrame->cols - 2 * MINBALLRADIUS);

	if ((nHeightTmp < 1) || (nWidthTmp < 1))
	{
		return;
	}

	int nCenNum = nHeightTmp * nWidthTmp;

	if (nCenNum < 1)
	{
		return;
	}


	vector<Point> CenArray(nCenNum);

	int cnt = 0;

	for (int y = MINBALLRADIUS; y < m_pOriginFrame->rows - MINBALLRADIUS; ++y)
	{
		for (int x = MINBALLRADIUS; x < m_pOriginFrame->cols - MINBALLRADIUS; ++x)
		{
			CenArray[cnt].x = x;
			CenArray[cnt].y = y;
			++cnt;
		}
	}
	int nMaxAccumIdx = 0;
	int nMaxAccumRadius = 0;
	int nRadiusMax = 0;
	int nMaxR = MAXBALLRADIUS * MAXBALLRADIUS - 1;
	int nMinR = MINBALLRADIUS * MINBALLRADIUS;
	int nArrayIdx = 0;
	int nMaxIdx = 0;

	CirInfo cir;

	vector<CirInfo> CirInfoArray(nCenNum);

	for (int nCenIdx = 0; nCenIdx < nCenNum; nCenIdx += 1)
	{
		int nEdgeCnt1;
		vector<Point> pEdgePos1(nEdgeCnt);
		Point CenArray1;
		CirInfo Info;

		memcpy(pEdgePos1.data(), pEdgePos.data(), sizeof(Point) * nEdgeCnt);
		CenArray1 = CenArray[nCenIdx];
		nEdgeCnt1 = nEdgeCnt;

		Info = CalcRadiusCenter(pEdgePos1, CenArray1, nEdgeCnt1);

		CirInfoArray[nCenIdx] = Info;
	}

	for (int nCenIdx = 0; nCenIdx < nCenNum; nCenIdx += 1)
	{
		if (nRadiusMax < CirInfoArray[nCenIdx].AccumPt)
		{
			nMaxIdx = nCenIdx;
			nRadiusMax = CirInfoArray[nCenIdx].AccumPt;
		}
	}

	cir.radius = CirInfoArray[nMaxIdx].radius;
	cir.center.x = CirInfoArray[nMaxIdx].center.x;
	cir.center.y = CirInfoArray[nMaxIdx].center.y;
	cir.AccumPt = CirInfoArray[nMaxIdx].AccumPt;

	CirInfof CirInfoArrayf[900];
	Point2f Centerf[900];

	memset(CirInfoArrayf, 0x0, sizeof(CirInfof) * 900);
	memset(Centerf, 0x0, sizeof(Point2f) * 900);
	int nCnt = 0;

	for (int i = 0; i < 30; ++i)
	{
		for (int j = 0; j < 30; ++j)
		{
			Centerf[nCnt] = Point2f(((float)cir.center.x - 1.f) + 0.1f * j, ((float)cir.center.y - 1.f) + 0.1f * i);
			nCnt++;
		}
	}

	for (int nCenIdx = 0; nCenIdx < 900; nCenIdx += 1)
	{
		int Idx;
		int nEdgeCnt1;
		int CirRadius;
		vector<Point> pEdgePos1(nEdgeCnt);
		Point2f CenArray1;
		CirInfof Info;

		memcpy(pEdgePos1.data(), pEdgePos.data(), sizeof(Point) * nEdgeCnt);
		CenArray1 = Centerf[nCenIdx];
		nEdgeCnt1 = nEdgeCnt;
		CirRadius = cir.radius;

		Info = CalcRadiusCenterSub(pEdgePos1, CenArray1, CirRadius, nEdgeCnt1);

		CirInfoArrayf[nCenIdx] = Info;
	}

	nRadiusMax = 0;
	nMaxIdx = 0;

	for (int nCenIdx = 0; nCenIdx < 900; nCenIdx += 1)
	{
		if (nRadiusMax < CirInfoArrayf[nCenIdx].AccumPt)
		{
			nMaxIdx = nCenIdx;
			nRadiusMax = CirInfoArrayf[nCenIdx].AccumPt;
		}
	}

	cir.radius = CirInfoArrayf[nMaxIdx].radius;
	cir.center.x = CirInfoArrayf[nMaxIdx].center.x;
	cir.center.y = CirInfoArrayf[nMaxIdx].center.y;
	cir.AccumPt = CirInfoArrayf[nMaxIdx].AccumPt;


	if (cir.AccumPt < 15)
	{
		return;
	}
}

CirInfo DetectorUtil::CalcRadiusCenter(vector<Point>& Contour, Point pt1, int nNum)
{
	int nDist;
	int nMaxR = MAXBALLRADIUS * MAXBALLRADIUS;
	int nMinR = MINBALLRADIUS * MINBALLRADIUS;
	int nDistArray[MAXBALLRADIUS + 1];
	int nMaxIdx = 0;

	memset(nDistArray, 0, sizeof(int) * (MAXBALLRADIUS + 1));
	int nArrayIdx = 0;
	int nMaxAccumIdx = 0;
	int nMaxAccumRadius = 0;
	CirInfo Circle;
	Circle.AccumPt = 0;
	Circle.center.x = 0;
	Circle.center.y = 0;
	Circle.radius = 0;

	int nTmpValX = 0;
	int nTmpValY = 0;

	for (int nConIdx = 0; nConIdx < nNum; nConIdx += 3)
	{
		nTmpValX = Contour[nConIdx].x - pt1.x;
		nTmpValY = Contour[nConIdx].y - pt1.y;

		nDist = nTmpValX * nTmpValX + nTmpValY * nTmpValY;

		if ((nDist < nMaxR) && (nDist > nMinR))
		{
			nArrayIdx = cvRound(sqrt((float)nDist));
			if (nArrayIdx < MAXBALLRADIUS)
			{
				nDistArray[nArrayIdx - 1] += 1;
				nDistArray[nArrayIdx] += 1;
				nDistArray[nArrayIdx + 1] += 1;
			}
		}
	}

	for (int Idx = 0; Idx < MAXBALLRADIUS + 1; ++Idx)
	{
		if (nMaxAccumIdx < nDistArray[Idx])
		{
			nMaxAccumIdx = nDistArray[Idx];
			nMaxAccumRadius = Idx;
			Circle.AccumPt = nMaxAccumIdx;
			Circle.center.x = pt1.x;
			Circle.center.y = pt1.y;
			Circle.radius = nMaxAccumRadius;
		}
	}

	return Circle;
}

CirInfof DetectorUtil::CalcRadiusCenterSub(vector<Point> Contour, Point2f pt1, float nRadius, int nNum)
{
	double dDist;
	double dMaxR = (nRadius + 2.f) * (nRadius + 2.f);
	double dMinR = (nRadius - 2.f) * (nRadius - 2.f);
	int nDistArray[300];
	memset(nDistArray, 0, sizeof(int) * (300));
	int nArrayIdx = 0;
	int nMaxAccumIdx = 0;
	int nMaxAccumRadius = 0;
	CirInfof Circle;
	Circle.AccumPt = 0;
	Circle.center.x = 0.f;
	Circle.center.y = 0.f;
	Circle.radius = 0.f;


	for (int nConIdx = 0; nConIdx < nNum; nConIdx += 3)
	{
		dDist = ((float)Contour[nConIdx].x - pt1.x) * ((float)Contour[nConIdx].x - pt1.x) + ((float)Contour[nConIdx].y - pt1.y) * ((float)Contour[nConIdx].y - pt1.y);

		if ((dDist < dMaxR) && (dDist > dMinR))
		{
			nArrayIdx = cvRound((sqrt(dDist) - nRadius) * 10.f) + 150;
			if (nArrayIdx < 299)
			{
				nDistArray[nArrayIdx - 1] += 1;
				nDistArray[nArrayIdx] += 1;
				nDistArray[nArrayIdx + 1] += 1;
			}
		}
	}

	for (int Idx = 0; Idx < 300; ++Idx)
	{
		if (nMaxAccumIdx < nDistArray[Idx])
		{
			nMaxAccumIdx = nDistArray[Idx];
			nMaxAccumRadius = Idx;
			Circle.AccumPt = nMaxAccumIdx;
			Circle.center.x = pt1.x;
			Circle.center.y = pt1.y;
			Circle.radius = ((float)(nMaxAccumRadius - 150) / 10.f + nRadius);
		}
	}

	return Circle;
}

int DetectorUtil::CustomProcess0()
{
	VideoCapture cap(0);

	if (!cap.isOpened())
	{
		printf("[ERROR] >>>>>>>>>>>> Can't open the camera! <<<<<<<<<<<< \n");
		return -1;
	}

	while (true)
	{
		Mat frame, hsv, mask, processed;
		cap >> frame;
		if (frame.empty()) break;

		// 1. 색상 필터링 (HSV 변환 후 흰색 마스크 생성)
		cvtColor(frame, hsv, COLOR_BGR2HSV);
		inRange(hsv, Scalar(0, 0, 180), Scalar(180, 50, 255), mask); // 흰색 범위 설정

		// 2. 모폴로지 연산 (노이즈 제거)
		Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
		morphologyEx(mask, processed, MORPH_OPEN, kernel);
		morphologyEx(processed, processed, MORPH_CLOSE, kernel);

		// 3. 윤곽선 검출
		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;
		findContours(processed, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

		for (const auto& contour : contours)
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

				if (circularity > 0.7)// 원형성이 높을 때만 처리
				{
					Rect boundingBox(center.x - radius, center.y - radius, radius * 2, radius * 2);
					rectangle(frame, boundingBox, Scalar(0, 255, 0), 2); // 네모 박스 그리기
				}
			}
		}

		// 결과 출력
		imshow("Golf Ball Detection", frame);
		imshow("Mask", mask); // 디버깅용 마스크 확인

		if (waitKey(1) == 27) break; // ESC 키로 종료
	}

	cap.release();
	destroyAllWindows();



	return 0;
}

int DetectorUtil::CustomProcess2()
{
	VideoCapture cap(0);

	if (!cap.isOpened()) // if not success, exit program
	{
		printf("[ERROR] >>>>>>>>>>>> Can't open the camera! <<<<<<<<<<<< \n");

		cap.release();
		destroyAllWindows();

		return false;
	}

	Mat src, srcGray, blurred, canny, thresh, hsv, filtered, cont, done;
	vector< vector<Point> > contours;
	vector< vector<Point> > ThreshContours;
	vector<Vec4i> heirarchy;

	while (true)
	{
		cap >> src;
		resize(src, src, Size(640, 480));

		//cvtColor(src,src,CV_BGR2GRAY);
		//blur(src,blurred,Size(49,51));
		//threshold(blurred,thresh,150,255,CV_THRESH_BINARY);
		//filtered = thresh;

		//Uses thresh or filtering
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

		findContours(filtered, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
		cont = Mat::zeros(filtered.size(), CV_8UC1);
		drawContours(cont, contours, -1, Scalar(255, 255, 255), 1);

		vector< vector<Point> > contours_poly(contours.size());
		vector<Rect> boundRect(contours.size());
		vector<Point2f> center(contours.size());
		vector<float> radius(contours.size());

		for (int i = 0; i < contours.size(); i++)
		{
			approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
			boundRect[i] = boundingRect(Mat(contours_poly[i]));
			minEnclosingCircle((Mat)contours_poly[i], center[i], radius[i]);
		}

		string text;
		for (int i = 0; i < contours.size(); i++)
		{
			if (contourArea(contours[i]) > 650)
				rectangle(src, boundRect[i].tl(), boundRect[i].br(), Scalar(255, 255, 255), 2);
			text = to_string(i);
		}

		int circles = contours.size() - 1;


		//show original
		namedWindow("Original");
		imshow("Original", src);

		//show HSV
		//namedWindow("HSV");
		//imshow("HSV",hsv);

		//show blurred
	   // namedWindow("blurred");
		//imshow("blurred",blurred);

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

		// ESC 키 누르면 종료
		if (waitKey(1) == 27) break;
	}

	cap.release();
	destroyAllWindows();

	return 0;
}



bool DetectorUtil::CustomAdaptiveProcess()
{
	bool rst = false;

	if (!IsCameraOpened())
		return rst;

	bool bLoop = true;
	do
	{
		(*m_pCapture) >> (*m_pOriginFrame);

		if (m_pOriginFrame->empty())
		{
			printf("[ERROR] [DetectorUtil::ProcessFrame(Mat& outputFrame)] m_pOriginFrame is Empty! \n");
			rst = false;
			bLoop = false;
			break;
		}

		rst = true;

		Mat processFrame = m_pOriginFrame->clone();

		// 1. BGR → Grayscale 변환 (HSV 변환 X)
		Mat grayFrame;
		cvtColor(processFrame, grayFrame, COLOR_BGR2GRAY);

		// 2. 그림자 제거 (Top-Hat 변환)
		Mat tophatFrame;
		Mat kernel = getStructuringElement(MORPH_RECT, Size(15, 15)); // 커널 크기 조정 가능
		morphologyEx(grayFrame, tophatFrame, MORPH_TOPHAT, kernel);// 배경보다 더 밝은 객체 강조

		// 3. 어댑티브 임계값 적용
		Mat binaryFrame;
		adaptiveThreshold(grayFrame, binaryFrame, COLOR_MAX, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 11, 2);//임계값을 계산할 블록 크기(11 : 값이 크면 넓은 영역을 고려), 계산된 임계값에서 조정하는 보정값(2 : 값이 크면 어두워짐)

		// 4. 노이즈 제거 (Morphological 연산)
		kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
		morphologyEx(binaryFrame, binaryFrame, MORPH_CLOSE, kernel);  // 작은 구멍을 메워 골프공이 깨지는 것을 방지
		morphologyEx(binaryFrame, binaryFrame, MORPH_OPEN, kernel);   // 작은 노이즈 제거


		//5. 결과 출력

		//// 결과를 표시할 컬러 이미지로 변환
		//Mat outFrame;
		//cvtColor(binaryFrame, outFrame, COLOR_GRAY2BGR);

		//// 원을 검출할 수 있도록 가우시안 블러 적용
		//Mat blurred;
		//GaussianBlur(outFrame, blurred, Size(5, 5), 1.5);

		////HoughCircles 함수를 사용하여 원을 검출.
		//vector<Vec3f> circles;//원을 검출할 벡터
		//HoughCircles(blurred, circles, HOUGH_GRADIENT, 1, 20, 100, 30, 10, 100);

		//// 바운딩 박스 좌표를 저장할 벡터
		//vector<Rect> boundingBoxes;

		//for (size_t i = 0; i < circles.size(); i++)
		//{
		//	// 원의 중심과 반지름을 계산
		//	Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		//	int radius = cvRound(circles[i][2]);

		//	// 원을 감싸는 바운딩 박스를 계산 (초록색 네모박스를 위한 좌표)
		//	Rect boundingBox(center.x - radius, center.y - radius, radius * 2, radius * 2);
		//	boundingBoxes.push_back(boundingBox);
		//}

		//// 검출된 공 주변에 초록색 네모 박스 그리기
		//for (const Rect& ball : boundingBoxes)
		//{
		//	rectangle(outFrame, ball, Scalar(0, COLOR_MAX, 0), 2);
		//}
		//imshow(MAIN_FRAME, outFrame);

		imshow("Original", *m_pOriginFrame);
		imshow("Grayscale", grayFrame);
		imshow("Top-Hat", tophatFrame);
		imshow("Adaptive Threshold", binaryFrame);

		bLoop = ESCKeyUser();
	} while (bLoop);

	return rst;
}

bool DetectorUtil::CustomProcess()
{
	bool rst = false;

	if (!IsCameraOpened())
		return rst;



	/////////////////////////////////////////////////


	bool bLoop = true;
	do
	{
		(*m_pCapture) >> (*m_pOriginFrame);

		if (m_pOriginFrame->empty())
		{
			printf("[ERROR] [DetectorUtil::ProcessFrame(Mat& outputFrame)] m_pOriginFrame is Empty! \n");
			rst = false;
			bLoop = false;
			break;
		}

		rst = true;

		///////////////////////////////////////////////////////////////////

		//1. Read the Image and convert it to Grayscale Format
		Mat grayFrame;
		cvtColor(*m_pOriginFrame, grayFrame, COLOR_BGR2GRAY);

		//2. Apply Binary Thresholding
		Mat threshFrame;
		threshold(grayFrame, threshFrame, 230, 255, THRESH_BINARY);
		imshow("Binary mage", threshFrame);


		Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
		Mat ContourImg;
		erode(ContourImg, ContourImg, kernel);
		dilate(ContourImg, ContourImg, kernel, Point(-1, -1), 4);


		Mat copyFrame = m_pOriginFrame->clone();
		bitwise_and(copyFrame, ContourImg, copyFrame);




		////3. Find the Contours
		//vector<vector<Point>> contours;
		//vector<Vec4i> hierarchy;
		//findContours(threshFrame, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

		////4. Draw Contours on the Original RGB Image
		//Mat copyFrame = m_pOriginFrame->clone();
		//drawContours(copyFrame, contours, -1, Scalar(0, 255, 0), 2);
		//imshow("None approximation", copyFrame);

		//destroyAllWindows();

		///////////////////////////////////////////////////////////////////












		Mat processFrame = m_pOriginFrame->clone();

		// 1. BGR → HSV 변환
		Mat hsvFrame;
		cvtColor(processFrame, hsvFrame, COLOR_BGR2HSV);

		// 2. HSV 채널 분리 (H, S, V)
		vector<Mat> hsvChannels;
		split(hsvFrame, hsvChannels);

		// 3. V 채널(밝기)에 히스토그램 평활화 적용
		equalizeHist(hsvChannels[2], hsvChannels[2]);

		// 4. 채널 병합
		merge(hsvChannels, hsvFrame);

		// 5. HSV → BGR 변환 (최종 출력)
		Mat resultFrame;
		cvtColor(hsvFrame, resultFrame, COLOR_HSV2BGR);


		// 원을 검출할 수 있도록 가우시안 블러 적용
		Mat blurred;
		GaussianBlur(resultFrame, blurred, Size(5, 5), 1.5);

		//HoughCircles 함수를 사용하여 원을 검출.
		vector<Vec3f> circles;//원을 검출할 벡터
		HoughCircles(blurred, circles, HOUGH_GRADIENT, 1, 20, 100, 30, 10, 100);

		// 바운딩 박스 좌표를 저장할 벡터
		vector<Rect> boundingBoxes;
		for (size_t i = 0; i < circles.size(); i++)
		{
			// 원의 중심과 반지름을 계산
			Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
			int radius = cvRound(circles[i][2]);

			// 원을 감싸는 바운딩 박스를 계산 (초록색 네모박스를 위한 좌표)
			Rect boundingBox(center.x - radius, center.y - radius, radius * 2, radius * 2);
			boundingBoxes.push_back(boundingBox);
		}

		// 검출된 공 주변에 초록색 네모 박스 그리기
		for (const Rect& ball : boundingBoxes)
		{
			rectangle(resultFrame, ball, Scalar(0, COLOR_MAX, 0), 2);
		}

		imshow(MAIN_FRAME, resultFrame); //show the original image


		bLoop = ESCKeyUser();
	} while (bLoop);

	return rst;
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
		cvtColor(im1, im1, CV_BGR2HSV);// HSV 변환
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
	HoughCircles(hough_in, circles, CV_HOUGH_GRADIENT, 40, 1, 100, 40);

	RNG rng(12345);
	Mat threshold_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	double f64Thresh = 100.f;
	double f64MaxValue = 255.f;
	threshold(hough_in, threshold_output, f64Thresh, f64MaxValue, THRESH_BINARY);

	//threshold(im1, im2, 180, COLOR_MAX, THRESH_BINARY);

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
		approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);//다각형 근사
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



void DetectorUtil::DrawBoundingBox(Mat& targetFrame, const vector<Rect>& boundingBoxes)
{
	// 컬러 버전 이미지 생성
	//cvtColor(targetFrame, outFrame, COLOR_GRAY2BGR);

	// 검출된 공 주변에 초록색 네모 박스 그리기
	for (const Rect& ball : boundingBoxes)
	{
		//rectangle(outFrame, boundingBoxes[i], Scalar(0, COLOR_MAX, 0), 2);// 초록색 네모 박스==Scalar(0, COLOR_MAX, 0)
		rectangle(targetFrame, ball, Scalar(0, COLOR_MAX, 0), 2);
	}

	//imshow("Detected Golf Ball Area", outFrame);
}

vector<Rect> DetectorUtil::DetectBallArea(const Mat& targetFrame)
{
	// 원을 검출할 수 있도록 가우시안 블러 적용
	Mat blurred;
	GaussianBlur(targetFrame, blurred, Size(7, 7), 0, 0);


	//HoughCircles 함수를 사용하여 원을 검출.

	vector<Vec3f> circles;//원을 검출할 벡터
	HoughCircles(blurred, circles, HOUGH_GRADIENT, 1, 20, 100, 30, 10, 100);

	// 바운딩 박스 좌표를 저장할 벡터
	vector<Rect> boundingBoxes;

	for (size_t i = 0; i < circles.size(); i++)
	{
		// 원의 중심과 반지름을 계산
		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);

		// 원을 감싸는 바운딩 박스를 계산 (초록색 네모박스를 위한 좌표)
		Rect boundingBox(center.x - radius, center.y - radius, radius * 2, radius * 2);
		boundingBoxes.push_back(boundingBox);
	}

	return boundingBoxes;
}


void DetectorUtil::ShowResult(const Mat& inFrame, Mat& outFrame)
{
	//// 골프공이 있는 영역에 대한 바운딩 박스를 계산
	//vector<Rect> ballBoxes = DetectBallArea(inFrame);

	//// 결과를 표시할 컬러 이미지로 변환 (초록색 네모 박스를 시각적으로 표시하기 위해)
	//cvtColor(inFrame, outFrame, COLOR_GRAY2BGR);

	//// 바운딩 박스를 그려주기
	//DrawBoundingBox(outFrame, ballBoxes);

	//// 결과 출력
	////imshow("Detected Golf Ball Area", outFrame);
}