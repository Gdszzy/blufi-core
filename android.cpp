#include <blufi.h>
#include <jni.h>
#include <map>

// every instance map to a callback object
std::map<jlong, jobject> callbackMap;

void throwRuntimeException(JNIEnv *env, const char *message) {
  jclass cls = env->FindClass("java/lang/RuntimeException");
  env->ThrowNew(cls, message);
}

extern "C" {

/**
 * create new core
 */
JNIEXPORT jlong Java_com_gdszzy_blufi_Core_newCore(
    JNIEnv *env, jobject obj,
    // mtu
    jint mtu,
    // callback object has 'void onMessage(Byte,Byte,ByteBuffer)' method
    jobject onMessageCallback) {
  // convert to global
  jobject globalOnMessageCallback = env->NewGlobalRef(onMessageCallback);
  jclass callbackCls = env->GetObjectClass(globalOnMessageCallback);
  jmethodID onMessageMethod =
      env->GetMethodID(callbackCls, "onMessage", "(BBLjava/nio/ByteBuffer;)V");
  // get vm
  JavaVM *vm;
  env->GetJavaVM(&vm);
  // create instance
  blufi::Core *ptr = new blufi::Core(mtu, [=](uint8_t type, uint8_t subType,
                                              uint8_t *data, size_t size) {
    JNIEnv *subEnv;
    vm->GetEnv((void **)&subEnv, JNI_VERSION_1_6);
    subEnv->CallVoidMethod(
        globalOnMessageCallback, onMessageMethod, static_cast<jbyte>(type),
        static_cast<jbyte>(subType), subEnv->NewDirectByteBuffer(data, size));
  });
  jlong res = (jlong)ptr;
  // 存储到全局
  callbackMap[res] = globalOnMessageCallback;
  return res;
}

/**
 * free core
 */
JNIEXPORT void Java_com_gdszzy_blufi_Core_freeCore(JNIEnv *env, jobject obj,
                                                   jlong ptr) {
  blufi::Core *core = (blufi::Core *)ptr;
  // delete core
  delete(core);
  // delete callback
  auto it = callbackMap.find(ptr);
  if(it != callbackMap.end()) {
    jobject callback = it->second;
    env->DeleteGlobalRef(callback);
    callbackMap.erase(it);
  }
}

/**
 * onReceiveData
 */
JNIEXPORT jbyte Java_com_gdszzy_blufi_Core_onReceiveData(JNIEnv *env,
                                                         jobject obj, jlong ptr,
                                                         jbyteArray bytes) {
  blufi::Core *core = (blufi::Core *)ptr;
  jsize len = env->GetArrayLength(bytes);
  jbyte *data = env->GetByteArrayElements(bytes, nullptr);
  return core->onReceiveData(std::span((uint8_t *)data, (size_t)len));
}

/**
 * negotiateKey
 */
JNIEXPORT jobject Java_com_gdszzy_blufi_Core_negotiateKey(JNIEnv *env,
                                                          jobject obj,
                                                          jlong ptr) {
  blufi::Core *core = (blufi::Core *)ptr;
  blufi::FlattenBuffer buffer;
  uint8_t ret = core->negotiateKey(buffer);
  if(ret) {
    throwRuntimeException(env, std::to_string(ret).c_str());
  }
  return env->NewDirectByteBuffer(buffer.data(), buffer.size());
}

/**
 * custom
 */
JNIEXPORT jobject Java_com_gdszzy_blufi_Core_custom(JNIEnv *env, jobject obj,
                                                    jlong ptr,
                                                    jbyteArray bytes) {
  blufi::Core *core = (blufi::Core *)ptr;
  jsize len = env->GetArrayLength(bytes);
  jbyte *data = env->GetByteArrayElements(bytes, nullptr);
  blufi::FlattenBuffer buffer;
  uint8_t ret = core->custom(std::span((uint8_t *)data, len), buffer);
  if(ret) {
    throwRuntimeException(env, std::to_string(ret).c_str());
  }
  return env->NewDirectByteBuffer(buffer.data(), buffer.size());
}

/**
 * scanWifi
 */
JNIEXPORT jobject Java_com_gdszzy_blufi_Core_scanWifi(JNIEnv *env, jobject obj,
                                                      jlong ptr) {
  blufi::Core *core = (blufi::Core *)ptr;
  blufi::FlattenBuffer buffer;
  uint8_t ret = core->scanWifi(buffer);
  if(ret) {
    throwRuntimeException(env, std::to_string(ret).c_str());
  }
  return env->NewDirectByteBuffer(buffer.data(), buffer.size());
}

/**
 * connectWifi
 */
JNIEXPORT jobject Java_com_gdszzy_blufi_Core_connectWifi(JNIEnv *env,
                                                         jobject obj, jlong ptr,
                                                         jstring ssid,
                                                         jstring pass) {
  blufi::Core *core = (blufi::Core *)ptr;
  const char *ssidLocal = env->GetStringUTFChars(ssid, nullptr);
  const char *passLocal = env->GetStringUTFChars(pass, nullptr);
  blufi::FlattenBuffer buffer;
  uint8_t ret = core->connectWifi(ssidLocal, passLocal, buffer);
  if(ret) {
    throwRuntimeException(env, std::to_string(ret).c_str());
  }
  return env->NewDirectByteBuffer(buffer.data(), buffer.size());
}
}
