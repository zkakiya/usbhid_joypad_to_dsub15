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
//管理するボタンの数
#define HANDLE_BUTTON_NUM       15

const unsigned long frame_ms = 1000/120;//1フレーム辺りの単位時間（ms)（秒間120フレーム管理）
unsigned long frame_checker;//フレーム管理時計の時刻
unsigned long merc;
unsigned long curr;//現在時刻をミリ秒で取得する変数
unsigned long curr_micro;//現在時刻をマイクロ秒で取得する変数
int frame_count;//フレームカウント変数

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

//プリセットの状態を管理する
int keyPresetNum;

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
        //仮想ジョイパッドの状態をarduinoのピンに反映させる。
        void pinStateOutput();

private:
        uint8_t hatState;//hatスイッチの状態を示す変数
        bool joypadButtonState[HANDLE_BUTTON_NUM];//接続されたジョイパッドが現在押されているボタンを示す配列
};

struct KeyAssign VGAssign;

void MyJoystickEvents::OnGamePadChanged(const GamePadEventData *evt) {
        //アナログスティック検出用の関数　現在はダミー状態
}
void MyJoystickEvents::OnHatSwitch(uint8_t hat) {
        // Serial.print("Hat Switch: ");
        // PrintHex<uint8_t > (hat, 0x80);
        // Serial.println("");

        hatState = hat;
        virtualJoypadArrowAssign();
}
void MyJoystickEvents::OnButtonUp(uint8_t but_id) {
        //物理ボタンの状態をチェック
        PButtonState[but_id - 1] = false;
        //virtualpadButtonAssignを参照して物理ボタンの状態を仮想ジョイパッドに割り当てる
        VButtonState[VGAssign.buttonAssignTable[but_id - 1]] = false;
}
void MyJoystickEvents::OnButtonDn(uint8_t but_id) {
        //物理ボタンの状態をチェック
        PButtonState[but_id - 1] = true;
        //virtualpadButtonAssignを参照して物理ボタンの状態を仮想ジョイパッドに割り当てる
        VButtonState[VGAssign.buttonAssignTable[but_id - 1]] = true;
}
void MyJoystickEvents::printActiveVButton (void) {
        Serial.print("preset:");
        Serial.print(keyPresetNum + 1);
        
        Serial.print(" vHat+vKey/pinState:");
        for(int i = 0;i < sizeof(VHatState);i++){
                Serial.print(VHatState[i]);
        };
        for(int i = 0;i < sizeof(VButtonState);i++){
                Serial.print(VButtonState[i]);
        };
        Serial.print(" pKey:");
        for(int i = 0;i < sizeof(PButtonState);i++){
                Serial.print(PButtonState[i]);
        };
        Serial.println();
}
void MyJoystickEvents::virtualJoypadArrowAssign(){
        //仮想ジョイパッドの方向キー状態を定義する関数。
        //hatStateが00~07(今回使ったパッドは真上から時計回り）時の状態に応じて仮想ジョイパッドに状態を割り振る
        
        //hatスイッチが↑要素に入っていた場合、VHatState[0]を真とする。(入っていない場合は偽）
        VHatState[0] = (((hatState == 7||hatState == 0||hatState == 1)&&(!VGAssign.isHatReverse))||((hatState == 3||hatState == 4||hatState == 5)&&(VGAssign.isHatReverse))) ? true : false;
        //hatスイッチが→要素に入っていた場合、VHatState[1]を真とする。(入っていない場合は偽）
        VHatState[1] = (((hatState == 1||hatState == 2||hatState == 3)&&(!VGAssign.isHatReverse))||((hatState == 5||hatState == 6||hatState == 7)&&(VGAssign.isHatReverse))) ? true : false;
        //hatスイッチが↓要素に入っていた場合、VHatState[2]を真とする。(入っていない場合は偽）
        VHatState[2] = (((hatState == 3||hatState == 4||hatState == 5)&&(!VGAssign.isHatReverse))||((hatState == 0||hatState == 1||hatState == 7)&&(VGAssign.isHatReverse))) ? true : false;
        //hatスイッチが←要素に入っていた場合、VHatState[3]を真とする。(入っていない場合は偽）
        VHatState[3] = (((hatState == 5||hatState == 6||hatState == 7)&&(!VGAssign.isHatReverse))||((hatState == 1||hatState == 2||hatState == 3)&&(VGAssign.isHatReverse))) ? true : false;
}
void MyJoystickEvents::pinStateOutput(void) {
        //仮想ジョイパッドの状態をarduinoのピンに出力する。
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

//ロータリSWのポジション検出用ピン指定
#define PIN_ROTARYSW A5

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

void RotarySwChecker::printRotSwStates() {
        sw_inp = analogRead(PIN_ROTARYSW);      // A/D変換値を取得
        Serial.print("ADC0: ");
        Serial.println(sw_inp, DEC);            // A/D変換値を送信

        byte sw_pos = getRotarySwStatus();     // ロータリSWの状態を取得
        Serial.print("Status: ");
        Serial.println(sw_pos, DEC);            // ロータリSWの状態を送信

}

//A/D変換値からロータリSWWの位置をチェックする
byte RotarySwChecker::getRotarySwStatus()
{
    if (sw_inp < 347){
        if (sw_inp < 145){
            if (sw_inp < 45) return 1;
            else return 2;
        } else {
            if (sw_inp < 249) return 3;
            else return 4;
        }
    } else if (sw_inp < 677){
        if(sw_inp < 519){
            if(sw_inp < 437) return 5;
            else return 6;
        } else {
            if (sw_inp < 599) return 7;
            else return 8;
        }
    } else if (sw_inp < 1024){
        return 9;
    }

    return 0;
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
        MyJoyEvents.pinStateOutput();

        if(frame_count % 30 == 0)
        {
                //【注意】ロータリスイッチを接続していない場合は必ずコメントアウトすること
                //ロータリスイッチ未接続かつ下記関数がアクティブの場合、装置全体が動作できない
                //キープリセットの操作が行われているか（変化があったかどうか）をチェックして、
                //操作があった際は使用するキープリセットを切り替える
                RotSwCheck.checkRotSwChange();
        }

        unsigned long curr = millis();//現在時刻取得
        if(curr - frame_checker > frame_ms) {//現在時刻とフレーム管理時計のタイミングズレチェック
                Serial.print("*** processing delay :");
                Serial.println(curr - frame_checker);
                frame_checker = millis();//現在時刻取得
        } else {
                while(curr - frame_checker < frame_ms){
                        curr = millis();
                }
                frame_checker = curr;
        }
        frame_count = frame_count + 1;
        if(frame_count > 10000){
                frame_count = 0;
        }

        //余剰時間を消化する処理。
        curr = millis();
        curr_micro = micros();//現在時刻を取得
        while(curr < merc){
                curr = millis();
        }
        merc += frame_ms;//フレーム管理時計を1フレーム分進める

}
