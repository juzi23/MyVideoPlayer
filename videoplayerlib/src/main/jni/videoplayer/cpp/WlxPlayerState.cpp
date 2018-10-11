//
// Created by qq798 on 2018/8/7.
//

#include "WlxPlayerState.h"

WlxPlayerState::WlxPlayerState() {
    init();
}

WlxPlayerState::~WlxPlayerState() {

}

void WlxPlayerState::init() {
    this->currentPlayState = EnumCurrentPlayState ::EXIT;
}
