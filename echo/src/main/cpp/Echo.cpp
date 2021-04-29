//
// Created by Joky on 2020/6/7 0007.
//
#include <jni.h>
#include <cstdio>
#include <cstdarg>
#include <cerrno>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <cstddef>

#define MAX_LOG_MESSAGE_LENGTH 256
#define MAX_BUFFER_SIZE 80

static void LogMessage(JNIEnv *env, jobject obj, const char *format, ...) {
    static jmethodID methodId = NULL;
    if (NULL == methodId) {
        jclass clazz = env->GetObjectClass(obj);
        methodId = env->GetMethodID(clazz, "logMessage", "(Ljava/lang/String;)V");
        env->DeleteLocalRef(clazz);
    }
    if (NULL != methodId) {
        char buffer[MAX_BUFFER_SIZE];
        va_list ap;
        va_start(ap, format);
        vsnprintf(buffer, MAX_LOG_MESSAGE_LENGTH, format, ap);
        va_end(ap);
        jstring message = env->NewStringUTF(buffer);
        if (NULL != message) {
            env->CallVoidMethod(obj, methodId, message);
            env->DeleteLocalRef(message);
        }
    }
}

static void ThrowException(JNIEnv *env, const char *className, const char *message) {
    jclass clazz = env->FindClass(className);
    if (NULL != clazz) {
        env->ThrowNew(clazz, message);
        env->DeleteLocalRef(clazz);
    }
}

static void ThrowErrnoException(JNIEnv *env, const char *className, int errnum) {
    char buffer[MAX_LOG_MESSAGE_LENGTH];
    if (-1 == strerror_r(errnum, buffer, MAX_LOG_MESSAGE_LENGTH)) {
        strerror_r(errno, buffer, MAX_LOG_MESSAGE_LENGTH);
    }
    ThrowException(env, className, buffer);
}

static int NewTcpSocket(JNIEnv *env, jobject obj) {
    LogMessage(env, obj, "Constructing a new TCP socket...");
    int tcpSocket = socket(PF_INET, SOCK_STREAM, 0);
    if (-1 == tcpSocket) {
        ThrowErrnoException(env, "java/io/IOException", errno);
    }
    return tcpSocket;
}

static void BindSocketToPort(JNIEnv *env, jobject obj, int sd, unsigned short port) {
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = PF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);
    LogMessage(env, obj, "Binding to port %hu.", port);

    if (-1 == bind(sd, (struct sockaddr *) &address, sizeof(address))) {
        ThrowErrnoException(env, "java/io/IOException", errno);
    }
}

static unsigned short getSocketPort(JNIEnv *env, jobject obj, int sd) {
    unsigned short port = 0;
    struct sockaddr_in address;
    socklen_t addresslength = sizeof(address);
    if (-1 == getsockname(sd, (struct sockaddr *) &address, &addresslength)) {
        ThrowErrnoException(env, "java/io/IOException", errno);
    } else {
        port = ntohs(address.sin_port);
        LogMessage(env, obj, "Binded to random port %hu.", port);
    }
    return port;
}

static void listenOnSocket(JNIEnv *env, jobject obj, int sd, int backlog) {
    LogMessage(env, obj, "listen on socket with a backlog of &d ", backlog);
    if (-1 == listen(sd, backlog)) {
        ThrowErrnoException(env, "java/io/IOException", errno);
    }
}

static void
logAddress(JNIEnv *env, jobject obj, const char *message, const struct sockaddr_in *address) {
    char ip[INET_ADDRSTRLEN];
    if (NULL == inet_ntop(PF_INET, &(address->sin_addr), ip, INET_ADDRSTRLEN)) {
        ThrowErrnoException(env, "java/io/IOException", errno);
    } else {
        unsigned short port = ntohs(address->sin_port);
        LogMessage(env, obj, "%s %s:%u.", message, ip, port);
    }
}

static int AcceptOnSocket(JNIEnv *env, jobject obj, int sd) {
    struct sockaddr_in address;
    socklen_t addressLength = sizeof(address);

    LogMessage(env, obj, "Waiting for client connection...");
    int clientSocket = accept(sd, (struct sockaddr *) &address, &addressLength);
    if (-1 == clientSocket) {
        ThrowErrnoException(env, "java/io/IOException", errno);
    } else {
        logAddress(env, obj, "Client connection from ", &address);
    }

    return clientSocket;
}

static ssize_t
ReceiveFromSocket(JNIEnv *env, jobject obj, int sd, char *buffer, size_t bufferSize) {
    LogMessage(env, obj, "Receiving from the socket...");
    size_t recvSize = recv(sd, buffer, bufferSize - 1, 0);
    if (-1 == recvSize) {
        ThrowErrnoException(env, "java/io/IOException", errno);
    } else {
        buffer[recvSize] = NULL;
        if (recvSize > 0) {
            LogMessage(env, obj, "Received %d bytes: %s", recvSize, buffer);
        } else {
            LogMessage(env, obj, "Client disconnected.");
        }
    }
    return recvSize;
}

static ssize_t
SendToSocket(JNIEnv *env, jobject obj, int sd, const char *buffer, size_t bufferSize) {
    LogMessage(env, obj, "Send to the socket...");
    size_t sendSize = send(sd, buffer, bufferSize - 1, 0);
    if (sendSize == -1) {
        ThrowErrnoException(env, "java/io/IOException", errno);
    } else {
        if (sendSize == 0) {
            LogMessage(env, obj, "Client disconnected.");
        } else {
            LogMessage(env, obj, "Send %d bytes: %s", sendSize, buffer);
        }
    }
    return sendSize;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jokyxray_echo_EchoServerActivity_nativeStartTcpServer(JNIEnv *env, jobject thiz,
                                                               jint port) {
    int serverSocket = NewTcpSocket(env, thiz);
    if (NULL == env->ExceptionOccurred()) {
        BindSocketToPort(env, thiz, serverSocket, port);
        if (NULL != env->ExceptionOccurred()) goto exit;
        if (0 == port) {
            getSocketPort(env, thiz, serverSocket);
            if (NULL != env->ExceptionOccurred()) goto exit;
        }
        listenOnSocket(env, thiz, serverSocket, 4);
        if (NULL != env->ExceptionOccurred()) goto exit;
        int clientSocket = AcceptOnSocket(env, thiz, serverSocket);
        if (NULL != env->ExceptionOccurred()) goto exit;
        char buffer[MAX_BUFFER_SIZE];
        ssize_t receiveSize;
        ssize_t sendSize;
        while (1) {
            receiveSize = ReceiveFromSocket(env, thiz, clientSocket, buffer, MAX_BUFFER_SIZE);
            if (0 == receiveSize || NULL != env->ExceptionOccurred()) break;
            sendSize = SendToSocket(env, thiz, clientSocket, buffer, (size_t) receiveSize);
            if (0 == sendSize || NULL != env->ExceptionOccurred()) break;
        }
        close(clientSocket);
    }
    exit:
    if (serverSocket > 0) {
        close(serverSocket);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_jokyxray_echo_EchoServerActivity_nativeStartUdpServer(JNIEnv *env, jobject thiz,
                                                               jint port) {

}