# HPYLM

C++によるHierarchical Pitman-Yor Language Modelの実装と文字n-gramの学習例です。

以下の論文をもとに実装を行っています。

- [A Hierarchical Bayesian Language Model based on Pitman-Yor Processes](http://www.gatsby.ucl.ac.uk/~ywteh/research/compling/acl2006.pdf)
- [A Bayesian Interpretation of Interpolated Kneser-Ney](http://www.gatsby.ucl.ac.uk/~ywteh/research/compling/hpylm.pdf) 

[この記事](http://musyoku.github.io/2016/07/26/A_Hierarchical_Bayesian_Language_Model_based_on_Pitman-Yor_Processes/)で実装したプログラムになります。

### 動作環境

- C++11
- Boost 1.6

### 実行

```
g++ main.cpp -O2 -std=c++11 -I/usr/local/include -L/usr/local/lib -lboost_serialization
```

boostのディレクトリとlibboost_serializationがあるディレクトリを指定します。

OS X El CapitanとUbuntu 14で動作を確認しています。

実行前に学習用テキストデータを`train.txt`に保存し、適当なディレクトリに入れておきます。

まだモデル保存用のディレクトリもあらかじめ作成しておいてください。

実行は

```
./a.out --text_dir alice --model_dir model
```

のように行います。
