
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


