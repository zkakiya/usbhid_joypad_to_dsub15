# usbhid_joypad_to_dsub15
arduinoにてusbHIDJoypadをD-sub15ピンコネクタ（ネオジオ）に変換するためのスケッチです。  
ネオジオ本機で動かす……というより、シリパラ変換のインタフェースとしてネオジオコントローラーの仕様に合わせた出力をしている……  
という方が実態に近く、JAMMA等の機器で動かすためのハブとして機能させることを意識してます。

ハードにはロータリースイッチを用いた装置を組み込んでおり、
キーコンフィグのプリセットを切り替えることができます。

また、ハード側は単にボタン入力を変換するだけでなく、連射装置やキーマクロ等も（理論上は）可能な構造になっています。

# 必須データ
USB_Host_Shield_2.0 library  
https://github.com/felis/USB_Host_Shield_2.0

# ハードについて
（配線図を今後準備予定）  
ピンアサインは  
2 UP  
3 DOWN  
4 LEFT  
5 RIGHT  
6 BTN_1  
7 BTN_2  
8 BTN_3  
A0 BTN_4  
A1 START  
A2 SELECT/COIN  
A3 BTN_5  
A4 BTN_6  
A5 (KeyPreset)  
A6 BTN_0

キープリセット用のロータリースイッチ回路は
* Arduino - ロータリスイッチの状態をA/D変換で取得する  
https://keitetsu.blogspot.com/2014/11/arduino-ad.html  
の回路図から制作しました。  
検出用のピンはA5ピンに接続するようにしています。
ただし、+5V部分はaruduino pro miniの場合、vccピンに接続しています。  
arduino UNOの場合は5Vピンです。

# 参考資料
USB_Host_Shield_2.0 library  
https://github.com/felis/USB_Host_Shield_2.0  
および、  
以下のスケッチを参考にしています。

* USB接続のJoystickをArduinoとUSB Host Shieldでボタン検知する  
https://qiita.com/MergeCells/items/e19b0f93459fe2daf661

* Arduino - ロータリスイッチの状態をA/D変換で取得する  
https://keitetsu.blogspot.com/2014/11/arduino-ad.html

* Arduinoで一定の単位時間（フレーム）ごとに処理を進める  
https://qiita.com/Ninagawa_Izumi/items/f8585c5c711bcf065656


# Author
* カキヤザクロ
* e-mail: z.kakiya@gmail.com
* discord: z_kakiya#2835

