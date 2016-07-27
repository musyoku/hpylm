#ifndef _hpylm_
#define _hpylm_
#include <vector>
#include <random>
#include <unordered_map> 
#include <boost/serialization/serialization.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include "sampler.h"
#include "node.h"
#include "const.h"
#include "vocab.h"

class HPYLM{
private:
	friend class boost::serialization::access;
	template <class Archive>
	void serialize(Archive& archive, unsigned int version)
	{
		static_cast<void>(version); // No use
		archive & _root;
		archive & _max_depth;
		archive & _g0;
		archive & _d_m;
		archive & _theta_m;
		archive & _a_m;
		archive & _b_m;
		archive & _alpha_m;
		archive & _beta_m;
	}
public:
	Node* _root;				// 文脈木のルートノード
	int _max_depth;				// 深さ
	int _bottom;				// VPYLMへ拡張時に使う
	double _g0;					// ゼログラム確率

	// 深さmのノードに関するパラメータ
	vector<double> _d_m;		// Pitman-Yor過程のディスカウント
	vector<double> _theta_m;	// Pitman-Yor過程のアレ

	// "A Bayesian Interpretation of Interpolated Kneser-Ney" Appendix C参照
	// http://www.gatsby.ucl.ac.uk/~ywteh/research/compling/hpylm.pdf
	vector<double> _a_m;		// Beta分布のパラメータ	dの推定用
	vector<double> _b_m;		// Beta分布のパラメータ	dの推定用
	vector<double> _alpha_m;	// Gamma分布のパラメータ	θの推定用
	vector<double> _beta_m;		// Gamma分布のパラメータ	θの推定用

	HPYLM(int ngram = 2){
		// 深さは0から始まることに注意
		// バイグラムなら最大深さは1
		_max_depth = ngram - 1;

		_root = new Node(0);
		_root->_depth = 0;	// ルートは深さ0

		for(int n = 0;n <= _max_depth;n++){
			_d_m.push_back(PYLM_INITIAL_D);	
			_theta_m.push_back(PYLM_INITIAL_THETA);
			_a_m.push_back(PYLM_INITIAL_A);	
			_b_m.push_back(PYLM_INITIAL_B);	
			_alpha_m.push_back(PYLM_INITIAL_ALPHA);
			_beta_m.push_back(PYLM_INITIAL_BETA);
		}
	}
	void add(id word_id){
		_root->addCustomer(word_id, _g0, _d_m, _theta_m);
	}
	void add(vector<id> &word_ids, int w_t_i){
		int w_t = word_ids[w_t_i];
		// HPYLMでは深さは固定
		// ただし先頭の文字などmax_depthに満たない例外はある
		int max_depth = w_t_i > _max_depth ? _max_depth : w_t_i;

		Node* node = _root;
		for(int depth = 0;depth < max_depth;depth++){
			int u_t = word_ids[w_t_i - depth - 1];
			Node* child = node->generateChildIfNeeded(u_t);
			if(child == NULL){
				printf("\x1b[41;97m");
				printf("WARNING");
				printf("\x1b[49;39m");
				printf(" Unexpected error occurred.\n");
				return;
			}
			node = child;
		}
		double parent_p_w = _g0;
		if(node->_parent){
			parent_p_w = node->_parent->Pw(w_t, _g0, _d_m, _theta_m);
		}
		node->addCustomer(w_t, parent_p_w, _d_m, _theta_m);
	}

	bool remove(vector<id> &word_ids, int w_t_i){
		int w_t = word_ids[w_t_i];
		int max_depth = w_t_i > _max_depth ? _max_depth : w_t_i;
		Node* node = _root;
		for(int depth = 0;depth < max_depth;depth++){
			int u_t = word_ids[w_t_i - depth - 1];
			Node* child = node->findChildWithId(u_t);
			if(child == NULL){
				return false;
			}
			node = child;
		}
		// node->removeCustomer(w_t);
		bool should_remove_from_parent = false;
		node->removeCustomer(w_t, should_remove_from_parent);
		if(should_remove_from_parent && node->_parent != NULL){
			node->_parent->deleteChildWithId(node->_id);
		}
		return true;
	}

	double Pw_h(vector<id> &word_ids, vector<id> context_ids){
		double p = 1;
		for(int n = 0;n < word_ids.size();n++){
			p *= Pw_h(word_ids[n], context_ids);
			context_ids.push_back(word_ids[n]);
		}
		return p;
	}

	double Pw_h(id word_id, vector<id> &context_ids){
		// どの深さまでノードが存在するかを調べる
		Node* node = _root;
		int depth = context_ids.size() < _max_depth ? context_ids.size() : _max_depth;

		for(int n = 0;n < depth;n++){
			int u_t = context_ids[context_ids.size() - n - 1];
			// cout << "u_t: " << u_t << endl;
			if(node == NULL){
				break;
			}
			Node* child = node->findChildWithId(u_t);
			if(child == NULL){
				break;
			}
			node = child;
		}
		return node->Pw(word_id, _g0, _d_m, _theta_m);
	}

	double Pw(id word_id){
		double p = _root->Pw(word_id, _g0, _d_m, _theta_m);
		return p;
	}

	double Pw(vector<id> &word_ids){
		if(word_ids.size() == 0){
			return 0;
		}
		double p = 1;
		vector<id> context_ids(word_ids.begin(), word_ids.begin() + _max_depth);
		for(int depth = _max_depth;depth < word_ids.size();depth++){
			id word_id = word_ids[depth];
			double _p = Pw_h(word_id, context_ids);
			p *= _p;
			context_ids.push_back(word_id);
		}
		return p;
	}
	double log_Pw(vector<id> &word_ids){
		if(word_ids.size() == 0){
			return 0;
		}
		double p = 0;
		vector<id> context_ids(word_ids.begin(), word_ids.begin() + _max_depth);
		for(int depth = _max_depth;depth < word_ids.size();depth++){
			id word_id = word_ids[depth];
			double _p = Pw_h(word_id, context_ids);
			p += log2(_p + 1e-10);
			context_ids.push_back(word_id);
		}
		return p;
	}

	id sampleNextWord(vector<id> &context_ids, id eos_id){
		Node* node = _root;
		int depth = context_ids.size() < _max_depth ? context_ids.size() : _max_depth;

		for(int n = 0;n < depth;n++){
			int u_t = context_ids[context_ids.size() - n - 1];
			if(node == NULL){
				break;
			}
			Node* child = node->findChildWithId(u_t);
			if(child == NULL){
				break;
			}
			node = child;
		}

		vector<id> word_ids;
		vector<double> probs;
		double sum = 0;
		for(auto elem: node->_arrangement){
			id word_id = elem.first;
			double p = Pw_h(word_id, context_ids);
			if(p > 0){
				word_ids.push_back(word_id);
				probs.push_back(p);
				sum += p;
			}
		}
		if(word_ids.size() == 0){
			return eos_id;
		}
		if(sum == 0){
			return eos_id;
		}
		double ratio = 1.0 / sum;
		double r = Sampler::uniform(0, 1);
		sum = 0;
		id sampled_word_id = word_ids.back();
		for(int i = 0;i < word_ids.size();i++){
			sum += probs[i] * ratio;
			if(sum > r){
				sampled_word_id = word_ids[i];
				break;
			}
		}
		return sampled_word_id;
	}


	// "A Bayesian Interpretation of Interpolated Kneser-Ney" Appendix C参照
	// http://www.gatsby.ucl.ac.uk/~ywteh/research/compling/hpylm.pdf
	void sumAuxiliaryVariablesRecursively(Node* node, vector<double> &sum_log_x_u_m, vector<double> &sum_y_ui_m, vector<double> &sum_1_y_ui_m, vector<double> &sum_1_z_uwkj_m){
		for(auto elem: node->_children){
			Node* child = elem.second;
			int depth = child->_depth;

			if(depth > _bottom){
				_bottom = depth;
			}
			if(depth >= _d_m.size()){
				while(_d_m.size() <= depth){
					_d_m.push_back(PYLM_INITIAL_D);
				}
			}
			if(depth >= _theta_m.size()){
				while(_theta_m.size() <= depth){
					_theta_m.push_back(PYLM_INITIAL_THETA);
				}
			}

			double d = _d_m[depth];
			double theta = _theta_m[depth];
			sum_log_x_u_m[depth] += child->auxiliary_log_x_u(theta);	// log(x_u)
			sum_y_ui_m[depth] += child->auxiliary_y_ui(d, theta);		// y_ui
			sum_1_y_ui_m[depth] += child->auxiliary_1_y_ui(d, theta);	// 1 - y_ui
			sum_1_z_uwkj_m[depth] += child->auxiliary_1_z_uwkj(d);		// 1 - z_uwkj

			sumAuxiliaryVariablesRecursively(child, sum_log_x_u_m, sum_y_ui_m, sum_1_y_ui_m, sum_1_z_uwkj_m);
		}
	}

	// dとθの推定
	void sampleHyperParams(){
		unordered_map<int, vector<Node*> > nodes_by_depth;
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
		_bottom = 0;
		sumAuxiliaryVariablesRecursively(_root, sum_log_x_u_m, sum_y_ui_m, sum_1_y_ui_m, sum_1_z_uwkj_m);

		for(int u = 0;u <= _bottom;u++){

			if(u >= _a_m.size()){
				while(_a_m.size() <= u){
					_a_m.push_back(PYLM_INITIAL_A);
				}
			}
			if(u >= _b_m.size()){
				while(_b_m.size() <= u){
					_b_m.push_back(PYLM_INITIAL_B);
				}
			}
			if(u >= _alpha_m.size()){
				while(_alpha_m.size() <= u){
					_alpha_m.push_back(PYLM_INITIAL_ALPHA);
				}
			}
			if(u >= _beta_m.size()){
				while(_beta_m.size() <= u){
					_beta_m.push_back(PYLM_INITIAL_BETA);
				}
			}
			
			_d_m[u] = Sampler::beta(_a_m[u] + sum_1_y_ui_m[u], _b_m[u] + sum_1_z_uwkj_m[u]);
			_theta_m[u] = Sampler::gamma(_alpha_m[u] + sum_y_ui_m[u], 1 / (_beta_m[u] - sum_log_x_u_m[u]));
		}

		int num_remove = _d_m.size() - _bottom;
		for(int n = 0;n < num_remove;n++){
			_d_m.pop_back();
			_theta_m.pop_back();
			_a_m.pop_back();
			_b_m.pop_back();
			_alpha_m.pop_back();
			_beta_m.pop_back();
		}
	}

	int maxDepth(){
		return _d_m.size() - 1;
	}

	int numChildNodes(){
		return _root->numChildNodes();
	}

	int numCustomers(){
		return _root->numCustomers();
	}

	void setActiveKeys(unordered_map<id, bool> &keys){
		_root->setActiveKeys(keys);
	}

	void save(string dir = "model/"){
		string filename = dir + "hpylm.model";
		std::ofstream ofs(filename);
		boost::archive::binary_oarchive oarchive(ofs);
		oarchive << static_cast<const HPYLM&>(*this);
		cout << "saved to " << filename << endl;
		cout << "	num_customers: " << numCustomers() << endl;
		cout << "	num_nodes: " << numChildNodes() << endl;
		cout << "	max_depth: " << maxDepth() << endl;
	}

	void load(string dir = "model/"){
		string filename = dir + "hpylm.model";
		std::ifstream ifs(filename);
		if(ifs.good()){
			boost::archive::binary_iarchive iarchive(ifs);
			iarchive >> *this;
		}
	}
};

#endif