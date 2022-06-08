// Satisfy IDE, which only needs to see the include statment in the ino.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

#include "usbhid_joypad_to_dsub15.h"

USB Usb;
USBHub Hub(&Usb);
HIDUniversal Hid(&Usb);

MyJoystickEvents MyJoyEvents;  //継承した新しいクラスに書き換える
JoystickReportParser Joy(&MyJoyEvents);  //新しいクラスのインスタンスを使う
RotarySwChecker RotSwCheck;

void setup() {
        Serial.begin(115200);
#if !defined(__MIPSEL__)
        while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
        Serial.println("Start");

        if (Usb.Init() == -1)
                Serial.println("OSC did not start.");

        //ピン状態リセット プルアップ抵抗は必要ない。（はず。プルアップ抵抗を使用すると動作しない。）
        for (int thisPin=2; thisPin <9; thisPin++)
                pinMode(thisPin, OUTPUT);
        for (int thisPin=11; thisPin <14; thisPin++)
                pinMode(thisPin, OUTPUT);
        for (int thisPin=A0; thisPin <= A4; thisPin++)
                pinMode(thisPin, OUTPUT);

        delay(200);

        if (!Hid.SetReportParser(0, &Joy))
                ErrorMessage<uint8_t > (PSTR("SetReportParser"), 1);

        if (isTestMode) frame_ms = testModeFrame_ms;//テストモードon時、キー状態更新レートをテストモード時専用の値にする

        //初期化時に仮想ジョイスティックの状態を確定する（キー割り当て用の配列に現在選ばれているキーコンフィグプリセットを割り当てる）
        //isUseRotarySwがfalseの時（ロータリースイッチ不使用の場合）キーコンフィグプリセットは1番目に固定される
        if (isUseRotarySw) {
                uint8_t rotKeyState = RotSwCheck.getRotarySwStatus();
                SetCurrentKeyPreset(rotKeyState - 1);
        } else {
                SetCurrentKeyPreset(0);
        }

}

void loop() {
        Usb.Task();

        MyJoyEvents.virtualJoypadButtonAssign();
        MyJoyEvents.executeJoypadState();
        MyJoyEvents.printActiveVButton();
        MyJoyEvents.pinStateOutput();
        RotSwCheck.checkRotSwChange();

        frameManagement();
}
