#include "stdafx.h"
#include "Image.h"
#include "PreProcess.h"
#include <math.h>

using namespace std;
using namespace cv;


bool compareContourAreas(vector<Point> contour1, vector<Point> contour2) {
	double i = fabs(contourArea(Mat(contour1)));
	double j = fabs(contourArea(Mat(contour2)));
	return (i > j);
}


int countWhiteSpots(Mat img) {
	Mat black_pixels;
	findNonZero(img, black_pixels);
	
	return img.total() - black_pixels.total();
}


void getCorners(Point2f inputQuad[], vector<Point2f> approx) {
	double x1 = (approx[0].x - approx[1].x)*(approx[0].x - approx[1].x);
	double y1 = (approx[0].y - approx[1].y)*(approx[0].y - approx[1].y);
	double d1 = sqrt(x1+y1);

	double x2 = (approx[1].x - approx[2].x)*(approx[1].x - approx[2].x);
	double y2 = (approx[1].y - approx[2].y)*(approx[1].y - approx[2].y);
	double d2 = sqrt(x2 + y2);

	if (d1 < d2) {
		inputQuad[0] = approx[1];
		inputQuad[1] = approx[0];
		inputQuad[2] = approx[3];
		inputQuad[3] = approx[2];
	}else{
		inputQuad[0] = approx[0];
		inputQuad[1] = approx[3];
		inputQuad[2] = approx[2];
		inputQuad[3] = approx[1];
	}
}

void findingCards(Mat card) {
	Mat diff, diff_blur, diff_thre;
	PreProcess pp1 = PreProcess(card);

	int upper = 0;
	int icard = -1;

	for (int i = 1; i < 53; i++) {
		string dir = "cards/"+ to_string(i) +".png";
		PreProcess pp2 = PreProcess(dir);

		absdiff(pp1.getProcessedImg(), pp2.getProcessedImg(), diff);
		GaussianBlur(diff, diff_blur, Size(5, 5), 5);
		threshold(diff_blur, diff_thre, 200, 255, THRESH_BINARY);
		//imshow("FINAL", diff_thre);

		int val = countWhiteSpots(diff_thre);

		if (upper < val) {
			upper = val;
			icard = i;
		}
	}
	cout << icard << endl;
}

int main()
{
	Mat gray, blur, thre, contoursConv, contours, dst;

	string dir = "";
	cout << "Dir: ";
	cin >> dir;

	//original image
	Image img = Image(dir);
	imshow("1. Original", img.getImage());

	//turn original image in an black/ white image
	cvtColor(img.getImage(), gray, COLOR_BGR2GRAY);
	imshow("2. Gray Scaled Image", gray);
	
	//gaussian blur to reduce noise and details 
	GaussianBlur(gray, blur, Size(1, 1), 1000, 0);
	imshow("3. Blur Image", blur);
	
	//color segmentation
	threshold(blur, thre, 120, 255, THRESH_BINARY);
	imshow("4. Threshold Image", thre);

	//find contours and sort them by area
	//bigger areas are in the first positions of the vector
	vector<vector<Point> > contoursVec;
	vector<Vec4i> hierarchy;
	contours = thre.clone();
	findContours(contours, contoursVec, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);
	sort(contoursVec.begin(), contoursVec.end(), compareContourAreas);
	imshow("5. Contours Image", contours);

	//homography
	//just the first 4 elements are selected because they correspond to the cards (biggest area)
	Mat lambda(2, 4, CV_32FC1);
	Point2f inputQuad[4];
	Point2f outputQuad[4];
	vector<Point2f> approx;
	for (int i = 0; i < /*contoursVec.size()*/4; i++) {
		drawContours(contours, contoursVec, i, Scalar(255, 255, 0), 4);

		lambda = Mat::zeros(img.getImage().rows, img.getImage().cols, img.getImage().type());

		double peri = arcLength(contoursVec[i], true);
		approxPolyDP(contoursVec[i], approx, 0.02*peri, true);
		
		//cout << approx[0] <<", "<< approx[1] << ", " << approx[2] << ", " << approx[3] << endl;

		getCorners(inputQuad, approx);

		outputQuad[0] = Point2f(0, 0);
		outputQuad[1] = Point2f(486, 0);
		outputQuad[2] = Point2f(486, 682);
		outputQuad[3] = Point2f(0, 682);

		lambda = getPerspectiveTransform(inputQuad, outputQuad);
		warpPerspective(img.getImage(), dst, lambda, Size(486, 682));
		imshow("6. Last " + i, dst);

		findingCards(dst);
	}

	imshow("Contours", contours);

	waitKey(0);
    return 0;
}



