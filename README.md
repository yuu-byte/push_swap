*This activity has been created as part of the 42 curriculum by aiwane, yuito.*

# push_swap

## Description

push_swap is a program that sorts a stack of integers — but instead of
printing the sorted numbers, it prints the **list of operations** needed to
sort them. Running `./push_swap 2 1 3` prints `sa`, meaning "one swap sorts
this input".

Two stacks are used: `a` holds the numbers, `b` starts empty. Only 11
operations are allowed:

| Operation | What it does |
| --- | --- |
| `sa` / `sb` / `ss` | swap the top 2 of a / b / both |
| `pa` / `pb` | move the top of b to a / of a to b |
| `ra` / `rb` / `rr` | rotate a / b / both up (top goes to bottom) |
| `rra` / `rrb` / `rrr` | rotate a / b / both down (bottom goes to top) |

## Instructions

```sh
make          # builds ./push_swap
make bonus    # builds ./checker
make clean / fclean / re
```

```sh
./push_swap 3 1 2                          # no flag = adaptive
./push_swap --simple 5 4 3 2 1             # force one strategy
./push_swap --bench --medium 5 4 3 2 1     # metrics go to stderr
./push_swap 3 2 1 0 | ./checker 3 2 1 0    # verify with the checker -> OK
```

Errors (non-integer, out of int range, duplicate, empty string, unknown
flag) print `Error` on stderr and exit 1. No arguments: prints nothing.
Sorted input: prints nothing (0 operations).

## File structure

```
push_swap/
├── Makefile                     # make / bonus / clean / fclean / re, no relink
├── include/push_swap.h          # structs, operation codes, prototypes
├── src/
│   ├── core/main.c              # main flow, strategy selection
│   ├── core/init.c              # init / malloc / free / Error handling
│   ├── core/parse.c             # split arguments into flags and numbers
│   ├── core/parse_tokens.c      # read numbers, int-range check, duplicates
│   ├── stack/stack.c            # swap / rotate / reverse rotate / is_sorted
│   ├── stack/ops.c              # push + ps_op (apply a move AND record it)
│   ├── stack/optimize.c         # delete useless pairs, merge ra+rb into rr
│   ├── sort/ranks.c             # disorder score, convert values to ranks
│   ├── sort/sort_simple.c       # selection sort O(n^2) + small-input sort
│   ├── sort/sort_medium.c       # chunk sort O(n*sqrt(n))
│   ├── sort/sort_medium_utils.c # helpers (find max, rotate b)
│   ├── sort/sort_complex.c      # binary radix sort O(n log n)
│   ├── utils/output.c           # print the recorded move list
│   ├── utils/bench.c            # --bench report (stderr)
│   └── utils/utils.c            # ft_strcmp / isdigit / putnbr etc.
└── bonus/checker_bonus.c        # checker: run moves from stdin, print OK/KO
```

## Program flow

1. Initialise.
2. Read the arguments (validate numbers, flags, duplicates). Stop here if
   there are no numbers.
3. Measure the disorder (before any move — required by the subject).
4. Replace each value by its rank (0, 1, 2, ...).
5. Run the strategy (move the internal stacks while recording each move).
6. Clean up the recorded list, then print it.
7. If `--bench` is set, print the metrics on stderr.
8. Free the memory and exit.

## Disorder and rank conversion

**Disorder** = number of pairs in the wrong order / total number of pairs.
0 = already sorted, 1 = fully reversed. A random shuffle is around 0.5.

**Rank conversion**: replace each value by "how many values are smaller".

```
original: [50, 10, 30, 20, 40]
ranks:    [ 4,  0,  2,  1,  3]
```

The order relation does not change, so sorting the ranks is the same as
sorting the original values. Ranks always fit in 0..n-1, which makes the
radix digit count exact and the chunk ranges easy to split.

## Algorithms and justification

### First, what "complexity" means here

It is a measure of **how fast the move count grows** as the number of
integers n grows. In this project we count printed operations, not CPU time.

| Class | When n becomes 10x, moves become | Rough figure at n=500 |
| --- | --- | --- |
| O(n^2) | about 100x | about 250,000 moves |
| O(n*sqrt(n)) | about 30x | about 11,000 moves |
| O(n log n) | about ten-something x | about 4,500 moves |

### simple (selection sort, O(n^2))

**How it works**
1. Find the smallest number in `a`.
2. Rotate it to the top (`ra`, or the shorter direction for small inputs).
3. `pb` it to `b`.
4. Repeat until 3 numbers remain, sort those directly, then `pa` all back.

**Why it sorts**: `b` receives the numbers smallest first, so inside `b` the
larger numbers sit on top. Bringing them back with `pa` puts the larger ones
on `a` first, and the smallest ends up on top — ascending order.

**Move count**: at most about n*n/2, which grows quickly. But on almost
sorted input the smallest number is already near the top, so the rotation in
step 2 costs almost nothing and the whole run finishes in about 2n moves.

**Why we chose it**: the subject lists insertion, selection, bubble and
simple min/max extraction. We chose selection sort because (1) it fits two
stacks naturally ("extract the minimum into b" is exactly `pb`), (2) bubble
sort needs a rotation for every adjacent swap, which inflates the move
count, (3) insertion sort's "insert at the right position" is awkward on a
stack, and (4) on nearly sorted input it drops to about 2n measured moves,
which makes it the right fit for the low-disorder regime.

- Time: O(n^2) — Space: O(n) — Used for: disorder < 0.2

### medium (chunk sort, O(n*sqrt(n)))

**How it works**
1. Define a window: a number may be pushed if its rank is below
   "already pushed + about 1.75*sqrt(remaining)".
2. Among the allowed numbers, push the one closest to the top (searching
   from both ends) with `pb`. Repeat until `a` is empty.
3. Find the largest number in `b`, rotate it to the top the shorter way,
   `pa`. Repeat until `b` is empty.

**Why it is faster**: simple's weakness is having to rotate a long way to
fetch one specific number. medium always has several acceptable candidates,
so one of them is nearly always close to the top. The pull side is cheap
too: larger values were pushed later, so the current maximum is always near
the top of `b`. Each side costs about sqrt(n) moves per element, so about
n*sqrt(n) in total.

**Why we chose it**: the subject lists chunk splitting, block partitioning,
buckets and range-based strategies. They all "move values in groups"; we
chose the chunk form because (1) values are already converted to ranks, so
splitting by rank range is the most natural, (2) it needs no extra container
— stack `b` alone is enough, and (3) making the window slide reduced the
measured move count further.

- Time: O(n*sqrt(n)) — Space: O(n) — Used for: disorder 0.2 to 0.5

### complex (LSD binary radix sort, O(n log n))

**How it works** (ranks are 0..n-1, so they fit in log2(n) binary digits —
9 digits for 500 numbers)
1. Go through `a` once. If the current digit of the top number is 0, `pb`
   it; if it is 1, `ra` it.
2. After the pass, `pa` everything back from `b`.
3. Move to the next digit and repeat. Stop early if `a` is already sorted.

**Why it sorts**: each pass keeps the relative order inside a group (the
0-group comes back in the order it left). So after pass 1 the stack is
ordered by the last digit, after pass 2 by the last two digits, and after
the final pass by all digits — fully sorted.

**Move count**: one pass is at most 2n moves and there are log2(n) passes,
so the total **never exceeds 2n*log2(n) for any input**. For 500 numbers
that is an upper bound of about 9,000; measured is about 6,700.

**Why we chose it**: the subject lists radix, merge, quicksort, heapsort and
Fenwick trees. Radix won because we can state with certainty that no input
exceeds 2n*log2(n). Quicksort can degrade to O(n^2) with bad pivots, which
breaks the claim. Merge and heap sort are complex to implement on two stacks
and the move count is hard to control. A Fenwick tree does not fit the
operation model. The subject requires the claimed complexity class to
actually hold, and radix is the one where that is easiest to show.

- Time: O(n log n) — Space: O(n) — Used for: disorder 0.5 and above

### adaptive (default)

It only measures the disorder and dispatches.

| Disorder | Method | Reason |
| --- | --- | --- |
| below 0.2 | simple | fastest in practice on nearly sorted input |
| 0.2 to 0.5 | medium | fewest moves on in-between input |
| 0.5 and up | complex | the move bound can be guaranteed |

The 0.2 / 0.5 cut-offs are the values fixed by the subject.

### Thresholds, per-regime techniques, and upper bounds

The three things the subject asks us to document — threshold rationale,
internal technique per regime, and time/space upper bounds in the Push_swap
model — in one table.

| Disorder regime | Technique | Threshold rationale | Time upper bound (moves) | Space |
| --- | --- | --- | --- | --- |
| d < 0.2 | selection sort | on nearly sorted input the minimum sits near the top and almost no rotation is needed; it drops to about 2n measured, so we deliberately use the plain O(n^2) method | n^2/2 + 2n + 3 <= O(n^2) | O(n) |
| 0.2 <= d < 0.5 | windowed chunk sort | partial order remains, so the "move values in groups" approach has the smallest constant here | push n*sqrt(n) + pull n*sqrt(n) <= O(n*sqrt(n)) | O(n) |
| d >= 0.5 | LSD binary radix | a random permutation has expected disorder exactly 0.5; beyond it we want a method whose bound does not depend on the input | 2n * ceil(log2 n) <= O(n log n) (holds for every input) | O(n) |

The 0.2 / 0.5 thresholds themselves are fixed by the subject; the table
shows that those values line up with where each method is strongest. Space
in every regime is the two stack arrays (n each) + one temporary rank array
(n) + the recorded move list (proportional to the move count) = O(n). No
global variables are used.

### Algorithms

**Q1. How does --simple work? (O(n^2))**
Find the minimum in `a`, rotate it up to the top with `ra`, `pb` it to `b`.
Repeat, then `pa` everything back: the numbers return smallest last, so `a`
ends up ascending. Each of the n elements costs up to about n moves, so
n*n — that is O(n^2).

**Q2. How does --medium work? (O(n*sqrt(n)))**
Define a window ("rank below pushed + about 1.75*sqrt(remaining) may go"),
and keep pushing whichever allowed number is closest to the top. Once `a` is
empty, pull back from `b` starting with the largest. There are always about
sqrt(n) acceptable candidates, so one element costs about sqrt(n) moves and
n elements cost n*sqrt(n) — that is O(n*sqrt(n)).

**Q3. How does --complex work? (O(n log n))**
Read the ranks in binary and, from the lowest digit up, sort one digit at a
time ("0 goes to `b`, 1 stays in `a`", then bring everything back). The
split preserves order, so after the last digit the stack is ascending. There
are log2(n) digits and each pass is at most 2n moves, so 2n*log2(n) — that
is O(n log n), and this bound holds for every input.

**Q4. How does --adaptive choose?**
Before any move it measures the disorder (share of inverted pairs, 0 to 1)
and uses simple below 0.2, medium from 0.2 to 0.5, and complex at 0.5 and
above. The thresholds come from the subject, and they match where each
method is strongest: nearly sorted = simple is fastest in practice,
in-between = medium has the fewest moves, random (about 0.5) or worse =
complex, whose bound is guaranteed.

## Contributions

| Login | Work |
| --- | --- |
| aiwane | simple・medium・adaptive、Makefile |
| yuito | complex, checker, tests, README |

Both members reviewed every file together and can explain any part of the
code.

## Resources

* https://en.wikipedia.org/wiki/Sorting_algorithm
* https://en.wikipedia.org/wiki/Selection_sort
* https://en.wikipedia.org/wiki/Radix_sort

## AI usage

AI (Anthropic Claude) was used for discussing and understanding the choice
of algorithms, writing test scripts, and proofreading this README. Every
generated piece of code or explanation was reviewed, tested and understood
by both members before it went into the project.

---
---

# push_swap(日本語)

## 説明

push_swap は、数字を並べ替えるプログラムです。ただし並べ替えた結果ではなく、
**並べ替えるための操作のリスト**を出力します。
`./push_swap 2 1 3` と実行すると `sa` と出ます。「sa を1回やれば並ぶ」という
意味です。

スタックは2本。`a` に数字が入っていて、`b` は空。使える操作は次の11個だけです。

| 操作 | 内容 |
| --- | --- |
| `sa` / `sb` / `ss` | a / b / 両方 の上2つを入れ替える |
| `pa` / `pb` | b の一番上を a へ / a の一番上を b へ移す |
| `ra` / `rb` / `rr` | a / b / 両方 を上に回す(一番上が一番下へ) |
| `rra` / `rrb` / `rrr` | a / b / 両方 を下に回す(一番下が一番上へ) |

## 使い方

```sh
make          # ./push_swap を作る
make bonus    # ./checker を作る
make clean / fclean / re
```

```sh
./push_swap 3 1 2                          # フラグなし = adaptive
./push_swap --simple 5 4 3 2 1             # 方法を指定
./push_swap --bench --medium 5 4 3 2 1     # 計測情報は stderr に出る
./push_swap 3 2 1 0 | ./checker 3 2 1 0    # checker で検証 → OK
```

エラー(整数でない・intの範囲外・重複・空文字列・知らないフラグ)は
stderr に `Error` を出して終了コード1。引数なしは何も出さない。
整列済みの入力も何も出さない(0手)。

## ファイル構成

```
push_swap/
├── Makefile                     # make / bonus / clean / fclean / re
├── include/push_swap.h          # 構造体・操作の番号・関数の宣言
├── src/
│   ├── core/main.c              # 全体の流れ。戦略の選択
│   ├── core/init.c              # 初期化・malloc/free・Error処理
│   ├── core/parse.c             # 引数をフラグと数字に振り分ける
│   ├── core/parse_tokens.c      # 数字の読み取り・int範囲チェック・重複検出
│   ├── stack/stack.c            # swap / rotate / 逆rotate / 整列チェック
│   ├── stack/ops.c              # push と ps_op(操作を実行して記録する)
│   ├── stack/optimize.c         # 無駄なペアを削除、ra+rb を rr にまとめる
│   ├── sort/ranks.c             # 乱れ度の計算、値を順位に変換
│   ├── sort/sort_simple.c       # 選択ソート O(n²) + 5個以下用の処理
│   ├── sort/sort_medium.c       # チャンクソート O(n√n)
│   ├── sort/sort_medium_utils.c # medium の補助(最大値探し、bの回転)
│   ├── sort/sort_complex.c      # 二進基数ソート O(n log n)
│   ├── utils/output.c           # 記録した操作の出力
│   ├── utils/bench.c            # --bench の表示(stderr)
│   └── utils/utils.c            # ft_strcmp / isdigit / putnbr など自作関数
└── bonus/checker_bonus.c        # checker:stdinの操作を実行して OK/KO を表示
```

## プログラムの流れ

1. 初期化
2. 引数を読む(数字の検証・フラグ・重複チェック)。数字が0個ならここで終了
3. 乱れ度を測る(必ず動かす前。課題のルール)
4. 各値を順位(0, 1, 2, …)に置き換える
5. 戦略を実行(内部のスタックを動かしながら操作を記録)
6. 記録を掃除してから出力
7. --bench があれば計測値を stderr に出す
8. メモリを解放して終了

## 乱れ度と順位変換

**乱れ度** = 順番が逆になっているペアの数 ÷ 全ペアの数。
0 = 整列済み、1 = 完全な逆順。ランダムに混ぜるとだいたい 0.5 になる。

**順位変換**:各値を「小さい方から何番目か」に置き換える。

```
元の値:  [50, 10, 30, 20, 40]
順位:    [ 4,  0,  2,  1,  3]
```

大小関係は変わらないので、順位を並べる手順は元の値を並べる手順と同じ。
順位なら必ず 0〜n-1 に収まるので、基数ソートの桁数が正確に決まり、
チャンクの範囲分けも簡単になる。

## アルゴリズムの説明と選定理由

### まず「計算量」とは

数字の個数 n が増えたとき、**手数がどんな勢いで増えるか**を表す物差しです。
この課題では実行時間ではなく、出力する操作の本数で数えます。

| 分類 | n が10倍になると手数は | n=500 のときの目安 |
| --- | --- | --- |
| O(n²) | 約100倍 | 約25万手 |
| O(n√n) | 約30倍 | 約1.1万手 |
| O(n log n) | 十数倍 | 約4500手 |

### simple(選択ソート・O(n²))

**動き**
1. a の中で一番小さい数を探す
2. ra で上方向に回して一番上に持ってくる(5個以下のときだけ近い方向)
3. pb で b に移す
4. a が3個になるまで繰り返す。残り3個は直接並べて、pa で全部戻す

**なぜ整うか**:b には小さい順に入っていくので、b の中では大きい数ほど上に
ある。pa で戻すと大きい数から順に a に乗り、最後に一番小さい数が一番上に
来て昇順になる。

**手数**:最大でも約 n×n÷2。数が増えると一気に増える。ただし、入力がほぼ
整列済みだと最小値が最初から上の方にいるので、回す手数がほぼなくなり、
実際は約 2n 手で終わる。

**選んだ理由**:課題の候補は挿入・選択・バブル・最小最大抽出。この中で
選択ソートを選んだのは、(1) スタック2本と相性がよい(「最小を b へ抜く」が
pb そのもの)、(2) バブルはスタックだと隣接交換のたびに回転が要り手数が
膨らむ、(3) 挿入は「正しい位置に差し込む」がスタックでは書きにくい、
(4) ほぼ整列済みの入力なら最小値が上にいて実測 2n 手まで落ちるから、
低乱れ度の担当にちょうどよい。

- 時間:O(n²) 空間:O(n) 使う場面:乱れ度 < 0.2

### medium(チャンクソート・O(n√n))

**動き**
1. 「送ってよい範囲(窓)」を決める。順位が「すでに送った個数 + 約1.75×√(残り)」
   より小さければ送ってよい
2. 送ってよい数のうち、一番上に近いもの(上からも下からも探す)を pb で送る。
   a が空になるまで繰り返す
3. b の中の最大値を近い方向で一番上まで回して pa。b が空になるまで繰り返す

**なぜ速いか**:simple の弱点は「特定の1個を取りに行くために大きく回す」こと。
medium は送ってよい数が常に複数あるので、一番上のすぐ近くにほぼ必ず候補が
いる。戻す方も、大きい値ほど後から積まれるので、その時点の最大値はいつも
b の上の方にいる。どちらも1個あたり √n 手くらいで済み、合計約 n×√n 手。

**選んだ理由**:課題の候補はチャンク分割・ブロック分割・バケット・範囲
ベース。どれも「まとまりで運ぶ」点は同じで、その中でチャンク方式を選んだ
のは、(1) 順位に変換済みなので「順位の範囲」で区切るのが一番自然、
(2) バケットのような追加の入れ物が要らず b 1本で済む、(3) 窓を
スライドさせる形にすると実測の手数がさらに減らせたから。

- 時間:O(n√n) 空間:O(n) 使う場面:乱れ度 0.2〜0.5

### complex(二進基数ソート・O(n log n))

**動き**(順位は 0〜n-1 なので、2進数で log₂n 桁に収まる。500個なら9桁)
1. a を一周する。一番上の数の「いま見ている桁」が 0 なら pb、1 なら ra
2. 一周したら b を pa で全部戻す
3. 次の桁に進んで繰り返す。途中で整列していたら終了

**なぜ整うか**:この仕分けは、グループの中の順番を崩さない(0のグループは
出て行った順に戻る)。だから1回目で一番下の桁の順に並び、2回目で下2桁の順、
最後の桁まで終えると全部の桁の順=完全な昇順になる。

**手数**:1回の処理は最大 2n 手、回数は log₂n 回。だから合計は
**どんな入力でも 2n×log₂n 手を超えない**。500個なら約9000手が上限で、
実測は約6700手。

**選んだ理由**:課題の候補は基数・マージ・クイック・ヒープ・Fenwick木。
基数を選んだ決め手は「どんな入力でも 2n×log₂n を超えない」と確実に言える
こと。クイックは選び方が悪いと最悪 O(n²) に落ちるので主張が崩れる。
マージやヒープはスタック2本の上で実装が複雑になり、手数の管理が難しい。
Fenwick木はそもそも操作モデルと相性が悪い。課題は「主張した計算量の分類が
本当に正しいこと」を要求しており、それを一番簡単に示せるのが基数だった。

- 時間:O(n log n) 空間:O(n) 使う場面:乱れ度 0.5 以上

### adaptive(自動選択・デフォルト)

乱れ度を測って振り分ける。

| 乱れ度 | 使う方法 | 理由 |
| --- | --- | --- |
| 0.2 未満 | simple | ほぼ整列済みなら実測で一番速い |
| 0.2〜0.5 | medium | 中間の入力で手数が一番少ない |
| 0.5 以上 | complex | 手数の上限を保証できる |

0.2 と 0.5 という境界は課題で指定された値。

### しきい値の根拠・各領域の技術・上界のまとめ

課題が README に書くよう求めている3点(しきい値の根拠 / 各領域で使う内部
技術 / Push_swapモデルでの時間・空間の上界)を1つの表にまとめる。

| 乱れ度の領域 | 使う技術 | しきい値の根拠 | 時間の上界(手数) | 空間の上界 |
| --- | --- | --- | --- | --- |
| d < 0.2 | 選択ソート | ほぼ整列済みなら最小値が頂上付近にあり、回転がほぼ不要。実測 約2n 手まで落ちるので、素朴な O(n²) 法をあえて使う | n²/2 + 2n + 3 ≦ O(n²) | O(n) |
| 0.2 ≦ d < 0.5 | 窓つきチャンクソート | 部分的な秩序が残る領域では、まとまりで運ぶ方式の係数が最小になる | 送り n√n + 戻し n√n ≦ O(n√n) | O(n) |
| d ≧ 0.5 | LSD二進基数ソート | ランダム順列の乱れ度の期待値がちょうど0.5。それ以上に乱れた入力には、入力によらない保証つきの方法を使う | 2n × ⌈log₂n⌉ ≦ O(n log n)(全入力で成立) | O(n) |

しきい値 0.2 / 0.5 自体は課題で固定された値であり、上の表はその値が
各手法の得意分野と一致していることを示している。空間はどの領域でも
「a・b の配列(各n)+ 順位計算の一時配列(n)+ 操作の記録(手数に比例)」
で O(n)。グローバル変数は使わない。

### アルゴリズムと計算量

**Q1. --simple はどう動く?(O(n²))**
a の最小値を探し、ra で上方向に回して一番上へ、pb で b へ。これを繰り返して
最後に pa で全部戻すと、小さい順に戻ってくるので a が昇順になります。
n 個それぞれに最大 n 手ほどかかるので n×n、つまり O(n²) です。

**Q2. --medium はどう動く?(O(n√n))**
「順位が(送った数+約1.75√残り)未満なら送ってよい」という窓を決め、窓に
入る数のうち一番上に近いものを b へ送り続けます。全部送ったら b の最大値
から順に戻します。送ってよい候補が常に√n個ほどあるので1個あたり√n手、
n 個で n×√n、つまり O(n√n) です。

**Q3. --complex はどう動く?(O(n log n))**
順位を2進数で見て、下の桁から1桁ずつ「0なら b へ、1なら a に残す」で
仕分けて全部戻す、を桁の数だけ繰り返します。仕分けは順番を崩さないので、
最後の桁まで終えると昇順になります。桁数は log₂n、1回あたり最大 2n 手
なので 2n×log₂n、つまり O(n log n)。この上限はどんな入力でも成り立ちます。

**Q4. --adaptive はどう選ぶ?**
動かす前に乱れ度(逆順ペアの割合、0〜1)を測り、0.2未満なら simple、
0.2〜0.5なら medium、0.5以上なら complex を使います。境界値は課題で指定
されたもので、ほぼ整列済み=simpleが実測最速、中間=mediumの手数が最少、
ランダム(約0.5)以上=上限を保証できる complex、という各方法の得意分野と
一致しています。

## 担当

| ログイン | 担当 |
| --- | --- |
| aiwane | simple・medium・adaptive、Makefile|
| yuito | complex、checker、テスト、README |

## 参考資料

* Wikipedia: Sorting algorithm — https://en.wikipedia.org/wiki/Sorting_algorithm
* Wikipedia: Selection sort — https://en.wikipedia.org/wiki/Selection_sort
* Wikipedia: Radix sort — https://en.wikipedia.org/wiki/Radix_sort

## AI利用

AIは、アルゴリズム選定の議論・理解、テストスクリプト作成、README校正に使用。
生成されたコードや説明はすべて、両メンバーがプロジェクトへの組み込み前に
レビュー・テスト・理解を行っています。
