//
// Created by qq798 on 2018/8/7.
//

#include "OnCallJava.h"

OnCallJava::OnCallJava(JavaVM * jvm, JNIEnv * jenv,jobject jobj) {
    this->jvm = jvm;
    this->jenv = jenv;
    this->jobj = jenv->NewGlobalRef(jobj);
    this->jclz = jenv->GetObjectClass(jobj);
    if(!jclz){
        if(ISDEBUG)
        {
            LOGE("get jclass wrong");
        }
        return;
    }

    jmid_onCallPrepared = jenv->GetMethodID(jclz,"onCallPrepared","()V");
}

OnCallJava::~OnCallJava() {
    this->jvm = NULL;
    this->jenv = NULL;
    this->jobj = NULL;
}

void OnCallJava::callPrepared() {
    JNIEnv * jniEnv = NULL;
    if(jvm->AttachCurrentThread(&jniEnv,NULL) != JNI_OK){
        if(ISDEBUG)
        {
            LOGE("get child thread jnienv worng");
        }
        return;
    }
    jniEnv->CallVoidMethod(jobj,jmid_onCallPrepared);
    jvm->DetachCurrentThread();
}
