#ifndef _match_h_
#define _match_h_

#include<opencv2/core/core.hpp>
#include<opencv2/features2d/features2d.hpp>

#include<vector>
#include<string>
#include<iostream>

using namespace std;
using namespace cv;

const double dis_ratio = 0.9;//����ںʹν��ھ������ֵ
const float ransac_error = 1.5;//ransac�㷨�����ֵ
const double FSC_ratio_low = 0.8;
const double FSC_ratio_up = 1;

enum DIS_CRIT{ Euclidean=0,COS};//�������׼��

/*�ú���������С�������ԭ�򣬼���任����*/
static Mat LMS(const Mat&points_1, const Mat &points_2, string model, float &rmse);

/*�ú���ʹ��ransac�㷨ɾ������ƥ����*/
Mat ransac(const vector<Point2f>&points_1, const vector<Point2f> &points_2, string model, float threshold, vector<bool> &inliers, float &rmse);

/*�ú���ʹ��FSC�㷨ɾ��������*/
Mat FSC(const vector<Point2f> &points1_low, const vector<Point2f> &points2_low,
	const vector<Point2f> &points1_up, const vector<Point2f> &points2_up, string model, float threshold, vector<bool> &inliers, float &rmse);

/*�ú����γ�����ͼ�������ͼ�������ں�����������ͼ��*/
void mosaic_map(const Mat &image_1, const Mat &image_2, Mat &chessboard_1, Mat &chessboard_2, Mat &mosaic_image, int width);

/*�ú�����������׼���ͼ������ں���Ƕ*/
void image_fusion(const Mat &image_1, const Mat &image_2, const Mat T, Mat &fusion_image, Mat &mosaic_image);

/*�ú������������ӵ�����ںʹν���ƥ��*/
void match_des(const Mat &des_1, const Mat &des_2, vector<vector<DMatch> > &dmatchs, DIS_CRIT dis_crite);

/*�ú���ɾ������ƥ���ԣ���������׼*/
Mat match(const Mat &image_1, const Mat &image_2, const vector<vector<DMatch> > &dmatchs, vector<KeyPoint> keys_1,
	vector<KeyPoint> keys_2, string model, vector<DMatch> &right_matchs, Mat &matched_line);

/*�ú�������ο�ͼ����һ�������Ӻʹ���׼ͼ��������������֮��ľ��룬����������ںʹν��ڣ��Լ�����*/
static void min_dis_idx(const float *ptr_1, const Mat &des_2, int num_des2, int dims_des, float dis[2], int idx[2]);

#endif
