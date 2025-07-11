package com.gdszzy.blufi

import java.nio.ByteBuffer
import java.nio.ByteOrder

class Core(mtu:Int,callback: OnMessageCallback) {

    private val ptr: Long=newCore(mtu,callback as Object)

    private fun parseFlattenBuffer(buffer: ByteBuffer):List<ByteArray>{
        buffer.order(ByteOrder.LITTLE_ENDIAN)
        val list= ArrayList<ByteArray>()
        while(buffer.remaining()>0){
            val len=buffer.getInt()
            val byteArray = ByteArray(len)
            buffer.get(byteArray)
            list.add(byteArray)
        }
        return list
    }

    fun onReceiveData(buffer: ByteArray):Byte{
        return onReceiveData(ptr,buffer)
    }

    fun custom(buffer: ByteArray):List<ByteArray>{
        return parseFlattenBuffer(custom(ptr,buffer))
    }

    fun scanWifi():List<ByteArray>{
        return parseFlattenBuffer(scanWifi(ptr))
    }

    fun connectWifi(ssid: String,pass: String): List<ByteArray>{
        return parseFlattenBuffer(connectWifi(ptr,ssid,pass))
    }

    fun negotiateKey():List<ByteArray>{
        return parseFlattenBuffer(negotiateKey(ptr))
    }

    protected fun finalize() {
        freeCore(ptr)
    }

    private external fun newCore(mtu:Int,callback: Object): Long
    private external fun freeCore(ptr:Long)
    private external fun onReceiveData(ptr:Long,buffer: ByteArray): Byte
    private external fun custom(ptr:Long,buffer: ByteArray): ByteBuffer
    private external fun scanWifi(ptr:Long): ByteBuffer
    private external fun connectWifi(ptr:Long,ssid:String,pass:String): ByteBuffer
    private external fun negotiateKey(ptr:Long): ByteBuffer

    companion object{
        init {
            System.loadLibrary("blufi")
        }
        const val TYPE_CONTROL_VALUE:Byte=0x00
        const val TYPE_VALUE:Byte=0x01
        // value
        const val SUBTYPE_NEG:Byte = 0x00
        const val SUBTYPE_SET_SEC_MODE:Byte = 0x01
        const val SUBTYPE_SET_SSID:Byte = 0x02
        const val SUBTYPE_SET_PWD:Byte = 0x03
        const val SUBTYPE_CUSTOM_DATA:Byte = 0x13
        const val SUBTYPE_WIFI_LIST_NEG:Byte = 0x11
        const val SUBTYPE_WIFI_STATUS:Byte = 0x0F
        const val SUBTYPE_ERROR:Byte = 0x12
        // control value
        const val SUBTYPE_END:Byte = 0x03
        const val SUBTYPE_WIFI_NEG:Byte = 0x09
    }

    interface OnMessageCallback{
        // When core received a complete message. This callback will be invoked
        // When SUBTYPE_NEG message received. Send the buffer to ESP32.
        fun onMessage(type:Byte,subType:Byte,buffer: ByteBuffer)
    }
}