void frameManagement(){
        unsigned long frame_checker;//フレーム管理時計の時刻
        unsigned long merc;
        unsigned long curr_micro;//現在時刻をマイクロ秒で取得する変数

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