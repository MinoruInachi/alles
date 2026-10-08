# Alles - メッシュ・シンセサイザー

![picture](https://raw.githubusercontent.com/shorepine/alles/main/pics/alles-revB-group.png)

 [![shore pine sound systems discord](https://raw.githubusercontent.com/shorepine/tulipcc/main/docs/pics/shorepine100.png) **Alles について Discord で話しましょう！**](https://discord.gg/TzBFkUb8pG)


**こちらの動画をご覧ください！**

[![The Alles mesh networking synthesizer - a field of sound at your control](https://i.ytimg.com/vi/8CmcsQXHVEo/maxresdefault.jpg)](https://www.youtube.com/watch?v=8CmcsQXHVEo "The Alles mesh networking synthesizer - a field of sound at your control")


**Alles** は、WiFi 経由で応答する多スピーカーの分散メッシュ・シンセサイザーです。各シンセ（1 つのメッシュに数百台置けます）は最大 120 個の加算合成オシレーターを持ち、オシレーターごとにフィルター、モジュレーション / LFO、ADSR を使えます。スピーカーのメッシュは、私たちの専用ハードウェア・スピーカーと、コンピューター上で動くプログラムを自由に組み合わせて構成できます。ソフトウェアもハードウェアもオープンソースなので、自分で作ることも、私たちから PCB を購入することもできます。

シンセサイザーは自動的にメッシュを形成し、WiFi のマルチキャスト・メッセージを受信します。メッシュはホストコンピューターから、任意のプログラミング言語や Max、Pd などの環境を使って制御できます。

最初の用途として想定していたのは、[Alles Machine](https://en.wikipedia.org/wiki/Bell_Labs_Digital_Synthesizer) / [Atari AMY](https://www.atarimax.com/jindroush.atari.org/achamy.html) 加算合成シンセサイザーの分散 / 空間版です。各スピーカーが最大 64 個の倍音（パーシャル）を受け持ち、全体としても個別にも制御できます。もちろん、単に数十台の独立したシンセサイザーとして扱い、好きなように使うこともできます。とても楽しいですよ！

今すぐ試してみたいですか？ [自分で Alles を作る](#diy-alles)か、[ソフトウェア版をインストール](#ソフトウェア版-alles-を使う)して、[入門チュートリアル](https://github.com/shorepine/alles/tree/main/getting-started.md)を読んでみてください！



## シンセサイザーの仕様

個々のシンセは [AMY シンセサイザー・ライブラリ](https://github.com/shorepine/amy/blob/main/README.md)で動いています。詳しくはそちらをご覧ください。

## ハードウェア版 Alles を使う

初回起動時、各ハードウェア・スピーカーは `alles-synth-X`（X はシンセの ID）という名前のキャプティブ WiFi ネットワークを作ります。（できればモバイル端末から）そのネットワークに接続すると、WiFi 設定用のキャプティブページにリダイレクトされるはずです。リダイレクトされない場合は、ネットワークに接続した後でブラウザから `http://10.10.0.1` を開いてください。各シンセに接続先 WiFi の SSID とパスワードを設定すると、シンセは再起動します。この設定はシンセごとに 1 回だけ必要です。

## ソフトウェア版 Alles を使う

Alles スピーカーを作ったり買ったりしたくない場合は、自分のコンピューター上で Alles を好きなだけ動かせます。各ソフトウェアが同じネットワーク内で動いていれば、ハードウェア・スピーカーと同じように自動的にメッシュを形成します。ハードウェアとソフトウェアのスピーカーは区別なく使えます。

コンピューター上で `alles` をビルドして実行するには、このリポジトリをクローンして次のようにします。

```bash
$ cd alles/main
$ make
$ ./alles
$ ./alles -h # 使用するチャンネル / サウンドカードや送信元 IP アドレスの変更など、便利なコマンドライン引数を表示します
```

## メッシュの制御

**チュートリアルとして、新しい[入門ガイド](https://github.com/shorepine/alles/tree/main/getting-started.md)をご覧ください！**

WiFi 上の UDP を使って、1 台のホストからメッシュ上のすべてのシンセを制御できます。1 つのメッセージで、メッシュ内の任意のシンセ、または全シンセを一度に指定でき、グループ指定も使えます。この方法は Max や Pd などの音楽環境、Python などの言語を使うミュージシャンや開発者、Alles の全機能を DAW とつなぎたいプラグイン開発者が利用できます。

Alles のワイヤープロトコルは、オシレーターのあらゆるパラメーターを表す、ASCII 文字で区切られた数値の並びです。これは、どんな環境からでもできるだけ簡単に Alles を使えるようにするための設計判断で、クライアント側にデータ構造や解析の負担がありません。人が読めてコンパクトで、MIDI よりはるかに表現力があり、ネットワーク、UART、関数やコマンドの引数として送ることができます。

Alles は ASCII のコマンドを受け付け、各コマンドは `Z` で区切ります（通信路がネットワークなら、オーバーヘッドを避けるために複数のメッセージを 1 つにまとめられます）。例えば次のようにします。

```
v0w4f440.0l0.9Z
```

シンセのパラメーターの一覧は [AMY の readme](https://github.com/shorepine/amy/blob/main/README.md) を参照してください。


## alles.py

Alles には、Python で書かれたフル機能のクライアントが付属しています。自由に改変したり、自分のクライアントに組み込んだりしてください。ドキュメント、API、テストスイートとしても使えます。`import alles` するだけでメッシュ全体を制御できます。

```bash
$ python3
>>> import alles
>>> alles.drums() # 全シンセでドラムパターンを演奏します
>>> alles.drums(client=2) # 1 台だけで演奏します
```

その他の例は、新しい[入門ガイド](https://github.com/shorepine/alles/tree/main/getting-started.md)をご覧ください。

## 個々のシンセサイザーの指定

既定では、メッセージは起動中のすべてのシンセサイザーで演奏されます。`client` パラメーターを使うと、個別に、またはグループで指定できます。

シンセサイザーはメッシュを形成し、どれが動いているかを互いに識別します。各シンセには 0 から 255 までの `client_id` が自動的に割り当てられます。メッシュ内で最初に起動したシンセが `0`、次が `1` という具合です。シンセの電源が切れるなどしてハートビート信号をメッシュに送らなくなると、`client_id` は常に連番になるように振り直されます。シンセが起動してからメッシュに参加し `client_id` が割り当てられるまで 10〜20 秒かかることがありますが、全シンセ宛てのメッセージは起動直後から受信します。

作曲者が扱いやすいように、`client` パラメーターは起動中のシンセの台数で折り返します。6 台のシンセが起動している場合、`client` が 0 なら 1 台目だけ、`1` なら 2 台目だけに届き、`7` なら 2 台目に届きます（`7 % 6 = 1`）。

`client` を 255 より大きい値にすると、グループを指定できます。例えば `client` が 257 のとき、起動中の各シンセは `my_client_id % (client-255) == 0` を判定します。これは 1 台おきのシンセだけを指定することになります。`client` が 259 なら 4 台に 1 台、という具合です。

ホスト側でシンセを列挙したい場合は、ハートビート・メッセージを受信できます。下記の `sync` を参照してください。

## タイミングとレイテンシー

Alles は、操作がすぐに音に反映される低レイテンシーのリアルタイム演奏楽器としては設計されていません。ホストで行った変更が各シンセに届くまでには固定のレイテンシー（現在の既定値は 1000ms）がかかります。WiFi の伝送レイテンシーは大きくばらつきますが、この固定レイテンシーによって、メッシュ内のすべてのシンセ（ESP32 ベースのものもコンピューター上で動くものも）にメッセージが間に合うように届き、完全に同期して演奏できます。これにより、広い空間に置いた数十台のスピーカーで、ミリ秒単位で正確なタイミングの演奏ができます。

ホストは、音を鳴らしたい相対時刻を `time` パラメーターとして送ってください。ホストが起動してからのミリ秒数を使うことをお勧めします。Python なら例えば次のようにします。

```python
def millis():
    d = datetime.datetime.now()
    return int((datetime.datetime.utcnow() - datetime.datetime(d.year, d.month, d.day)).total_seconds()*1000)
```

`alles.py` を使っていれば、これは自動で行われます！

Max を使う場合は、`cpuclock` オブジェクトを `time` パラメーターに使ってください。

`time` 付きのメッセージを最初に送ったとき、シンセのメッシュはそれを使って、自分の時刻とホストが想定する時刻との差分を求めます（`time` パラメーターを一度も送らない場合は、WiFi のジッター次第になります）。以降のメッセージは、固定のレイテンシーを伴いつつ、メッセージ間でミリ秒単位の精度になります。音速による遅延を補正したい場合は、クライアントごとに `time` を調整することもできます。

`time` パラメーターは、クライアント側で遠い未来の予定を組むためのものではありません。想定される差分から 20,000ms 以上ずれた `time` を送ると、時刻の基準が再計算されます。演奏の状態や今後のイベントの管理は、ホストが主な「シーケンサー」として行うようにしてください。

レイテンシーは調整できます。ネットワークに自信があれば短くできますし、ローカル（127.0.0.1）接続を使う場合や、コード内で直接メッセージを送る場合は 0 にもできます。

## シンセの列挙

`sync` コマンド（`alles.sync()` を参照）を送ると、オンラインの各シンセサイザーから即座に応答が返ってきます。応答は `_s65201i4c248y2` のような形式で、s はクライアントの時刻、i は応答対象のインデックス、y はバッテリーの状態（対応しているバージョンのみ）、c はクライアント ID です。これにより、起動中の各シンセの一覧を作れるだけでなく、インデックスを変えて多数のメッセージを送れば、シンセごとの往復レイテンシーと信頼性も求められます。

## 演奏時の WiFi と信頼性

UDP マルチキャストは本質的に「ロスがある」方式で、メッセージがシンセに届く保証はありません。多くの要因に左右されますが、特に無線ルーターと他の機器の存在によっては、信頼性が 70% まで下がることもあります。演奏用途では、既存の WiFi ネットワークではなく専用の無線ルーターを使うことを強くお勧めします。多くの「QoS（サービス品質）」機能をオフにできる必要があります（これらは無作為に選んだシンセを優先するため、同期が難しくなります）。また、理想的には WiFi の直接のクライアントがシンセサイザーだけになるようにします。簡単な方法は、専用の無線ルーターを用意し、インターネットには接続しないことです。ラップトップやホストマシンはルーターに有線で接続し（必要なら USB-イーサネット・アダプターを使います）、ラップトップの WiFi など他のインターネット接続は有効にしておきます。制御用のソフトウェアでは、マルチキャスト・パケットを送受信する送信元のネットワークアドレスを指定するだけです。`alles_util.py` にそのための設定コードがあります。こうすれば、ホストマシンは普段のネットワークにつながったまま、2 つ目のインターフェースからシンセを制御できます。

ネットワークを管理できない場所では、メッセージを N 回送ることで信頼性の低下を緩和できます。（同じ `time` パラメーターを持つ）重複したメッセージを複数送っても、シンセに悪影響はありません。


## クライアント

最小限の Python の例です。

```python
import socket
multicast_group = ('232.10.11.12', 9294)
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def send(oscillator=0, freq=0, vel=1):
    sock.sendto("v%df%fl%fZ" % (oscillator, freq, vel), multicast_group)

def c_major(octave=2):
    send(oscillator=0,freq=220.5*octave)
    send(oscillator=1,freq=138.5*octave)
    send(oscillator=2,freq=164.5*octave)

```

より良い例は [`alles.py`](https://github.com/shorepine/alles/blob/main/alles.py) を参照してください。ソケットとマルチキャストに対応した言語なら何でも使えます。新しいクライアントのプルリクエストを歓迎します！

Max や Pd でも簡単に使えます。

![Max](https://raw.githubusercontent.com/shorepine/alles/main/pics/max.png)


# シンセサイザーの詳細

シンセサイザー自体の詳細は [AMY の readme](https://github.com/shorepine/amy/blob/main/README.md) を参照してください。


# 自分の Alles を手に入れよう！

**[スピーカーの筐体に PCB を組み込む方法はこちら！](speaker-assembly.md)**

このページや動画に出てくる小さな円形のスピーカーは簡単に手に入ります。Alibaba のリンクはこちらです：[円形の A60](https://www.alibaba.com/product-detail/A60-Wooden-Grain-Portable-Wireless-Bluetooth_1600291216741.html?spm=a2700.wholesale.0.0.3e6c344biON4Qa)。[四角い A70](https://www.alibaba.com/product-detail/A70-Wood-Speaker-Grain-Portable-Wireless_1600291333969.html) も使えます！ そのうえで、スピーカーとバッテリー用の端子がない Alles PCB を私たちから購入できます。筐体側の配線は基板に直接はんだ付けします。私の場合、A60 や A70 スピーカーに Alles PCB を組み込むのに 3 分ほどかかります。この方法なら、必要な道具はワイヤーストリッパー、小さなドライバー、はんだごてだけです。

**重要：** Alles は「ベストエフォート」のコミュニティサポートを前提に販売しています。最新の Alles ファームウェアを書き込んだ動作品の PCB をお送りしますが、その後のことはコミュニティの助けに委ねられます。Alles を使い始めるには、コンピューターとネットワークの知識が少し必要です。できるだけ簡単にしたつもりですが、幅広い人に使ってもらえるようになるまでには、まだやるべきことがあると考えています。困ったことがあれば GitHub の issue をご利用ください。解決できるよう最善を尽くします。

![blinkinlabs PCB](https://raw.githubusercontent.com/shorepine/alles/main/pics/alles_reva.png)


## DIY Alles

Sparkfun、Adafruit、Amazon などの電子部品販売店で手に入る部品で、とても簡単に自作できます。

Alles シンセを自作するには、次のものが必要です。

* [ESP32 開発ボード（どれでも構いませんが、ピンが引き出されているもの）](https://www.amazon.com/gp/product/B07Q576VWZ/)（2 個入り、1 個 $7.45）。OTA（無線）アップデートを使いたい場合は、8MB フラッシュのものを選んでください。
* [Adafruit の I2S モノラル・アンプ](https://www.adafruit.com/product/3006)（$5.95）
* [4Ω のスピーカー。これは特に良いです](https://www.parts-express.com/peerless-by-tymphany-tc6fd00-04-2-full-range-paper-cone-woofer-4-ohm--264-1126?gclid=EAIaIQobChMIwcX3-vXi5wIVgpOzCh0a7gjuEAYYASABEgLwf_D_BwE)（$9.77。音質にこだわらなければ、安いものにして大きく節約できます）。[このブックシェルフ・スピーカー](https://www.amazon.com/Pyle-PCB3BK-100-Watt-Bookshelf-Speakers/dp/B000MCGF1O/ref=sr_1_1?dchild=1&keywords=pyle+home+speaker&qid=1592156929&s=electronics&sr=1-1)のような、筐体付きのスピーカーも気に入っています。
* ブレッドボード、自作 PCB、またはジャンパーワイヤーだけでも OK！

5V 入力（USB バッテリー、USB 入力、電源入力に直結した充電池）で、両方のボードとスピーカーをかなりの音量で鳴らせます。3.7V の LiPo バッテリーでも動きますが、3.7V を与えると I2S アンプは（歪まずに）それほど大きな音を出せない点に注意してください。DIY Alles を持ち運びたい場合は、[低電流で自動的に電源が切れる](https://www.element14.com/community/groups/test-and-measurement/blog/2018/10/15/on-using-a-usb-battery-for-a-portable-project-power-supply)機能のない USB バッテリーパックをお勧めします。ユニット全体の消費電流は、大音量時で約 90mA、待機時で 40mA です。

DIY Alles は次のように配線します（I2S -> ESP）。

```
LRC -> GPIO25
BCLK -> GPIO26
DIN -> GPIO27
GAIN -> I2S Vin（私は I2S ボード上でジャンパーしています）
SD -> 未接続
GND -> GND
Vin -> Vin / USB / 3.3（または 5V 電源に直結）
スピーカー端子 -> スピーカー
```

![DIY Alles 1](https://raw.githubusercontent.com/shorepine/alles/main/pics/diy_alles_1.png)
![DIY Alles 2](https://raw.githubusercontent.com/shorepine/alles/main/pics/diy_alles_2.png)


### DIY 用ブリッジ PCB

*DIY Alles を作るのにこの PCB は必要ありません！* ジャンパーワイヤーだけでも動きます。ただ、DIY Alles をたくさん作っていて安定性を高めたい場合のために、ボード同士をつなぐ小さな基板を作りました。こんな感じです。

![closeup](https://raw.githubusercontent.com/shorepine/alles/main/pics/adapter.jpg)

これは、推奨の ESP32 開発ボードとそのピン配置を使う前提です。別のボードを使う場合は、`alles.h` で GPIO の割り当てを変更すれば対応できるはずです。Fritzing ファイルはこのリポジトリの `pcbs` フォルダーにあり、[Aisler でも公開しています](https://aisler.net/p/TEBMDZWQ)。特に GAIN の接続について、短いジャンパーワイヤーを切って配線するよりはるかに安定し、配線も簡単です。

## 4MB ボードで OTA を無効にする

自分の開発ボードを使っていて、フラッシュが 8MB 未満（4MB がよくあります）の場合は、まず `alles_partitions.csv` を `alles_4mb_partitions.csv` の内容で上書きし、`idf.py menuconfig` を実行してフラッシュサイズを 8MB から 4MB に変更してください（Serial Flasher Config の中にあります）。これで OTA によるファームウェア更新は無効になりますが、それ以外は問題なく動きます。Alles のバイナリは合計で 4MB 近くあり、OTA にはフラッシュ上に 2 つ分を保存する容量が必要です。

## ESP32 ファームウェア

Alles は完全なオープンソースで、現在の機能を超えて改造していく楽しいプラットフォームになります。

USB 接続を使って、ファームウェアを更新したり、自作のファームウェアを書き込んだりできます。[ハードウェア版 Alles スピーカーへの書き込みガイド](https://github.com/shorepine/alles/tree/main/alles-flashing.md)を参照してください（DIY 版にも私たちの製品にも使えます）。

## M5Stack Atom VoiceS3R

Alles は [M5Stack Atom VoiceS3R](https://docs.m5stack.com/ja/core/Atom_VoiceS3R)（ESP32-S3、ES8311 コーデック、1W スピーカー）でも動きます。ESP-IDF 5.3 以降でビルドして書き込みます。

```bash
$ idf.py set-target esp32s3   # 初回のみ必要です。sdkconfig.defaults が読み込まれます
$ idf.py build
$ idf.py -p /dev/cu.usbmodemXXXX flash monitor
```

Atom にはボタンが 1 つあります。短押しで音量が 1 段階上がり（最大の次は最小に戻ります）、3 秒長押しすると保存済みの WiFi 設定を消去して `alles-synth-X` の設定用ネットワークで再起動します。スピーカーが 1 つなので、出力はモノラルにミックスされます。バッテリーも電源スイッチもないため、WiFi に接続できないときは電源を切る代わりに 2 分後に WiFi 待ちのチャイムを止めます。また、OTA アップデート（ESP32 用のイメージを取得します）は使いません。

## M5Stack Atom Voice

Alles は [M5Stack Atom Voice](https://docs.m5stack.com/ja/atom/Atom_Voice)（ESP32-PICO-D4、NS4168 I2S アンプ、0.8W スピーカー）でも動きます。ESP-IDF 5.3 以降でビルドして書き込みます。

```bash
$ idf.py set-target esp32   # 初回のみ必要です。sdkconfig.defaults と sdkconfig.defaults.esp32 が読み込まれます
$ idf.py build
$ idf.py -p /dev/cu.usbserial-XXXX -b 115200 flash monitor
```

Atom の USB シリアルは速い通信速度では途切れるため、115200bps で書き込んでください。ボタン、モノラル出力、WiFi まわりの動作は Atom VoiceS3R と同じです。Atom Voice には PSRAM がないため、AMY のタスクスタック、デルタプール、MIDI SysEx バッファを小さくしてビルドし、オシレーターは 120 個ではなく 64 個（使うときに確保されます）にしています。WiFi 接続後、オシレーター用に約 90KB の RAM が残ります。


## 謝辞

* Alles は [DAn Ellis](https://research.google/people/DanEllis/) の助けなしには実現しませんでした。オシレーター周りの大部分を手伝ってもらい、アルゴリズム・シンセについて何度も深く掘り下げ、ESP32 のコードについても多くの優れたアイデアや修正をもらいました。
* Douglas Repetto
* [MSFA](https://github.com/google/music-synthesizer-for-android) の開発者である [Raph Levien](https://www.levien.com)。FM の実装について多くのヒントをもらいました
* mark fell
* [esp32 WiFi Manager](https://github.com/tonyp7/esp32-wifi-manager)
* kyle mcdonald
* Matt Mets / [Blinkinlabs](https://blinkinlabs.com)


## TODO

* ~~電源ボタン~~
* ~~WiFi 設定で省電力 / レイテンシーの既定値を尋ねるべきか ―― 今のところしない~~
* ~~複数のサイン波を混ぜたときの高振幅での歪みを取り除く~~
* ~~FM~~
* ~~シンセ同士が互いに自己識別すべきか？ Max で使いやすくなる~~
* ~~netgear ルーターでの UDP レイテンシーの大きな揺れに対処できないか調べる~~
* ~~エンベロープ / ノートオン・オフ / LFO~~
* ~~Max/Pd から UDP がまだ動くか確認する~~
* ~~矩形波 / のこぎり波 / 三角波オシレーターの帯域制限~~
* ~~Karplus-Strong~~
* ~~現場で設定するための WiFi ホットスポット・モード~~
* ~~複数台向けのブロードキャスト UDP~~
* ~~パケットの欠落~~
* ~~複数デバイス間での同期と列挙~~
* ~~アドレス指定 / 1 台またはグループとの通信（「半分 / 4 分の 1 / 全部で鳴らす」など）~~
* ~~タイミング / ジッターについてできる限りの対策 ―― 時刻の同期？ 時刻付きメッセージ？~~
* ~~筐体 / バッテリーの構成~~
* ~~音量のオーバーロード（FM のみだと思う）でクラッシュする~~
* ~~UDP メッセージのクリックノイズ~~
* デスクトップ用 USB 書き込みツール
* キャプティブポータルの代わりに BT / アプリでの設定（将来）
