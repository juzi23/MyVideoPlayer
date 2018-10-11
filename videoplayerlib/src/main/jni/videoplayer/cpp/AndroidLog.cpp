#include "AndroidLog.h"

void LOGD(const char * varChar,...){
    if (ISDEBUG) {
        va_list pArgList;
        va_start(pArgList, varChar);
        __android_log_vprint(ANDROID_LOG_DEBUG,"System.out",varChar,pArgList);
        va_end(pArgList);
    }
}

void LOGE(const char * varChar,...){
    if (ISDEBUG) {
        va_list pArgList;
        va_start(pArgList, varChar);
        __android_log_vprint(ANDROID_LOG_ERROR,"System.out",varChar,pArgList);
        va_end(pArgList);
    }
}

