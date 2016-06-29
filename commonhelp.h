/*=============================================================================
#     FileName: commonhelp.h
#         Desc:  公共的东西
#       Author: minwenliu
#        Email: minwenliu@tencent.com
#     HomePage: https://github.com/minwenliu
#       Editor: vim (ts=4,sw=4,et)
#      Version: 0.0.1
#   LastChange: 2016-04-20 16:16:31
#      History:
=============================================================================*/
#ifndef _COMMONHELP_H_
#define _COMMONHELP_H_
#include "macro.h" 
#include <string> 
#include <string.h> 
#include <fstream> 
#include <vector> 
#include "log.h" 
#include <unistd.h> 
#include <dirent.h> 
using namespace std;
#define COMMANDARGLEN 20
namespace{
    class Help{
        public:
            static string& Trim(string& str)
            {
                if(str.empty())
                    return str;
                str.erase(0,str.find_first_not_of(" "));
                str.erase(str.find_last_not_of(" ")+1);
                return str;
            }
            static void Spilt(const string& str,const string& sp,vector<string>& vecs)
            {
                std::string::size_type pos;
                string::size_type size=str.size();
                for(string::size_type i=0;i<size;)
                {
                    pos=str.find(sp,i);
                    string tmps;
                    if(pos!=str.npos)
                    {
                        tmps=str.substr(i,pos-i);
                        i=pos+sp.size();
                        vecs.push_back(tmps);
                    }
                    else
                    {
                        tmps=str.substr(i);
                        vecs.push_back(tmps);
                        break;
                    }
                }
            }
            static void PathConversion(string& path)
            {

                if(path[0]!='/')
                {
                    char dir[200]={0};
                    getcwd(dir,200);
                    path=dir+("/"+path);
                }
                string::size_type pos=path.find("/./");
                if(pos!=path.npos)
                    path.erase(pos+1,2);
                pos=path.find("..");
                while(pos!=path.npos)
                {
                    string::size_type pos1=path.rfind("/",pos-2);
                    if(pos1!=path.npos)
                    {
                        path.erase(pos1,pos-pos1+2);
                        pos=path.find("..");
                    }
                    else 
                        break;
                }
            }
            static string GetSvrName(const string& path)
            {
                string rootdir="server"; //todo  load config
                string::size_type pos=path.find(rootdir);
                if(pos==path.npos)
                    return "";
                string::size_type startpos=path.find("/",pos);
                if(startpos==path.npos)
                    return "";
                string::size_type endpos=path.find("/",startpos+1);
                if(endpos==path.npos)
                    return "";
                return path.substr(startpos+1,endpos-startpos-1);
            }
            static bool IsSameSvr(const string& pathA,const string& pathB)
            {
                string svrA=GetSvrName(pathA);
                return svrA==GetSvrName(pathB)&&svrA!="";
            }

    };
    struct str_commamdargs{
        str_commamdargs()
        {
            set_filterfile("filter.txt");
            set_datafilename("Data.dat");
        }
        GETSETCSTRING(filterfile,COMMANDARGLEN)
            GETSETCSTRING(datafilename,COMMANDARGLEN)
    };

    struct str_filter{
        std::vector<string> filters;
        void load(const string filterfile)
        {
            ifstream fin(filterfile.c_str());
            const int TMP_LEN=200;
            char tmp[TMP_LEN]={0};
            //vector<string> splstings; 指定文件名，结构名过滤方式不用
            //while (fin.getline(tmp,TMP_LEN) ) { 
            //    splstings.clear();
            //    string tmpstring=tmp;
            //    Help::Trim(tmpstring);
            //    Help::Spilt(tmpstring,"|",splstings);
            //    string filename=*splstings.begin();
            //    string key;
            //    if(splstings.size()<2)
            //        break;
            //    for(auto it=splstings.begin()+1;it!=splstings.end();it++)
            //    {
            //        key="";
            //        key=filename+*it;
            //        filters.push_back(key);
            //    } 
            //} 
            //指定目录名，在该目录以外定义的结构名不做记录，一般定义到svr以及，第三方库的结构可以排除
            while(fin.getline(tmp,TMP_LEN)){
                string tmpstring=tmp;
                Help::Trim(tmpstring);
                filters.push_back(tmpstring);
            }
        }

        void DebugString()
        {
            string tmp="";
            for(auto it=filters.begin();it!=filters.end();it++)
                tmp=tmp+*it; 
            ERROR("%s",tmp.c_str());

        }
    };
}
#endif    /* --- #ifndef COMMONHELP_H ---*/
