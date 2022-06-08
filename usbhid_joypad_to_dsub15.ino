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

unsigned long frame_ms = 2;//キー状態の更新レート。1フレーム辺りの単位時間（ms)
unsigned long frame_checker;//フレーム管理時計の時刻


//ロータリSWのポジション検出用ピン指定
#define PIN_ROTARYSW A5

//テストモードのフラグ（デフォルト:false）。true時はシリアルモニタに現在のキー状態を表示する。
//重い処理のため、true時はキー状態の更新レートがtestModeFrame_msの数値になる（→遅延が大きくなる）
bool isTestMode = false;
unsigned long testModeFrame_ms = 1000 / 60;

//仮想ジョイパッドの状態を管理
//方向キーの状態（方向キー）
bool VHatState[HANDLE_ARROW_DIRECTION];
//方向キー以外の状態（方向キー以外）
bool VButtonState[HANDLE_BUTTON_NUM + HANDLE_MACRO_NUM];
// マクロの活性化フラグ
bool isMacroActive[HANDLE_MACRO_NUM];

//物理ジョイパッドの状態を管理
//方向キーの状態（方向キー）
bool PHatState[HANDLE_ARROW_DIRECTION];
//方向キー以外の状態（方向キー以外）
bool PButtonState[HANDLE_BUTTON_NUM];

//キーアサインの定義
struct KeyAssign {
        //仮想スティックにアサインする物理スイッチを選択する。
        //0（デフォルト）:hatスイッチ、1:アナログスティック1、2:アナログスティック2
        int directionSwSelctor;
        bool isDirectionReverse;//hatスイッチの上下反転フラグ（デフォルトは反転なし→false）
        uint8_t stick01Assign[HANDLE_ARROW_DIRECTION];//スティック01のキーアサイン
        uint8_t stick02Assign[HANDLE_ARROW_DIRECTION];//スティック02のキーアサイン
        //hatスイッチ以外のキーアサイン
        uint8_t buttonAssignTable[HANDLE_BUTTON_NUM];
};
//コントロールキュー（マクロの記述およびピン出力準備に使うデータ型）の定義
struct controlQueue {
        bool hatState[HANDLE_ARROW_DIRECTION];
        bool buttonAssignTable[HANDLE_BUTTON_NUM];
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
        //ボタン出力の前処理
        void executeJoypadState();
        //仮想ジョイパッドの状態をarduinoのピンに反映させる。
        void pinStateOutput();

private:
        int hatState[HANDLE_ARROW_DIRECTION];//hatスイッチの状態を示す変数
        bool stick01State[HANDLE_ARROW_DIRECTION];//スティック01（DualShock4の場合左スティック）の状態を示す変数
        bool stick02State[HANDLE_ARROW_DIRECTION];//スティック02（DualShock4の場合左スティック）の状態を示す変数        
        bool joypadButtonState[HANDLE_BUTTON_NUM];//接続されたジョイパッドが現在押されているボタンを示す配列
};

struct KeyAssign VGAssign;
struct controlQueue outputBuf;


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

void MyJoystickEvents::pinStateOutput(void) {
        //仮想ジョイパッドの状態をarduinoのピンに出力する。
        //方向キー割り当て

        outputBuf.hatState[0] ? digitalWrite(2, LOW) : digitalWrite(2, HIGH);
        outputBuf.hatState[1] ? digitalWrite(5, LOW) : digitalWrite(5, HIGH);
        outputBuf.hatState[2] ? digitalWrite(3, LOW) : digitalWrite(3, HIGH);
        outputBuf.hatState[3] ? digitalWrite(4, LOW) : digitalWrite(4, HIGH);
        //その他ボタン割り当て
        outputBuf.buttonAssignTable[0] ? digitalWrite(6, LOW) : digitalWrite(6, HIGH);
        outputBuf.buttonAssignTable[1] ? digitalWrite(7, LOW) : digitalWrite(7, HIGH);
        outputBuf.buttonAssignTable[2] ? digitalWrite(8, LOW) : digitalWrite(8, HIGH);
        outputBuf.buttonAssignTable[3] ? digitalWrite(A0, LOW) : digitalWrite(A0, HIGH);
        outputBuf.buttonAssignTable[4] ? digitalWrite(A1, LOW) : digitalWrite(A1, HIGH);
        outputBuf.buttonAssignTable[5] ? digitalWrite(A2, LOW) : digitalWrite(A2, HIGH);
        outputBuf.buttonAssignTable[6] ? digitalWrite(A3, LOW) : digitalWrite(A3, HIGH);
        outputBuf.buttonAssignTable[7] ? digitalWrite(A4, LOW) : digitalWrite(A4, HIGH);
        //A5ピンはロータリースイッチ検出に使用するため、JAMMAキー入力には不使用とする。
        // arduino UNO用（11～13ピン）
        outputBuf.buttonAssignTable[8] ? digitalWrite(11, LOW) : digitalWrite(11, HIGH);
        outputBuf.buttonAssignTable[9] ? digitalWrite(12, LOW) : digitalWrite(12, HIGH);
        outputBuf.buttonAssignTable[10] ? digitalWrite(13, LOW) : digitalWrite(13, HIGH);
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

        if (isTestMode) frame_ms = testModeFrame_ms;//テストモードon時、キー状態更新レートをテストモード時専用の値にする

        //初期化時に仮想ジョイスティックの状態を確定する（キー割り当て用の配列に現在選ばれているキーコンフィグプリセットを割り当てる）
        uint8_t rotKeyState = RotSwCheck.getRotarySwStatus();
        SetCurrentKeyPreset(rotKeyState - 1);

}

void loop() {
        Usb.Task();

        MyJoyEvents.virtualJoypadButtonAssign();
        MyJoyEvents.executeJoypadState();
         if (isTestMode) MyJoyEvents.printActiveVButton();
        MyJoyEvents.pinStateOutput();
        
        //【注意】ロータリスイッチを接続していない場合は必ずコメントアウトすること
        //ロータリスイッチ未接続かつ下記関数がアクティブの場合、装置全体が動作できない
        //キープリセットの操作が行われているか（変化があったかどうか）をチェックして、
        //操作があった際は使用するキープリセットを切り替える
        RotSwCheck.checkRotSwChange();

        frameManagement();
}
