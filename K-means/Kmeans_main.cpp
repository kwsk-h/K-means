#include <stdio.h>
#include <vector>
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <random>

#define GNUPLOT_PATH "D:/gnuplot/bin/gnuplot.exe -persist" //gnuplot.exe があるパス
#define  K 3 //クラスタ数
using namespace std;

struct _subData
{
    string cluster = "black";
    double x;
    double y;
};

void fileset(string filename, vector<_subData>& datas);                             //ファイル読み込み
void init_cog(vector<_subData> datas, vector<_subData>& cog_axis);      //重心の初期値
void clustering(vector<_subData>& datas, vector<_subData> cog_axis);    //重心からデータのクラスタリング
void update_cog(vector<_subData> datas, vector<_subData>& cog_axis); //各クラスタのデータから重心を更新

int main() {
    vector<string> dataFiles;
    vector<string> useData = { "1001"};  //使用するデータ(この文字列の含まれるデータを使用)
    vector<_subData> datas;
    vector<_subData> cog_axis(K);
    vector<_subData> bef_cog(K);
    string path = "./data/";

    for (const auto& file : filesystem::directory_iterator(path)) {        
        dataFiles.push_back(file.path().string());
    }
    for (int i = 0; i < dataFiles.size(); i++) {
        for (const auto& key : useData) {
            if (dataFiles[i].find(key) != string::npos) {
                cout << dataFiles[i] << endl;
                fileset(dataFiles[i], datas);//1フォルダに全データを格納
                break;
            }
        }
    }

    //結果保存ディレクトリ
    filesystem::create_directory("result") ?
        cout << "created directory - result" << endl :
        cout << "create_directory() failed" << endl << endl;

    FILE* gp;
    // gnuplot の起動コマンド
    if ((gp = _popen(GNUPLOT_PATH, "w")) == NULL) { // gnuplot をパイプで起動
        fprintf(stderr, "ファイルが見つかりません %s.", GNUPLOT_PATH);
        exit(EXIT_FAILURE);
    }
    /***************************************
    gnuplot へコマンドを送る．動かしたいコマンドを，
    fprintf(gp, "コマンド\n");
    のように記述する．
    ***************************************/
    fprintf(gp, "set terminal gif animate optimize delay 50\n");//gnuplotの結果保存設定// size 480,360
    fprintf(gp, "set output 'result/result.gif'\n");//保存先
    fprintf(gp, "set palette model RGB rgbformulae 35,13,10\n");
    fprintf(gp, "set nokey\n");// 凡例を表示しない
    fprintf(gp, "set nocolorbox\n");// カラーバー表示しない
    fprintf(gp, "set xrange [-5:10]\n");
    fprintf(gp, "set yrange [-5:10]\n");

    init_cog(datas, cog_axis);//初期値
    for (int k = 0; k < 20; k++) {
        clustering(datas, cog_axis);//クラスタリング        
        fprintf(gp, "set multiplot\n"); // マルチプロットモード
        //全データプロット
        for (int i = 0; i < datas.size(); i++) {
            fprintf(gp, "plot '-' lc rgb \"%s\"\n", datas[i].cluster.c_str());
            fprintf(gp, "%f\t%f\n", datas[i].x, datas[i].y);
            fprintf(gp, "e\n");
        }
        //重心位置プロット
        for (int c = 0; c < K; c++) {
            fprintf(gp, "plot '-' pt 7 ps 2 lc rgb \"%s\"\n", "black");//見やすさのための黒枠用
            fprintf(gp, "%f\t%f\n", cog_axis[c].x, cog_axis[c].y);
            fprintf(gp, "e\n");
            fprintf(gp, "plot '-' pt 7 ps 1 lc rgb \"%s\"\n", cog_axis[c].cluster.c_str());//重心位置(カラー)
            fprintf(gp, "%f\t%f\n", cog_axis[c].x, cog_axis[c].y);
            fprintf(gp, "e\n");
        }
        fprintf(gp, "set nomultiplot\n"); // マルチプロットモード終了
        for (int c = 0; c < K; c++) {
            bef_cog[c].x = cog_axis[c].x;
            bef_cog[c].y = cog_axis[c].y;
        }
        update_cog(datas, cog_axis);//重心の更新
        int flg = 0;
        for (int c = 0; c < K; c++) {
            if ((bef_cog[c].x != cog_axis[c].x) || (bef_cog[c].y != cog_axis[c].y)) { break; }
            flg++;
        }//終了条件
        if (flg != 0) { break; }
    }
    fprintf(gp, "set terminal windows\n");//保存出力設定
    fprintf(gp, "set output\n");//保存
    fflush(gp); // バッファに格納されているデータを吐き出す（必須）
    //system("pause");
    fprintf(gp, "exit\n"); // gnuplot の終了
    _pclose(gp);
    return 0;
}

//ファイル読み込み
void fileset(string filename, vector<_subData>& datas) {
    ifstream ifs_file(filename);
    if (ifs_file.fail()) {	// ファイルオープンに失敗したらそこで終了
        cerr << "cannot open the file - '" << filename << "'" << endl;
        exit(1);
    }
    string line;
    while (getline(ifs_file, line)) { // 1行読んで
        replace(line.begin(), line.end(), ',', ' '); //カンマ区切りを空白区切りに
        istringstream iss(line);
        struct  _subData subdata;
        iss >> subdata.x >> subdata.y;
        datas.push_back(subdata);
    }
}

//重心の初期値
void init_cog(vector<_subData> datas, vector<_subData>& cog_axis) {
    random_device rnd;     // 非決定的な乱数生成器
    mt19937 mt(rnd());            // メルセンヌ・ツイスタの32ビット版、引数は初期シード
    uniform_int_distribution<> randint(0, datas.size());  // [0, 1500] 範囲の一様乱数
    string color = "black";
    for (int i = 0; i < K; i++) {
        if (i == 0) color = "red";
        else if (i == 1) color = "green";
        else if (i == 2) color = "blue";
        cog_axis[i].cluster = color;
        cog_axis[i].x = datas[randint(mt)].x;
        cog_axis[i].y =datas[randint(mt)].y;
    }
}

//重心からデータのクラスタリング
void clustering(vector<_subData>& datas, vector<_subData> cog_axis) {
    for (auto& data : datas) {
        double min_dis = 1e+5;
        for (const auto cog : cog_axis) {
            double dis = sqrt(pow((cog.x - data.x), 2) + pow((cog.y - data.y), 2));//二次元ユークリッド距離
            if (dis <= min_dis) {
                min_dis = dis;
                data.cluster = cog.cluster;
            }
        }
    }
}

//各クラスタのデータから重心を更新
void update_cog(vector<_subData> datas, vector<_subData>& cog_axis) {
    vector<_subData>RGB = { { "red",0.0,0.0 }, { "green",0.0,0.0 }, { "blue",0.0,0.0 } };//各クラスタの総和用
    vector<int> rgb(K, 0);//各クラスタの個数
    for (const auto data : datas) {
        for (int i = 0; i < K; i++) {
            if (data.cluster == RGB[i].cluster) { RGB[i].x += data.x; RGB[i].y += data.y; rgb[i]++; break; }
        }
    }
    for (int i = 0; i < K; i++) {
        if (rgb[i] != 0) { cog_axis[i].x = RGB[i].x / rgb[i]; cog_axis[i].y = RGB[i].y / rgb[i]; }
    }
}