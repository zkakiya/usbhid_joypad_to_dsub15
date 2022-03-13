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
