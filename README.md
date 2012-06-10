 MiniSynth
------------------------------------------------------------

#### 概要  
シンプルなVSTiシンセサイザーです。  
C++で書かれています。  
別途 VST Audio Plug-Ins SDK 2.4 が必要です。 
<http://www.steinberg.net/en/company/developer.html>

#### 機能  
* 32音ポリフォニック
* 2オシレーター
* ADSRエンベロープ
* LPF
* monoモード
* ポルタメント
* 適当ディレイ

#### TODO
* LFO
* ユニゾン
* デチューン
* 各種エフェクト
* パラメータをなめらかに変化するようにする
* コメントを書く

#### クラス説明
VstXSynth: SDKとSynthesizerクラスを繋ぐクラス  
  
SignalProcessor: 音声処理の基底クラス  
  
Synthesizer: シンセサイザー本体クラス（SignalProcessorを継承）  
* MIDIを処理してVoiceControllerに処理を投げる
* VoiceControllerから出てきた音をmixする
* 各種エフェクトに処理を投げる
* パラメータやパッチの管理
  
VoiceController: VCO,VCF,VCAを管理するクラス（SignalProcessorを継承）  
* VCO,VCFに処理を投げて音量を調整する
* エンベロープの管理
  
Oscillator: オシレータクラス（SignalProcessorを継承）  
* Wavetableを参照して波形を生成
  
Filter: フィルタクラス（SignalProcessorを継承）  
  
Delay: ディレイクラス（SignalProcessorを継承）  
  
Wavetable: 波形テーブルクラス  
* singleton
* 固定小数点による一次補間
  
Random: 乱数クラス（未実装）  
  
#### その他
Synth1の作者Daichiさんのサンプルコードを参考にしています。  
<http://www.geocities.jp/daichi1969/vstisample/>  
  
 Copyright (c) 2012 y3eadgbe
