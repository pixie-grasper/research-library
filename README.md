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
現在までに

- Burrows Wheeler Transform
- Move To Front
- Range Coder
- Fast Fourier Transform
- Discrete Cosine Transform

が実装されています。

## TODO
以下のアルゴリズムを実装する予定です。

- Wavelet Transform family
- Prediction by Partial Matching

## Authors
- pixie-grasper

## License
The MIT License
