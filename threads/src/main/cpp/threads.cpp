#include <jni.h>
#include <cstdio>
#include <unistd.h>
#include <pthread.h>
#include <android/log.h>
#include <semaphore.h>
//
// Created by Joky on 2020/5/27 0027.
//

struct NativeWorkArgs {
    jint id;
    jint iterations;
};

static jmethodID gOnNativeMessage = nullptr;
JavaVM *gVm = nullptr;
jobject gobj = nullptr;
static pthread_mutex_t mutex;


jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    __android_log_print(5,"jni_onload","JNI_Onload invoked");
    gVm = vm;
    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_threads_MainActivity_nativeInit(JNIEnv *env, jobject thiz) {
    if(0!= pthread_mutex_init(&mutex, nullptr)){
        jclass exceptionClass = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exceptionClass,"unable to init mutex");
        goto exit;
    }
    if (gobj == nullptr) {
        gobj = env->NewGlobalRef(thiz);
        if (gobj == nullptr)
            goto exit;
    }
    if (nullptr == gOnNativeMessage) {
        jclass clazz = env->GetObjectClass(thiz);
        gOnNativeMessage = env->GetMethodID(clazz, "onNativeMessage", "(Ljava/lang/String;)V");
        if (nullptr == gOnNativeMessage) {
            jclass exceptionClazz = env->FindClass("java/lang/RuntimeException");
            env->ThrowNew(exceptionClazz, "unable to find method");
        }
    }
    exit:
    return;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_threads_MainActivity_nativeFree(JNIEnv *env, jobject thiz) {
    if (gobj != nullptr) {
        env->DeleteGlobalRef(gobj);
        gobj = nullptr;
    }
    if(0!= pthread_mutex_destroy(&mutex)){
        jclass exceptionClass = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exceptionClass,"unable to destroy mutex");
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_threads_MainActivity_nativeWorker(JNIEnv *env, jobject thiz, jint id,
                                                   jint iterations) {
    if(0!= pthread_mutex_lock(&mutex)){
        jclass exceptionClass = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exceptionClass,"unable to lock mutex");
        goto exit;
    }
    for (jint i = 0; i < iterations; i++) {
        char message[26];
        sprintf(message, "Worker %d: Iteration %d", id, i);\
        jstring messageString = env->NewStringUTF(message);
        env->CallVoidMethod(thiz, gOnNativeMessage, messageString);
        if (nullptr != env->ExceptionOccurred()) break;
        sleep(1);
    }
    if(0!= pthread_mutex_unlock(&mutex)){
        jclass exceptionClass = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(exceptionClass,"unable to unlock mutex");
    }
    exit:
    return;
}

static void *nativeWorkerThread(void *args) {
    JNIEnv *env = nullptr;
    if (0 == gVm->AttachCurrentThread(&env, nullptr)) {
        auto *nativeWorkArgs = (NativeWorkArgs *) args;
        Java_com_example_threads_MainActivity_nativeWorker(env, gobj, nativeWorkArgs->id,
                                                           nativeWorkArgs->iterations);
        delete nativeWorkArgs;
        gVm->DetachCurrentThread();
    }
    return (void *) 1;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_threads_MainActivity_posixThreads(JNIEnv *env, jobject thiz, jint threads,
                                                   jint iterations) {
    pthread_t* handles = new pthread_t[threads];
    for (int i = 0; i < threads; ++i) {
        auto *nativeWorkArgs = new NativeWorkArgs();
        nativeWorkArgs->iterations = iterations;
        nativeWorkArgs->id = i;
        int result = pthread_create(&handles[i], nullptr, nativeWorkerThread, (void *) nativeWorkArgs);
        if(result != 0){
            jclass clazz = env->FindClass("java/lang/RuntimeException");
            env->ThrowNew(clazz,"unable to create thread");
            goto exit;
        }
    }

//    for (int i = 0; i < threads; ++i) {
//        void* result = nullptr;
//        if(0!=pthread_join(handles[i],&result)){
//            jclass clazz = env->FindClass("java/lang/RuntimeException");
//            env->ThrowNew(clazz,"unable to join thread");
//        }else{
//            char message[26];
//            sprintf(message,"Worker %d returned %d",i,result);
//            jstring messageString = env->NewStringUTF(message);
//            env->CallVoidMethod(thiz,gOnNativeMessage,messageString);
//            if (nullptr!= env->ExceptionOccurred()){
//                goto exit;
//            }
//        }
//    }
    exit:
    return;
}

