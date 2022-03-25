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
//外部出力するボタンの数
#define HANDLE_OUTPUTBUTTON_NUM 6
//管理するキーマクロの数
#define HANDLE_KEYMACRO_NUM 5
//秒間何フレームで処理するか
#define BASE_FRAME 120

const unsigned long frame_ms = 1000/BASE_FRAME;//1フレーム辺りの単位時間（ms)（秒間120フレーム管理）
unsigned long frame_checker;//フレーム管理時計の時刻
int frame_count;//フレームカウント

//ロータリSWのポジション検出用ピン指定
#define PIN_ROTARYSW A5

//マクロ実行中か否かのフラグ
bool isMacroInProcess = false;

//仮想ジョイパッドの状態を管理
//方向キーの状態（方向キー）
bool VHatState[4];
//方向キー以外の状態（方向キー以外）
bool VButtonState[HANDLE_BUTTON_NUM + HANDLE_KEYMACRO_NUM];

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

//キーマクロの型を定義
struct KeyMacroStep {
        bool MHatState[4];
        bool MButtonState[HANDLE_OUTPUTBUTTON_NUM];
        int RemainFrame;
};
//キーマクロ実際の記述（実験用）
struct KeyMacroStep MacroDetail[] = {
        {{0,0,0,1},{0,0,0,0,0,0},45},
        {{0,1,0,0},{0,0,0,0,0,0},45},
        {{0,0,0,0},{0,0,0,0,0,0},20},
        {{0,1,0,0},{0,0,0,0,0,0},20},
};

class KeyMacro {
public:
        void playKeyMacro();//キーマクロ再生（ステップを進める？）
        void startKeyMacro();//キーマクロ初期化
        void checkKeyMacroFrag();//キーマクロの開始フラグを検知

private:
        struct KeyMacroStep procedure_arg;//現在再生しているキーマクロステップ処理用構造体
        int procedure_step;//現在再生しているキーマクロのステップ数
        int size_arg_macro;//現在再生しているマクロの要素全ての数
        int vButton_for_keymacro[HANDLE_KEYMACRO_NUM];//キーマクロ用に定義された仮想ボタン
};


USB Usb;
USBHub Hub(&Usb);
HIDUniversal Hid(&Usb);
MyJoystickEvents MyJoyEvents;  //継承した新しいクラスに書き換える
JoystickReportParser Joy(&MyJoyEvents);  //新しいクラスのインスタンスを使う
RotarySwChecker RotSwCheck;
KeyMacro KeyMacroProcedure;


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

        // Serial.print(frame_count);
        MyJoyEvents.printActiveVButton();
        // MyJoyEvents.pinStateOutput();

        KeyMacroProcedure.checkKeyMacroFrag();

        //キーマクロ1ステップを1/60秒で行うための調整
        if(frame_count % (BASE_FRAME / 60) == 0){
                KeyMacroProcedure.playKeyMacro();
        }

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
