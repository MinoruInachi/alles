


# Alles スピーカー RevB 入門ガイド

![picture](https://raw.githubusercontent.com/shorepine/alles/main/pics/alles-revb-moss.png)

こんにちは！ あなたは少なくとも 1 台の Alles スピーカーを手にした幸運な持ち主です。このガイドで使い始め方を説明します。


## スピーカー

### まだスピーカーを持っていない場合

大丈夫です！ ソフトウェアのスピーカーを動かすこともできます。[セットアップ](#セットアップ)の節まで進み、[コンピューター上で Alles を動かす](#コンピューター上で-alles-を動かす)の手順でソフトウェア版のスピーカーを動かしてください。その後は、このガイドの続きをそのまま進められます。

### ハードウェアのスピーカーに戻ります

[Alles PCB をスピーカーの筐体に組み込む方法はこちら！](speaker-assembly.md)

各 Alles スピーカーには、上部に 4 つのボタン、背面に USB micro 端子があります。

始める前に、スピーカーが充電済みか充電中であることを確認しましょう。付属の USB ケーブルで、スピーカーとスマホの充電器などの USB 充電ポートをつなぐだけです。（コンピューターにつなぐと、スピーカーはデバッグやアップグレード用の USB デバイスとして認識されますが、今は気にしなくて大丈夫です。操作はすべて無線で行います。）バッテリーは、音楽を鳴らし続けて「数時間」、まばらに鳴らしたり無音だったりすればもっと長く持ちます。充電は 1 時間程度で済むはずです。電源ボタンを押しても何も音がしない場合は、おそらくバッテリー切れです。

上部のボタンは、スピーカー側を向いて左から電源、音量 + と -、WiFi 設定（再生 / 一時停止のボタン）です。音量はこれらのボタンでも、作品のコードから `alles.volume()` を使っても設定できます。`alles.volume()` を使うと、ボタンで設定した値は上書きされます。

![Picture of speaker](https://raw.githubusercontent.com/shorepine/alles/main/pics/revb-top.png)

電源ボタンでスピーカーのオン / オフを切り替えます。オンにすると、ネットワークとメッシュに再び参加します。

WiFi ボタンを押すと、保存されている WiFi の情報を消去し、デバイスを WiFi 設定モードにします。WiFi がまだ設定されていない場合は、これが既定の状態です。


## WiFi ネットワークに参加する

スピーカーを初めて使う場合は、どの WiFi ネットワークに参加するかを設定する必要があります。後から簡単に変更できますが、一度設定すればもう設定し直す必要はありません。

電源ボタンを押してください。「チャイム音」が繰り返し鳴り始めます。これは「WiFi を探している」音で、WiFi のアクセスポイントが見つかるか、もう一度電源ボタンが押されるか、2 分が経過するまで鳴り続けます。2 分が経過すると電源が切れます。

WiFi ネットワークを設定済みであれば、数秒後にチャイム音が止まり、「ピッ」という音が鳴ってから静かになります。これで WiFi ネットワークに参加し、準備が整ったことを意味します。

WiFi をまだ設定していない場合は、手近なスマホか、ブラウザのある WiFi 機器を用意してください。その機器で新しい WiFi ネットワークへの接続を開くと、`alles-synth-XXXXXX` という名前のネットワークが表示されます。`XXXXXX` はスピーカーごとに固有の文字列です（スピーカーがたくさんあるときに便利です！）。そのネットワークに接続してください。ほとんどの機器（特に iPhone や Android）では、数秒後にログインページのブラウザ画面が表示されます。ホテルのネットワークなど、キャプティブポータルに接続したときと同じような動作です。ページが表示されない場合は、ネットワークに接続した後でブラウザから `http://10.10.0.1` を開いてみてください。

![Alles WiFi settings](https://raw.githubusercontent.com/shorepine/alles/main/pics/alles-wifi.png)

数秒待つと、ログインページに周囲で見つかった WiFi ステーションが一覧表示されます。スピーカーに参加させたいネットワークが表示されたらタップし、パスワードを慎重に入力してください。Alles を操作するコンピューターと同じネットワークにしてください。しばらくするとスピーカーのチャイムが止まり、ネットワークへの参加と、内部ストレージへの設定の保存が成功したことを示します。

うまくいかない場合は、WiFi ボタン（再生 / 一時停止）を押すと、同じ手順をもう一度やり直せます。

## ファームウェアの更新

Alles スピーカーのファームウェアを最新にしておくと役に立ちます。最近のスピーカーなら、無線で更新できます。古いスピーカーの場合は少し手間がかかりますが、数分の準備で済みます（その後は無線で更新できるようになります）。[ハードウェア版 Alles スピーカーの更新ガイド](https://github.com/shorepine/alles/tree/main/alles-flashing.md)を参照してください。

## Alles を操作する

メッシュ（スピーカーのグループ）には、オシレーターの状態を定義するメッセージを明示的に送ります。想像できるものは何でも、高い精度とミリ秒単位の正確さで制御できます。メッシュ内の任意の台数のスピーカーそれぞれで、最大 64 個のオシレーターを制御できます。操作には、Python（私たちが使っているもの）のようなプログラミング言語か、Max/MSP（または Max for Ableton Live）のような環境を使います。Python は Mac に標準で入っていて、慣れればとても簡単に使えます。小さなプログラムを書いて面白い音を作れます！ このチュートリアルでは Python を使います。ただし、Python で変更するのと同じパラメーターすべてにアクセスする方法を示す Max パッチもダウンロードできます。お好きな方をどうぞ！

では Python を起動しましょう。ターミナルを開いてください。Mac ならターミナル.app、Windows なら WSL の利用をお勧めします。Linux ならインストール済みのものを使ってください。まだなら[このリポジトリをクローン](https://github.com/shorepine/alles/)します（`git clone https://github.com/shorepine/alles.git`）。サブモジュールも更新してください（`cd alles; git submodule update --init --recursive`）。リポジトリのディレクトリにいることを確認して、`python3` と入力します。Mac でこういう作業をしたことがない場合、初めて Python を実行するときに Apple から小さなツールのダウンロードを求められることがあります。完了するまで待ってください。すると `>>>` のようなプロンプトが表示されます。

まず、メッシュを操作するための Python モジュールをインポートします：`import alles`


### 簡単な例

`alles.drums()` を実行すると、現在電源の入っているすべてのスピーカーからテストパターンが鳴ります。すべて同期して、同じものを演奏するはずです。

`alles.volume(2)` でスピーカーの音量を設定してみてください。最大で 10 くらいまで上げられます。既定値は 1 です。スピーカーの + と - のボタンも使えますが、`alles.volume()` で設定した値はボタンの設定を上書きします。

スピーカーを静かにしたいときや、挙動がおかしいときは、`alles.reset()` を使ってください。すべてのスピーカーを既定の状態に戻します。いろいろ試しているうちにオシレーターがおかしな状態になることがありますが、`alles.reset()` はそこからの脱出口です。例えば `alles.reset(osc=5)` のようにして、1 つのオシレーターだけをリセットすることもできます。

まず単純なサイン波を設定しましょう。

```python
alles.send(osc=0, wave=alles.SINE, freq=220, amp=1)
```

やっていることはわかりやすいと思います。オシレーター 0 を、220Hz・振幅 1 のサイン波に設定しています。`alles.PULSE` や `alles.SAW_DOWN` なども試してみてください。

**まだ何も聞こえないのはなぜでしょう？** このオシレーターのノートオンをトリガーしていないからです。ノートのオン / オフ（`vel=0` でオフ）を切り替える `vel`（ベロシティ）というパラメーターがあります。オシレーターの設定はできたので、`alles.send(osc=0, vel=1)` でオンにするだけです。オシレーターは状態と設定をすべて覚えています。ノートをオフにするには、`alles.send(osc=0, vel=0)` とするだけです。

`amp` や `vel` を 1 より大きくすると、オシレーターの音を大きくできます。

すべてを止めるために `alles.reset()` も忘れずに試してください。

`freq` の代わりに、いつでも `note`（MIDI ノート番号）を使えます。

```python
alles.send(osc=0, wave=alles.SINE, note=57, vel=1)
```

では、たくさんのサイン波を鳴らしてみましょう！

```python
import time
alles.reset()
for i in range(16):
    alles.send(osc=i, wave=alles.SINE, freq=110+(i*80), vel=((16-i)/32.0))
    time.sleep(0.5) # 0.5 秒待つ
```

いいですね！ たくさんのオシレーターを制御できることが、いかにシンプルで強力かがわかると思います。オシレーターは最大 64 個使えます。もっと面白くしましょう。古典的なアナログ・サウンドといえば、フィルターをかけたのこぎり波です。作ってみましょう。

```python
alles.send(osc=0,wave=alles.SAW_DOWN,filter_freq=2500, resonance=5, filter_type=alles.FILTER_LPF)
alles.send(osc=0, vel=1, note=40)
```

いい音です。でも、古典的なフィルタースイープの音にするために、フィルター周波数を時間とともに下げたいところです。ブレークポイントを使いましょう！ ブレークポイントは (時間, 値) の単純なリストで、このペアを最大 8 個、異なるものを制御するためのセットを最大 3 つまで持てます。ADSR と同じようなものですが、もっと強力です。ブレークポイントでは、振幅、周波数、デューティ比、フィードバック、フィルター周波数、レゾナンスを制御できます。ブレークポイントはノートと同時にトリガーされます。では、フィルター周波数を開始時の 2500 から 100 ミリ秒後に 1250 まで下げ、ノートがオフになったら 25 ミリ秒かけて 0 まで下げるブレークポイントを作りましょう。

```python
alles.send(osc=0,wave=alles.SAW_DOWN,filter_freq=2500, resonance=5, filter_type=alles.FILTER_LPF)
alles.send(osc=0, bp0="100,0.5,25,0", bp0_target=alles.TARGET_FILTER_FREQ)
alles.send(osc=0, vel=1, note=40)
```

素晴らしい。複数のターゲットを足し合わせることもできます。例えば、ブレークポイントでフィルター周波数とレゾナンスの両方を制御したい場合は、`bp0_target=alles.TARGET_FILTER_FREQ+alles.TARGET_RESONANCE` とします。試してみてください！

LFO もあります。LFO は、あるオシレーターで別のオシレーターを変調する形で実装されています。低い周波数のオシレーターを設定し、それで聞こえる側のオシレーターのパラメーターを制御します。人気の、古典的な 8 ビットのデューティ比パルス幅変調を作ってみましょう。

```python
alles.send(osc=1, wave=alles.SAW_DOWN, freq=0.5, amp=0.75)
alles.send(osc=0, wave=alles.PULSE, duty=0.5, freq=220, mod_source=1, mod_target=alles.TARGET_DUTY)
alles.send(osc=0, vel=0.5)
```

まず変調用のオシレーター（0.5Hz・振幅 0.75 ののこぎり波。振幅は LFO の「深さ」を表します）を設定しています。次に、変調される側のオシレーターとして、変調元をオシレーター 1、変調先をデューティ比にしたパルス波を設定しています。デューティ比は 0.5 から始まり、ティックごとにオシレーター 1 の状態が掛けられて、C64 などでおなじみの太いのこぎり波のようなラインになります。変調はノートオンのたびに再トリガーされます。ブレークポイントと同じく、デューティ比、振幅、周波数、フィルター周波数、レゾナンス、フィードバックを変調できます！ 周波数とデューティ比のように複数を変調したい場合は、足し合わせるだけです。

```python
alles.send(osc=1, wave=alles.TRIANGLE, freq=5, amp=0.25)
alles.send(osc=0, wave=alles.PULSE, duty=0.5, freq=110, mod_source=1, mod_target=alles.TARGET_DUTY+alles.TARGET_FREQ)
alles.send(osc=0, vel=0.5)
```

ほかにも試せるパラメーターや機能がたくさんあります。全一覧は [Alles の README](https://github.com/shorepine/alles/blob/main/README.md) を見るか、Python で alles.message を確認してください。

```python
# alles.message():
(osc=0, wave=-1, patch=-1, note=-1, vel=-1, amp=-1, freq=-1, duty=-1, feedback=-1, timestamp=None, reset=-1, phase=-1, \
        client=-1, retries=1, volume=-1, filter_freq = -1, resonance = -1, bp0="", bp1="", bp2="", bp0_target=-1, bp1_target=-1, bp2_target=-1, mod_target=-1, \
        debug=-1, mod_source=-1, eq_l = -1, eq_m = -1, eq_h = -1, filter_type= -1, algorithm=-1, ratio = -1, algo_source=None)
```

`alles.py` には便利なプリセットがいくつか入っていて、そのまま使うことも追加することもできます。先ほどのフィルター・ベースを作るには、`alles.preset(1, osc=0)` を実行してから `alles.send(osc=0, vel=1, note=40)` で鳴らすだけです。別の例です。

```python
alles.preset(0, osc=2) # オシレーター 2 に単純なサイン波の音色を設定します
alles.send(osc=2, note=50, vel=1.5) # ベロシティ 1.5 でノートを鳴らします
alles.send(osc=2, vel=0) # 「ノートオフ」を送ります。ノートのリリースが聞こえます
alles.send(osc=2, freq=220.5, vel=1.5) # 同じですが周波数で指定します
alles.reset()
```

### 複数のスピーカー

Alles を複数台持っていますか？ そうだといいのですが。本当の楽しさはそこにあります。物理的なハードウェア・スピーカーでも、コンピューター上で動く Alles デスクトップ・プログラムでも、同じネットワーク上のスピーカーはすべて 1 つの大きなメッシュとして制御できます。スピーカーは自分たちが何台いるかを把握し、同期を保ち、起動して数秒後に `client` と呼ぶ番号が割り当てられます。通常、最初に電源を入れたスピーカーの `client` が 0、次が 1、という具合です。スピーカーの電源を切ると、残りのスピーカーがそのうち欠番を埋めて並び直します。

Alles にメッセージを送るとき、`client` を指定しなければ、すべてのスピーカーが同じメッセージを受け取り、まったく同時に演奏します。でも、スピーカーごとに違う種類の音を同期して鳴らす方が面白いでしょう。16 個のサイン波を鳴らした先ほどの例がちょうど良い題材です。スピーカーごとに違うサイン波を鳴らすように変えてみましょう。

現在メッシュで動いているスピーカーの台数を手早く知るには、`alles.sync()` を実行します。数秒後に、次のような一覧が出力されます。

```json
{0: {'reliability': 1.0, 'avg_rtt': 124.0, 'ipv4': 1, 'battery': ('charged', 4)},
 2: {'reliability': 1.0, 'avg_rtt': 67.3, 'ipv4': 5, 'battery': ('charged', 4)},
 1: {'reliability': 1.0, 'avg_rtt': 293.5, 'ipv4': 3, 'battery': ('charged', 4)}}
```

ここには、各スピーカーのバッテリーの状態やタイミングの詳細が表示されます。作品の中で台数をさっと知りたいときは `len(alles.sync())` を使えます。次のようにします。

```python
import time
alles.reset()
speakers = len(alles.sync())
for i in range(16):
    alles.send(osc=i, wave=alles.SINE, freq=110+(i*80), vel=((16-i)/32.0), client=i % speakers)
    time.sleep(0.5) # 0.5 秒待つ
```

メッセージに `client` パラメーターを追加し、各サイン波をクライアント ID `i`（0〜16）を `speakers`（私の場合は 3）で割った余り（`%`）のスピーカーで鳴らすようにしました。つまり、サイン波 0 はスピーカー 0、サイン波 1 はスピーカー 1、波 2 はスピーカー 2、波 3 は再びスピーカー 0、波 4 はスピーカー 1、という具合にラウンドロビンで鳴ります。試してみてください！

`client` には便利な使い方がほかにもあります。`client` に 255 より大きい数を指定すると、スピーカーのグループを指定できます。たくさんのスピーカーがあって、半分のスピーカーや 3 台に 1 台にだけメッセージを送りたいときに便利です。257 なら 2 台に 1 台、258 なら 3 台に 1 台、という具合です。

### 加算合成

シンセの歴史に詳しい人は、Alles がなぜこの名前なのかもうご存じでしょう。最初は、Hal Alles が発明した 70〜80 年代のベル研究所のシンセ「[Alles Machine](https://en.wikipedia.org/wiki/Bell_Labs_Digital_Synthesizer)」のマルチチャンネル版として作りました。Alles Machine は、私たちのものと同じく、オシレーターとエンベロープとフィルターのバンクとして作られていました。違いは、私たちのオシレーターは空間のどこにでも配置するようにプログラムでき、数もずっと多いことです。

加算合成とは、オシレーターを足し合わせてより複雑な音を作ることです。合成は単にサイン波を特定の振幅と周波数（と位相）で鳴らしているだけなので、オシレーターのブレークポイントを時間とともに変化させて、例えば音程や時間をアーティファクトなしで変えることができます。特定の種類の楽器によく向いています。

![Partials](https://raw.githubusercontent.com/shorepine/alles/main/pics/partials.png)

私たちはいくつかの楽器の倍音（パーシャル）を分析し、プリセットとしてスピーカーに組み込みました。各パッチは、時間とともに変化する複数のサイン波オシレーターで構成されています。プリセットは `PARTIALS` タイプで使えます。

```python
alles.send(osc=0,vel=1,note=50,wave=alles.PARTIALS,patch=5) # きれいなオルガンの音
alles.send(osc=0,vel=1,note=55,wave=alles.PARTIALS,patch=5) # 周波数を変える
alles.send(osc=0,vel=1,note=50,wave=alles.PARTIALS,patch=6,ratio=0.2) # ratio でパーシャルの再生を遅くする
```

各スピーカーには 17 個のプリセットが保存されているので、`patch` は 0〜16 の範囲で指定できます。

私たちのパーシャル・ブレークポイント分析器は「ノイズ励起による帯域幅拡張」も出力します。これは、サイン波の振幅をフィルターをかけたノイズ信号で変調することで、サイン波だけでは生成しにくい音を再現しようとするものです。パッチに `feedback` を加えると試せます。

```python
alles.send(osc=0,vel=1,note=50,wave=alles.PARTIALS,patch=6,feedback=0) # 帯域幅なし
alles.send(osc=0,vel=1,note=50,wave=alles.PARTIALS,patch=6,feedback=0.5) # 帯域幅を増やす
```

後の上級編では、自分のオーディオを分析し、そのパーシャルをホストから複数のスピーカーで再生する方法を紹介します。可能性は無限大です！


### 周波数変調で遊ぶ

パーシャルによる加算合成だけでなく、Alles はサイン波の周波数変調（FM）も得意です。これを `ALGO` と呼んでいます。きっとよく耳にしたことのある合成方式で、遊んでいて楽しいものです。いちばん手軽に試すには、Alles に組み込んだ 201 個のプリセットのどれかを使ってみてください。次のようにします。

```python
alles.send(wave=alles.ALGO,osc=0,patch=0,note=50,vel=1)
alles.send(wave=alles.ALGO,osc=0,patch=1,note=50,vel=1)
```

`patch` でプリセットを選びます。0〜200 を指定できます。もう 1 つ面白いパラメーターが `ratio` で、ALGO タイプのパッチでは、パッチのエンベロープを再生する速さを表します。遅くするとすごく面白いです！

```python
alles.send(wave=alles.ALGO,osc=0,note=40,vel=1,ratio=0.5,patch=8) # 半分の速さ
alles.send(wave=alles.ALGO,osc=0,note=40,vel=1,ratio=0.05,patch=8)  # すごーくゆっくり
alles.send(wave=alles.ALGO,osc=0,note=30,vel=1,ratio=0.1,patch=19) # これが大好き
```

古典的な FM のベルの音を、プリセットを使わずに自分で作ってみましょう。オペレーターを 2 つ（サイン波 2 つ）だけ使い、一方でもう一方を変調します。

```python
alles.reset()
alles.send(wave=alles.SINE,ratio=0.2,amp=0.1,osc=0,bp0_target=alles.TARGET_AMP,bp0="1000,0,0,0")
alles.send(wave=alles.SINE,ratio=1,amp=1,osc=1)
alles.send(wave=alles.ALGO,algorithm=0,algo_source="-1,-1,-1,-1,1,0",osc=2)
```

最後の行を詳しく見てみましょう。ここでは、最大 6 個の他のオシレーターを制御する ALGO「オシレーター」を設定しています。必要なのは 2 つだけなので、`algo_source` のほとんどを -1（未使用）にし、オシレーター 1 でオシレーター 0 を変調するようにしています。オペレーター同士は、あらゆる奇抜な組み合わせで連携させられます。この簡単な例では DX7 のアルゴリズム #1 を使います（ただし 0 から数えるので、アルゴリズム 0 です）。そして、オペレーター 2 と 1 だけを使います。そのため、`algo_source` には使うオシレーターを 6 から逆順に並べています。オペレーター 2 と 1 だけを使い、オシレーター 1 でオシレーター 0 を変調する、という指定です。

`ratio` と `amp` は何をしているのでしょう？ FM 合成のオペレーターでの ratio は、そのオペレーターの周波数と基音の周波数の比を意味します。つまり、オシレーター 0 は基音の 20% の周波数で、オシレーター 1 は基音と同じ周波数で鳴ります。`amp` は FM 合成で「ベータ」と呼ばれるもので、変調の強さを表します。ブレークポイントを使って、ベータを 1,000 ミリ秒かけて下げていることに注目してください。これが「ベルが鳴り響く」効果の鍵です。

これでオシレーターの設定ができました。では聞いてみましょう！

```python
alles.send(osc=2, note=60, vel=3)
```

ベルのような音が聞こえるはずです。いいですね。2 オペレーターのもう 1 つの古典的な音色は、逆に低い音で高い音を変調してフィルタースイープのようにするものです。5 秒かけてやってみましょう。

```python
alles.reset()
alles.send(osc=0,ratio=0.2,amp=0.5,bp0_target=alles.TARGET_AMP,bp0="0,0,5000,1,0,0")
alles.send(osc=1,ratio=1)
alles.send(osc=2,algorithm=0,wave=alles.ALGO,algo_source="-1,-1,-1,-1,0,1")
```

ブレークポイントのおさらいです。ここでは、ベータ・パラメーター（変調する側の音の振幅）を 0.5 に設定しつつ、時刻 0 では 0 から始め、5000ms の時点で 0.5 の 1.0 倍（つまり 0.5）になるようにしています。ノートのリリース時には、ベータを直ちに 0 にします。次のように鳴らします。

```python
alles.send(osc=2,vel=2,note=50)
```

いいですね。面白く変化していく音を作る方法が無限にあることがわかると思います。

### Karplus-Strong

Karplus-Strong（KS）は、弦楽器を合成するための簡単な手法です。スピーカーごとに 1 つずつ鳴らせます。`feedback` パラメーターを適切な値に設定することだけ忘れないでください。

```python
alles.reset()
alles.send(osc=0,wave=alles.KS,note=60,vel=1,feedback=0.996)
```

### PCM サンプル

加算合成や FM 合成では通常表現しにくいため、Alles にはドラム系や楽器の PCM サンプルが 67 個付属しています。タイプ `PCM` とパッチ番号 0〜66 を使って試してみてください。周波数やノートのパラメーターを指定しなければサンプル本来の音程で鳴りますが、変更もできます。

```python
alles.send(osc=0, wave=alles.PCM, vel=1, patch=10) # カウベル
alles.send(osc=0, wave=alles.PCM, vel=1, patch=10, note=70) # 高いカウベル！
```

`feedback` を使うとサンプルのループ再生をオンにでき、楽器に便利です。

```python
alles.send(wave=alles.PCM,vel=1,patch=21,feedback=0) # クリーンなギターの弦、ループなし
alles.send(wave=alles.PCM,vel=1,patch=21,feedback=1) # ノートオフまでずっとループする
alles.send(vel=0) # ノートオフ
alles.send(wave=alles.PCM,vel=1,patch=35,feedback=1) # きれいなバイオリン
```

### 次のステップ

詳細はメインの [Alles README](https://github.com/shorepine/alles/blob/main/README.md) を必ず読んでください！！


## 上級編

冒険心があれば、Alles にはとても面白い技があります。コンピューターの準備が少し必要ですが、それほど複雑ではありません。最近の Mac と Linux で動作を確認しています。準備が済めば、自分でソフトウェアの Alles スピーカーを作れるほか、任意のオーディオから自分でパーシャルを生成し、メッシュ全体で再生できます。とても楽しいですよ！

### セットアップ

こういう作業をしたことがなければ、まずコンピューターに Homebrew をインストールすることをお勧めします。必要な開発ツールを安全かつ手早く揃えられます。ターミナルで `brew` と入力して、すでに入っているか確認してください。入っていなければ、次の行をコピーして貼り付けます。

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

その後、Alles リポジトリのルートにいることを確認して（`cd Downloads/alles-main/` など）、必要なものをインストールします。（経験のある方向けに言うと、必要なのは Python 3、swig、ffmpeg と、Python モジュールの pydub と numpy です。）

```bash
brew install python3 swig ffmpeg
python3.9 -m pip install pydub numpy --user
tar xvf loris-1.8.tar
cd loris-1.8
CPPFLAGS=`python3-config --includes` PYTHON=`which python3.9` ./configure --with-python
make
sudo make install
cd ..
```

### 自分でパーシャル再生シンセサイザーを作る

先ほどのセットアップで、優れたサイン波分解ツールの 1 つである Loris をインストールしました。（ほかにもツールはあります。この分野に興味が出てきたら、Loris を MQ や SMS と比較できる素晴らしい [`simpl`](https://github.com/johnglover/simpl) プロジェクトをお勧めします。）Loris は PCM オーディオを分析して、パーシャル（スペクトログラム上の、時間とともに変化するサイン波と考えてください）の集まりに分解します。各パーシャルは一連のブレークポイントを持ち、それぞれが時間、周波数、振幅、帯域幅、位相を指定します。先ほど遊んだ PARTIALS のプリセットは、楽器のサンプルを Loris で分析したものに基づいています。でも、自分で分析したものを使って Alles を制御することもできます。

```python
import partials
(m,s) = partials.sequence("sleepwalk.mp3")
109 partials and 1029 breakpoints, max oscs used at once was 8

partials.play(s, amp_ratio=2, bw_ratio=0)
```

https://user-images.githubusercontent.com/76612/131150119-6fa69e3c-3244-476b-a209-1bd5760bc979.mp4


このように、どんなオーディオファイルでも、そのサイン波分解版を Alles 全体で聞くことができます。このサウンドからは 109 個のパーシャルが得られ、メッシュで再生するためのブレークポイントは合計 1029 個になりました。109 個のパーシャルのうち、同時に鳴るのは 8 個だけです。`partials.sequence()` はボイス・スティーリングを行い、再生に必要なオシレーターをできるだけ少なくします。

Loris には、試せる（そして試すべき！）パラメーターがたくさんあります。`partials.sequence` と `partials.play` は次の引数を取ります（既定値付き）。

```python
def sequence(filename, # 任意のオーディオファイル名
                max_len_s = 10, # 最初の N 秒を分析する
                amp_floor=-30, # この振幅（dB）以上のパーシャルだけを採用する。値が小さいほどパーシャルが増える
                hop_time=0.04, # 分析窓の間隔。ブレークポイント間の距離に影響する
                max_oscs=alles.OSCS, # 使用する Alles のオシレーター数の上限。複数のスピーカーを使うなら 64 を超えてもよい
                freq_res = 10, # 分析器の周波数分解能。値が大きいほどパーシャルとブレークポイントが減る
                freq_drift=20, # 1 つのパーシャル内での周波数差の上限（Hz）
                analysis_window = 100 # 分析窓のサイズ
                ) # (metadata, sequence) を返す

def play(sequence, # partials.sequence の戻り値
                osc_offset=0, # このオシレーター番号から使い始める
                sustain_ms = -1, # 楽器をサステインさせる場合の位置（ms）
                sustain_len_ms = 0, # サステインする長さ
                time_ratio = 1, # 再生速度。0.5 で半分の速さ
                pitch_ratio = 1, # 周波数の倍率。0.5 で半分の周波数
                amp_ratio = 1, # 振幅の倍率
                bw_ratio = 1, # 帯域幅 / ノイズの倍率
                round_robin=True # パーシャルを 1 つずつスピーカーにラウンドロビンで割り振って再生する
                )
```

このセットアップでいろいろ実験して、素晴らしい音楽を作ってもらえたらうれしいです。


### コンピューター上で Alles を動かす

スピーカーはライブ演奏や森じゅうへの設置に最適ですが、スタジオでは、録音したりさまざまなエフェクトに送ったりできるように、自分の機材につなぎたいこともあるでしょう。そのためには、コンピューター上で直接 Alles スピーカーを（いくつでも！）起動します。コンピューターがスピーカーと同じネットワークにあれば、プログラムはメッシュ内の他のスピーカーとまったく同じように振る舞い、同期を保つはずです。

必要なのは、`alles` というプログラムをコンパイルしてローカルで実行することだけです。スピーカーで起動するのと同じコードを、コンピューター上で動かすだけです。セットアップの手順を済ませていれば、次のようにします。

```bash
cd main
make
./alles
Multicast IF is 192.168.1.85. Client tag (not ID) is 1. Listening on 232.10.11.12:9294
Using device ID 2, device Studio 1824c, channel -1  (all)
```

これで、メッシュに送られたメッセージは、既定のオーディオ出力からも鳴るようになります。

デスクトップ版の Alles には、異なるネットワークで動かしたり、出力先を別のサウンドカードにしたりするための起動オプションがあります。例えば、18 出力の USB オーディオ・インターフェースの出力ごとに 1 台ずつ Alles スピーカーを動かすこともできます！

```bash
./alles -h
usage: alles
    [-i multicast interface ip address, default, autodetect]
    [-d sound device id, use -l to list, default, autodetect]
    [-c sound channel, default -1 for all channels on device]
    [-o offset for client ID, use for multiple copies of this program on the same host, default is 0]
    [-l list all sound devices and exit]
    [-h show this help and exit]
```

1 台のマシンで複数の Alles を動かしたい場合は、次のように IP アドレスと「クライアント・オフセット」を指定します。

```bash
./alles -i 192.168.1.85 -o 100 -c 0
# ターミナル.app の新しいタブで
./alles -i 192.168.1.85 -o 101 -c 1
# ……以下同様
```

クライアント・オフセットは、ネットワーク上の他のデバイスと衝突しない番号にしてください。この例では、想定されるクライアント・オフセットは 85（IP アドレスの最後の数字）ですが、1 台目のスピーカーには 100、2 台目には 101 を加えています。こうすれば、ネットワーク上の別のデバイスに番号を取られることはありません。実際にはプライベートなネットワーク（-i パラメーターで送信元 IP を指定します）を使うので、あまり気にする必要はありませんが、自宅で作曲するときには覚えておいてください。



