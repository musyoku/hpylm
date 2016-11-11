#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <random>
#include <chrono>
#include <algorithm>
#include <map>
#include <unordered_map> 
#include <boost/format.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <stdio.h>
#include <wchar.h>
#include <locale>
#include "core/c_printf.h"
#include "core/node.h"
#include "core/hpylm.h"
#include "core/vocab.h"
#include "util.h"

using namespace std;

class Model{
public:
	string hpylm_filename = "model/hpylm.model";
	string trainer_filename = "model/hpylm.trainer";
	vector<bool> is_first_addition;
	HPYLM* hpylm;
	Model(int ngram, double g0){
		c_printf("[*]%s\n", "HPYLMを初期化しています ...");
		hpylm = new HPYLM(ngram);
		hpylm->set_g0(g0);
		c_printf("[*]%s\n", (boost::format("G0 <- %lf") % g0).str().c_str());
		hpylm->load(hpylm_filename);
	}
	void init_trainer(vector<vector<id>> &dataset){
		is_first_addition.clear();
		for(int data_index = 0;data_index < dataset.size();data_index++){
			is_first_addition.push_back(true);
		}
	}
	void load_trainer(){
		std::ifstream ifs(trainer_filename);
		if(ifs.good()){
			boost::archive::binary_iarchive iarchive(ifs);
			iarchive >> is_first_addition;
		}
	}
	void save_model(){
		hpylm->save(hpylm_filename);
	}
	void save_trainer(){
		std::ofstream ofs(trainer_filename);
		boost::archive::binary_oarchive oarchive(ofs);
		oarchive << is_first_addition;
	}
	void generate_words(Vocab* vocab, wstring spacer){
		c_printf("[*]%s\n", "文章を生成しています ...");
		int num_sample = 50;
		int max_length = 400;
		id bos_id = vocab->string_to_token_id(L"<bos>");
		id eos_id = vocab->string_to_token_id(L"<eos>");
		vector<id> token_ids;
		for(int s = 0;s < num_sample;s++){
			token_ids.clear();
			for(int i = 0;i < hpylm->ngram();i++){
				token_ids.push_back(bos_id);
			}
			for(int i = 0;i < max_length;i++){
				id token_id = hpylm->sample_next_token(token_ids, eos_id);
				token_ids.push_back(token_id);
				if(token_id == eos_id){
					break;
				}
			}
			for(auto token_id: token_ids){
				if(token_id == bos_id){
					continue;
				}
				if(token_id == eos_id){
					continue;
				}
				wstring word = vocab->token_id_to_string(token_id);
				wcout << word << spacer;
			}
			cout << endl;
		}
	}
	void train(Vocab* vocab, vector<vector<id>> &dataset){
		init_trainer(dataset);
		load_trainer();
		vector<int> rand_indices;
		for(int i = 0;i < dataset.size();i++){
			rand_indices.push_back(i);
		}
		int max_epoch = 100;
		int num_data = dataset.size();
		int ngram = hpylm->ngram();

		for(int epoch = 1;epoch <= max_epoch;epoch++){
			// printf("Epoch %d / %d", epoch, max_epoch);
			auto start_time = chrono::system_clock::now();
			random_shuffle(rand_indices.begin(), rand_indices.end());

			for(int step = 0;step < num_data;step++){
				show_progress(step, num_data);
				int data_index = rand_indices[step];
				vector<id> &token_ids = dataset[data_index];

				for(int token_t_index = ngram - 1;token_t_index < token_ids.size();token_t_index++){
					if(is_first_addition[data_index] == false){
						hpylm->remove_customer_at_timestep(token_ids, token_t_index);
					}
					hpylm->add_customer_at_timestep(token_ids, token_t_index);
				}
				is_first_addition[data_index] = false;
			}

			hpylm->sample_hyperparams();

			auto end_time = chrono::system_clock::now();
			auto duration = end_time - start_time;
			auto msec = chrono::duration_cast<chrono::milliseconds>(duration).count();

			// パープレキシティ
			double ppl = 0;
			for(int data_index = 0;data_index < num_data;data_index++){
				vector<id> &token_ids = dataset[data_index];
				double log_p = hpylm->log2_Pw(token_ids) / token_ids.size();
				ppl += log_p;
			}
			ppl = exp(-ppl / num_data);
			printf("Epoch %d / %d - %.1f lps - %.3f ppl - %d nodes - %d customers\n", epoch, max_epoch, (double)num_data / msec * 1000.0, ppl, hpylm->get_num_nodes(), hpylm->get_num_customers());
			if(epoch % 100 == 0){
				save_model();
				save_trainer();
			}
		}

		save_model();
		save_trainer();

		// <!-- デバッグ用
		//客を全て削除した時に客数が本当に0になるかを確認する場合
		// for(int step = 0;step < num_data;step++){
		// 	int data_index = rand_indices[step];
		// 	vector<id> token_ids = dataset[data_index];
		// 	for(int token_t_index = ngram - 1;token_t_index < token_ids.size();token_t_index++){
		// 		hpylm->remove_customer_at_timestep(token_ids, token_t_index);
		// 	}
		// }
		//  -->

		cout << hpylm->get_max_depth() << endl;
		cout << hpylm->get_num_nodes() << endl;
		cout << hpylm->get_num_customers() << endl;
		cout << hpylm->get_sum_stop_counts() << endl;
		cout << hpylm->get_sum_pass_counts() << endl;
	}
};

int main(int argc, char *argv[]){
	// 日本語周り
	setlocale(LC_CTYPE, "ja_JP.UTF-8");
	ios_base::sync_with_stdio(false);
	locale default_loc("ja_JP.UTF-8");
	locale::global(default_loc);
	locale ctype_default(locale::classic(), default_loc, locale::ctype); //※
	wcout.imbue(ctype_default);
	wcin.imbue(ctype_default);

	string text_filename;
	int ngram = 3;
	// cout << "num args = " << argc << endl;
	if(argc % 2 != 1){
		c_printf("[r]%s [*]%s\n", "エラー:", "テキストファイルを指定してください. -t example.txt");
		exit(1);
	}else{
		for(int i = 0; i < argc; i++){
			// cout << i << "-th args = " << argv[i] << endl; 
			if (string(argv[i]) == "-t" || string(argv[i]) == "--text"){
				if(i + 1 >= argc){
					c_printf("[r]%s [*]%s %s\n", "エラー:", "不正なコマンドライン引数です.", string(argv[i]).c_str());
					exit(1);
				}
				text_filename = string(argv[i + 1]);
			}
			else if (string(argv[i]) == "-n" || string(argv[i]) == "--ngram"){
				if(i + 1 >= argc){
					c_printf("[r]%s [*]%s %s\n", "エラー:", "不正なコマンドライン引数です.", string(argv[i]).c_str());
					exit(1);
				}
				ngram = atoi(argv[i + 1]);
			}
		}
	}
	vector<vector<id>> dataset;
	Vocab* vocab;
	load_words_in_textfile(text_filename, dataset, vocab, ngram);

	int num_chars = vocab->num_tokens();
	double g0 = (1.0 / num_chars);
	Model* hpylm = new Model(ngram, g0);
	hpylm->train(vocab, dataset);
	hpylm->generate_words(vocab, L" ");
	return 0;
}
