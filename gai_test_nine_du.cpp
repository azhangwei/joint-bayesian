
#include<iostream>
#include<string>
#include<io.h>
#include<vector>
#include <opencv2\opencv.hpp>
#include "cv.h"
#include "highgui.h"
#include <fstream>
#include "time.h"
using namespace cv;
using namespace std;
//const int length_chac=5824;//特征向量维数
const int EM_time=3;//em迭代次数
const double G_weight=2.0;
const long int MAX=30*1024*1024;//一次性开辟缓存大小
int main()
{    

    string path="D:/data_2000_600_2400.txt";//特征txt保存位置，格式为： 特征长度 总共人数 每个人对应样本数 相应特征
	int max_person_num=0;//个人最大数
	int max_person_num_test=0;//训练人最大数量
	int total_train=0;//训练数据量
	int total_test=0;
	double data=0;

	int total_num_start;
    int train_num=0;
	int test_num=0;
	int length_chac;
	//读取文件

	FILE *fp;
    fp=fopen(path.c_str(),"rb");
	fread(&length_chac,4,1,fp);
	fread(&total_num_start,4,1,fp);
	float *mem;
	float *memm;
	mem=(float*)malloc(length_chac*sizeof(float));
	memm=(float*)malloc(total_num_start*sizeof(float));

	float *max;
	max=(float*)malloc(MAX);

	long int total_length=0;

	fread(memm,4,total_num_start,fp);
	vector<int>person_num_individual(total_num_start);//每个人的样本数
	int num_individual_sum=0;//人数累加量
	vector<vector<int>>groups(10);//将每个人的样本数分为十组
	vector<int>line_groups(total_num_start);//人数分解点
	int size_for_groups;

	for(int i=0;i<total_num_start;i++)
	{
		person_num_individual[i]=*(memm+i);
		if(max_person_num<person_num_individual[i])
				max_person_num=person_num_individual[i];
	}

	long int num_for_long=0;
	int num_long_for_person=0;
    bool for_long=0;
	for(int i=0;i<total_num_start;i++)
	{
		for(int j=0;j<person_num_individual[i];j++)
		{
			num_for_long+=length_chac*4;
			if(num_for_long>MAX)
			{   
				for_long=1;
				break;
			}	
		}
		if(for_long)
			break;
		num_long_for_person++;
		total_length+=person_num_individual[i];
	}

	fread(max,4,total_length*length_chac,fp);
	train_num=num_long_for_person*5/6;
	total_train=0;
	for(int i=0;i<train_num;i++)
	{
		size_for_groups=person_num_individual[i]/10;
		groups[size_for_groups].push_back(i);
		total_train+=person_num_individual[i];
		line_groups[i]=total_train;
	}

	test_num=num_long_for_person-train_num;
	vector<vector<vector<double>>> ghost(train_num,vector<vector<double>>(max_person_num,vector<double>(length_chac)));
	vector<vector<vector<double>>> ghost_test(test_num,vector<vector<double>>(max_person_num,vector<double>(length_chac)));
    vector<double> all_m(length_chac);//所有均值
	vector<vector<double>> person_m(train_num,vector<double>(length_chac));//每个人的均值

	int individual=0;

	for(int i=0;i<train_num;i++)
	{
		for(int j=0;j<person_num_individual[i];j++)
			{
			   for(int mm=0;mm<length_chac;mm++)
			   { 
				   data=*(max+mm+individual*length_chac);
				   ghost[i][j][mm]=data;

			   }
			   individual++;
			}
	}
	for(int i=0;i<test_num;i++)
	{
		for(int j=0;j<person_num_individual[i+train_num];j++)
			{
			   for(int mm=0;mm<length_chac;mm++)
			   {
				   data=*(max+mm+individual*length_chac);
				   ghost_test[i][j][mm]=data;

			   }
			   individual++;
			}
	}

	fclose(fp);
	delete max;

for(int i=0;i<train_num;i++)
{
  for(int j=0;j<person_num_individual[i];j++)
  {
	  for(int mm=0;mm<length_chac;mm++)
		{
			 all_m[mm]+=ghost[i][j][mm];
	    }
  }
}
for(int mm=0;mm<length_chac;mm++)
{
	all_m[mm]/=total_train;
}

for(int i=0;i<train_num;i++)
   {
		for(int j=0;j<person_num_individual[i];j++)
			{
			 for(int k=0;k<length_chac;k++)
			 {
				 ghost[i][j][k]=ghost[i][j][k]-all_m[k];
			 }
			}
	}
for(int i=0;i<test_num;i++)
   {
		for(int j=0;j<person_num_individual[i+train_num];j++)
			{
			 for(int k=0;k<length_chac;k++)
			 {
				 ghost_test[i][j][k]=ghost_test[i][j][k]-all_m[k];
			 }
			}
	}
		    

//求取每个人的特征均值(在全部零均值之后的数据上操作）
	for(int i=0;i<train_num;i++)
	  for(int mm=0;mm<length_chac;mm++)
	  {
		for(int j=0;j<person_num_individual[i];j++)
			   {
				   person_m[i][mm]+=ghost[i][j][mm];
			   }
		person_m[i][mm]/=person_num_individual[i];//保存每个人的特征均值。
	  }


//PCA
//下面进入EM迭代求解(首先创建相应变量）

	 CvMat* X_person=cvCreateMat(total_train,length_chac,CV_64FC1);
	 CvMat* X_person_test=cvCreateMat(total_train,length_chac,CV_64FC1);//测试矩阵
	 CvMat* X_person_tem=cvCreateMat(total_train,length_chac,CV_64FC1);
	 CvMat* X_person_T=cvCreateMat(length_chac,total_train,CV_64FC1);
	 CvMat* X_person_TT=cvCreateMat(length_chac,total_train,CV_64FC1);
	 CvMat* X_individual=cvCreateMat(length_chac,1,CV_64FC1);
	 CvMat* X_individual_T=cvCreateMat(length_chac,1,CV_64FC1);
	 CvMat* E_person=cvCreateMat(total_train,length_chac,CV_64FC1);
	 CvMat* U_total=cvCreateMat(train_num,length_chac,CV_64FC1);
	 CvMat* Su_mat=cvCreateMat(length_chac,length_chac,CV_64FC1);
	 CvMat* Se_mat=cvCreateMat(length_chac,length_chac,CV_64FC1);
	 CvMat* F=cvCreateMat(length_chac,length_chac,CV_64FC1);
	 CvMat* G=cvCreateMat(length_chac,length_chac,CV_64FC1);
	 CvMat* U=cvCreateMat(length_chac,length_chac,CV_64FC1);
	 CvMat* G1=cvCreateMat(length_chac,length_chac,CV_64FC1);
	 CvMat* G2=cvCreateMat(length_chac,length_chac,CV_64FC1);
	 CvMat* Gx=cvCreateMat(length_chac,length_chac,CV_64FC1);
	 CvMat* U_mean=cvCreateMat(1,length_chac,CV_64FC1);
	 CvMat* A=cvCreateMat(length_chac,length_chac,CV_64FC1);
	 CvMat* U_2=cvCreateMat(1,length_chac,CV_64FC1);
	 CvMat* U_1=cvCreateMat(length_chac,1,CV_64FC1);
	 CvMat* U_temp=cvCreateMat(length_chac,length_chac,CV_64FC1);
	 //归一化后全体人脸存入X_person
	 num_individual_sum=0;
	  for(int i=0;i<train_num;i++)
	  {
	       for(int j=0;j<person_num_individual[i];j++)
		       for(int mm=0;mm<length_chac;mm++)
			   {
				   cvmSet(X_person,num_individual_sum+j,mm,ghost[i][j][mm]);
			   }
			   num_individual_sum+=person_num_individual[i];
	  }
	//各个类别均值存入U_total
	  for(int i=0;i<train_num;i++)
		    for(int mm=0;mm<length_chac;mm++)
			   {
				   cvmSet(U_total,i,mm,person_m[i][mm]);
			   }
    //所有人脸减去相应均值后存入E_person
		num_individual_sum=0;
	    for(int i=0;i<train_num;i++)
		{   
			for(int j=0;j<person_num_individual[i];j++)
		       for(int mm=0;mm<length_chac;mm++)
			   {
				   cvmSet(E_person,num_individual_sum+j,mm,ghost[i][j][mm]-person_m[i][mm]);
			   }
			   num_individual_sum+=person_num_individual[i];
		}

		 //cvCalcCovarMatrix((const void **)&U_total,1,Su_mat,NULL,CV_COVAR_NORMAL | CV_COVAR_ROWS);

		  for(int i=0;i<length_chac;i++)
			{
                for(int mm=0;mm<length_chac;mm++)
				{
					 cvmSet(Su_mat,i,mm,0.0);
				}
			}
			for(int i=0;i<train_num;i++)
			{
				if(person_num_individual[i]==0)
					continue;
                for(int mm=0;mm<length_chac;mm++)
				{
					data=(double)CV_MAT_ELEM(*U_total,double,i,mm);
					 cvmSet(U_1,mm,0,data);
					 cvmSet(U_2,0,mm,data);
				}
				cvmMul(U_1,U_2,U_temp);
				cvConvertScale(U_temp,U_temp,person_num_individual[i],0 );
				cvAdd(U_temp,Su_mat,Su_mat,0);
			}
            cvConvertScale( Su_mat, Su_mat, 1.0 / ( total_train-1) );
/*			 cvCalcCovarMatrix((const void **)&U_total,1,Su_mat,NULL,CV_COVAR_NORMAL | CV_COVAR_ROWS);        
             cvConvertScale( Su_mat, Su_mat, 1.0 / (train_num - 1 ) ); */    

    //计算E的协方差矩阵
	       cvCalcCovarMatrix((const void **)&E_person,1,Se_mat,NULL,CV_COVAR_NORMAL | CV_COVAR_ROWS);       
           cvConvertScale( Se_mat, Se_mat, 1.0 / ( total_train - 1 ) );     
       

//正式进入EM迭代
 for(int em=0;em<EM_time;em++)
   {

	//求F 文档2公式（5）
	    cvInvert(Se_mat, F, CV_SVD);//CV_LU -最佳主元选取的高斯消除法; CV_SVD - 奇异值分解法 (SVD); CV_SVD_SYM - 正定对称矩阵的 SVD 方法
	//求G 文档2公式（6）
	num_individual_sum=0;
	for(int i=1;i<10;i++)
	{
		    int temp=i*10;
			cvmMul(Su_mat,F,G2);
			cvConvertScale(Su_mat,G1,temp,0 );
			cvAdd(G1,Se_mat,G1,0);
			cvInvert(G1,G1,CV_SVD);
			cvmMul(G1,G2,G);
			cvConvertScale(G,G,-1,0);

		//求U 文档2公式（7）
			cvConvertScale(G,Gx,temp,0 );
			cvAdd(F,Gx,Gx,0);
			cvmMul(Su_mat,Gx,G1);
			cvmMul(Se_mat,G,Gx);
		for(vector<int>::iterator ite=groups[i].begin();ite!=groups[i].end();ite++)
		{
			if(*ite==0)
				num_individual_sum=0;
			else
			num_individual_sum=line_groups[*ite-1];
		   for(int j=0;j<length_chac;j++)
		   {
			    data=0;
				for(int k=0;k<temp;k++)
				{
					data+=(double)CV_MAT_ELEM(*X_person,double,num_individual_sum+k,j);
				}
			   cvmSet(X_individual,j,0,data);
		   }
	 		cvmMul(G1,X_individual,X_individual_T);
			for(int mm=0;mm<length_chac;mm++)
			  {
				  data=(double)CV_MAT_ELEM(*X_individual_T,double,mm,0);
				  cvmSet(U_total,*ite,mm,data);
			  }
		
		 //求e 文档2 公式（8)
			 cvmMul(Gx,X_individual,X_individual_T);
			for(int j=0;j<temp;j++)
			 {
				 for(int mm=0;mm<length_chac;mm++)
				    {
					  data=(double)CV_MAT_ELEM(*X_person,double,num_individual_sum+j,mm);   
					  data+=(double)CV_MAT_ELEM(*X_individual_T,double,mm,0);
					  cvmSet(E_person,num_individual_sum+j,mm,data);
				     }
			}
		}
	
	}
	//计算U的协方差矩阵
		    for(int i=0;i<length_chac;i++)
			{
                for(int mm=0;mm<length_chac;mm++)
				{
					 cvmSet(Su_mat,i,mm,0.0);
				}
			}
			for(int i=0;i<train_num;i++)
			{
				if(person_num_individual[i]==0)
					continue;
                for(int mm=0;mm<length_chac;mm++)
				{
					data=(double)CV_MAT_ELEM(*U_total,double,i,mm);
					 cvmSet(U_1,mm,0,data);
					 cvmSet(U_2,0,mm,data);
				}
				cvmMul(U_1,U_2,U_temp);
				cvConvertScale(U_temp,U_temp,person_num_individual[i],0 );
				cvAdd(U_temp,Su_mat,Su_mat,0);
			}
            cvConvertScale( Su_mat, Su_mat, 1.0 / ( total_train - 1 ) );
    //计算E的协方差矩阵
	       cvCalcCovarMatrix((const void **)&E_person,1,Se_mat,NULL,CV_COVAR_NORMAL | CV_COVAR_ROWS);      
           cvConvertScale( Se_mat, Se_mat, 1.0 / ( total_train- 1 ) );     
   


 }
   //EM算法结束
	 //求F 文档2公式（5）
	 cvInvert(Se_mat, F, CV_LU);//CV_LU -最佳主元选取的高斯消除法; CV_SVD - 奇异值分解法 (SVD); CV_SVD_SYM - 正定对称矩阵的 SVD 方法
	 //求G 文档2公式（6）
	 cvmMul(Su_mat,F,G2);
	 cvConvertScale(Su_mat,G1,2,0 );
	 cvAdd(G1,Se_mat,G1,0);
	 cvInvert(G1,G1,CV_LU);
	 cvmMul(G1,G2,G);
	 cvConvertScale(G,G,-1,0);

	 //求A 文档1 公式（5）
      cvAdd(Su_mat,Se_mat,A,0);
	  cvInvert(A,A,CV_LU);
      cvAdd(F,G,Gx,0);
	  cvConvertScale(Gx,Gx,-1,0);
	  cvAdd(Gx,A,A,0);

	//程序验证阶段(方法1）
	   CvMat* X1=cvCreateMat(1,length_chac,CV_64FC1);
	   CvMat* X2=cvCreateMat(1,length_chac,CV_64FC1);
	   CvMat* X=cvCreateMat(1,length_chac,CV_64FC1);
	   CvMat* X_T=cvCreateMat(length_chac,1,CV_64FC1);
	   CvMat* one=cvCreateMat(1,1,CV_64FC1);

    //想法验证阶段
	   CvMat* New_idea_two=cvCreateMat(length_chac,length_chac,CV_64FC1);
	   CvMat* New_idea_one=cvCreateMat(2*length_chac,length_chac,CV_64FC1);
	   CvMat* New_idea=cvCreateMat(2*length_chac,length_chac,CV_64FC1);
	   CvMat* New_x=cvCreateMat(2*length_chac,1,CV_64FC1);
	   CvMat* New_x_yuan=cvCreateMat(length_chac,1,CV_64FC1);
	   double a=0,b=0,c=0;//相似度三部分  文档1 公式（4）
	   double similar=0;
       vector<vector<double>> C_test(test_num,vector<double>(max_person_num));
	   for(int i=0;i<test_num;i++)
		 for(int j=0;j<person_num_individual[i+train_num];j++)
		 {
			     for(int mm=0;mm<length_chac;mm++)
			      {
				   cvmSet(X1,0,mm,ghost_test[i][j][mm]);
				  }
				   cvmMul(X1,A,X);
				   cvTranspose(X1,X_T);
				   cvmMul(X,X_T,one);
				   a=(double)CV_MAT_ELEM(*one,double,0,0);  //求a
				   C_test[i][j]=a;
				
	    } 
	
	 ofstream dui("same.txt");
	 for(int i=0;i<test_num;i++)
		 for(int j=0;j<person_num_individual[i+train_num]-1;j++)
			 for(int k=j+1;k<person_num_individual[i+train_num];k++)
			 {
			     for(int mm=0;mm<length_chac;mm++)
			      {
				    cvmSet(X1,0,mm,ghost_test[i][j][mm]);
					cvmSet(X2,0,mm,ghost_test[i][k][mm]);
				   }

				   a=C_test[i][j];  //求a
				   b=C_test[i][k];  //求b

				   cvmMul(X1,G,X);
				   cvTranspose(X2,X_T);
				   cvmMul(X,X_T,one);
				   c=-G_weight*(double)CV_MAT_ELEM(*one,double,0,0);//求c

				   similar=a+b+c;
				   dui<<a<<"    "<<b<<"   "<<c<<endl;;
			  } 
			 dui.close();

	 ofstream duii("dif.txt");
	 for(int i=0;i<test_num-1;i++)
		 for(int j=0;j<person_num_individual[i+train_num];j++)
			 for(int k=0;k<person_num_individual[i+1+train_num];k++)
			 {   
			     for(int mm=0;mm<length_chac;mm++)
			      {
				    cvmSet(X1,0,mm,ghost_test[i][j][mm]);
					cvmSet(X2,0,mm,ghost_test[i+1][k][mm]);
				   }
				   a=C_test[i][j];  //求a
				   b=C_test[i+1][k];  //求b
		           cvmMul(X1,G,X);
				   cvTranspose(X2,X_T);
				   cvmMul(X,X_T,one);
				   c=-G_weight*(double)CV_MAT_ELEM(*one,double,0,0);//求c
				   similar=a+b+c;
				   duii<<a<<"    "<<b<<"   "<<c<<endl;;
			  } 
	    duii.close();

		cvReleaseMat(&Su_mat);
        cvReleaseMat(&Se_mat);
        cvReleaseMat(&F);
		cvReleaseMat(&X_person);
        cvReleaseMat(&E_person);
		cvReleaseMat(&X_person_tem);
        cvReleaseMat(&G);
		cvReleaseMat(&G1);
		cvReleaseMat(&G2);
		cvReleaseMat(&U_total);
		cvReleaseMat(&X_person_test);
        cvReleaseMat(&X_person_T);
		cvReleaseMat(&X_person_TT);
        cvReleaseMat(&U);
		cvReleaseMat(&Gx);
        cvReleaseMat(&U_mean);
		cvReleaseMat(&A);
        cvReleaseMat(&X1);
		cvReleaseMat(&X2);
		cvReleaseMat(&X);
		cvReleaseMat(&X_T);
		cvReleaseMat(&one);
		cvReleaseMat(&U_1);
		cvReleaseMat(&U_2);
		cvReleaseMat(&U_temp);
		cvReleaseMat(&New_idea);
		cvReleaseMat(&New_idea_two);
		cvReleaseMat(&New_idea_one);
		cvReleaseMat(&New_x);
		cvReleaseMat(&New_x_yuan);
    return 0;

}

