#include <jni.h>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/system_properties.h>

#include "my_log.h"

static jstring func1(JNIEnv *env) {
    jintArray javaArray = env->NewIntArray(-1);
    env = 0;
    return env->NewStringUTF("hello from jni");
}

extern "C"
JNIEXPORT jstring JNICALL Java_com_example_hellojni_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject thiz) {
//    func1(env);
    getpid();
    getuid();
    std::string hello = "Hello from C++";
    __android_log_write(ANDROID_LOG_WARN, "hello-jni", "warning log");
    MY_LOG_VERBOSE("The stringFromJNI is called.");
    MY_LOG_DEBUG("env=%p thiz=%p", env, thiz);
    MY_LOG_ASSERT(0 != env, "JNIEnv cannot be NULL");
    MY_LOG_INFO("return a new string");
    return env->NewStringUTF(hello.c_str());
}

static void dynamicMalloc() {
    int *dynamicIntArray = (int *) malloc(sizeof(int) * 16);

    int *newDynamicIntArray = (int *) realloc(dynamicIntArray, sizeof(int) * 32);

    if (NULL == newDynamicIntArray) {
        //不能分配足够内存
    } else {
        dynamicIntArray = newDynamicIntArray;
    }

    if (NULL == dynamicIntArray) {
        //不能分配足够内存
    } else {
        *dynamicIntArray = 0;
        dynamicIntArray[8] = 8;
        free(dynamicIntArray);
        *dynamicIntArray = NULL;
    }
}

static void ioFunc() {
    FILE *stream = fopen("/data/data/com.example.hellojni/test.txt", "w");
    if (NULL == stream) {
        //写文件打不开
    } else {
        char data[] = {'h', 'e', 'l', 'l', 'o', '\n'};
        int count = sizeof(data) / sizeof(data[0]);
        if (count != fwrite(data, sizeof(char), count, stream)) {
            //写入发生错误
        }
        if (EOF == fputs("hello\n", stream)) {
            //写入发生错误
        }
        char c = 'c';
        if (c != fputc(c, stream)) {
            //error
        }
        if (0 > fprintf(stream, "this %s is %d .", "number", 2)) {
            //error
        }
        if (EOF == fflush(stream)) {}

        char buffer[5];
        size_t count2 = 4;
        if (count2 == fread(buffer, sizeof(char), count2, stream)) {
            //error
        } else {
            buffer[4] = NULL;
            MY_LOG_INFO("read : %s", buffer);
        }
        char buffer2[1024];
        if (NULL == fgets(buffer2, 1024, stream)) {
            //error
        } else {
            MY_LOG_INFO("read : %s", buffer2);
        }
        unsigned char ch;
        int result;
        result = fgetc(stream);
        if (EOF == result) {
//            error
        } else {
            ch = static_cast<unsigned char>(result);
        }
        char s[5];
        int i;
        if (2 != fscanf(stream, "this %s is %d", s, &i)) {
            //error
        }
        char buffer3[1024];
        while (0 == feof(stream)) {
            fgets(buffer3, 1024, stream);
            MY_LOG_INFO("read : %s", buffer3);
        }
        fputs("abcd", stream);
        if (0 != fseek(stream, -4, SEEK_CUR)) {
            //error
        }
        fputs("1234", stream);
        if (0 != ferror(stream)) {
            //has error
        }
        if (0 != fclose(stream)) {
            //error
        };

        int r = system("mkdir /data/data/com.example.hellojni/temp");
        if (r == -1 || r == 127) {
            //error
        }

        FILE *command;
        command = popen("ls", "r");
        if (command != NULL) {
            char buffer5[1024];
            int status;
            while (NULL != fgets(buffer5, 1024, command)) {
                MY_LOG_INFO("read : %s", command);
            }
            status = pclose(command);
            MY_LOG_INFO("command exited with status %d", status);
        } else {
            MY_LOG_INFO("unable to execute command");
        }

        char value[PROP_VALUE_MAX];
        if (0 == __system_property_get("ro.product.model", value)) {
            //not find or null
        } else {
            MY_LOG_INFO("product model : %s", value);
        }

        const prop_info *property;
        property = __system_property_find("ro.product.model");
        if (NULL == property) {
            //not find
        } else {
            char name[PROP_NAME_MAX];
            char value2[PROP_VALUE_MAX];
            if (__system_property_read(property, name, value2) != 0) {
                MY_LOG_INFO("%s : %s", name, value2);
            } else {
                MY_LOG_INFO("%s is empty", name);
            }
        }
        //用户ID
        uid_t uid = getuid();
        //组ID
        gid_t gid = getgid();
        //用户名
        char *username;
        username = getlogin();
        MY_LOG_INFO("username: %s , uid: %d , gid: %d",username,uid,gid);
    }
}