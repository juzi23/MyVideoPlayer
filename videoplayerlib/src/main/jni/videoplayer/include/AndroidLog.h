//
// 用来控制日志的打印
//

#ifndef MYFFMPEGPLAYER_ANDROIDLOG_H
#define MYFFMPEGPLAYER_ANDROIDLOG_H

#include <android/log.h>
#include <stdio.h>
#include <string>
#include <malloc.h>

#define ISDEBUG true

void LOGD(const char * varChar,...);
void LOGE(const char * varChar,...);
#endif //MYFFMPEGPLAYER_ANDROIDLOG_H
