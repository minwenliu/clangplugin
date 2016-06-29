/*=============================================================================
#     FileName: Struct_Record.h
#         Desc: 定义结构输出。访问字段文件名|访问字段函数名|访问字段行号|是否被赋值|结构定义所在的文件名|结构定义所在的行号|结构名|被访问的字段 
#       Author: minwenliu 
#        Email: minwenliu@tencent.com 
#     HomePage: https://github.com/minwenliu 
#       Editor: vim (ts=4,sw=4,et) 
#      Version: 0.0.1
#   LastChange: 2016-04-28 11:12:17
#      History:1.增加输出所在的函数名
#              2.增加输出访问时是否是被赋值
=============================================================================*/
#ifndef _STRUCT_RECORD_H_
#define _STRUCT_RECORD_H_
#include <string> 
#include <string.h> 
#include <sstream> 
#include <tr1/unordered_map> 
#include <unordered_set> 
#include <fstream> 
#include <set> 
#include <map> 
#include "log.h" 
#include "macro.h"
using namespace std;
#define FILELENGTH 150
#define RECORDLEN 50
#define FUNCTIONNAMELENGTH 50
struct OutRecord
{
    OutRecord(const char* pfname,string pfuncname,unsigned pline,string pfieldname,const char* prefname,unsigned preline,string precordname,bool pisassign)
    {
        set_filename(pfname);
        set_functionname(pfuncname);
        set_line(pline);
        set_fieldname(pfieldname);
        set_recordfilename(prefname);
        set_recorddefineline(preline);
        set_recordname(precordname);
        set_isassign(pisassign);
    }
    OutRecord()
    {
    }
    GETSETCSTRING(recordfilename,FILELENGTH)
        GETSETVAR(unsigned,recorddefineline)
        GETSETCSTRING(recordname,RECORDLEN)
        GETSETCSTRING(filename, FILELENGTH)
        GETSETCSTRING(functionname,FUNCTIONNAMELENGTH)
        GETSETVAR(unsigned,line)
        GETSETCSTRING(fieldname,RECORDLEN)
        GETSETVAR(bool,isassign)
    public:
        string ToString() const
        {
            std::ostringstream outstream;
            outstream<<"RecordFileName:"<<recordfilename()<<"|"<<"RecordLine:"<<recorddefineline()<<"|"<<"RecordName:"<<recordname() \
                <<"|"<<"File:"<<filename()<<"|"<<"Function:"<<functionname()<<"|"<<"Line:"<<line()<<"|"<<"Fieldname:"<<fieldname()<<"|"<<"IsAssign:"<<isassign(); 
            return outstream.str();
        }
};
//记录的hash函数，先拼接成一个string，然后用string的hash
class HashofOutRecord{
    public:
        size_t operator()(const OutRecord& record) const
        {
            char tmp[1024]={0}; 
            snprintf(tmp,1024,"%s_%d_%s_%s_%d",record.filename(),record.line(),record.fieldname(),record.recordfilename(),record.recorddefineline());
            string tmps=tmp;
            std::hash<std::string> strhash;
            return strhash(tmps);

        }
};
//一条记录数据，访问结构文件名，行号，被访问结构文件名，结构定义行号，字段名可以确定记录唯一性，定义一个函数对象用于判断判断记录是否相等
class EqualOfOutRecord{
    public:
        bool operator()(const OutRecord& rc1,const OutRecord& rc2) const
        {
            int result= strcmp(rc1.filename(),rc2.filename());
            int result2=strcmp(rc1.fieldname(),rc2.fieldname());
            int result3=strcmp(rc1.recordfilename(),rc2.recordfilename());
            return (result==0&&result2==0&&result3==0&&rc1.line()==rc2.line()&&rc1.recorddefineline()&&rc2.recorddefineline());

        }
};
//记录的key，用定义的数据结构名和定义数据结构的行号，可以确定一个唯一的数据结构定义。因此用此二者做key
struct RecordKey
{
    RecordKey(unsigned int pline,const char* pname)
    {
        set_line(pline);
        set_filename(pname);
    }
    RecordKey(){}
    GETSETVAR(unsigned int,line)
        GETSETCSTRING(filename,FILELENGTH)

};
class HashOfRecordKey
{
    public:
        size_t operator()(const RecordKey& key) const
        {
            char tmp[160]={0};
            snprintf(tmp,160,"%s_%d",key.filename(),key.line());
            string tmps=tmp;
            std::hash<std::string> strhash;
            return strhash(tmps);
        }

};
class EqualOfRecordKey
{
    public:
        bool operator()(const RecordKey& key1,const RecordKey& key2) const
        {
            int result= strcmp(key1.filename(),key2.filename());
            return (result==0&&key1.line()==key2.line());
        }

};
//内存中的结构存储。
class MGRRECORD 
{ 
    public: 
        MGRRECORD (){} 
        virtual ~MGRRECORD (){} 
        typedef std::unordered_set<OutRecord,HashofOutRecord,EqualOfOutRecord> RecordSet;
        typedef RecordSet::iterator rsit;
        typedef tr1::unordered_map<RecordKey,RecordSet,HashOfRecordKey,EqualOfRecordKey>::iterator pool_it;
        typedef tr1::unordered_map<RecordKey,RecordSet,HashOfRecordKey,EqualOfRecordKey>::const_iterator poll_cit;
        bool addrecord(const RecordKey& key,const OutRecord& record )
        {
            std::pair<rsit,bool> result=recordpool[key].insert(record);
            return result.second;
        }
        void PrintPoolContent(const string filename) const
        {
            if(recordpool.size()<=0)
                return;
            //int fd=open("Data.dat",O_CREAT);
            ofstream file(filename.c_str(),ios::app);
            auto iter =recordpool.begin();
            auto iter_end=recordpool.end();
            string allcontext="";
            for(;iter!=iter_end;iter++)
            {
                const RecordSet& rcset=iter->second;
                auto set_it=rcset.begin();
                auto set_it_end=rcset.end();
                for(;set_it!=set_it_end;set_it++)
                {
                    const OutRecord& rc=*set_it;
                    //ERROR("%s",rc.ToString().c_str());
                    allcontext=allcontext+rc.ToString()+"\n";
                }
            }
            file<<allcontext<<endl;
            //ERROR("%s",allcontext.c_str());
        }
    private:
        tr1::unordered_map<RecordKey,RecordSet,HashOfRecordKey,EqualOfRecordKey> recordpool; 
}; 

struct st_funcall{
    st_funcall(unsigned int pline,const string& pcallname,const string& pfilename)
    {
        set_callfunction(pcallname);
        set_line(pline);
        set_filename(pfilename);
        setcalleefunction.clear();
    }
    public:
    void addcallee(string calleename)
    {
        setcalleefunction.insert(calleename);
    }
    string DebugString() const
    {
        std::ostringstream tmp;
        tmp<<"FILENAME:"<<filename()<<"|CALLER FUNCTION:"<<callfunction()<<"|LINE:"<<line();
        tmp<<"|CALLEE:{";
        for(auto it=setcalleefunction.begin();it!=setcalleefunction.end();it++)
        {
            tmp<<*it+":";
        }
        tmp<<"}";
        return tmp.str();
    }
    GETSETCSTRING(callfunction,FUNCTIONNAMELENGTH)
        GETSETVAR(unsigned int, line)
        GETSETCSTRING(filename,FILELENGTH)
    private:
        std::set<string> setcalleefunction;

};

class MGRFunctionCall
{
    typedef map<int,st_funcall>::iterator map_it;
    typedef map<int,st_funcall>::const_iterator map_cit;
    public:
    MGRFunctionCall(){funmap.clear();}
    bool insertfuncall(int key,st_funcall& funcall)
    {
        map_it it=funmap.find(key);
        if(it!=funmap.end())
            return false;
        std::pair<map_it,bool> result=funmap.insert(make_pair(key,funcall));
        if(result.second)
            return true;
        return false;
    }
    st_funcall* getByKey(int key)
    {
        map_it it=funmap.find(key);
        if(it!=funmap.end())
            return &it->second;
        return 0;
    }
    void DebugString()const
    {
        if(funmap.size()<=0)
            return;
        for(map_cit it=funmap.begin();it!=funmap.end();it++)
        {
            const st_funcall& tmp=it->second; 
            ERROR("%s",tmp.DebugString().c_str());
        }
    }
    private:
    map<int,st_funcall> funmap;

};
#endif    /* --- #ifndef STRUCT_RECORD_H ---*/
