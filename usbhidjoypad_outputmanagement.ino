void MyJoystickEvents::OnGamePadChanged(const GamePadEventData *evt) {
        //アナログスティックを閾値でデジタル型に変換
        stick01State[0] = (evt->Z1 < 65) ? true : false;//↑要素
        stick01State[1] = (evt->Y > 190) ? true : false;//→要素
        stick01State[2] = (evt->Z1 > 190) ? true : false;//↓要素
        stick01State[3] = (evt->Y < 65) ? true : false;//←要素
        stick02State[0] = (evt->Rz < 65) ? true : false;//↑要素
        stick02State[1] = (evt->Z2 > 190) ? true : false;//→要素
        stick02State[2] = (evt->Rz > 190) ? true : false;//↓要素
        stick02State[3] = (evt->Z2 < 65) ? true : false;//←要素
        virtualJoypadArrowAssign();
}
void MyJoystickEvents::OnHatSwitch(uint8_t hat) {
        //hatの状態に合わせてhatStateを定義
        hatState[0] = (hat == 7||hat == 0||hat == 1) ? true : false;//↑要素
        hatState[1] = (hat == 1||hat == 2||hat == 3) ? true : false;//→要素
        hatState[2] = (hat == 3||hat == 4||hat == 5) ? true : false;//↓要素
        hatState[3] = (hat == 5||hat == 6||hat == 7) ? true : false;//←要素
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
        //isTestModeがtrue時（テストモード）、シリアルモニタに物理ジョイパッドの状態を表示する
        if (isTestMode) {
                Serial.print("preset:");
                Serial.print(keyPresetNum + 1);
                Serial.print(" vHat+vKey/pinState:");
                for(int i = 0;i < sizeof(outputBuf.hatState);i++){
                        Serial.print(outputBuf.hatState[i]);
                };
                for(int i = 0;i < sizeof(outputBuf.buttonAssignTable);i++){
                        Serial.print(outputBuf.buttonAssignTable[i]);
                };
                Serial.print(" pKey:");
                for(int i = 0;i < sizeof(PButtonState);i++){
                        Serial.print(PButtonState[i]);
                };
                Serial.print(" S1:");
                for(int i = 0;i < sizeof(stick01State);i++){
                        Serial.print(stick01State[i]);
                };
                Serial.print(" S2:");
                for(int i = 0;i < sizeof(stick02State);i++){
                        Serial.print(stick02State[i]);
                };
                Serial.println();
        }
}
void MyJoystickEvents::virtualJoypadArrowAssign(){
        //仮想ジョイパッドの方向キー状態を定義する関数。
        //hatStateが00~07(今回使ったパッドは真上から時計回り）時の状態に応じて仮想ジョイパッドに状態を割り振る
        
        if (VGAssign.directionSwSelctor == 1) {
                //VGAssign.directionSwSelctorが1→スティック1の状態を仮想スティックに割り当てる場合
                //スティックが↑要素に入っていた場合、VHatState[0]を真とする。(入っていない場合は偽）
                VHatState[0] = (((stick01State[0])&&(!VGAssign.isDirectionReverse))||((stick01State[2])&&(VGAssign.isDirectionReverse))) ? true : false;
                //スティックが→要素に入っていた場合、VHatState[1]を真とする。(入っていない場合は偽）
                VHatState[1] = (((stick01State[1])&&(!VGAssign.isDirectionReverse))||((stick01State[3])&&(VGAssign.isDirectionReverse))) ? true : false;
                //スティックが↓要素に入っていた場合、VHatState[2]を真とする。(入っていない場合は偽）
                VHatState[2] = (((stick01State[2])&&(!VGAssign.isDirectionReverse))||((stick01State[0])&&(VGAssign.isDirectionReverse))) ? true : false;
                //スティックが←要素に入っていた場合、VHatState[3]を真とする。(入っていない場合は偽）
                VHatState[3] = (((stick01State[3])&&(!VGAssign.isDirectionReverse))||((stick01State[1])&&(VGAssign.isDirectionReverse))) ? true : false;
                for(int i=0;i < sizeof(VGAssign.stick02Assign);i++){VButtonState[VGAssign.stick02Assign[i]] = stick02State[i] ? true : false;}
        } else if(VGAssign.directionSwSelctor == 2) {
                //VGAssign.directionSwSelctorが2→スティック2の状態を仮想スティックに割り当てる場合
                //スティック↑要素時：VHatState[0]を真
                VHatState[0] = (((stick02State[0])&&(!VGAssign.isDirectionReverse))||((stick02State[2])&&(VGAssign.isDirectionReverse))) ? true : false;
                //スティック→要素時：VHatState[1]を真
                VHatState[1] = (((stick02State[1])&&(!VGAssign.isDirectionReverse))||((stick02State[3])&&(VGAssign.isDirectionReverse))) ? true : false;
                //スティック↓要素時：VHatState[2]を真
                VHatState[2] = (((stick02State[2])&&(!VGAssign.isDirectionReverse))||((stick02State[0])&&(VGAssign.isDirectionReverse))) ? true : false;
                //スティック←要素時：VHatState[3]を真
                VHatState[3] = (((stick02State[3])&&(!VGAssign.isDirectionReverse))||((stick02State[1])&&(VGAssign.isDirectionReverse))) ? true : false;
                for(int i=0;i < sizeof(VGAssign.stick01Assign);i++){VButtonState[VGAssign.stick01Assign[i]] = stick01State[i] ? true : false;}
        } else {
                //VGAssign.directionSwSelctorが0およびその他→hatスイッチの状態を仮想スティックに割り当てる場合
                //hat↑要素時：VHatState[0]を真
                VHatState[0] = (((hatState[0])&&(!VGAssign.isDirectionReverse))||((hatState[2])&&(VGAssign.isDirectionReverse))) ? true : false;
                //hat→要素時：VHatState[1]を真
                VHatState[1] = (((hatState[1])&&(!VGAssign.isDirectionReverse))||((hatState[3])&&(VGAssign.isDirectionReverse))) ? true : false;
                //hat↓要素時：VHatState[2]を真
                VHatState[2] = (((hatState[2])&&(!VGAssign.isDirectionReverse))||((hatState[0])&&(VGAssign.isDirectionReverse))) ? true : false;
                //hat←要素時：VHatState[3]を真
                VHatState[3] = (((hatState[3])&&(!VGAssign.isDirectionReverse))||((hatState[1])&&(VGAssign.isDirectionReverse))) ? true : false;
                for(int i=0;i < sizeof(VGAssign.stick01Assign);i++){VButtonState[VGAssign.stick01Assign[i]] = stick01State[i] ? true : false;}
                for(int i=0;i < sizeof(VGAssign.stick02Assign);i++){VButtonState[VGAssign.stick02Assign[i]] = stick02State[i] ? true : false;}
        }
}
void MyJoystickEvents::virtualJoypadButtonAssign(void){
        //マクロ用仮想ボタン押下中判定時の処理
        //マクロ用仮想ボタン01
        if(VButtonState[HANDLE_BUTTON_NUM + 1]){
                outputBuf.buttonAssignTable[0] = true;
                outputBuf.buttonAssignTable[1] = true;
        }
}
void MyJoystickEvents::executeJoypadState(void){
        //ピン出力準備
        //仮想ジョイパッドの状態とマクロ用のキューを組み合わせる
        for(int i=0;i < HANDLE_ARROW_DIRECTION;i++){
                outputBuf.hatState[i] = VHatState[i];
        }
        for(int i=0;i < HANDLE_BUTTON_NUM;i++){
                outputBuf.buttonAssignTable[i] = VButtonState[i];
        }

        virtualJoypadButtonAssign();

}