#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <random>
#include <chrono>
#include <algorithm>
#include <map>
#include <unordered_map> 
#include <boost/serialization/serialization.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <stdio.h>
#include <wchar.h>
#include <locale>
#include "node.h"
#include "hpylm.h"
#include "vocab.h"

using namespace std;

Vocab* load(string &filename, vector<wstring> &dataset){
	wifstream ifs(filename.c_str());
	wstring str;
	if (ifs.fail())
	{
		cout << "failed to load " << filename << endl;
		return NULL;
	}
	Vocab* vocab = new Vocab();
	while (getline(ifs, str))
	{
		dataset.push_back(str);
		for(int i = 0;i < str.length();i++){
			vocab->addCharacter(str[i]);
		}
	}
	cout << "loading " << dataset.size() << " lines." << endl;
	return vocab;
}

void show_progress(int step, int total, double &progress){
	progress = step / (double)(total - 1);
	int barWidth = 70;

	cout << "[";
	int pos = barWidth * progress;
	for (int i = 0; i < barWidth; ++i) {
		if (i < pos) cout << "=";
		else if (i == pos) cout << ">";
		else cout << " ";
	}
	cout << "] " << int(progress * 100.0) << " %\r";
	cout.flush();

	progress += 0.16; // for demonstration only
}

void train_hpylm(Vocab* vocab, vector<wstring> &dataset, string model_dir){
	HPYLM* hpylm = new HPYLM(3);
	int num_chars = vocab->numCharacters();
	hpylm->_g0 = 1.0 / (double)num_chars;
	cout << "g0: " << hpylm->_g0 << endl;

	vector<id> sentence_char_ids;

	int max_epoch = 100;
	// int train_per_epoch = dataset.size();
	int train_per_epoch = dataset.size();

	vector<int> rand_perm;
	for(int i = 0;i < dataset.size();i++){
		rand_perm.push_back(i);
	}
	random_shuffle(rand_perm.begin(), rand_perm.end());

	// 読み込み
	hpylm->load(model_dir);
	vocab->load(model_dir);

	printf("training in progress...\n");
	for(int epoch = 1;epoch < max_epoch;epoch++){
		// cout << "##########################################" << endl;
		// cout << "EPOCH " << epoch << endl;
		// cout << "##########################################" << endl;
		auto start_time = chrono::system_clock::now();
		double progress = 0.0;
		random_shuffle(rand_perm.begin(), rand_perm.end());

		for(int step = 0;step < train_per_epoch;step++){
			// cout << "\x1b[40;97m[STEP]\x1b[49;39m" << endl;

			show_progress(step, train_per_epoch, progress);

			int data_index = rand_perm[step];

			wstring sentence = dataset[data_index];
			if(sentence.length() == 0){
				continue;
			}
			sentence_char_ids.clear();
			sentence_char_ids.push_back(vocab->bosId());
			for(int i = 0;i < sentence.length();i++){
				int id = vocab->char2id(sentence[i]);
				sentence_char_ids.push_back(id);
			}
			sentence_char_ids.push_back(vocab->eosId());
			for(int c_t_i = 0;c_t_i < sentence_char_ids.size();c_t_i++){
				if(epoch != 1){
					bool success = hpylm->remove(sentence_char_ids, c_t_i);
					if(success == false){
						printf("\x1b[41;97m");
						printf("WARNING");
						printf("\x1b[49;39m");
						printf(" Failed to remove customer.\n");
					}
				}
				hpylm->add(sentence_char_ids, c_t_i);
			}
		}

		hpylm->sampleHyperParams();

		auto end_time = chrono::system_clock::now();
		auto dur = end_time - start_time;
		auto msec = chrono::duration_cast<chrono::milliseconds>(dur).count();

		// パープレキシティ
		double hpylm_ppl = 0;
		for(int i = 0;i < train_per_epoch;i++){
			wstring sentence = dataset[rand_perm[i]];
			if(sentence.length() == 0){
				continue;
			}
			// if(sentence.length() > 200){
			// 	continue;
			// }
			sentence_char_ids.clear();
			sentence_char_ids.push_back(vocab->bosId());
			for(int i = 0;i < sentence.length();i++){
				int id = vocab->char2id(sentence[i]);
				sentence_char_ids.push_back(id);
			}
			sentence_char_ids.push_back(vocab->eosId());
			double log_p = hpylm->log_Pw(sentence_char_ids) / (double)sentence_char_ids.size();
			hpylm_ppl += log_p;
		}
		hpylm_ppl = exp(-hpylm_ppl / (double)train_per_epoch);

		cout << endl << "[epoch " << epoch << "] " <<  (double)train_per_epoch / msec * 1000.0 << " sentences / sec; perplexity " << hpylm_ppl << endl;

		if(epoch % 10 == 0){
			hpylm->save(model_dir);
			vocab->save(model_dir);
		}

	}

	// すべての客を削除
	// 1人でも客が残っていればバグっている
	for(int step = 0;step < dataset.size();step++){
		// cout << "\x1b[40;97m[STEP]\x1b[49;39m" << endl;
		int data_index = rand_perm[step];

		wstring sentence = dataset[data_index];
		if(sentence.length() == 0){
			continue;
		}
		sentence_char_ids.clear();
		sentence_char_ids.push_back(vocab->bosId());
		for(int i = 0;i < sentence.length();i++){
			int id = vocab->char2id(sentence[i]);
			sentence_char_ids.push_back(id);
		}
		sentence_char_ids.push_back(vocab->eosId());
		for(int c_t_i = 0;c_t_i < sentence_char_ids.size();c_t_i++){
			hpylm->remove(sentence_char_ids, c_t_i);
		}
	}


	cout << hpylm->maxDepth() << endl;
	cout << hpylm->numChildNodes() << endl;
	cout << hpylm->numCustomers() << endl;
}

void hpylm_generate_sentence(Vocab* vocab, vector<wstring> &dataset, string model_dir){
	HPYLM* hpylm = new HPYLM(3);
	int num_chars = vocab->numCharacters();
	hpylm->_g0 = 1.0 / (double)num_chars;

	// 読み込み
	hpylm->load(model_dir);
	vocab->load(model_dir);

	int num_sample = 100;
	int max_length = 400;
	vector<id> sentence_char_ids;
	for(int s = 0;s < num_sample;s++){
		sentence_char_ids.clear();
		sentence_char_ids.push_back(vocab->bosId());
		for(int i = 0;i < max_length;i++){
			id word_id = hpylm->sampleNextWord(sentence_char_ids, vocab->eosId());
			sentence_char_ids.push_back(word_id);
			if(word_id == vocab->eosId()){
				break;
			}
		}
		wcout << vocab->characters2string(sentence_char_ids) << endl;
	}
}

int main(int argc, char *argv[]){
	// 日本語周り
	setlocale(LC_CTYPE, "ja_JP.UTF-8");
	ios_base::sync_with_stdio(false);
	locale default_loc("ja_JP.UTF-8");
	locale::global(default_loc);
	locale ctype_default(locale::classic(), default_loc, locale::ctype); //※
	wcout.imbue(ctype_default);
	wcin.imbue(ctype_default);
	vector<wstring> dataset;

	// [arguments]
	// --text_dir ****
	// 訓練データtrain.txtの入っているフォルダを指定
	// --model_dir ****
	// モデル保存用フォルダを指定

	string filename;
	string model_dir;
	cout << "num args = " << argc << endl;
	if(argc % 2 != 1){
		cout << "invalid command line arguments." << endl;
		return -1;
	}else{
		for (int i = 0; i < argc; i++) {
			cout << i << "-th args = " << argv[i] << endl; 
			if (string(argv[i]) == "--text_dir") {
				if(i + 1 >= argc){
					cout << "invalid command line arguments." << endl;
					return -1;
				}
				filename = string(argv[i + 1]) + string("/train.txt");
			}
			else if (string(argv[i]) == "--model_dir") {
				if(i + 1 >= argc){
					cout << "invalid command line arguments." << endl;
					return -1;
				}
				model_dir = string(argv[i + 1]) + string("/");
			}
		}
	}

	Vocab* vocab = load(filename, dataset);

	train_hpylm(vocab, dataset, model_dir);
	hpylm_generate_sentence(vocab, dataset, model_dir);
	return 0;
}
