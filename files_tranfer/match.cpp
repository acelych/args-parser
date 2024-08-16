#include"match.h"
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<algorithm>
#include<vector>
#include<cmath>

using namespace std;
using namespace cv;
//using namespace gpu;

RNG rng(100);

/********�ú���������ȷ��ƥ���ԣ������ͼ��֮��ı任��ϵ********/
/*match1_xy��ʾ�ο�ͼ�����������꼯��,[M x 2]����M��ʾ�����ĸ���
 match2_xy��ʾ����׼ͼ�������㼯�ϣ�[M x 2]����M��ʾ�����㼯��
 model��ʾ�任���ͣ������Ʊ任��,"����任","͸�ӱ任"
 rmse��ʾ���������
 ����ֵΪ����õ���3 x 3�������
 */
static Mat LMS(const Mat&match1_xy, const Mat &match2_xy, string model, float &rmse)
{

	if (match1_xy.rows != match2_xy.rows)
		CV_Error(cv::Error::StsBadArg, "LMSģ������������Ը�����һ�£�");

	if (!(model == string("affine") || model == string("similarity") || 
		model ==string("perspective")))
		CV_Error(cv::Error::StsBadArg, "LMSģ��ͼ��任�����������");

	const int N = match1_xy.rows;//���������
	
	Mat match2_xy_trans, match1_xy_trans;//����ת��
	transpose(match1_xy, match1_xy_trans);
	transpose(match2_xy, match2_xy_trans);

	Mat change = Mat::zeros(3, 3, CV_32FC1);

	//A*X=B,���������ַ���任��͸�ӱ任һ��,��������������M����A=[2*M,6]����
	//A=[x1,y1,0,0,1,0;0,0,x1,y1,0,1;.....xn,yn,0,0,1,0;0,0,xn,yn,0,1]
	Mat A = Mat::zeros(2*N,6,CV_32FC1);
	for (int i = 0; i < N; ++i)
	{
		A.at<float>(2 * i, 0) = match2_xy.at<float>(i, 0);//x
		A.at<float>(2 * i, 1) = match2_xy.at<float>(i, 1);//y
		A.at<float>(2*i, 4) = 1.f;

		A.at<float>(2 * i + 1, 2) = match2_xy.at<float>(i, 0);
		A.at<float>(2 * i + 1, 3) = match2_xy.at<float>(i, 1);
		A.at<float>(2 * i+1, 5) = 1.f;
	}

	//��������������M,�Ǹ�B=[2*M,1]����
	//B=[u1,v1,u2,v2,.....,un,vn]
	Mat B;
	B.create(2 * N, 1, CV_32FC1);
	for (int i = 0; i < N; ++i)
	{
		B.at<float>(2 * i, 0) = match1_xy.at<float>(i, 0);//x
		B.at<float>(2 * i + 1, 0) = match1_xy.at<float>(i, 1);//y
	}

	//����Ƿ���任
	if (model == string("affine"))
	{
		Vec6f values;
		solve(A, B, values, DECOMP_QR);
		change = (Mat_<float>(3,3)<<values(0), values(1), values(4),
			values(2), values(3), values(5),
			+0.0f, +0.0f, 1.0f);

		Mat temp_1 = change(Range(0, 2), Range(0, 2));//�߶Ⱥ���ת��
		Mat temp_2 = change(Range(0, 2), Range(2, 3));//ƽ����
		
		Mat match2_xy_change = temp_1 * match2_xy_trans + repeat(temp_2, 1, N);
		Mat diff = match2_xy_change - match1_xy_trans;//���
		pow(diff,2.f,diff);
		rmse = (float)sqrt(sum(diff)(0)*1.0/N);//sum����Ǹ���ͨ���ĺ�
	}
	//�����͸�ӱ任
	else if (model == string("perspective"))
	{
		/*͸�ӱ任ģ��
		[u'*w,v'*w, w]'=[u,v,w]' = [a1, a2, a5;
		                            a3, a4, a6;
		                            a7, a8, 1] * [x, y, 1]'
		[u',v']'=[x,y,0,0,1,0,-u'x, -u'y;
		         0, 0, x, y, 0, 1, -v'x,-v'y] * [a1, a2, a3, a4, a5, a6, a7, a8]'
		����Y = A*X     */

		Mat A2;
		A2.create(2 * N, 2, CV_32FC1);
		for (int i = 0; i < N; ++i)
		{
			A2.at<float>(2*i, 0) = match1_xy.at<float>(i, 0)*match2_xy.at<float>(i, 0)*(-1.f);
			A2.at<float>(2*i, 1) = match1_xy.at<float>(i, 0)*match2_xy.at<float>(i, 1)*(-1.f);

			A2.at<float>(2 * i + 1, 0) = match1_xy.at<float>(i, 1)*match2_xy.at<float>(i, 0)*(-1.f);
			A2.at<float>(2 * i + 1, 1) = match1_xy.at<float>(i, 1)*match2_xy.at<float>(i, 1)*(-1.f);
		}

		Mat A1;
		A1.create(2 * N, 8, CV_32FC1);
		A.copyTo(A1(Range::all(), Range(0, 6)));
		A2.copyTo(A1(Range::all(), Range(6, 8)));

		Mat values;
		solve(A1, B, values, DECOMP_QR);
		change.at<float>(0, 0) = values.at<float>(0);
		change.at<float>(0, 1) = values.at<float>(1);
		change.at<float>(0, 2) = values.at<float>(4);
		change.at<float>(1, 0) = values.at<float>(2);
		change.at<float>(1, 1) = values.at<float>(3);
		change.at<float>(1, 2) = values.at<float>(5);
		change.at<float>(2, 0) = values.at<float>(6);
		change.at<float>(2, 1) = values.at<float>(7);
		change.at<float>(2, 2) = 1.f;

		Mat temp1 = Mat::ones(1, N, CV_32FC1);
		Mat temp2;
		temp2.create(3, N, CV_32FC1);
		match2_xy_trans.copyTo(temp2(Range(0, 2), Range::all()));
		temp1.copyTo(temp2(Range(2, 3), Range::all()));

		Mat match2_xy_change = change * temp2;
		Mat match2_xy_change_12 = match2_xy_change(Range(0, 2), Range::all());
		float *temp_ptr = match2_xy_change.ptr<float>(2);
		for (int i = 0; i < N; ++i)
		{
			float div_temp = temp_ptr[i];
			match2_xy_change_12.at<float>(0, i) = match2_xy_change_12.at<float>(0, i) / div_temp;
			match2_xy_change_12.at<float>(1, i) = match2_xy_change_12.at<float>(1, i) / div_temp;
		}
		Mat diff = match2_xy_change_12 - match1_xy_trans;
		pow(diff, 2, diff);
		rmse = (float)sqrt(sum(diff)(0)*1.0/ N);//sum����Ǹ���ͨ���ĺ�
	}
	//��������Ʊ任
	else if (model == string("similarity"))
	{
		/*[x, y, 1, 0;
		  y, -x, 0, 1] * [a, b, c, d]'=[u,v]*/

		Mat A3;
		A3.create(2 * N, 4, CV_32FC1);
		for (int i = 0; i < N; ++i)
		{
			A3.at<float>(2 * i, 0) = match2_xy.at<float>(i, 0);
			A3.at<float>(2 * i, 1) = match2_xy.at<float>(i, 1);
			A3.at<float>(2 * i, 2) = 1.f;
			A3.at<float>(2 * i, 3) = 0.f;

			A3.at<float>(2 * i+1, 0) = match2_xy.at<float>(i, 1);
			A3.at<float>(2 * i+1, 1) = match2_xy.at<float>(i, 0)*(-1.f);
			A3.at<float>(2 * i+1, 2) = 0.f;
			A3.at<float>(2 * i + 1, 3) = 1.f;
		}

		Vec4f values;
		solve(A3, B, values, DECOMP_QR);
		change = (Mat_<float>(3, 3) << values(0), values(1), values(2),
			values(1)*(-1.0f), values(0), values(3),
			+0.f, +0.f, 1.f);

		Mat temp_1 = change(Range(0, 2), Range(0, 2));//�߶Ⱥ���ת��
		Mat temp_2 = change(Range(0, 2), Range(2, 3));//ƽ����

		Mat match2_xy_change = temp_1 * match2_xy_trans + repeat(temp_2, 1, N);
		Mat diff = match2_xy_change - match1_xy_trans;//���
		pow(diff, 2, diff);
		rmse = (float)sqrt(sum(diff)(0)*1.0 / N);//sum����Ǹ���ͨ���ĺ�
	}

	return change;
}

/*********************�ú���ɾ�������ƥ����****************************/
/*points_1��ʾ�ο�ͼ����ƥ���������λ��
 points_2��ʾ����׼ͼ���ϵ�������λ�ü���
 model��ʾ�任ģ�ͣ���similarity��,"affine"����perspective��
 threshold��ʾ�ڵ���ֵ
 inliers��ʾpoints_1��points_2�ж�Ӧ�ĵ���Ƿ�����ȷƥ�䣬����ǣ���ӦԪ��ֵΪ1������Ϊ0
 rmse��ʾ���������ȷƥ���Լ�����������
 ����һ��3 x 3���󣬱�ʾ����׼ͼ�񵽲ο�ͼ��ı任����
 */
Mat ransac(const vector<Point2f> &points_1, const vector<Point2f> &points_2, string model, float threshold, vector<bool> &inliers, float &rmse)
{
	if (points_1.size() != points_2.size())
		CV_Error(cv::Error::StsBadArg, "ransacģ������������������һ�£�");

	if (!(model == string("affine") || model == string("similarity") ||
		model == string("perspective")))
		CV_Error(cv::Error::StsBadArg, "ransacģ��ͼ��任�����������");

	const size_t N = points_1.size();//���������
	int n;
	size_t max_iteration, iterations;
	if (model == string("similarity")){
		n = 2;
		max_iteration = N*(N - 1) / 2;
	}
	else if (model == string("affine")){
		n = 3; 
		max_iteration = N*(N - 1)*(N - 2) / (2 * 3);
	}
	else if (model == string("perspective")){
		n = 4;
		max_iteration = N*(N - 1)*(N - 2) / (2 * 3);
	}

	if (max_iteration > 800)
		iterations = 800;
	else
		iterations = max_iteration;

	//ȡ��������points_1��points_2�еĵ����꣬������Mat�����У����㴦��
	Mat arr_1, arr_2;//arr_1,��arr_2��һ��[3 x N]�ľ���ÿһ�б�ʾһ��������,������ȫ��1
	arr_1.create(3, (int)N, CV_32FC1);
	arr_2.create(3, (int)N, CV_32FC1);
	float *p10 = arr_1.ptr<float>(0), *p11 = arr_1.ptr<float>(1),*p12 = arr_1.ptr<float>(2);
	float *p20 = arr_2.ptr<float>(0), *p21 = arr_2.ptr<float>(1), *p22 = arr_2.ptr<float>(2);
	for (size_t i = 0; i < N; ++i)
	{
		p10[i] = points_1[i].x;
		p11[i] = points_1[i].y;
		p12[i] = 1.f;

		p20[i] = points_2[i].x;
		p21[i] = points_2[i].y;
		p22[i] = 1.f;
	}

	Mat rand_mat;
	rand_mat.create(1, n, CV_32SC1);
	int *p = rand_mat.ptr<int>(0);
	Mat sub_arr1, sub_arr2;
	sub_arr1.create(n, 2, CV_32FC1);
	sub_arr2.create(n, 2, CV_32FC1);
	Mat T;//����׼ͼ�񵽲ο�ͼ��ı任����
	int most_consensus_num = 0;//��ǰ����һ�¼�������ʼ��Ϊ0
	vector<bool> right;
	right.resize(N);
	inliers.resize(N);

	for (size_t i = 0; i < iterations;++i)
	{
		//���ѡ��n����ͬ�ĵ��
		while (1)
		{
			randu(rand_mat, 0, double(N-1));//�������n����Χ��[0,N-1]֮�����

			//��֤��n�������겻��ͬ
			if (n == 2 && p[0] != p[1] && 
				(p10[p[0]] != p10[p[1]] || p11[p[0]] != p11[p[1]]) &&
				(p20[p[0]] != p20[p[1]] || p21[p[0]] != p21[p[1]]))
				break;

			if (n == 3 && p[0] != p[1] && p[0] != p[2] && p[1] != p[2] &&
				(p10[p[0]] != p10[p[1]] || p11[p[0]] != p11[p[1]]) &&
				(p10[p[0]] != p10[p[2]] || p11[p[0]] != p11[p[2]]) &&
				(p10[p[1]] != p10[p[2]] || p11[p[1]] != p11[p[2]]) &&
				(p20[p[0]] != p20[p[1]] || p21[p[0]] != p21[p[1]]) &&
				(p20[p[0]] != p20[p[2]] || p21[p[0]] != p21[p[2]]) &&
				(p20[p[1]] != p20[p[2]] || p21[p[1]] != p21[p[2]]))
				break;

			if (n == 4 && p[0] != p[1] && p[0] != p[2] && p[0] != p[3] &&
				p[1] != p[2] && p[1] != p[3] && p[2] != p[3] &&
				(p10[p[0]] != p10[p[1]] || p11[p[0]] != p11[p[1]]) &&
				(p10[p[0]] != p10[p[2]] || p11[p[0]] != p11[p[2]]) &&
				(p10[p[0]] != p10[p[3]] || p11[p[0]] != p11[p[3]]) &&
				(p10[p[1]] != p10[p[2]] || p11[p[1]] != p11[p[2]]) &&
				(p10[p[1]] != p10[p[3]] || p11[p[1]] != p11[p[3]]) &&
				(p10[p[2]] != p10[p[3]] || p11[p[2]] != p11[p[3]]) &&
				(p20[p[0]] != p20[p[1]] || p21[p[0]] != p21[p[1]]) &&
				(p20[p[0]] != p20[p[2]] || p21[p[0]] != p21[p[2]]) &&
				(p20[p[0]] != p20[p[3]] || p21[p[0]] != p21[p[3]]) &&
				(p20[p[1]] != p20[p[2]] || p21[p[1]] != p21[p[2]]) &&
				(p20[p[1]] != p20[p[3]] || p21[p[1]] != p21[p[3]]) &&
				(p20[p[2]] != p20[p[3]] || p21[p[2]] != p21[p[3]]))
				break;
		}

		//��ȡ��n�����
		for (int i = 0; i < n; ++i)
		{
			sub_arr1.at<float>(i, 0) = p10[p[i]];
			sub_arr1.at<float>(i, 1) = p11[p[i]];

			sub_arr2.at<float>(i, 0) = p20[p[i]];
			sub_arr2.at<float>(i, 1) = p21[p[i]];
		}

		//������n����ԣ�����任����T
		T = LMS(sub_arr1, sub_arr2, model, rmse);
		
		int consensus_num = 0;//��ǰһ�¼�����
		if(model == string("perspective"))
		{

			Mat match2_xy_change = T * arr_2;
			Mat match2_xy_change_12 = match2_xy_change(Range(0, 2), Range::all());
			float *temp_ptr = match2_xy_change.ptr<float>(2);
			for (size_t i = 0; i < N; ++i)
			{
				float div_temp = temp_ptr[i];
				match2_xy_change_12.at<float>(0, (int)i) = match2_xy_change_12.at<float>(0, (int)i) / div_temp;
				match2_xy_change_12.at<float>(1, (int)i) = match2_xy_change_12.at<float>(1, (int)i) / div_temp;
			}
			Mat diff = match2_xy_change_12 - arr_1(Range(0,2),Range::all());
			pow(diff, 2, diff);

			//��һ�к͵ڶ������
			
			Mat add = diff(Range(0, 1), Range::all()) + diff(Range(1, 2), Range::all());
			float *p_add = add.ptr<float>(0);
			for (size_t i = 0; i < N; ++i)
			{
				if (p_add[i] < threshold){//���С����ֵ
					right[i] = true;
					++consensus_num;
				}
				else
					right[i] = false;
			}
		}

		else if (model == string("affine") || model == string("similarity"))
		{
			Mat match2_xy_change = T * arr_2;
			Mat diff = match2_xy_change - arr_1;
			pow(diff, 2, diff);

			//��һ�к͵ڶ������
			Mat add = diff(Range(0, 1), Range::all()) + diff(Range(1, 2), Range::all());
			float *p_add = add.ptr<float>(0);
			for (size_t i = 0; i < N; ++i)
			{
				if (p_add[i] < threshold){//���С����ֵ
					right[i] = true;
					++consensus_num;
				}
				else
					right[i] = false;
			}
		}

		//�жϵ�ǰһ�¼��Ƿ�������֮ǰ����һ�¼�
		if (consensus_num>most_consensus_num){
				most_consensus_num = consensus_num;
				for (size_t i = 0; i < N; ++i)
					inliers[i] = right[i];
			}//���µ�ǰ����һ�¼�����

	}

	//ɾ���ظ����
	for (size_t i = 0; i < N-1; ++i)
	{
		for (size_t j = i + 1; j < N; ++j)
		{
			if (inliers[i] && inliers[j])
			{
				if (p10[i] == p10[j] && p11[i] == p11[j] && p20[i] == p20[j] && p21[i] == p21[j])
				{
					inliers[j] = false;
					--most_consensus_num;
				}
			}
		}
	}

	//�����������������һ�¼��ϣ�������Щ����һ�¼��ϼ�������յı任��ϵT
	Mat consensus_arr1, consensus_arr2;
	consensus_arr1.create(most_consensus_num, 2, CV_32FC1);
	consensus_arr2.create(most_consensus_num, 2, CV_32FC1);
	int k = 0;
	for (size_t i = 0; i < N; ++i)
	{
		if (inliers[i])
		{
			consensus_arr1.at<float>(k, 0) = p10[i];
			consensus_arr1.at<float>(k, 1) = p11[i];

			consensus_arr2.at<float>(k, 0) = p20[i];
			consensus_arr2.at<float>(k, 1) = p21[i];
			++k;
		}
	}

	T = LMS(consensus_arr1, consensus_arr2, model, rmse);
	return T;
}

/*********************�ú�������FSC�㷨ɾ�������,FSC�㷨����RANSAC�㷨****************************/
/*points1_low��ʾʹ�õ;������ֵ��õĲο�ͼ������������
 points2_low��ʾʹ�õ;������ֵ��õĴ���׼ͼ������������
 points1_up��ʾʹ�ø߾������ֵ��õĲο�ͼ������������
 points2_up��ʾʹ�ø߾������ֵ��õĴ���׼ͼ�����������꣬�ɴ˿ɼ�points1_up �� points2_up���������������
 ����points1_low �� points2_low���������������
 model��ʾ�任ģ�ͣ���similarity��,"affine"����perspective��
 threshold��ʾ�ڵ���ֵ
 inliers��ʾpoints1_up��points2_up�ж�Ӧ�ĵ���Ƿ�����ȷƥ�䣬����ǣ���ӦԪ��ֵΪ1������Ϊ0
 rmse��ʾ���������ȷƥ���Լ�����������
 ����һ��3 x 3���󣬱�ʾ����׼ͼ�񵽲ο�ͼ��ı任����
*/
Mat FSC(const vector<Point2f> &points1_low, const vector<Point2f> &points2_low, 
	const vector<Point2f> &points1_up,const vector<Point2f> &points2_up,string model, float threshold, vector<bool> &inliers, float &rmse)
{
	if (points1_low.size() != points2_low.size() && points1_up.size() != points2_up.size())
		CV_Error(cv::Error::StsBadArg, "ransacģ������������������һ�£�");

	if (!(model == string("affine") || model == string("similarity") ||
		model == string("perspective")))
		CV_Error(cv::Error::StsBadArg, "ransacģ��ͼ��任�����������");

	const size_t N = points1_low.size();//�;������ֵʱ�����������
	const size_t M = points1_up.size();//�߾������ֵʱ�������������M>N��
	int n;
	size_t max_iteration, iterations;
	if (model == string("similarity")){
		n = 2;
		max_iteration = N*(N - 1) / 2;
	}
	else if (model == string("affine")){
		n = 3;
		max_iteration = N*(N - 1)*(N - 2) / (2 * 3);
	}
	else if (model == string("perspective")){
		n = 4;
		max_iteration = N*(N - 1)*(N - 2) / (2 * 3);
	}

	if (max_iteration > 800)
		iterations = 800;
	else
		iterations = max_iteration;

	//ȡ��������points1_low��points2_low�еĵ����꣬������Mat�����У����㴦��
	Mat arr1_low, arr2_low;//arr1_low,��arr2_low��һ��[3 x N]�ľ���ÿһ�б�ʾһ��������,������ȫ��1
	arr1_low.create(3, (int)N, CV_32FC1);
	arr2_low.create(3, (int)N, CV_32FC1);
	float *p10 = arr1_low.ptr<float>(0), *p11 = arr1_low.ptr<float>(1), *p12 = arr1_low.ptr<float>(2);
	float *p20 = arr2_low.ptr<float>(0), *p21 = arr2_low.ptr<float>(1), *p22 = arr2_low.ptr<float>(2);
	for (size_t i = 0; i < N; ++i)
	{
		p10[i] = points1_low[i].x;
		p11[i] = points1_low[i].y;
		p12[i] = 1.f;

		p20[i] = points2_low[i].x;
		p21[i] = points2_low[i].y;
		p22[i] = 1.f;
	}
	//ȡ��������points1_up��points2_up�еĵ����꣬������Mat�����У����㴦��
	Mat arr1_up, arr2_up;//arr1_up,��arr2_up��һ��[3 x ]�ľ���ÿһ�б�ʾһ��������,������ȫ��1
	arr1_up.create(3, (int)M, CV_32FC1);
	arr2_up.create(3, (int)M, CV_32FC1);
	float *up10 = arr1_up.ptr<float>(0), *up11 = arr1_up.ptr<float>(1), *up12 = arr1_up.ptr<float>(2);
	float *up20 = arr2_up.ptr<float>(0), *up21 = arr2_up.ptr<float>(1), *up22 = arr2_up.ptr<float>(2);
	for (size_t i = 0; i < M; ++i)
	{
		up10[i] = points1_up[i].x;
		up11[i] = points1_up[i].y;
		up12[i] = 1.f;

		up20[i] = points2_up[i].x;
		up21[i] = points2_up[i].y;
		up22[i] = 1.f;
	}

	Mat rand_mat;
	rand_mat.create(1, n, CV_32SC1);
	int *p = rand_mat.ptr<int>(0);
	Mat sub_arr1, sub_arr2;
	sub_arr1.create(n, 2, CV_32FC1);
	sub_arr2.create(n, 2, CV_32FC1);
	Mat T;//����׼ͼ�񵽲ο�ͼ��ı任����
	int most_consensus_num = 0;//��ǰ����һ�¼�������ʼ��Ϊ0
	vector<bool> right;
	right.resize(M);
	inliers.resize(M);

	for (size_t i = 0; i < iterations; ++i)
	{
		//���ѡ��n����ͬ�ĵ��
		while (1)
		{
			randu(rand_mat, 0, double(N - 1));//�������n����Χ��[0,N-1]֮�����

			//��֤��n�������겻��ͬ
			if (n == 2 && p[0] != p[1] &&
				(p10[p[0]] != p10[p[1]] || p11[p[0]] != p11[p[1]]) &&
				(p20[p[0]] != p20[p[1]] || p21[p[0]] != p21[p[1]]))
				break;

			if (n == 3 && p[0] != p[1] && p[0] != p[2] && p[1] != p[2] &&
				(p10[p[0]] != p10[p[1]] || p11[p[0]] != p11[p[1]]) &&
				(p10[p[0]] != p10[p[2]] || p11[p[0]] != p11[p[2]]) &&
				(p10[p[1]] != p10[p[2]] || p11[p[1]] != p11[p[2]]) &&
				(p20[p[0]] != p20[p[1]] || p21[p[0]] != p21[p[1]]) &&
				(p20[p[0]] != p20[p[2]] || p21[p[0]] != p21[p[2]]) &&
				(p20[p[1]] != p20[p[2]] || p21[p[1]] != p21[p[2]]))
				break;

			if (n == 4 && p[0] != p[1] && p[0] != p[2] && p[0] != p[3] &&
				p[1] != p[2] && p[1] != p[3] && p[2] != p[3] &&
				(p10[p[0]] != p10[p[1]] || p11[p[0]] != p11[p[1]]) &&
				(p10[p[0]] != p10[p[2]] || p11[p[0]] != p11[p[2]]) &&
				(p10[p[0]] != p10[p[3]] || p11[p[0]] != p11[p[3]]) &&
				(p10[p[1]] != p10[p[2]] || p11[p[1]] != p11[p[2]]) &&
				(p10[p[1]] != p10[p[3]] || p11[p[1]] != p11[p[3]]) &&
				(p10[p[2]] != p10[p[3]] || p11[p[2]] != p11[p[3]]) &&
				(p20[p[0]] != p20[p[1]] || p21[p[0]] != p21[p[1]]) &&
				(p20[p[0]] != p20[p[2]] || p21[p[0]] != p21[p[2]]) &&
				(p20[p[0]] != p20[p[3]] || p21[p[0]] != p21[p[3]]) &&
				(p20[p[1]] != p20[p[2]] || p21[p[1]] != p21[p[2]]) &&
				(p20[p[1]] != p20[p[3]] || p21[p[1]] != p21[p[3]]) &&
				(p20[p[2]] != p20[p[3]] || p21[p[2]] != p21[p[3]]))
				break;
		}

		//��ȡ��n�����
		for (int i = 0; i < n; ++i)
		{
			sub_arr1.at<float>(i, 0) = p10[p[i]];
			sub_arr1.at<float>(i, 1) = p11[p[i]];

			sub_arr2.at<float>(i, 0) = p20[p[i]];
			sub_arr2.at<float>(i, 1) = p21[p[i]];
		}

		//������n����ԣ�����任����T
		T = LMS(sub_arr1, sub_arr2, model, rmse);

		int consensus_num = 0;//��ǰһ�¼�����
		if (model == string("perspective"))
		{

			Mat match2_xy_change = T * arr2_up;
			Mat match2_xy_change_12 = match2_xy_change(Range(0, 2), Range::all());
			float *temp_ptr = match2_xy_change.ptr<float>(2);
			for (size_t i = 0; i < M; ++i)
			{
				float div_temp = temp_ptr[i];
				match2_xy_change_12.at<float>(0, (int)i) = match2_xy_change_12.at<float>(0, (int)i) / div_temp;
				match2_xy_change_12.at<float>(1, (int)i) = match2_xy_change_12.at<float>(1, (int)i) / div_temp;
			}
			Mat diff = match2_xy_change_12 - arr1_up(Range(0, 2), Range::all());
			pow(diff, 2, diff);

			//��һ�к͵ڶ������

			Mat add = diff(Range(0, 1), Range::all()) + diff(Range(1, 2), Range::all());
			float *p_add = add.ptr<float>(0);
			for (size_t i = 0; i < M; ++i)
			{
				if (p_add[i] < threshold){//���С����ֵ
					right[i] = true;
					++consensus_num;
				}
				else
					right[i] = false;
			}
		}

		else if (model == string("affine") || model == string("similarity"))
		{
			Mat match2_xy_change = T * arr2_up;
			Mat diff = match2_xy_change - arr1_up;
			pow(diff, 2, diff);

			//��һ�к͵ڶ������
			Mat add = diff(Range(0, 1), Range::all()) + diff(Range(1, 2), Range::all());
			float *p_add = add.ptr<float>(0);
			for (size_t i = 0; i < M; ++i)
			{
				if (p_add[i] < threshold){//���С����ֵ
					right[i] = true;
					++consensus_num;
				}
				else
					right[i] = false;
			}
		}

		//�жϵ�ǰһ�¼��Ƿ�������֮ǰ����һ�¼�
		if (consensus_num>most_consensus_num){
			most_consensus_num = consensus_num;
			for (size_t i = 0; i < M; ++i)
				inliers[i] = right[i];
		}//���µ�ǰ����һ�¼�����

	}

	//ɾ���ظ����
	for (size_t i = 0; i < M - 1; ++i)
	{
		for (size_t j = i + 1; j < M; ++j)
		{
			if (inliers[i] && inliers[j])
			{
				if ((up10[i] == up10[j] && up11[i] == up11[j] && up20[i] == up20[j] && up21[i] == up21[j]) ||
					(up10[i] == up10[j] && up11[i] == up11[j]) || (up20[i] == up20[j] && up21[i] == up21[j]))
				{
					inliers[j] = false;
					--most_consensus_num;
				}
			}
		}
	}

	//�����������������һ�¼��ϣ�������Щ����һ�¼��ϼ�������յı任��ϵT
	Mat consensus_arr1, consensus_arr2;
	consensus_arr1.create(most_consensus_num, 2, CV_32FC1);
	consensus_arr2.create(most_consensus_num, 2, CV_32FC1);
	int k = 0;
	for (size_t i = 0; i < M; ++i)
	{
		if (inliers[i])
		{
			consensus_arr1.at<float>(k, 0) = up10[i];
			consensus_arr1.at<float>(k, 1) = up11[i];

			consensus_arr2.at<float>(k, 0) = up20[i];
			consensus_arr2.at<float>(k, 1) = up21[i];
			++k;
		}
	}

	T = LMS(consensus_arr1, consensus_arr2, model, rmse);
	return T;
}


/********************�ú�����������ͼ����������ͼ*************************/
/*image_1��ʾ�ο�ͼ��
 image_2��ʾ��׼��Ĵ���׼ͼ��
 chessboard_1��ʾimage_1������ͼ��
 chessboard_2��ʾimage_2������ͼ��
 mosaic_image��ʾimage_1��image_2����Ƕͼ��
 width��ʾ���������С
 */
void mosaic_map(const Mat &image_1, const Mat &image_2, Mat &chessboard_1, Mat &chessboard_2, Mat &mosaic_image, int width)
{
	if (image_1.size != image_2.size)
		CV_Error(cv::Error::StsBadArg, "mosaic_mapģ����������ͼ��С����һ�£�");

	//����image_1����������ͼ
	chessboard_1 = image_1.clone();
	int rows_1 = chessboard_1.rows;
	int cols_1 = chessboard_1.cols;

	int row_grids_1 = cvFloor((double)rows_1 / width);//�з����������
	int col_grids_1 = cvFloor((double)cols_1 / width);//�з����������

	for (int i = 0; i < row_grids_1; i = i + 2){
		for (int j = 1; j < col_grids_1; j = j + 2){
			Range range_x(j*width, (j + 1)*width);
			Range range_y(i*width, (i + 1)*width);
			chessboard_1(range_y, range_x) = 0;
		}
	}

	for (int i = 1; i < row_grids_1; i = i + 2){
		for (int j = 0; j < col_grids_1; j = j + 2){
			Range range_x(j*width, (j + 1)*width);
			Range range_y(i*width, (i + 1)*width);
			chessboard_1(range_y, range_x) = 0;
		}
	}

	//����image_2����������ͼ
	chessboard_2 = image_2.clone();
	int rows_2 = chessboard_2.rows;
	int cols_2 = chessboard_2.cols;

	int row_grids_2 = cvFloor((double)rows_2 / width);//�з����������
	int col_grids_2 = cvFloor((double)cols_2 / width);//�з����������

	for (int i = 0; i < row_grids_2; i = i + 2){
		for (int j = 0; j < col_grids_2; j = j + 2){
			Range range_x(j*width, (j + 1)*width);
			Range range_y(i*width, (i + 1)*width);
			chessboard_2(range_y, range_x) = 0;
		}
	}

	for (int i = 1; i < row_grids_2; i = i + 2){
		for (int j = 1; j < col_grids_2; j = j + 2){
			Range range_x(j*width, (j + 1)*width);
			Range range_y(i*width, (i + 1)*width);
			chessboard_2(range_y, range_x) = 0;
		}
	}

	mosaic_image = chessboard_1 + chessboard_2;
}


/***************�ú�������׼���ͼ������ں�*****************/
/*image_1��ʾ�ο�ͼ��
 image_2��ʾ����׼ͼ��
 T��ʾ����׼ͼ�񵽲ο�ͼ��ı任��ϵ����
 fusion_image��ʾ�ںϺ��ͼ��
 mosaic_image��ʾ�ں���Ƕ���ͼ��
 */
void image_fusion(const Mat &image_1, const Mat &image_2, const Mat T, Mat &fusion_image, Mat &mosaic_image)
{
	if (!(image_1.depth() == CV_8U && image_2.depth() == CV_8U))
		CV_Error(cv::Error::StsBadArg, "image_fusionģ���֧��uchar����ͼ��");

	int rows_1 = image_1.rows, cols_1 = image_1.cols;
	int rows_2 = image_2.rows, cols_2 = image_2.cols;
	int channel_1 = image_1.channels();
	int channel_2 = image_2.channels();

	Mat image_1_temp, image_2_temp;
	if (channel_1 == 3 && channel_2 == 3){
		image_1_temp = image_1;
		image_2_temp = image_2;
	}
	else if (channel_1 == 1 && channel_2 == 3){
		image_1_temp = image_1;
		cvtColor(image_2, image_2_temp, cv::COLOR_RGB2GRAY);
	}
	else if (channel_1 == 3 && channel_2 == 1){
		cvtColor(image_1, image_1_temp, cv::COLOR_RGB2GRAY);
		image_2_temp = image_2;
	}
	else if (channel_1 == 1 && channel_2 == 1){
		image_1_temp = image_1;
		image_2_temp = image_2;
	}
		
	Mat T_temp = (Mat_<float>(3,3)<<1, 0, cols_1,
		                           0, 1, rows_1,
		                                0, 0, 1);
	Mat T_1 = T_temp*T;

	//�Բο�ͼ��ʹ���׼ͼ����б任
	Mat trans_1,trans_2;//same type as image_2_temp 
	trans_1=Mat::zeros(3 * rows_1, 3 * cols_1, image_1_temp.type());
	image_1_temp.copyTo(trans_1(Range(rows_1,2*rows_1), Range(cols_1,2*cols_1)));
	warpPerspective(image_2_temp, trans_2, T_1, Size(3 * cols_1, 3 * rows_1), INTER_LINEAR);

	Mat trans = trans_2.clone();
	int nRows = rows_1;
	int nCols = cols_1*image_1_temp.channels();
	int len = nCols;
	for (int i = 0; i < nRows; ++i)
	{
		uchar *ptr_1 = trans_1.ptr<uchar>(i+rows_1);
		uchar *ptr = trans.ptr<uchar>(i+rows_1);
		for (int j = 0; j < nCols; ++j)
		{
			if (ptr[j + len] == 0)//���ڷ��غ�����
				ptr[j + len] = ptr_1[j + len];
			else//�����غ�����
				ptr[j + len] = saturate_cast<uchar>(((float)ptr[j + len] + (float)ptr_1[j + len]) / 2);
		}
	}

	//ɾ�����������
	Mat left_up = T_1*(Mat_<float>(3, 1) << 0, 0, 1);//���Ͻ�
	Mat left_down = T_1*(Mat_<float>(3, 1) << 0, rows_2 - 1, 1);//���½�
	Mat right_up = T_1*(Mat_<float>(3, 1) << cols_2 - 1, 0, 1);//���Ͻ�
	Mat right_down = T_1*(Mat_<float>(3, 1) << cols_2 - 1, rows_2 - 1, 1);//���½�

	//����͸�ӱ任����Ҫ����һ������
	left_up = left_up / left_up.at<float>(2, 0);
	left_down = left_down / left_down.at<float>(2, 0);
	right_up = right_up / right_up.at<float>(2, 0);
	right_down = right_down / right_down.at<float>(2, 0);

	//����x,y����ķ�Χ
	float temp_1 = min(left_up.at<float>(0, 0), left_down.at<float>(0, 0));
	float temp_2 = min(right_up.at<float>(0, 0), right_down.at<float>(0, 0));
	float min_x = min(temp_1, temp_2);

	temp_1 = max(left_up.at<float>(0, 0), left_down.at<float>(0, 0));
	temp_2 = max(right_up.at<float>(0, 0), right_down.at<float>(0, 0));
	float max_x = max(temp_1, temp_2);

	temp_1 = min(left_up.at<float>(1, 0), left_down.at<float>(1, 0));
	temp_2 = min(right_up.at<float>(1, 0), right_down.at<float>(1, 0));
	float min_y = min(temp_1, temp_2);

	temp_1 = max(left_up.at<float>(1, 0), left_down.at<float>(1, 0));
	temp_2 = max(right_up.at<float>(1, 0), right_down.at<float>(1, 0));
	float max_y = max(temp_1, temp_2);

	int X_min = max(cvFloor(min_x), 0);
	int X_max = min(cvCeil(max_x), 3 * cols_1-1);
	int Y_min = max(cvFloor(min_y), 0);
	int Y_max = min(cvCeil(max_y), 3 * rows_1-1);


	if (X_min>cols_1)
		X_min = cols_1;
	if (X_max<2 * cols_1-1)
		X_max = 2 * cols_1 - 1;
	if (Y_min>rows_1)
		Y_min = rows_1;
	if (Y_max<2 * rows_1-1)
		Y_max = 2 * rows_1 - 1;

	Range Y_range(Y_min, Y_max+1);
	Range X_range(X_min, X_max+1);
	fusion_image = trans(Y_range, X_range);
	Mat matched_image = trans_2(Y_range, X_range);

	Mat chessboard_1, chessboard_2;
	mosaic_map(trans_1(Y_range, X_range), trans_2(Y_range, X_range), 
		chessboard_1, chessboard_2, mosaic_image, 50);
	imwrite(".\\image_save\\�ο�ͼ������ͼ��.jpg", chessboard_1);
	imwrite(".\\image_save\\����׼ͼ������ͼ��.jpg", chessboard_2);
	imwrite(".\\image_save\\��׼��Ĵ���׼ͼ��.jpg", matched_image);
}

/*************�ú������������������ںʹν���ƥ��**************/
/*des_1��ʾ�ο�ͼ��������Ӿ��󣬡�M x 128��,M��ʾ�ο�ͼ�������Ӹ���
 des_2��ʾ����׼ͼ��������Ӿ���,��N x 128��,N��ʾ����׼ͼ�������Ӹ���
 dmatchs��ʾƥ����������
 dis_crite��ʾʹ�õľ���׼�򣬡�0����ʾŷʽ���룬��1����ʾCOS����,ʹ��ŷʽ�����ٶȷǳ���
 */
/*void match_des(const Mat &des_1, const Mat &des_2, vector<vector<DMatch>> &dmatchs,DIS_CRIT dis_crite)
{
	Mat trans_des_2;
	if (dis_crite==1)//�����cos����
		transpose(des_2, trans_des_2);//ת�òο������Ӿ���

	int num_des_1 = des_1.rows;
	int num_des_2 = des_2.rows;
	dmatchs.clear();

	//���ڲο�ͼ���ϵ�ÿһ�㣬�ʹ���׼ͼ�����ƥ��
	vector<DMatch> match;
	match.resize(2);
	Mat temp_dis;
	temp_dis.create(1, num_des_2, CV_32FC1);
	float *ptr_dis = temp_dis.ptr<float>(0);
	for (int i = 0; i < num_des_1; ++i)
	{
		if (dis_crite == 0)//�����ŷʽ����
		{
			Mat temp = repeat(des_1.row(i), num_des_2, 1);
			Mat diff = temp - des_2;
			pow(diff, 2, diff);
			for (int j = 0; j < num_des_2; ++j)
				ptr_dis[j] = (float)sum(diff.row(j))[0];
			sqrt(temp_dis, temp_dis);
		}
		else if (dis_crite == 1)//�����cos����
		{
			temp_dis = des_1.row(i)*trans_des_2;
			for (int k = 0; k < num_des_2; ++k)
				ptr_dis[k] = acosf(ptr_dis[k]);//������
		}
		
		Mat sort_dis, sort_idx;
		cv::sort(temp_dis, sort_dis, CV_SORT_EVERY_ROW + CV_SORT_ASCENDING);//��������
		cv::sortIdx(temp_dis, sort_idx, CV_SORT_EVERY_ROW + CV_SORT_ASCENDING);

		match[0].queryIdx = i;
		match[0].trainIdx = sort_idx.at<int>(0, 0);
		match[0].distance = sort_dis.at<float>(0, 0);

		match[1].queryIdx = i;
		match[1].trainIdx = sort_idx.at<int>(0, 1);
		match[1].distance = sort_dis.at<float>(0, 1);
		dmatchs.push_back(match);
	}
}*/

/******�ú�������ο�ͼ��һ�������ӺͲο�ͼ�����������ӵ�ŷʽ���룬���������ںʹν��ھ��룬�Լ���Ӧ������*/
/*sub_des_1��ʾ�ο�ͼ���һ��������
 des_2��ʾ����׼ͼ��������
 num_des_2ֵ����׼ͼ�������Ӹ���
 dims_desָ����������ά��
 dis��������ںʹν��ھ���
 idx��������ںʹν�������
 */
static void min_dis_idx(const float *ptr_1, const Mat &des_2,int num_des2,int dims_des,float dis[2],int idx[2])
{
	float min_dis1=1000, min_dis2=2000;
	int min_idx1, min_idx2;

	for (int j = 0; j < num_des2; ++j)
	{
		const float *ptr_des_2 = des_2.ptr<float>(j);
		float cur_dis=0;
		for (int k = 0; k < dims_des; ++k)
		{
			float diff = ptr_1[k] - ptr_des_2[k];
			cur_dis+=diff*diff;
		}
		if (cur_dis < min_dis1){
			min_dis1 = cur_dis;
			min_idx1 = j;
		}
		else if (cur_dis>=min_dis1 && cur_dis < min_dis2){
			min_dis2 = cur_dis;
			min_idx2 = j;
		}
			
	}
	dis[0] = sqrt(min_dis1); dis[1] = sqrt(min_dis2);
	idx[0] = min_idx1; idx[1] = min_idx2;
}

/*���ٰ汾��������ƥ�亯��,�ü��ٰ汾��ǰ��汾�ٶ�������3��*/
void match_des(const Mat &des_1, const Mat &des_2, vector<vector<DMatch> > &dmatchs, DIS_CRIT dis_crite)
{
	int num_des_1 = des_1.rows;
	int num_des_2 = des_2.rows;
	int dims_des = des_1.cols;

	vector<DMatch> match(2);

	//���ڲο�ͼ���ϵ�ÿһ�㣬�ʹ���׼ͼ�����ƥ��
	if (dis_crite == 0)//ŷ����þ���
	{
		for (int i = 0; i < num_des_1; ++i)//���ڲο�ͼ���е�ÿ��������
		{
			const float *ptr_des_1 = des_1.ptr<float>(i);
			float dis[2];
			int idx[2];
			min_dis_idx(ptr_des_1, des_2, num_des_2,dims_des,dis, idx);
			match[0].queryIdx = i;
			match[0].trainIdx = idx[0];
			match[0].distance = dis[0];

			match[1].queryIdx = i;
			match[1].trainIdx = idx[1];
			match[1].distance = dis[1];

			dmatchs.push_back(match);
		}
	}
	else if (dis_crite == 1)//cos����
	{
		Mat trans_des2;
		transpose(des_2, trans_des2);
		double aa = (double)getTickCount();
		Mat mul_des=des_1*trans_des2;
		//gemm(des_1, des_2, 1, Mat(), 0, mul_des, GEMM_2_T);
		double time1 = ((double)getTickCount() - aa) / getTickFrequency();
		cout << "cos�����о���˷�����ʱ�䣺 " << time1 << "s" << endl;

		for (int i = 0; i < num_des_1; ++i)
		{ 
			float max_cos1 = -1000, max_cos2 = -2000;
			int max_idx1, max_idx2;

			float *ptr_1 = mul_des.ptr<float>(i);
			for (int j = 0; j < num_des_2; ++j)
			{
				float cur_cos = ptr_1[j];
				if (cur_cos>max_cos1){
					max_cos1 = cur_cos;
					max_idx1 = j;
				}
				else if (cur_cos<=max_cos1 && cur_cos > max_cos2){
					max_cos2 = cur_cos;
					max_idx2 = j;
				}
			}

			match[0].queryIdx = i;
			match[0].trainIdx = max_idx1;
			match[0].distance = acosf(max_cos1);

			match[1].queryIdx = i;
			match[1].trainIdx = max_idx2;
			match[1].distance = acosf(max_cos2);
			dmatchs.push_back(match);
		}
	}		
}

/*******************�ú���ɾ������ƥ����,�����ƥ��************************/
/*image_1��ʾ�ο�ͼ��
 image_2��ʾ����׼ͼ��
 dmatchs��ʾ����ںʹν���ƥ����
 keys_1��ʾ�ο�ͼ�������㼯��
 keys_2��ʾ����׼ͼ�������㼯��
 model��ʾ�任ģ��
 right_matchs��ʾ�ο�ͼ��ʹ���׼ͼ����ȷƥ����
 matched_line��ʾ�ڲο�ͼ��ʹ���׼ͼ���ϻ���������
 �ú������ر任ģ�Ͳ���
 */
Mat match(const Mat &image_1, const Mat &image_2, const vector<vector<DMatch> > &dmatchs, vector<KeyPoint> keys_1,
	       vector<KeyPoint> keys_2,string model,vector<DMatch> &right_matchs,Mat &matched_line)
{
	//��ȡ��ʼƥ��Ĺؼ����λ��
	vector<Point2f> points1_low, points2_low;
	vector<Point2f> points1_up, points2_up;

	vector<DMatch> low_matchs;
	vector<DMatch> up_matchs;
	for (size_t i = 0; i < dmatchs.size(); ++i)
	{
		double dis_1 = dmatchs[i][0].distance;
		double dis_2 = dmatchs[i][1].distance;
		//points1_low��points2_low���������������ȹ�ϵ��������
		if ((dis_1 / dis_2) < FSC_ratio_low)
		{
			points1_low.push_back(keys_1[dmatchs[i][0].queryIdx].pt);
			points2_low.push_back(keys_2[dmatchs[i][0].trainIdx].pt);
			low_matchs.push_back(dmatchs[i][0]);//������ȷ��dmatchs
		}
		//points1_up��points2_up���������������ȹ�ϵ�������㣬�����治�������ȹ�ϵ�ĵ�
		if ((dis_1 / dis_2) <= FSC_ratio_up)
		{
			points1_up.push_back(keys_1[dmatchs[i][0].queryIdx].pt);
			points2_up.push_back(keys_2[dmatchs[i][0].trainIdx].pt);
			points1_up.push_back(keys_1[dmatchs[i][1].queryIdx].pt);
			points2_up.push_back(keys_2[dmatchs[i][1].trainIdx].pt);
			up_matchs.push_back(dmatchs[i][0]);
			up_matchs.push_back(dmatchs[i][1]);
		}
		
	}
	cout << "�Ľ���FSC�㷨�;����ƥ������������ǣ� " << low_matchs.size() << endl;
	cout << "�Ľ���FSC�㷨�߾����ƥ������������ǣ� " << up_matchs.size() << endl;

	//ʹ��FSC�㷨ɾ��������
	vector<bool> inliers;
	float rmse;
	Mat homography = FSC(points1_low, points2_low, points1_up, points2_up, model, ransac_error, inliers, rmse);

	//��ȡ������ȷƥ����
	auto itt = up_matchs.begin();
	for (auto it = inliers.begin(); it != inliers.end(); ++it, ++itt)
	{
		if (*it)//�������ȷƥ����
		{
			right_matchs.push_back((*itt));
		}

	}
	cout << "ʹ�øĽ���FSCɾ�������Է�����ȷƥ������� " << right_matchs.size() << endl;
	cout << "���rmse: " << rmse << endl;

	//���Ƴ�ʼƥ��������ͼ
	Mat initial_matched;
	drawMatches(image_1, keys_1, image_2, keys_2, low_matchs,
		initial_matched, Scalar(0, 255, 0), Scalar(255, 0, 0),
		vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
	imwrite(".\\image_save\\��ʼƥ����.jpg", initial_matched);

	//������ȷƥ��������ͼ
	drawMatches(image_1, keys_1, image_2, keys_2, right_matchs,
		matched_line, Scalar(0, 255, 0), Scalar(255, 0, 0),
		vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
	imwrite(".\\image_save\\��ȷƥ����.jpg", matched_line);

	//�������ʾ��⵽��������
	Mat keys_image_1, keys_image_2;
	drawKeypoints(image_1, keys_1, keys_image_1, Scalar(0, 255, 0), DrawMatchesFlags::DEFAULT);
	drawKeypoints(image_2, keys_2, keys_image_2, Scalar(0, 255, 0), DrawMatchesFlags::DEFAULT);
	imwrite(".\\image_save\\�ο�ͼ���⵽��������.jpg", keys_image_1);
	imwrite(".\\image_save\\����׼ͼ���⵽��.jpg", keys_image_2);

	return homography;
}
