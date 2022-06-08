#include <usbhid.h>
#include <hiduniversal.h>
#include <usbhub.h>

#include "hidjoystickrptparser.h"


// USBHIDJoystick.inoを改変した
// 「USBジョイパッドの状態をarduinoピンおよびシリアルモニタに書き出すスケッチ」
// ロータリースイッチでキープリセットを切り替えることが可能

unsigned long frame_ms = 2;//キー状態の更新レート。1フレーム辺りの単位時間（ms)
unsigned long frame_checker;//フレーム管理時計の時刻

//キープリセット選択用ロータリスイッチを使用するか否かのフラグ（デフォルト:true）
//【重要】ロータリスイッチを接続していないときは必ずfalseにすること。
//ロータリスイッチ未接続かつtrueとした場合、装置が動作できない
bool isUseRotarySw = true;
//ロータリSWのポジション検出用ピン指定
#define PIN_ROTARYSW A5

//テストモードのフラグ（デフォルト:false）。true時はシリアルモニタに現在のキー状態を表示する。
//重い処理のため、true時はキー状態の更新レートがtestModeFrame_msの数値になる（→遅延が大きくなる）
bool isTestMode = false;
unsigned long testModeFrame_ms = 1000 / 60;


//管理するキーアサインプリセットの数
#define HANDLE_ASSIGN_NUM       9
//管理する方向キーの方向を示す数値
#define HANDLE_ARROW_DIRECTION  4
//管理する出力用ボタンの数
#define HANDLE_BUTTON_NUM       15
//管理するマクロの数
#define HANDLE_MACRO_NUM        5

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

int keyPresetNum;//参照中のキープリセット
struct KeyAssign VGAssign;//仮想ジョイパッド構築時に参照するキープリセット状態

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
        bool PHatState[HANDLE_ARROW_DIRECTION];//物理ジョイパッド方向キーの状態
        bool PButtonState[HANDLE_BUTTON_NUM];//物理ジョイパッド方向キー以外の状態
        bool VHatState[HANDLE_ARROW_DIRECTION];//仮想ジョイパッド方向キー状態
        bool VButtonState[HANDLE_BUTTON_NUM + HANDLE_MACRO_NUM];//仮想ジョイパッド方向キー以外の状態
        int hatState[HANDLE_ARROW_DIRECTION];//hatスイッチの状態を示す変数
        bool stick01State[HANDLE_ARROW_DIRECTION];//スティック01（DualShock4の場合左スティック）の状態を示す変数
        bool stick02State[HANDLE_ARROW_DIRECTION];//スティック02（DualShock4の場合左スティック）の状態を示す変数     
        bool joypadButtonState[HANDLE_BUTTON_NUM];//接続されたジョイパッドが現在押されているボタンを示す配列
        struct controlQueue outputBuf;//ピンに出力する予定のボタン状態を一時記録する構造体
        bool isMacroActive[HANDLE_MACRO_NUM];// マクロの活性化フラグ
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