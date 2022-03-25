//流れは
//キーマクロ再生フラグ検出（仮想ジョイパッド15~19ボタン押下？）
//物理ジョイパッドの状態検知一時停止
//キーマクロをフレームごとに再生する
//物理ジョイパッドの状態検知を復帰する


//キーマクロ再生（ステップを進める？）
void KeyMacro::playKeyMacro() {
        if(procedure_arg.RemainFrame > 0){
                for (int i=0;i<4;i++){
                        VHatState[i] = procedure_arg.MHatState[i];
                }
                for (int j=0;j<=HANDLE_OUTPUTBUTTON_NUM;j++){
                        VButtonState[j] = procedure_arg.MButtonState[j];
                        // Serial.print(VButtonState[j]);
                        // Serial.println();
                }
                // Serial.print("step");
                // Serial.print(procedure_step);
                // Serial.print("Remain");
                procedure_arg.RemainFrame--;
        } else {
                procedure_step++;
                if(procedure_step > size_arg_macro){
                        procedure_arg = MacroDetail[procedure_step];
                        // Serial.print(procedure_arg.MHatState[0]);
                        // Serial.print(procedure_arg.MButtonState[0]);
                        // Serial.print(procedure_arg.RemainFrame);

                } else {
                        isMacroInProcess = false;
                }
        }
}
//キーマクロ初期化
void KeyMacro::startKeyMacro() {
        isMacroInProcess = true;
        procedure_step = 0;
        procedure_arg = MacroDetail[procedure_step];
        size_arg_macro = (sizeof MacroDetail) / (sizeof(struct KeyMacroStep));
}
//キーマクロの開始フラグを検知
void KeyMacro::checkKeyMacroFrag() {
        if(!isMacroInProcess){
                //Serial.println("Macro Check");
                for(int i=HANDLE_BUTTON_NUM;i<HANDLE_BUTTON_NUM + HANDLE_KEYMACRO_NUM;i++){
                        if(VButtonState[i]) {
                                KeyMacro::startKeyMacro();
                                Serial.println("Macro Start");
                        }
                }
        }
}
