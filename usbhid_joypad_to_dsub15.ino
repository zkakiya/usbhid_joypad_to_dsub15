#include <usbhid.h>
#include <hiduniversal.h>
#include <usbhub.h>

// Satisfy IDE, which only needs to see the include statment in the ino.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

#include "hidjoystickrptparser.h"


// USBHIDJoystick.inoを改変した
// 「USBジョイパッドの状態をarduinoピンおよびシリアルモニタに書き出すスケッチ」
// ロータリースイッチでキープリセットを切り替えることが可能

//管理するキーアサインプリセットの数
#define HANDLE_ASSIGN_NUM       9
//管理する方向キーの方向を示す数値
#define HANDLE_ARROW_DIRECTION  4
//管理する出力用ボタンの数
#define HANDLE_BUTTON_NUM       15
//管理するマクロの数
#define HANDLE_MACRO_NUM        5

const unsigned long frame_ms = 1000/120;//1フレーム辺りの単位時間（ms)（秒間120フレーム管理）
int frame_count;//フレームカウント

//ロータリSWのポジション検出用ピン指定
#define PIN_ROTARYSW A5

//仮想ジョイパッドの状態を管理
//方向キーの状態（方向キー）
bool VHatState[4];
//方向キー以外の状態（方向キー以外）
bool VButtonState[HANDLE_BUTTON_NUM];

//物理ジョイパッドの状態を管理
//方向キーの状態（方向キー）
bool PHatState[4];
//方向キー以外の状態（方向キー以外）
bool PButtonState[HANDLE_BUTTON_NUM];

//キーアサインの定義
struct KeyAssign {
        //hatスイッチの上下反転フラグ（デフォルトは反転なし→false）
        bool isHatReverse;
        //hatスイッチ以外のキーアサイン
        uint8_t buttonAssignTable[HANDLE_BUTTON_NUM];
};

//仮想ジョイパッドの配列一時期億
struct KeyAssign VGAssign;

//プリセットの状態を管理する
int keyPresetNum;

//物理ジョイパッドの状態変化によるイベントを受け、仮想ジョイパッドの状態を変更する
class MyJoystickEvents : public JoystickEvents {
public:
        void OnGamePadChanged(const GamePadEventData *evt);
        void OnHatSwitch(uint8_t hat);
        void OnButtonUp(uint8_t but_id);
        void OnButtonDn(uint8_t but_id);

        //以下独自実装
        void printActiveVButton();//現在の仮想ボタンの状態をシリアルに書き出す

        //仮想ボタン割り当て（方向キー）
        void virtualJoypadArrowAssign();
        //仮想ボタン割り当て（方向キー以外）
        void virtualJoypadButtonAssign();
private:
        uint8_t hatState;//hatスイッチの状態を示す変数
        bool joypadButtonState[HANDLE_BUTTON_NUM];//接続されたジョイパッドが現在押されているボタンを示す配列
};

class RotarySwChecker {
public:
        byte getRotarySwStatus();//ロータリSW状態取得関数
        void printRotSwStates();
        void checkRotSwChange();
private:     
        int sw_inp;//ロータリSWのアナログ入力
        byte sw_pos;//ロータリSWの状態
        byte temp_sw_pos = 1;//ロータリSWの変化を検知するための一時的な記録値
};

//仮想ジョイパッドの状態をarduinoのピンに反映させる。
void pinStateOutput() {
//方向キー割り当て

        VHatState[0] ? digitalWrite(2, LOW) : digitalWrite(2, HIGH);
        VHatState[1] ? digitalWrite(5, LOW) : digitalWrite(5, HIGH);
        VHatState[2] ? digitalWrite(3, LOW) : digitalWrite(3, HIGH);
        VHatState[3] ? digitalWrite(4, LOW) : digitalWrite(4, HIGH);
        //その他ボタン割り当て
        VButtonState[0] ? digitalWrite(6, LOW) : digitalWrite(6, HIGH);
        VButtonState[1] ? digitalWrite(7, LOW) : digitalWrite(7, HIGH);
        VButtonState[2] ? digitalWrite(8, LOW) : digitalWrite(8, HIGH);
        VButtonState[3] ? digitalWrite(A0, LOW) : digitalWrite(A0, HIGH);
        VButtonState[4] ? digitalWrite(A1, LOW) : digitalWrite(A1, HIGH);
        VButtonState[5] ? digitalWrite(A2, LOW) : digitalWrite(A2, HIGH);
        VButtonState[6] ? digitalWrite(A3, LOW) : digitalWrite(A3, HIGH);
        VButtonState[7] ? digitalWrite(A4, LOW) : digitalWrite(A4, HIGH);
        //A5ピンはロータリースイッチ検出に使用するため、JAMMAキー入力には不使用とする。
        // arduino UNO用（11～13ピン）
        VButtonState[8] ? digitalWrite(11, LOW) : digitalWrite(11, HIGH);
        VButtonState[9] ? digitalWrite(12, LOW) : digitalWrite(12, HIGH);
        VButtonState[10] ? digitalWrite(13, LOW) : digitalWrite(13, HIGH);
}



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
        // for (int thisPin=A5; thisPin <= A5; thisPin++)
        //         pinMode(thisPin, INPUT);

        delay(200);

        if (!Hid.SetReportParser(0, &Joy))
                ErrorMessage<uint8_t > (PSTR("SetReportParser"), 1);

        //初期化時に仮想ジョイスティックの状態を確定する（キー割り当て用の配列に現在選ばれているキーコンフィグプリセットを割り当てる）
        uint8_t rotKeyState = RotSwCheck.getRotarySwStatus();
        SetCurrentKeyPreset(rotKeyState - 1);
        // SetCurrentKeyPreset(0);

}

void loop() {
        Usb.Task();

        Serial.println(frame_count);
        MyJoyEvents.printActiveVButton();
        pinStateOutput();

        //ボトルネックになる可能性があるため、30フレームごとの処理としている
        if(frame_count % 60 == 0)
        {
                //【注意】ロータリスイッチを接続していない場合は必ずコメントアウトすること
                //ロータリスイッチ未接続かつ下記関数がアクティブの場合、装置全体が動作できない
                //キープリセットの操作が行われているか（変化があったかどうか）をチェックして、
                //操作があった際は使用するキープリセットを切り替える
                RotSwCheck.checkRotSwChange();
        }

        frameManagement();
}
