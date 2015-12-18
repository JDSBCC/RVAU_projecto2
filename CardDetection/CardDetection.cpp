#include "stdafx.h"
#include "Image.h"
#include "Table.h"
#include "PreProcess.h"
#include <math.h>

#include <opencv2/xfeatures2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/core/core.hpp>

using namespace std;
using namespace cv;

#define CARD_WIDTH 500 //500*726 --486*682
#define CARD_HEIGHT 726
#define MIN_AREA 50000
#define MAX_AREA 60000

int isHorizontal[4];
bool exist4Cards = true;
RNG rng(12345);

double distanceBetween2Points(int xa, int xb, int ya, int yb) {
	double x = (xa - xb)*(xa - xb);
	double y = (ya - yb)*(ya - yb);
	return sqrt(x + y);
}

void setAnotherVertex(vector<Point2f> &approx, Rect boundRect) {
	approx = vector<Point2f>(4);

	cout << "--------tl = " << boundRect.tl() << endl;
	cout << "--------br = " << boundRect.br() << endl;
	approx[0] = (Point2f)boundRect.tl();
	approx[1] = Point2f(boundRect.tl().x, boundRect.br().y);
	approx[2] = (Point2f)boundRect.br();
	approx[3] = Point2f(boundRect.br().x, boundRect.tl().y);
}

void showFinal(Mat &src1, Mat src2) {
	Mat gray, gray_inv, src1final, src2final;
	cvtColor(src2, gray, CV_BGR2GRAY);
	threshold(gray, gray, 0, 255, CV_THRESH_BINARY);
	//adaptiveThreshold(gray,gray,255,ADAPTIVE_THRESH_MEAN_C,THRESH_BINARY,5,4);
	bitwise_not(gray, gray_inv);
	src1.copyTo(src1final, gray_inv);
	src2.copyTo(src2final, gray);
	src1 = src1final + src2final;
}

void drawTextInTheMiddle(Mat &img, string text, int fontFace, double fontScale, int thickness, Scalar color) {
	int baseline = 0;
	Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
	baseline += thickness;
	Point textOrg((img.cols - textSize.width) / 2, (img.rows + textSize.height) / 2);
	putText(img, text, textOrg, fontFace, fontScale, color, thickness, 8);
	//imshow("Test", img);
}

void filterMatchesByAbsoluteValue(vector<DMatch> &matches, float maxDistance)
{
	vector<DMatch> filteredMatches;
	for (size_t i = 0; i < matches.size(); i++)
	{
		//cout << i << "-" << matches[i].distance << endl;
		if (matches[i].distance < maxDistance)
			filteredMatches.push_back(matches[i]);
	}
	matches = filteredMatches;
}

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

void getCorners(Point2f inputQuad[], vector<Point2f> approx, int it) {
	double d1 = distanceBetween2Points(approx[0].x, approx[1].x, approx[0].y, approx[1].y);
	double d2 = distanceBetween2Points(approx[1].x, approx[2].x, approx[1].y, approx[2].y);

	if (d1 < d2) {
		inputQuad[0] = approx[1];
		inputQuad[1] = approx[0];
		inputQuad[2] = approx[3];
		inputQuad[3] = approx[2];
		isHorizontal[it] = false;
	}
	else {
		inputQuad[0] = approx[0];
		inputQuad[1] = approx[3];
		inputQuad[2] = approx[2];
		inputQuad[3] = approx[1];
		isHorizontal[it] = true;
	}
}

void findingCardsDIFF(Mat card) {
	Mat diff, diff_blur, diff_thre;
	PreProcess pp1 = PreProcess(card);

	int upper = 0;
	int icard = -1;

	for (int i = 1; i < 53; i++) {
		string dir = "cards_2/" + to_string(i) + ".png";
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

void findingCardsSIFT(Mat card, int &result) {
	int bigger = 0;

	vector<vector<KeyPoint>>keypoints = vector<vector<KeyPoint>>();
	FlannBasedMatcher* matcher = new FlannBasedMatcher();
	vector<DMatch> matches = vector<DMatch>();

	//detect the keypoints of hand cards using SIFT Detector
	Ptr<FeatureDetector> detector = xfeatures2d::SIFT::create();
	vector<KeyPoint> card_keypoints;
	detector->detect(card, card_keypoints);

	//calculate descriptors (feature vectors)
	Ptr<DescriptorExtractor> extractor = xfeatures2d::SIFT::create();
	Mat card_descriptor;
	extractor->compute(card, card_keypoints, card_descriptor);

	for (int i = 1; i < 53; i++) {
		//read deck cards
		string dir = "cards_2/" + to_string(i) + ".png";
		Image card2 = Image(dir);

		//detect the keypoints of deck cards using SIFT Detector
		vector<KeyPoint> deck_keypoints;
		detector->detect(card2.getImage(), deck_keypoints);

		//Calculate descriptors (feature vectors)
		Mat deck_descriptor;
		extractor->compute(card2.getImage(), deck_keypoints, deck_descriptor);

		//matching descriptor vectors using FLANN matcher
		matcher->match(card_descriptor, deck_descriptor, matches);

		//draw only "good" matches (i.e. whose distance is less than 3*min_dist )
		filterMatchesByAbsoluteValue(matches, 50);
		//cout << i << "-" << matches.size() << endl;

		if (bigger < matches.size()) {
			bigger = matches.size();
			result = i;
		}
	}
}

bool exists4Cards(vector<Point> card_contour) {
	double area = fabs(contourArea(Mat(card_contour)));
	cout << "area = " << area << endl;
	if (area>MIN_AREA && area<MAX_AREA) {
		return true;
	}
	return false;
}

void drawResults(Mat original, vector<vector<Point> > contoursVec, vector<vector<Point2f> > approxs, Table table) {
	Mat finalOut = original;
	for (int i = 0; i < /*contoursVec.size()*/4; i++) {
		vector<Point2f> output;
		drawContours(finalOut, contoursVec, i, Scalar(255, 255, 0), 4);

		if (isHorizontal[i]) {
			output.push_back(Point2f(0, 0));//0
			output.push_back(Point2f(0, CARD_HEIGHT));//3
			output.push_back(Point2f(CARD_WIDTH, CARD_HEIGHT));//2
			output.push_back(Point2f(CARD_WIDTH, 0));//1
		}else {
			output.push_back(Point2f(0, CARD_HEIGHT));//3
			output.push_back(Point2f(CARD_WIDTH, CARD_HEIGHT));//2
			output.push_back(Point2f(CARD_WIDTH, 0));//1
			output.push_back(Point2f(0, 0));//0
		}

		Mat white_card = Mat::zeros(Size(CARD_WIDTH, CARD_HEIGHT), finalOut.type());
		if (table.getResult(i) == 0) {
			drawTextInTheMiddle(white_card, "Loser", FONT_HERSHEY_PLAIN, 6, 7, Scalar(0, 0, 255));
		}else {
			drawTextInTheMiddle(white_card, "Winner", FONT_HERSHEY_PLAIN, 6, 7, Scalar(0, 255, 0));
		}
		Mat H = findHomography(output, approxs[i], 0);
		Mat warped;
		warpPerspective(white_card, warped, H, finalOut.size());
		showFinal(finalOut, warped);
	}
	imshow("FinalResult", finalOut);
}

int main(){
	Mat gray, blur, thre, contoursConv, contours, dst;

	string dir = "";
	Image img;
	do {
		cout << "Dir: ";
		cin >> dir;

		//original image
		img = Image(dir);
		if (img.getImage().empty()) {
			cout << "The file does not exist." << endl;
		}
	} while (img.getImage().empty());

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
	vector <vector<Point2f> > approxs;
	int cards[4];
	vector<Rect> boundRect(contoursVec.size());
	Mat drawing = Mat::zeros(img.getImage().size(), CV_8UC3);
	for (int i = 0; i < /*contoursVec.size()*/4; i++) {
		vector<Point2f> output;
		drawContours(contours, contoursVec, i, Scalar(255, 255, 0), 4);
		drawContours(drawing, contoursVec, i, Scalar(255, 255, 0), 4);

		if (!exists4Cards(contoursVec[i])) {
			exist4Cards = false;
			cout << "Not enough cards on the table" << endl;
			break;
		}

		lambda = Mat::zeros(img.getImage().rows, img.getImage().cols, img.getImage().type());

		double peri = arcLength(contoursVec[i], true);
		approxPolyDP(contoursVec[i], approx, 0.02*peri, true);
		boundRect[i] = boundingRect(Mat(approx));
		rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), Scalar(0,255,0), 2, 8, 0);

		if (approx.size() > 4) {
			cout << "contours = " << approx.size() << endl;
			setAnotherVertex(approx, boundRect[i]);
		}

		//cout << approx[0] <<", "<< approx[1] << ", " << approx[2] << ", " << approx[3] << endl;

		getCorners(inputQuad, approx, i);

		outputQuad[0] = Point2f(0, 0);
		outputQuad[1] = Point2f(CARD_WIDTH, 0);
		outputQuad[2] = Point2f(CARD_WIDTH, CARD_HEIGHT);
		outputQuad[3] = Point2f(0, CARD_HEIGHT);

		lambda = getPerspectiveTransform(inputQuad, outputQuad);
		warpPerspective(img.getImage(), dst, lambda, Size(CARD_WIDTH, CARD_HEIGHT));
		imshow("6. Last " + i, dst);

		//findingCardsDIFF(dst);

		cout << "iteration ----" << i << endl;
		findingCardsSIFT(dst, cards[i]);
		cout << "card = " << cards[i] << endl;
		approxs.push_back(approx);

		//just points
		for (int i = 0; i < approx.size(); i++) {
			circle(contours, approx[i], 2, Scalar(100, 100, 0), 2);
		}
	}
	imshow("Something", drawing);

	if (exist4Cards) {
		imshow("Contours", contours);

		Table table = Table(cards);
		table.processTable();
		drawResults(img.getImage(), contoursVec, approxs, table);
	}



	waitKey(0);
	return 0;
}