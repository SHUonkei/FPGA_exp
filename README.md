# FPGA_exp
FPGA実験

# discussion
- start, ending 画面は作る
- 7セグは、ゲームの残り時間表示
　- 99sec
- キーパッドで、プレイヤーは操作
　- プレイヤー１
    - 1,0で上下移動
    - 7で強打
    - 4で弱打
  - プレイヤー2
    - A, Dで上下移動
  　- Bで強打
    - Cで弱打
- Aボタンでスタート
- 音流す
 - bgm
 - ラケットにボールが当たった音
- 一定確率で斜めに反射する
- スピード調節は、スタート画面で選択
- 1pモード
  - スタート画面で選択
  - CPUは最強設定で、すべてを打ち返す


### 内部仕様
- status
  - どの画面にいるかの
- show_start(), play(), show_end()
 - スタート画面、ゲーム、結果画面の表示関数    
 - show_start()
   以下をいじれるようにする。
   - speed
   - mode
   関数
   - set_speed()
    - A,B,Cの三択
    - A < B < Cの順に早い
   - set_mode()
    - 1, 2
     - 1: single mode
     - 2: vs mode
   - Eぼたんで、1をreturn
 - 
