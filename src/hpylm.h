#pragma once
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <unordered_set>
#include <vector>
#include <cassert>
#include <fstream>
#include "common.h"
#include "sampler.h"
#include "node.h"

class HPYLM{
public:
	Node* _root;				// 文脈木のルートノード
	int _depth;				// 最大の深さ
	double _g0;					// ゼログラム確率

	// 深さmのノードに関するパラメータ
	vector<double> _d_m;		// Pitman-Yor過程のディスカウント係数
	vector<double> _theta_m;	// Pitman-Yor過程の集中度

	// "A Bayesian Interpretation of Interpolated Kneser-Ney" Appendix C参照
	// http://www.gatsby.ucl.ac.uk/~ywteh/research/compling/hpylm.pdf
	vector<double> _a_m;		// ベータ分布のパラメータ	dの推定用
	vector<double> _b_m;		// ベータ分布のパラメータ	dの推定用
	vector<double> _alpha_m;	// ガンマ分布のパラメータ	θの推定用
	vector<double> _beta_m;		// ガンマ分布のパラメータ	θの推定用

	HPYLM(int ngram = 2){
		// 深さは0から始まることに注意
		// 2-gramなら最大深さは1. root(0) -> 2-gram(1)
		// 3-gramなら最大深さは2. root(0) -> 2-gram(1) -> 3-gram(2)
		_depth = ngram - 1;

		_root = new Node(0);
		_root->_depth = 0;	// ルートは深さ0

		for(int n = 0;n < ngram;n++){
			_d_m.push_back(HPYLM_INITIAL_D);	
			_theta_m.push_back(HPYLM_INITIAL_THETA);
			_a_m.push_back(HPYLM_INITIAL_A);	
			_b_m.push_back(HPYLM_INITIAL_B);	
			_alpha_m.push_back(HPYLM_INITIAL_ALPHA);
			_beta_m.push_back(HPYLM_INITIAL_BETA);
		}
	}
	~HPYLM(){
		_delete_node(_root);
	}
	void _delete_node(Node* node){
		for(auto &elem: node->_children){
			Node* child = elem.second;
			_delete_node(child);
		}
		delete node;
	}
	int ngram(){
		return _depth + 1;
	}
	void set_g0(double g0){
		_g0 = g0;
	}
	// 単語列のindex番目の単語をモデルに追加
	bool add_customer_at_timestep(vector<id> &token_ids, int token_t_index){
		Node* node = find_node_by_tracing_back_context(token_ids, token_t_index, _depth, true);
		assert(node != NULL);
		assert(node->_depth == _depth);
		id token_t = token_ids[token_t_index];
		node->add_customer(token_t, _g0, _d_m, _theta_m);
		return true;
	}
	bool remove_customer_at_timestep(vector<id> &token_ids, int token_t_index){
		Node* node = find_node_by_tracing_back_context(token_ids, token_t_index, _depth, false);
		assert(node != NULL);
		assert(node->_depth == _depth);
		id token_t = token_ids[token_t_index];
		node->remove_customer(token_t);
		// 客が一人もいなくなったらノードを削除する
		if(node->need_to_remove_from_parent()){
			node->remove_from_parent();
		}
		return true;
	}
	// token列の位置tからorderだけ遡る
	// token_ids:        [0, 1, 2, 3, 4, 5]
	// token_t_index:4          ^     ^
	// order_t: 2               |<- <-|
	Node* find_node_by_tracing_back_context(vector<id> &token_ids, int token_t_index, int order_t, bool generate_node_if_needed = false, bool return_middle_node = false){
		if(token_t_index - order_t < 0){
			return NULL;
		}
		Node* node = _root;
		for(int depth = 1;depth <= order_t;depth++){
			id context_token_id = token_ids[token_t_index - depth];
			Node* child = node->find_child_node(context_token_id, generate_node_if_needed);
			if(child == NULL){
				if(return_middle_node){
					return node;
				}
				return NULL;
			}
			node = child;
		}
		return node;
	}
	double compute_Pw_h(vector<id> &token_ids, vector<id> context_token_ids){
		double p = 1;
		for(int n = 0;n < token_ids.size();n++){
			p *= compute_Pw_h(token_ids[n], context_token_ids);
			context_token_ids.push_back(token_ids[n]);
		}
		return p;
	}
	double compute_Pw_h(id token_id, vector<id> &context_token_ids){
		// HPYLMでは深さは固定
		assert(context_token_ids.size() >= _depth);
		Node* node = find_node_by_tracing_back_context(context_token_ids, context_token_ids.size(), _depth, false, true);
		assert(node != NULL);
		return node->compute_Pw(token_id, _g0, _d_m, _theta_m);
	}
	double compute_Pw(id token_id){
		return _root->compute_Pw(token_id, _g0, _d_m, _theta_m);
	}
	double compute_Pw(vector<id> &token_ids){
		assert(token_ids.size() >= _depth + 1);
		double mult_pw_h = 1;
		vector<id> context_token_ids(token_ids.begin(), token_ids.begin() + _depth);
		for(int t = _depth;t < token_ids.size();t++){
			id token_id = token_ids[t];
			mult_pw_h *= compute_Pw_h(token_id, context_token_ids);;
			context_token_ids.push_back(token_id);
		}
		return mult_pw_h;
	}
	double compute_log_Pw(vector<id> &token_ids){
		assert(token_ids.size() >= _depth + 1);
		double sum_pw_h = 0;
		vector<id> context_token_ids(token_ids.begin(), token_ids.begin() + _depth);
		for(int t = _depth;t < token_ids.size();t++){
			id token_id = token_ids[t];
			double pw_h = compute_Pw_h(token_id, context_token_ids);
			assert(pw_h > 0);
			sum_pw_h += log(pw_h);
			context_token_ids.push_back(token_id);
		}
		return sum_pw_h;
	}
	double compute_log2_Pw(vector<id> &token_ids){
		assert(token_ids.size() >= _depth + 1);
		double sum_pw_h = 0;
		vector<id> context_token_ids(token_ids.begin(), token_ids.begin() + _depth);
		for(int t = _depth;t < token_ids.size();t++){
			id token_id = token_ids[t];
			double pw_h = compute_Pw_h(token_id, context_token_ids);
			assert(pw_h > 0);
			sum_pw_h += log2(pw_h);
			context_token_ids.push_back(token_id);
		}
		return sum_pw_h;
	}
	id sample_next_token(vector<id> &context_token_ids, unordered_set<id> &all_token_ids){
		Node* node = find_node_by_tracing_back_context(context_token_ids, context_token_ids.size(), _depth, false, true);
		assert(node != NULL);
		vector<id> token_ids;
		vector<double> pw_h_array;
		double sum = 0;
		for(id token_id: all_token_ids){
			if(token_id == ID_BOS){
				continue;
			}
			double pw_h = compute_Pw_h(token_id, context_token_ids);
			if(pw_h > 0){
				token_ids.push_back(token_id);
				pw_h_array.push_back(pw_h);
				sum += pw_h;
			}
		}
		if(token_ids.size() == 0){
			return ID_EOS;
		}
		if(sum == 0){
			return ID_EOS;
		}
		double normalizer = 1.0 / sum;
		double bernoulli = sampler::uniform(0, 1);
		double stack = 0;
		for(int i = 0;i < token_ids.size();i++){
			stack += pw_h_array[i] * normalizer;
			if(stack > bernoulli){
				return token_ids[i];
			}
		}
		return token_ids.back();
	}
	// "A Bayesian Interpretation of Interpolated Kneser-Ney" Appendix C参照
	// http://www.gatsby.ucl.ac.uk/~ywteh/research/compling/hpylm.pdf
	void sum_auxiliary_variables_recursively(Node* node, vector<double> &sum_log_x_u_m, vector<double> &sum_y_ui_m, vector<double> &sum_1_y_ui_m, vector<double> &sum_1_z_uwkj_m){
		for(auto elem: node->_children){
			Node* child = elem.second;
			int depth = child->_depth;
			double d = _d_m[depth];
			double theta = _theta_m[depth];
			sum_log_x_u_m[depth] += child->auxiliary_log_x_u(theta);	// log(x_u)
			sum_y_ui_m[depth] += child->auxiliary_y_ui(d, theta);		// y_ui
			sum_1_y_ui_m[depth] += child->auxiliary_1_y_ui(d, theta);	// 1 - y_ui
			sum_1_z_uwkj_m[depth] += child->auxiliary_1_z_uwkj(d);		// 1 - z_uwkj

			sum_auxiliary_variables_recursively(child, sum_log_x_u_m, sum_y_ui_m, sum_1_y_ui_m, sum_1_z_uwkj_m);
		}
	}
	// dとθの推定
	void sample_hyperparams(){
		int max_depth = _d_m.size() - 1;

		// 親ノードの深さが0であることに注意
		vector<double> sum_log_x_u_m(max_depth + 1, 0.0);
		vector<double> sum_y_ui_m(max_depth + 1, 0.0);
		vector<double> sum_1_y_ui_m(max_depth + 1, 0.0);
		vector<double> sum_1_z_uwkj_m(max_depth + 1, 0.0);

		// _root
		sum_log_x_u_m[0] = _root->auxiliary_log_x_u(_theta_m[0]);			// log(x_u)
		sum_y_ui_m[0] = _root->auxiliary_y_ui(_d_m[0], _theta_m[0]);		// y_ui
		sum_1_y_ui_m[0] = _root->auxiliary_1_y_ui(_d_m[0], _theta_m[0]);	// 1 - y_ui
		sum_1_z_uwkj_m[0] = _root->auxiliary_1_z_uwkj(_d_m[0]);				// 1 - z_uwkj

		// それ以外
		sum_auxiliary_variables_recursively(_root, sum_log_x_u_m, sum_y_ui_m, sum_1_y_ui_m, sum_1_z_uwkj_m);

		for(int u = 0;u <= _depth;u++){
			_d_m[u] = sampler::beta(_a_m[u] + sum_1_y_ui_m[u], _b_m[u] + sum_1_z_uwkj_m[u]);
			_theta_m[u] = sampler::gamma(_alpha_m[u] + sum_y_ui_m[u], _beta_m[u] - sum_log_x_u_m[u]);
		}
	}
	int get_num_nodes(){
		return _root->get_num_nodes();
	}
	int get_num_customers(){
		return _root->get_num_customers();
	}
	int get_num_tables(){
		return _root->get_num_tables();
	}
	int get_sum_stop_counts(){
		return _root->sum_stop_counts();
	}
	int get_sum_pass_counts(){
		return _root->sum_pass_counts();
	}
	void count_tokens_of_each_depth(unordered_map<int, int> &map){
		_root->count_tokens_of_each_depth(map);
	}
	void enumerate_phrases_at_depth(int depth, vector<vector<id>> &phrases){
		assert(depth <= _depth);
		// 指定の深さのノードを探索
		vector<Node*> nodes;
		_root->enumerate_nodes_at_depth(depth, nodes);
		for(auto &node: nodes){
			vector<id> phrase;
			while(node->_parent){
				phrase.push_back(node->_token_id);
				node = node->_parent;
			}
			phrases.push_back(phrase);
		}
	}
	template <class Archive>
	void serialize(Archive& archive, unsigned int version)
	{
		archive & _root;
		archive & _depth;
		archive & _g0;
		archive & _d_m;
		archive & _theta_m;
		archive & _a_m;
		archive & _b_m;
		archive & _alpha_m;
		archive & _beta_m;
	}
	bool save(string filename = "hpylm.model"){
		std::ofstream ofs(filename);
		boost::archive::binary_oarchive oarchive(ofs);
		oarchive << *this;
		return true;
	}
	bool load(string filename = "hpylm.model"){
		std::ifstream ifs(filename);
		if(ifs.good() == false){
			return false;
		}
		boost::archive::binary_iarchive iarchive(ifs);
		iarchive >> *this;
		return true;
	}
};