//
//#include<iostream>
//#include<string>
//#include<io.h>
//#include<vector>
//#include <opencv2\opencv.hpp>
//#include "cv.h"
//#include "highgui.h"
//#include <fstream>
//#include "time.h"
//using namespace cv;
//using namespace std;
//
////const int length_chac=5824;//特征向量维数
//const int length_chac=1000;//特征向量维数
//const int length_ori=2000;//特征向量维数
//vector<vector<string>> name(2600);
//const bool new_idea=0;
//const int total_num=2500;
//
//void filesearch(string path,int layer,int num_name,bool flag)
//
//{   
//	if(flag)
//		num_name++;
//	
//    struct _finddata_t filefind;
//    string curr=path+"\\*.*";
//	string cun;
//    int done=0,i,handle;
//    if((handle=_findfirst(curr.c_str(),&filefind))==-1)return;
//    while(!(done=_findnext(handle,&filefind)))
//    {
//        if(!strcmp(filefind.name,".."))
//			continue;
//        for(i=0;i<layer;i++)cout<<" ";
//        if ((_A_SUBDIR==filefind.attrib))
//        {      
//            cout<<filefind.name<<"(dir)"<<endl;
//			num_name++;
//            curr=path+"\\"+filefind.name;
//            filesearch(curr,layer+1,num_name,!flag);
//        }
//        else 
//        {
//            cout<<filefind.name<<endl;
//			cun=path+"\\"+filefind.name;
//			name[num_name].push_back(cun);
//        }
//    }    
//    _findclose(handle);      
//}
//int main()
//{    
//
//
//    string path="E:\\tezheng2";
//    filesearch(path,2,-2,0);
//	int max_person_num=0;//个人最大数
//	int per_num_test_max=0;//训练人最大数量
//	int total_train=0;//训练数据量
//	double data=0;
//
//	//时间测试
//
//	int all_num=0;
//	string address;
//	vector<int>person_num_individual(total_num);//每个人的样本数
//	int num_individual_sum=0;//人数累加量
//	vector<vector<int>>groups(10);//将每个人的样本数分为十组
//	vector<int>line_groups(total_num);//人数分解点
//	float *mem;
//	mem=(float*)malloc((length_chac)*sizeof(float));
//
//	string ress=".txt";
//    string add= "D:/data";
//    FILE *XIE;
//    address=add+ress;
//    XIE=fopen(address.c_str(),"wb");
//    fwrite(&length_chac,4,1,XIE);
//	fwrite(&(total_num),4,1,XIE);
//	int size_for_groups;
//	for(int i=0;i<total_num;i++)
//	{
//		size_for_groups=name[i].size();
//		size_for_groups/=10;
//		if(size_for_groups>9)
//			size_for_groups=9;
//		groups[size_for_groups].push_back(i);
//		person_num_individual[i]=size_for_groups*10;
//		total_train+=person_num_individual[i];
//		line_groups[i]=total_train;
//		if(person_num_individual[i]>max_person_num)
//			max_person_num=person_num_individual[i];
//	}
//
//	float cun_one[total_num];
//	float cun_chac[length_chac];
//
//	for(int i=0;i<total_num;i++)
//	{
//		cun_one[i]=person_num_individual[i];
//	}
//	fwrite(cun_one,4,total_num,XIE);
//
//
//	  for(int i=0;i<total_num;i++)
//	   {
//		for(int j=0;j<person_num_individual[i];j++)
//			{
//				FILE *fp;
//				address=name[i].at(j);
//                fp=fopen(address.c_str(),"rb");		
//                fread(mem,1,100,fp);
//                fread(mem,4,length_chac,fp);
//				fclose(fp);
//			   for(int mm=0;mm<length_chac;mm++)
//			   {
//				   data=*(mem+mm);
//				   cun_chac[mm]=data;
//			   }
//			    fwrite(cun_chac,4,length_chac,XIE);
//			}
//	   }
//
// fclose(XIE);
// return 0;
//}
//
//
//	
//
//
//
//	
//
