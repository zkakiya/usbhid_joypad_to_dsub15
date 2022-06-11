void frameManagement()
{
    unsigned long curr = millis(); //現在時刻取得
    if (curr - frame_checker > frame_ms)
    { //現在時刻とフレーム管理時計のタイミングズレチェック
        Serial.print("*** processing delay :");
        Serial.println(curr - frame_checker);
        frame_checker = millis(); //現在時刻取得
    }
    else
    {
        // 余剰時間を消化する処理。時間がオーバーしていたらこの処理を自然と飛ばす。
        while (curr - frame_checker < frame_ms)
        {
            curr = millis();
        }
        frame_checker = curr;
    }
}
