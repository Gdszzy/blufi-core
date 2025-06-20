#include <blufi.h>
#include <jni.h>

jbyteArray copyToJavaByteArray(JNIEnv *env, const uint8_t *data, size_t size) {
  jbyteArray byteArray = env->NewByteArray(size);
  if(!byteArray)
    return nullptr;
  env->SetByteArrayRegion(byteArray, 0, size,
                          reinterpret_cast<const jbyte *>(data));
  return byteArray;
}

JNIEXPORT jlong JNICALL Java_com_gdszzy_blufi_Core_newCore(
    JNIEnv *env, jobject obj, jint mtu, jobject onMessageCallback) {
  jobject globalOnMessageCallback = env->NewGlobalRef(onMessageCallback);
  jclass callbackCls = env->GetObjectClass(globalOnMessageCallback);
  jmethodID onMessageMethod =
      env->GetMethodID(callbackCls, "onMessage", "(BB[B;)V");
  blufi::Core *ptr = new blufi::Core(mtu, [=](uint8_t type, uint8_t subType,
                                              uint8_t *data, size_t size) {
    env->CallVoidMethod(globalOnMessageCallback, onMessageMethod,
                        static_cast<jbyte>(type), static_cast<jbyte>(subType),
                        copyToJavaByteArray(env, data, size));
  });
  return (jlong)ptr;
}

JNIEXPORT void JNICALL Java_com_gdszzy_blufi_Core_freeCore(JNIEnv *env,
                                                           jobject obj,
                                                           jlong ptr) {
  blufi::Core *core = (blufi::Core *)ptr;
  delete(core);
}

JNIEXPORT jbyte JNICALL Java_com_gdszzy_blufi_Core_onReceiveData(
    JNIEnv *env, jobject obj, jlong ptr, jbyteArray bytes) {
  blufi::Core *core = (blufi::Core *)ptr;
  jsize len = env->GetArrayLength(bytes);
  jbyte *data = env->GetByteArrayElements(bytes, nullptr);
  return core->onReceiveData(std::span((uint8_t *)data, (size_t)len));
}

JNIEXPORT jbyteArray JNICALL Java_com_gdszzy_blufi_Core_negotiateKey(
    JNIEnv *env, jobject obj, jlong ptr, jobject bytesArray) {
  blufi::Core *core = (blufi::Core *)ptr;
  DataChan datachan = newDataChan();
  std::ignore = core->negotiateKey(datachan);
}
JNIEXPORT jbyteArray JNICALL Java_com_gdszzy_blufi_Core_custom(
    JNIEnv *env, jobject obj, jlong ptr, DataChan *sendData, uint8_t *data,
    size_t size);
JNIEXPORT jbyteArray JNICALL Java_com_gdszzy_blufi_Core_scanWifi(
    JNIEnv *env, jobject obj, jlong ptr, DataChan *sendData);
JNIEXPORT jbyteArray JNICALL Java_com_gdszzy_blufi_Core_connectWifi(
    JNIEnv *env, jobject obj, jlong ptr, DataChan *sendData, const char *ssid,
    const char *pass);