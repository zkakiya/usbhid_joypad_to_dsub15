//キーマクロ再生（ステップを進める？）
void KeyMacro::playKeyMacro() {
        if(procedure_arg.RemainFrame > 0){
                for (int i=0;i<4;i++){
                        VHatState[i] = procedure_arg.MHatState[i];
                }
                for (int j=0;j<=HANDLE_OUTPUTBUTTON_NUM;j++){
                        VButtonState[j] = procedure_arg.MButtonState[j];
                }
                Serial.print("step");
                Serial.print(procedure_step);
                Serial.print("Remain");
                Serial.println(procedure_arg.RemainFrame);
                procedure_arg.RemainFrame--;
        } else {
                procedure_step++;
                if(procedure_step >= size_arg_macro){
                        procedure_arg = MacroDetail[procedure_step];
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
        vButton_for_keymacro[0] = HANDLE_BUTTON_NUM + 0;
        vButton_for_keymacro[1] = HANDLE_BUTTON_NUM + 1;
        vButton_for_keymacro[2] = HANDLE_BUTTON_NUM + 2;
        vButton_for_keymacro[3] = HANDLE_BUTTON_NUM + 3;
        vButton_for_keymacro[4] = HANDLE_BUTTON_NUM + 4;
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
