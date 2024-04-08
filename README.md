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
D-sub15ピンコネクタ（メス）からarduinoのピンへの接続は  
1 GND  
2 A4  
3 A2  
4 A0  
5 7  
6 5  
7 3  
8 RAW  
9 A6  
10 A3  
11 A1  
12 8  
13 6  
14 4  
15 2  
としました。

キープリセット用のロータリースイッチ回路は
* Arduino - ロータリスイッチの状態をA/D変換で取得する  
https://keitetsu.blogspot.com/2014/11/arduino-ad.html

の回路図から制作しました。  
検出用のピンはA5ピンに接続しています。  
ただし、+5V部分はaruduino pro mini(3.3v)の場合、vccピン(3.3v)に接続しています。  
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


# Lisence

This project is licensed under the MIT License, see the 
LICENSE.txt
file for details


# Author
* カキヤザクロ
* e-mail: z.kakiya@gmail.com
* discord: z_kakiya#2835

