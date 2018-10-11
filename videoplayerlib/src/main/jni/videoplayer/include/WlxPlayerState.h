//
// Created by qq798 on 2018/8/7.
//

#ifndef MYFFMPEGPLAYER_WLXPLAYERSTATE_H
#define MYFFMPEGPLAYER_WLXPLAYERSTATE_H

#include "EnumCurrentPlayState.h"

class WlxPlayerState {
public:
    EnumCurrentPlayState currentPlayState;

public:
    WlxPlayerState();
    ~WlxPlayerState();

    void init();
};


#endif //MYFFMPEGPLAYER_WLXPLAYERSTATE_H
