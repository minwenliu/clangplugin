/*=============================================================================
#     FileName: macro.h
#         Desc: 
#       Author: minwenliu
#        Email: minwenliu@tencent.com
#     HomePage: https://github.com/minwenliu
#       Editor: vim (ts=4,sw=4,et)
#      Version: 0.0.1
#   LastChange: 2016-04-29 16:52:02
#      History:
=============================================================================*/
#ifndef _MACRO_H_
#define _MACRO_H_

#define GETSETVAR(type, name) \
    public: \
const type& name() const { return name##_; } \
void set_##name(const type& newval) { name##_ = newval; } \
private: \
type name##_;
#define GETSETCSTRING(name, maxsize) \
    public: \
const char* name() const { return name##_; } \
void set_##name(const char* newval) { \
    size_t len = strlen(newval); if (len > maxsize-1) { \
        len=maxsize-1;} memcpy(name##_, newval, len); name##_[len] = 0;} \
void set_##name(std::string newval) { \
    size_t len = newval.length(); if (len > maxsize-1) { \
        len=maxsize-1;} memcpy(name##_, newval.c_str(), len); name##_[len] = 0;} \
private: \
char name##_[maxsize];

#endif    /* --- #ifndef MACRO_H ---*/
