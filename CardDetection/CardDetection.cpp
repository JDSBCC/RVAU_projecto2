#include "stdafx.h"
#include "Image.h"

using namespace std;
using namespace cv;


bool compareContourAreas(vector<Point> contour1, vector<Point> contour2) {
	double i = fabs(contourArea(Mat(contour1)));
	double j = fabs(contourArea(Mat(contour2)));
	return (i > j);
}

int main()
{
	string dir = "";
	cout << "Dir: ";
	cin >> dir;

	Image img = Image(dir);
	imshow("1. Original", img.getImage());

	Mat gray, blur, thre, contoursConv, contours, dst;

	cvtColor(img.getImage(), gray, COLOR_BGR2GRAY);
	imshow("2. Gray Scaled Image", gray);
	
	GaussianBlur(gray, blur, Size(1, 1), 1000, 0);
	imshow("3. Blur Image", blur);
	
	threshold(blur, thre, 120, 255, THRESH_BINARY);
	imshow("4. Threshold Image", thre);

	vector<vector<Point> > contoursVec;
	vector<Vec4i> hierarchy;
	contours = thre.clone();
	findContours(contours, contoursVec, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
	sort(contoursVec.begin(), contoursVec.end(), compareContourAreas);
	imshow("5. Contours Image", contours);

	Mat lambda(2, 4, CV_32FC1);
	Point2f inputQuad[4];
	Point2f outputQuad[4];
	vector<Point2f> approx;
	for (int i = 0; i < /*contoursVec.size()*/4; i++) {
		drawContours(contours, contoursVec, i, Scalar(255, 255, 0), 4);

		lambda = Mat::zeros(img.getImage().rows, img.getImage().cols, img.getImage().type());

		double peri = arcLength(contoursVec[i], true);
		approxPolyDP(contoursVec[i], approx, 0.02*peri, true);
		
		cout << approx[0] <<", "<< approx[1] << ", " << approx[2] << ", " << approx[3] << endl;

		inputQuad[0] = approx[1];
		inputQuad[1] = approx[0];
		inputQuad[2] = approx[3];
		inputQuad[3] = approx[2];

		outputQuad[0] = Point2f(0, 0);
		outputQuad[1] = Point2f(486, 0);
		outputQuad[2] = Point2f(486, 682);
		outputQuad[3] = Point2f(0, 682);

		lambda = getPerspectiveTransform(inputQuad, outputQuad);
		warpPerspective(img.getImage(), dst, lambda, Size(486, 682));
		imshow("6. Last " + i, dst);
	}

	imshow("Contours", contours);
	waitKey(0);
    return 0;
}
