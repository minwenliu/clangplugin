/*=============================================================================
#     FileName: log.cpp
#         Desc:
#       Author: minwenliu
#        Email: minwenliu@tencent.com
#     HomePage: http://www.vimer.cn
#      Version: 0.0.1
#   LastChange: 2016-04-20 11:14:24
#      History:增加两种输出兼容性 
=============================================================================*/
#include "log.h" 
#include <stdarg.h>
#include <stdio.h>

#ifdef USE_LLVM_OUT
#include "llvm/Support/raw_ostream.h"
//using namespace clang;
#endif

int Log::BuildMsg(const char* file,int linenum,const char* function,const char* format,...)
{
    va_list va;
    int len=0;
#ifndef USE_LLVM_OUT
    len=snprintf(buf,MAX_LOG_LEN,"filename:%s:line:%d:function:%s:",file,linenum,function);
#endif
    va_start(va,format);
    len=vsprintf(buf+len,format,va);
    va_end(va);
    return len;
}

bool Log::check_level(int level)
{
    return level<=level_;
}


int Log::Get_Loglevel()
{
    return(level_); 
}


void Log::Log_Debug()
{
#ifdef USE_LLVM_OUT
    llvm::errs()<<"Debug:"<<buf<<"\n";
#else
        printf("Debug:%s\n",buf);
#endif
}



void Log::Log_Error()
{
#ifdef USE_LLVM_OUT
    llvm::errs()<<"ERROR:"<<buf<<"\n";
#else
        printf("ERROR:%s\n",buf);
#endif
        
}


void Log::Log_Info()
{
#ifdef USE_LLVM_OUT
    llvm::errs()<<"INFO:"<<buf<<"\n";
#else
        printf("INFO:%s\n",buf);
#endif

}


void Log::Log_Warn()
{
#ifdef USE_LLVM_OUT
    llvm::errs()<<"WARN"<<buf<<"\n";
#else
        printf("WARN:%s\n",buf);
#endif
}


void Log::Set_Loglevel(int slevel)
{
    level_=slevel;
}

Log::~Log()
{
}
