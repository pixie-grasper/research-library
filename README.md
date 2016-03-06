# Research Library
## 何これ？
これは、色々なアルゴリズムを片っ端から実装するプロジェクトです。
実装言語はC++14を用い、clangでコンパイルでき、追加のライブラリを必要とせず、MITライセンスで提供しています。

## どう使うの？
```
$ git clone https://github.com/pixie-grasper/research-library.git
$ cd research-library
$ make check
$ sudo make install
```

した後に

```
$ clang++ `pkg-config --cflags libresearch` foo.cc
```

する事によって各種アルゴリズムを利用できるようになります。

## 何ができるの？
- Burrows Wheeler Transform (using Larsson-Sadakane's Algorithm)
- Move To Front
- Range Coder (Static, Adaptive)
- Fast Fourier Transform
- Discrete Cosine Transform
- Wavelet Transform (Haar, CDF 5/3, CDF 9/7)
- Prediction by Partial Matching
- Unary Coding
- Elias Coding
- Golomb Coding

が既に実装されています。又、

- size\_type\_t型: テンプレート引数としてコンパイル時に決定可能な整数の1, 2, 4, 8を許し、その大きさの符号なし整数型を表す
- BitsToBytes class: ビットストリームをバイトストリームに変換する為のクラス
- BytesToBits class: バイトストリームからビットストリームを切り出す為のクラス

が利用できます。詳しい使い方に関しては

```
$ make docs
```

を行うことでdoxygenを利用して生成されるドキュメントを参照してください。

## TODO
以下のアルゴリズムを実装する予定です。

- Huffman Coding

## NOT TODO
以下のアルゴリズムは宗教的な理由により実装*しない*予定です。

- Arithmetic Coding
- Lempel-Ziv-Welch Algorithm
- Karmarker's Method

## Authors
- pixie-grasper

## License
The MIT License
