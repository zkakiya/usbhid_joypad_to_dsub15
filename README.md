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
配線図を今後準備予定

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

