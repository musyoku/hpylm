#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cassert>
#include <random>
#include <chrono>
#include <algorithm>
#include <map>
#include <unordered_map> 
#include <stdio.h>
#include <wchar.h>
#include <locale>
#include "c_printf.h"
#include "node.h"
#include "hpylm.h"
#include "vocab.h"

using namespace std;
void test_node(){
	int ngram = 3;
	HPYLM* hpylm = new HPYLM(ngram);
	hpylm->set_g0(0.01);
	vector<id> token_ids;
	for(int i = 0;i < 10;i++){
		token_ids.push_back(i % 10);
	}
	for(int token_t_index = ngram - 1;token_t_index < token_ids.size();token_t_index++){
		Node* node = hpylm->find_node_by_tracing_back_context(token_ids, token_t_index, ngram - 1, true);
		if(node){
			cout << *node << endl;
		}
	}
}

void test_remove_customer(){
	int ngram = 3;
	HPYLM* hpylm = new HPYLM(ngram);
	hpylm->set_g0(0.01);
	vector<id> token_ids;
	for(int i = 0;i < 100;i++){
		token_ids.push_back(i % 10);
	}
	for(int trial = 0;trial < 1000;trial++){
		for(int t = ngram - 1;t < token_ids.size();t++){
			hpylm->add_customer_at_timestep(token_ids, t);
			hpylm->remove_customer_at_timestep(token_ids, t);
		}
	}
	printf("depth: %d\n", hpylm->get_max_depth(false));
	printf("# of nodes: %d\n", hpylm->get_num_nodes());
	printf("# of customers: %d\n", hpylm->get_num_customers());
	printf("# of tables: %d\n", hpylm->get_num_tables());
	printf("stop count: %d\n", hpylm->get_sum_stop_counts());
	printf("pass count: %d\n", hpylm->get_sum_pass_counts());
}

void test_train(){
	double num_types_token = 100;
	random_device rnd;
	mt19937 mt(rnd());
	uniform_int_distribution<> rand(0, num_types_token - 1);

	int ngram = 4;
	HPYLM* hpylm = new HPYLM(ngram);
	hpylm->set_g0(1.0 / num_types_token);
	vector<id> token_ids;
	for(int i = 0;i < 5000;i++){
		token_ids.push_back(rand(mt));
	}
	vector<int> prev_orders(5000, -1);
	vector<int> rand_indices;
	for(int i = 0;i < token_ids.size() - ngram + 1;i++){
		rand_indices.push_back(i + ngram - 1);
	}
	random_shuffle(rand_indices.begin(), rand_indices.end());
	int max_epoch = 500;
	for(int epoch = 0;epoch < max_epoch;epoch++){
		random_shuffle(rand_indices.begin(), rand_indices.end());
		for(int t = 0;t < rand_indices.size();t++){
			int token_t_index = rand_indices[t];
			int prev_order_t = prev_orders[token_t_index];
			if(prev_order_t != -1){
				bool success = hpylm->remove_customer_at_timestep(token_ids, token_t_index);
				assert(success == true);
			}
			bool success = hpylm->add_customer_at_timestep(token_ids, token_t_index);
			assert(success == true);
			prev_orders[token_t_index] = ngram - 1;
		}
		hpylm->sample_hyperparams();
		if(epoch % 10 == 0){
			double log_p = hpylm->log2_Pw(token_ids);
			assert(log_p != -1);
			log_p /= token_ids.size();
			double ppl = exp(-log_p);
			printf("ppl: %lf\n", ppl);
		}
	}
	printf("depth: %d\n", hpylm->get_max_depth(false));
	printf("# of nodes: %d\n", hpylm->get_num_nodes());
	printf("# of customers: %d\n", hpylm->get_num_customers());
	printf("# of tables: %d\n", hpylm->get_num_tables());
	printf("stop count: %d\n", hpylm->get_sum_stop_counts());
	printf("pass count: %d\n", hpylm->get_sum_pass_counts());
	hpylm->save("./");
	hpylm->load("./");
	printf("depth: %d\n", hpylm->get_max_depth(false));
	printf("# of nodes: %d\n", hpylm->get_num_nodes());
	printf("# of customers: %d\n", hpylm->get_num_customers());
	printf("# of tables: %d\n", hpylm->get_num_tables());
	printf("stop count: %d\n", hpylm->get_sum_stop_counts());
	printf("pass count: %d\n", hpylm->get_sum_pass_counts());
	for(int token_t_index = ngram - 1;token_t_index < token_ids.size();token_t_index++){
		hpylm->remove_customer_at_timestep(token_ids, token_t_index);
	}
	printf("depth: %d\n", hpylm->get_max_depth(false));
	printf("# of nodes: %d\n", hpylm->get_num_nodes());
	printf("# of customers: %d\n", hpylm->get_num_customers());
	printf("# of tables: %d\n", hpylm->get_num_tables());
	printf("stop count: %d\n", hpylm->get_sum_stop_counts());
	printf("pass count: %d\n", hpylm->get_sum_pass_counts());
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

	// test_node();
	// test_remove_customer();
	test_train();	
}
