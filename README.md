# Research Library
## 何これ？
これは、色々なアルゴリズムを片っ端から実装するプロジェクトです。
実装言語はC++14を用い、clangでコンパイルでき、追加のライブラリを必要とせず、MITライセンスで提供しています。

## どう使うの？
```bash
$ git clone https://github.com/pixie-grasper/research-library.git
$ cd research-library
$ make check
$ sudo make install
```

した後に

```bash
$ clang++ `pkg-config --cflags libresearch` foo.cc
```

する事によって各種アルゴリズムを利用できるようになります。
例えば、1.datから系列を読み込んでPPM法で符号化するには、次のように書きます。

```c_cpp
#include <stdio.h>
#include <prediction-by-partial-matching.h>
#include <file-vector-converter.h>

int main() {
  auto buffer = ResearchLibrary::File::load("1.dat");
  auto ppm = ResearchLibrary::PredictionByPartialMatching::NumericEncode<MethodC, 4>(buffer);
  printf("%zu -> %zu\n", buffer.size(), ppm.first.size());
  return 0;
}
```

## 何ができるの？
- Adler-32 Check Sum Function
- Burrows Wheeler Transform (using Larsson-Sadakane's Algorithm)
- CRC-32 Check Sum Function
- Deflate/Inflate Function
- Discrete Cosine Transform
- Elias Coding
- Fast Fourier Transform
- Golomb Coding
- Huffman Coding
- Lempel-Ziv-Storer-Szymanski Algorithm
- Move To Front
- Prediction by Partial Matching
- Range Coder (Static, Adaptive)
- Unary Coding
- Wavelet Transform (Haar, CDF 5/3, CDF 9/7)
- Ziv-Lempel 77 Algorithm

が既に実装されています。又、

- size\_type\_t型: テンプレート引数としてコンパイル時に決定可能な整数の1, 2, 4, 8を許し、その大きさの符号なし整数型を表す
- BitsToBytes class: ビットストリームをバイトストリームに変換する為のクラス
- BytesToBits class: バイトストリームからビットストリームを切り出す為のクラス
- File class: ファイルとstd::vector\<std::uint8_t\>を相互に変換する為のクラス
- RasterImage class: ラスタ画像の読み書きを行う為のクラス

が利用できます。詳しい使い方に関しては

```bash
$ make docs
```

を行うことでdoxygenを利用して生成されるドキュメントを参照してください。

## TODO
- グラフィックス関連機能の実装
- 最小二乗法の実装

## NOT TODO
以下のアルゴリズムは宗教的な理由により実装*しない*予定です。

- Arithmetic Coding
- Lempel-Ziv-Welch Algorithm
- Karmarker's Method

## Authors
- pixie-grasper

## Special Thanks
- bolero-MURAKAMI

## License
The MIT License
