#include "stdafx.h"

int main(int ac, char** av)
{
	int i32Rst = 0;

	VideoCapture vCapture(0);
	vCapture.set(CAP_PROP_FRAME_WIDTH, 1920);
	vCapture.set(CAP_PROP_FRAME_HEIGHT, 1080);

	if (!vCapture.isOpened())
	{	// isOpened() : ī�޶� ������ �ִ��� ������ (1: �ִ�, 0: ����)
		printf("Can't open the camera");

		i32Rst = -1;
	}
	else
	{
		//Mat img = imread("Lenna.png"); //�ڽ��� �����Ų �̹��� �̸��� �ԷµǾ�� ��, Ȯ���ڱ���
		//imshow("img", img);
		//waitKey(0);
		//return 0;

		Mat img;

		while (true) {
			vCapture >> img;

			imshow("camera img", img);

			if (waitKey(1) == 27)
				break;//esc Ű �Է½� ����
		}
	}

	return i32Rst;
}