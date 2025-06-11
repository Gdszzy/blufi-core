(() => {
  Module.MsgType = {
    CONTROL_VALUE: 0,
    VALUE: 1
  }
  Module.MsgSubType = {
    // value
    NEG: 0x00,
    SET_SEC_MODE: 0x01,
    SET_SSID: 0x02,
    SET_PWD: 0x03,
    CUSTOM_DATA: 0x13,
    WIFI_LIST_NEG: 0x11,
    WIFI_STATUS: 0x0F,
    ERROR: 0x12,
    // control value
    END: 0x03,
    WIFI_NEG: 0x09,
  }
})();
