# K-means

## ***K-means clustering***

ISO C++ 17 以降

> **WARNING**: Be careful!\
> 現状，クラスタの指定をカラーにしているためクラスタ数3から増やすとエラー

### Default DATA

`data`フォルダに以下のデータがある．

    data_subA_20211001.csv\
    data_subA_20211002.csv\
    data_subA_20211003.csv\
    data_subA_20211004.csv\
    data_subA_20211005.csv

    data_subB_20211001.csv\
    data_subB_20211002.csv\
    data_subB_20211003.csv\
    data_subB_20211004.csv\
    data_subB_20211005.csv

    data_subC_20211001.csv\
    data_subC_20211002.csv\
    data_subC_20211003.csv\
    data_subC_20211004.csv\
    data_subC_20211005.csv

`Kmeans_main.cpp`の以下のXXXX部分に，使いたいデータに含まれる文字列を入れて指定．(例："1001", "sub")

```cpp
28  vector<string> useData = {"XXXX","XXXX", ... }; 
```
