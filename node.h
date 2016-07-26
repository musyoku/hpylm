#ifndef _node_
#define _node_
#include <algorithm>
#include <numeric>
#include <string>
#include <iostream>
#include <random>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <boost/serialization/serialization.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include "sampler.h"
#include "const.h"
#include "vocab.h"

using namespace std;

class Node{
private:
	bool addCustomerToEmptyArrangementWithId(id word_id, double parent_p_w, vector<double> &d_m, vector<double> &theta_m){
		if(_arrangement.find(word_id) == _arrangement.end()){
			vector<int> tables = {1};
			_arrangement[word_id] = tables;
			_num_customers++;
			_num_tables++;
			if(_parent != NULL){
				_parent->addCustomer(word_id, parent_p_w, d_m, theta_m);
			}
			return true;
		}
		return false;
	}
	bool addCustomerWithIdAndTable(id word_id, int k, double parent_p_w, vector<double> &d_m, vector<double> &theta_m){
		if(_arrangement.find(word_id) == _arrangement.end()){
			return addCustomerToEmptyArrangementWithId(word_id, parent_p_w, d_m, theta_m);
		}
		if(k < _arrangement[word_id].size()){
			_arrangement[word_id][k]++;
			_num_customers++;
			return true;
		}
		return false;
	}
	bool addCustomerToNewTableWithId(id word_id, double parent_p_w, vector<double> &d_m, vector<double> &theta_m){
		_arrangement[word_id].push_back(1);
		_num_tables++;
		_num_customers++;
		if(_parent != NULL){
			_parent->addCustomer(word_id, parent_p_w, d_m, theta_m);
		}
		return true;
	}
	bool removeCustomerWithIdAndTable(id word_id, int k){
		if(_arrangement.find(word_id) == _arrangement.end()){
			return false;
		}
		if(k < _arrangement[word_id].size()){
			vector<int> &tables = _arrangement[word_id];
			tables[k]--;
			_num_customers--;
			if(tables[k] < 0){
				printf("\x1b[41;97m");
				printf("WARNING");
				printf("\x1b[49;39m");
				printf(" _arrangement has fallen bellow 0.\n");
				return false;
			}
			if(tables[k] == 0){
				if(_parent != NULL){
					bool should_remove_from_parent = false;
					_parent->removeCustomer(word_id, should_remove_from_parent);
				}
				tables.erase(tables.begin() + k);
				_num_tables--;
				if(tables.size() == 0){
					_arrangement.erase(word_id);
					// cout << "Node::removeCustomerWithIdAndTable _arrangement " << word_id << " has been deleted." << endl;
				}
				// cout << "Node::removeCustomerWithIdAndTable table " << k << " has been deleted." << endl;
			}
			return true;
		}
		return false;
	}
	friend class boost::serialization::access;
	template <class Archive>
	void serialize(Archive& archive, unsigned int version)
	{
		static_cast<void>(version); // No use
		archive & _children;
		archive & _arrangement;
		archive & _num_tables;
		archive & _num_customers;
		archive & _parent;
		archive & _stop_count;
		archive & _pass_count;
		archive & _id;
		archive & _depth;
		archive & _identifier;
		archive & _auto_increment;
	}
public:
	static id _auto_increment;						// identifier用 VPYLMとは無関係
	unordered_map<id, Node*> _children;				// 子の文脈木
	unordered_map<id, vector<int> > _arrangement;	// 客の配置 vector<int>のk番目の要素がテーブルkの客数を表す
	int _num_tables;								// 総テーブル数
	int _num_customers;								// 客の総数
	Node* _parent;									// 親ノード
	int _stop_count;								// 停止回数
	int _pass_count;								// 通過回数
	id _id;											// 単語ID　文字ID
	int _depth;										// ノードの深さ　rootが0
	id _identifier;									// 識別用　特別な意味は無い VPYLMとは無関係

	Node(id word_id = 0){
		_num_tables = 0;
		_num_customers = 0;
		_stop_count = 0;
		_pass_count = 0;
		_identifier = _auto_increment;
		_auto_increment++;
		_id = word_id;
		_parent = NULL;
	}
	bool childExists(int id){
		return !(_children.find(id) == _children.end());
	}
	int numTablesServingWord(int id){
		if(_arrangement.find(id) == _arrangement.end()){
			return 0;
		}
		return _arrangement[id].size();
	}
	int numCustomersEatingWord(int id){
		if(_arrangement.find(id) == _arrangement.end()){
			return 0;
		}
		vector<int> &tables = _arrangement[id];
		int sum = 0;
		for(int i = 0;i < tables.size();i++){
			sum += tables[i];
		}
		return sum;
	}
	Node* findChildWithId(int id){
		auto itr = _children.find(id);
		if (itr == _children.end()) {
			return NULL;
		}
		return itr->second;
	}
	Node* generateChildIfNeeded(int id){
		Node* child = NULL;
		if(childExists(id)){
			child = _children[id];
		}else{
			child = new Node(id);
			child->_parent = this;
			child->_depth = _depth + 1;
			_children[id] = child;
		}
		return child;
	}
	void addCustomer(id word_id, double parent_p_w, vector<double> &d_m, vector<double> &theta_m){
		if(_depth >= d_m.size()){
			while(d_m.size() <= _depth){
				d_m.push_back(PYLM_INITIAL_D);
			}
			while(theta_m.size() <= _depth){
				theta_m.push_back(PYLM_INITIAL_THETA);
			}
		}

		double d_u = d_m[_depth];
		double theta_u = theta_m[_depth];
		if(_arrangement.find(word_id) == _arrangement.end()){
			if(!addCustomerToEmptyArrangementWithId(word_id, parent_p_w, d_m, theta_m)){
				printf("\x1b[41;97m");
				printf("WARNING");
				printf("\x1b[49;39m");
				printf(" Failed to add a customer to an empty _arrangement\n");
			}
			incrementStopCount();
		}else{
			vector<int> &tables = _arrangement[word_id];
			double rand_max = 0.0;
			for(int k = 0;k < tables.size();k++){
				rand_max += std::max(0.0, tables[k] - d_u);
			}
			double t_u = (double)_num_tables;
			rand_max += (theta_u + d_u * t_u) * parent_p_w;

			uniform_real_distribution<double> rand(0, rand_max);
			double r = rand(Sampler::mt);

			double sum = 0.0;
			for(int k = 0;k < tables.size();k++){
				sum += std::max(0.0, tables[k] - d_u);
				// cout << "Node." << _identifier << "::addCustomer " << r << " <= " << sum << endl;
				if(r <= sum){
					// cout << "Node." << _identifier << "::addCustomer sum is bigger than r." << endl;
					if(!addCustomerWithIdAndTable(word_id, k, parent_p_w, d_m, theta_m)){
						printf("\x1b[41;97m");
						printf("WARNING");
						printf("\x1b[49;39m");
						printf(" Failed to add a customer\n");
					}
					incrementStopCount();
					return;
				}
			}

			addCustomerToNewTableWithId(word_id, parent_p_w, d_m, theta_m);
			incrementStopCount();
		}
	}

	bool removeCustomer(id word_id, bool &should_remove_from_parent){
		should_remove_from_parent = false;
		if(_arrangement.find(word_id) == _arrangement.end()){
			return false;
		}

		vector<int> &tables = _arrangement[word_id];
		double max = 0.0;
		for(int k = 0;k < tables.size();k++){
			max += tables[k];
		}
		
		uniform_real_distribution<double> rand(0, max);
		double r = rand(Sampler::mt);

		double sum = 0.0;
		for(int k = 0;k < tables.size();k++){
			sum += tables[k];
			if(r <= sum){
				if(!removeCustomerWithIdAndTable(word_id, k)){
					printf("\x1b[41;97m");
					printf("WARNING");
					printf("\x1b[49;39m");
					printf(" Failed to remove a customer.\n");
				}
				decrementStopCount();
				if(_children.size() == 0 and _arrangement.size() == 0){
					should_remove_from_parent = true;
				}
				return true;
			}
		}
		if(!removeCustomerWithIdAndTable(word_id, tables.size() - 1)){
			printf("\x1b[41;97m");
			printf("WARNING");
			printf("\x1b[49;39m");
			printf(" Failed to remove a customer.\n");
		}
		decrementStopCount();

		if(_children.size() == 0 and _arrangement.size() == 0){
			should_remove_from_parent = true;
		}
		return true;
	}

	double Pw(id word_id, double g0, vector<double> &d_m, vector<double> &theta_m){
		if(word_id == 0){
			return 1;
		}
		if(_depth >= d_m.size()){
			while(d_m.size() <= _depth){
				d_m.push_back(PYLM_INITIAL_D);
			}
			while(theta_m.size() <= _depth){
				theta_m.push_back(PYLM_INITIAL_THETA);
			}
		}

		double d_u = d_m[_depth];
		double theta_u = theta_m[_depth];
		double t_u = (double)_num_tables;
		double c_u = (double)_num_customers;
		double mult = (theta_u + d_u * t_u) / (theta_u + c_u);
		if(_arrangement.find(word_id) == _arrangement.end()){
			if(_parent != NULL){
				return _parent->Pw(word_id, g0, d_m, theta_m) * mult;
			}
			return g0 * mult;
		}
		if(_parent != NULL){
			vector<int> &tables = _arrangement[word_id];
			double c_uw = std::accumulate(tables.begin(), tables.end(), 0);
			double t_uw = tables.size();

			double first_coeff = (c_uw - d_u * t_uw) / (theta_u + c_u);
			if(first_coeff < 0){
				first_coeff = 0;
			}
			double second_coeff = (theta_u + d_u * t_u) / (theta_u + c_u);

			double parent_p = _parent->Pw(word_id, g0, d_m, theta_m);
			double p = first_coeff + second_coeff * parent_p;
			return p;
		}

		vector<int> &tables = _arrangement[word_id];
		double c_uw = std::accumulate(tables.begin(), tables.end(), 0);
		double t_uw = tables.size();

		double first_coeff = (c_uw - d_u * t_uw) / (theta_u + c_u);
		if(first_coeff < 0){
			first_coeff = 0;
		}
		double second_coeff = (theta_u + d_u * t_u) / (theta_u + c_u);

		double p = first_coeff + second_coeff * g0;
		return p;

	}
	double p_stop(double beta_stop, double beta_pass){
		double p = (_stop_count + beta_stop) / (_stop_count + _pass_count + beta_stop + beta_pass);
		if(_parent != NULL){
			p *= _parent->p_pass(beta_stop, beta_pass);
		}
		return p;
	}
	double p_pass(double beta_stop, double beta_pass){
		double p = (_pass_count + beta_pass) / (_stop_count + _pass_count + beta_stop + beta_pass);
		if(_parent != NULL){
			p *= _parent->p_pass(beta_stop, beta_pass);
		}
		return p;
	}
	void incrementStopCount(){
		_stop_count++;
		if(_parent != NULL){
			_parent->incrementPassCount();
		}
	}
	void decrementStopCount(){
		_stop_count--;
		if(_stop_count < 0){
			printf("\x1b[41;97m");
			printf("WARNING");
			printf("\x1b[49;39m");
			printf(" _stop_count has fallen bellow 0.\n");
		}
		if(_parent != NULL){
			_parent->decrementPassCount();
		}
	}
	void incrementPassCount(){
		_pass_count++;
		if(_parent != NULL){
			_parent->incrementPassCount();
		}
	}
	void decrementPassCount(){
		_pass_count--;
		if(_pass_count < 0){
			printf("\x1b[41;97m");
			printf("WARNING");
			printf("\x1b[49;39m");
			printf(" _pass_count has fallen bellow 0.\n");
		}
		if(_parent != NULL){
			_parent->decrementPassCount();
		}
	}
	void deleteChildWithId(id word_id){
		Node* child = findChildWithId(word_id);
		if(child){
			_children.erase(word_id);
			delete child;
		}
		if(_children.size() == 0 && _arrangement.size() == 0){
			if(_parent != NULL){
				_parent->deleteChildWithId(this->_id);
			}
		}
	}
	int maxDepth(int base){
		int max_depth = base;
		for(auto elem: _children){
			int depth = elem.second->maxDepth(base + 1);
			if(depth > max_depth){
				max_depth = depth;
			}
		}
		return max_depth;
	}
	int numChildNodes(){
		int num = _children.size();
		for(auto elem: _children){
			num += elem.second->numChildNodes();
		}
		return num;
	}

	int numCustomers(){
		int num = 0;
		for(auto elem: _arrangement){
			vector<int> customers = elem.second;
			for(int i = 0;i < customers.size();i++){
				num += customers[i];
			}
		}
		if(num != _num_customers){
			printf("\x1b[41;97m");
			printf("WARNING");
			printf("\x1b[49;39m");
			printf(" _num_customers is broken.\n");
		}
		for(auto elem: _children){
			num += elem.second->numCustomers();
		}
		return num;
	}

	void setActiveKeys(unordered_map<id, bool> &keys){
		for(auto elem: _arrangement){
			id word_id = elem.first;
			keys[word_id] = true;
		}
		for(auto elem: _children){
			elem.second->setActiveKeys(keys);
		}
	}

	// dとθの推定用
	// "A Bayesian Interpretation of Interpolated Kneser-Ney" Appendix C参照
	// http://www.gatsby.ucl.ac.uk/~ywteh/research/compling/hpylm.pdf
	double auxiliary_log_x_u(double theta_u){
		if(_num_customers >= 2){
			double x_u = Sampler::beta(theta_u + 1, _num_customers - 1);
			return log(x_u + 1e-8);
		}
		return 0;
	}
	double auxiliary_y_ui(double d_u, double theta_u){
		if(_num_tables >= 2){
			double sum_y_ui = 0;
			for(int i = 0;i < _num_tables - 1;i++){
				double denominator = theta_u + d_u * (double)i;
				if(denominator == 0){
					printf("\x1b[41;97m");
					printf("WARNING");
					printf("\x1b[49;39m");
					printf(" Division by zero.\n");
					continue;
				}
				sum_y_ui += Sampler::bernoulli(theta_u / denominator);;
			}
			return sum_y_ui;
		}
		return 0;
	}
	double auxiliary_1_y_ui(double d_u, double theta_u){
		if(_num_tables >= 2){
			double sum_1_y_ui = 0;
			for(int i = 0;i < _num_tables - 1;i++){
				double denominator = theta_u + d_u * (double)i;
				if(denominator == 0){
					printf("\x1b[41;97m");
					printf("WARNING");
					printf("\x1b[49;39m");
					printf(" Division by zero.\n");
					continue;
				}
				sum_1_y_ui += 1.0 - Sampler::bernoulli(theta_u / denominator);
			}
			return sum_1_y_ui;
		}
		return 0;
	}
	double auxiliary_1_z_uwkj(double d_u){
		double sum_z_uwkj = 0;
		// c_u..
		for(auto elem: _arrangement){
			// c_uw.
			vector<int> &tables = elem.second;
			for(int k = 0;k < tables.size();k++){
				// c_uwk
				int c_uwk = tables[k];
				if(c_uwk >= 2){
					for(int j = 0;j < c_uwk - 1;j++){
						if(j - d_u == 0){
							printf("\x1b[41;97m");
							printf("WARNING");
							printf("\x1b[49;39m");
							printf(" Division by zero.\n");
							continue;
						}
						sum_z_uwkj += 1 - Sampler::bernoulli((j - 1) / (j - d_u));
					}
				}
			}
		}
		return sum_z_uwkj;
	}
	friend ostream& operator<<(ostream& os, const Node& node){
		os << "[Node." << node._identifier << ":id." << node._id << ":depth." << node._depth << "]" << endl;
		os << "_num_tables: " << node._num_tables << ", _num_customers: " << node._num_customers << endl;
		os << "_stop_count: " << node._stop_count << ", _pass_count: " << node._pass_count << endl;
		os << "- _arrangement" << endl;
		for(auto elem: node._arrangement){
			os << "  [" << elem.first << "]" << endl;
			os << "    ";
			for(auto customers: elem.second){
				os << customers << ",";
			}
			os << endl;
		}
		os << endl;
		os << "- _children" << endl;
		os << "    ";
		for(auto elem: node._children){
			os << "  [Node." << elem.second->_identifier << ":id." << elem.first << "]" << ",";
		}
		os << endl;
		return os;
	}
};

id Node::_auto_increment = 1;

#endif