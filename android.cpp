#include <blufi.h>
#include <jni.h>
#include <map>

std::map<jlong, jobject> callbackMap;
extern "C" {
JNIEXPORT jlong JNICALL Java_com_gdszzy_blufi_Core_newCore(
    JNIEnv *env, jobject obj, jint mtu, jobject onMessageCallback) {
  jobject globalOnMessageCallback = env->NewGlobalRef(onMessageCallback);
  jclass callbackCls = env->GetObjectClass(globalOnMessageCallback);
  jmethodID onMessageMethod =
      env->GetMethodID(callbackCls, "onMessage", "(BBLjava/nio/ByteBuffer;)V");
  JavaVM *vm;
  env->GetJavaVM(&vm);
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

JNIEXPORT void JNICALL Java_com_gdszzy_blufi_Core_freeCore(JNIEnv *env,
                                                           jobject obj,
                                                           jlong ptr) {
  blufi::Core *core = (blufi::Core *)ptr;
  delete(core);
  // 删除全局引用callback
  auto it = callbackMap.find(ptr);
  if(it != callbackMap.end()) {
    jobject callback = it->second;
    env->DeleteGlobalRef(callback);
    callbackMap.erase(it);
  }
}

JNIEXPORT jbyte JNICALL Java_com_gdszzy_blufi_Core_onReceiveData(
    JNIEnv *env, jobject obj, jlong ptr, jbyteArray bytes) {
  blufi::Core *core = (blufi::Core *)ptr;
  jsize len = env->GetArrayLength(bytes);
  jbyte *data = env->GetByteArrayElements(bytes, nullptr);
  return core->onReceiveData(std::span((uint8_t *)data, (size_t)len));
}

JNIEXPORT jobject JNICALL Java_com_gdszzy_blufi_Core_negotiateKey(JNIEnv *env,
                                                                  jobject obj,
                                                                  jlong ptr) {
  blufi::Core *core = (blufi::Core *)ptr;
  std::ignore = core->negotiateKey();
  auto buffer = core->getFlattenBuffer();
  return env->NewDirectByteBuffer(buffer.data(), buffer.size());
}
JNIEXPORT jobject JNICALL Java_com_gdszzy_blufi_Core_custom(JNIEnv *env,
                                                            jobject obj,
                                                            jlong ptr,
                                                            jbyteArray bytes) {
  blufi::Core *core = (blufi::Core *)ptr;
  jsize len = env->GetArrayLength(bytes);
  jbyte *data = env->GetByteArrayElements(bytes, nullptr);
  std::ignore = core->custom(std::span((uint8_t *)data, len));
  auto buffer = core->getFlattenBuffer();
  return env->NewDirectByteBuffer(buffer.data(), buffer.size());
}

JNIEXPORT jobject JNICALL Java_com_gdszzy_blufi_Core_scanWifi(JNIEnv *env,
                                                              jobject obj,
                                                              jlong ptr) {
  blufi::Core *core = (blufi::Core *)ptr;
  std::ignore = core->scanWifi();
  auto buffer = core->getFlattenBuffer();
  return env->NewDirectByteBuffer(buffer.data(), buffer.size());
}

JNIEXPORT jobject JNICALL Java_com_gdszzy_blufi_Core_connectWifi(
    JNIEnv *env, jobject obj, jlong ptr, jstring ssid, jstring pass) {
  blufi::Core *core = (blufi::Core *)ptr;
  const char *ssidLocal = env->GetStringUTFChars(ssid, nullptr);
  const char *passLocal = env->GetStringUTFChars(pass, nullptr);
  std::ignore = core->connectWifi(ssidLocal, passLocal);
  auto buffer = core->getFlattenBuffer();
  return env->NewDirectByteBuffer(buffer.data(), buffer.size());
}
}
