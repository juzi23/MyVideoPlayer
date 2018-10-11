//
// Created by qq798 on 2018/10/11.
//

#ifndef MYVIDEOPLAYER_MYTIME_H
#define MYVIDEOPLAYER_MYTIME_H

#include <chrono>
// 获取当前时间 ms
long long inline getCurrentTimeMS(){
    auto time_now = std::chrono::system_clock::now();
    auto duration_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_now.time_since_epoch());
    return duration_in_ms.count();
}

#endif //MYVIDEOPLAYER_MYTIME_H
