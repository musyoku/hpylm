#include <boost/python.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <string>
#include <unordered_map> 
#include "core/c_printf.h"
#include "core/node.h"
#include "core/hpylm.h"
#include "core/vocab.h"

using namespace boost;

void split_word_by(const wstring &str, wchar_t delim, vector<wstring> &elems){
	elems.clear();
	wstring item;
	for(wchar_t ch: str){
		if (ch == delim){
			if (!item.empty()){
				elems.push_back(item);
			}
			item.clear();
		}
		else{
			item += ch;
		}
	}
	if (!item.empty()){
		elems.push_back(item);
	}
}

template<class T>
python::list list_from_vector(vector<T> &vec){  
	 python::list list;
	 typename vector<T>::const_iterator it;

	 for(it = vec.begin(); it != vec.end(); ++it)   {
		  list.append(*it);
	 }
	 return list;
}

class PyHPYLM{
private:
	HPYLM* _hpylm;
	Vocab* _vocab;
	vector<vector<id>> _dataset_train;
	vector<vector<id>> _dataset_test;
	vector<int> _rand_indices;
	// 統計
	unordered_map<id, int> _word_count;
	int _sum_word_count;
	bool _gibbs_first_addition;

public:
	PyHPYLM(int ngram){
		setlocale(LC_CTYPE, "ja_JP.UTF-8");
		ios_base::sync_with_stdio(false);
		locale default_loc("ja_JP.UTF-8");
		locale::global(default_loc);
		locale ctype_default(locale::classic(), default_loc, locale::ctype); //※
		wcout.imbue(ctype_default);
		wcin.imbue(ctype_default);

		_hpylm = new HPYLM(ngram);
		_vocab = new Vocab();
		_gibbs_first_addition = true;
	}
	void load_textfile(string filename, int train_split){
		c_printf("[*]%s\n", (boost::format("%sを読み込んでいます ...") % filename.c_str()).str().c_str());
		wifstream ifs(filename.c_str());
		wstring line_str;
		if (ifs.fail()){
			c_printf("[R]%s [*]%s", "エラー", (boost::format("%sを開けません.") % filename.c_str()).str().c_str());
			exit(1);
		}
		vector<wstring> lines;
		while (getline(ifs, line_str) && !line_str.empty()){
			if (PyErr_CheckSignals() != 0) {		// ctrl+cが押されたかチェック
				return;
			}
			lines.push_back(line_str);
		}
		assert(lines.size() > train_split);
		vector<int> rand_indices;
		for(int i = 0;i < lines.size();i++){
			rand_indices.push_back(i);
		}
		shuffle(rand_indices.begin(), rand_indices.end(), Sampler::mt);	// データをシャッフル
		for(int i = 0;i < rand_indices.size();i++){
			wstring &line_str = lines[rand_indices[i]];
			if(i < train_split){
				add_train_data(line_str);
			}else{
				add_test_data(line_str);
			}
		}
		c_printf("[*]%s\n", (boost::format("%sを読み込みました.") % filename.c_str()).str().c_str());
	}
	void add_train_data(wstring line_str){
		_add_data_to(line_str, _dataset_train);
	}
	void add_test_data(wstring line_str){
		_add_data_to(line_str, _dataset_test);
	}
	void _add_data_to(wstring &line_str, vector<vector<id>> &dataset){
		vector<wstring> word_str_array;
		split_word_by(line_str, L' ', word_str_array);	// スペースで分割
		if(word_str_array.size() > 0){
			vector<id> words;
			for(auto word_str: word_str_array){
				if(word_str.size() == 0){
					continue;
				}
				id token_id = _vocab->string_to_token_id(word_str);
				words.push_back(token_id);
				_word_count[token_id] += 1;
				_sum_word_count += 1;
			}
			words.push_back(ID_EOS);
			_word_count[ID_EOS] += 1;
			_sum_word_count += 1;
			dataset.push_back(words);
		}
	}
	void set_g0(double g0){
		_hpylm->_g0 = g0;
	}
	void load(string dirname){
		_vocab->load(dirname + "hpylm.vocab");
		_hpylm->load(dirname + "hpylm.model");
	}
	void save(string dirname){
		_vocab->save(dirname + "hpylm.vocab");
		_hpylm->save(dirname + "hpylm.model");
	}
	void perform_gibbs_sampling(){
		if(_rand_indices.size() != _dataset_train.size()){
			_rand_indices.clear();
			for(int data_index = 0;data_index < _dataset_train.size();data_index++){
				_rand_indices.push_back(data_index);
			}
		}
		shuffle(_rand_indices.begin(), _rand_indices.end(), Sampler::mt);	// データをシャッフル
		for(int n = 0;n < _dataset_train.size();n++){
			if (PyErr_CheckSignals() != 0) {		// ctrl+cが押されたかチェック
				return;
			}
			int data_index = _rand_indices[n];
			vector<id> &token_ids = _dataset_train[data_index];
			for(int token_t_index = _hpylm->ngram() - 1;token_t_index < token_ids.size();token_t_index++){
				if(_gibbs_first_addition == false){
					_hpylm->remove_customer_at_timestep(token_ids, token_t_index);
					_gibbs_first_addition = true;
				}
				_hpylm->add_customer_at_timestep(token_ids, token_t_index);
			}
		}
	}
	int get_num_train_data(){
		return _dataset_train.size();
	}
	int get_num_test_data(){
		return _dataset_test.size();
	}
	int get_max_depth(){
		return _hpylm->get_max_depth();
	}
	int get_num_nodes(){
		return _hpylm->get_num_nodes();
	}
	int get_num_customers(){
		return _hpylm->get_num_customers();
	}
	int get_num_types_of_words(){
		return _word_count.size();
	}
	int get_num_words(){
		return _sum_word_count;
	}
	python::list count_tokens_of_each_depth(){
		unordered_map<int, int> counts_by_depth;
		_hpylm->count_tokens_of_each_depth(counts_by_depth);

		// ソート
		std::map<int, int> sorted_counts_by_depth(counts_by_depth.begin(), counts_by_depth.end());

		std::vector<int> counts;
		for(auto it = sorted_counts_by_depth.begin(); it != sorted_counts_by_depth.end(); ++it){
			counts.push_back(it->second);
		}
		return list_from_vector(counts);
	}
	python::list get_discount_parameters(){
		return list_from_vector(_hpylm->_d_m);
	}
	python::list get_strength_parameters(){
		return list_from_vector(_hpylm->_theta_m);
	}
	void sample_hyperparameters(){
		_hpylm->sample_hyperparams();
	}
	id sample_next_token(python::list &_context_token_ids){
		std::vector<id> context_token_ids;
		int len = python::len(_context_token_ids);
		for(int i = 0; i<len; i++) {
			context_token_ids.push_back(python::extract<id>(_context_token_ids[i]));
		}
		return _hpylm->sample_next_token(context_token_ids);
	}
	// データセット全体の対数尤度を計算
	double compute_log_Pdataset_train(){
		return _compute_log_Pdataset(_dataset_train);
	}
	double compute_log_Pdataset_test(){
		return _compute_log_Pdataset(_dataset_test);
	}
	double _compute_log_Pdataset(vector<vector<id>> &dataset){
		double log_Pdataset = 0;
		for(int data_index = 0;data_index < dataset.size();data_index++){
			if (PyErr_CheckSignals() != 0) {		// ctrl+cが押されたかチェック
				return 0;
			}
			vector<id> &token_ids = dataset[data_index];
			log_Pdataset += _hpylm->compute_log_Pw(token_ids);;
		}
		return log_Pdataset;
	}
	double compute_perplexity_train(){
		return _compute_perplexity(_dataset_train);
	}
	double compute_perplexity_test(){
		return _compute_perplexity(_dataset_test);
	}
	double _compute_perplexity(vector<vector<id>> &dataset){
		double log_Pdataset = 0;
		for(int data_index = 0;data_index < dataset.size();data_index++){
			if (PyErr_CheckSignals() != 0) {		// ctrl+cが押されたかチェック
				return 0;
			}
			vector<id> &token_ids = dataset[data_index];
			log_Pdataset += _hpylm->compute_log_Pw(token_ids) / token_ids.size();
		}
		return exp(-log_Pdataset / (double)dataset.size());
	}
};

BOOST_PYTHON_MODULE(model){
	python::class_<PyHPYLM>("hpylm", python::init<int>())
	.def("set_g0", &PyHPYLM::set_g0)
	.def("load_textfile", &PyHPYLM::load_textfile)
	.def("perform_gibbs_sampling", &PyHPYLM::perform_gibbs_sampling)
	.def("get_max_depth", &PyHPYLM::get_max_depth)
	.def("get_num_nodes", &PyHPYLM::get_num_nodes)
	.def("get_num_customers", &PyHPYLM::get_num_customers)
	.def("get_discount_parameters", &PyHPYLM::get_discount_parameters)
	.def("get_strength_parameters", &PyHPYLM::get_strength_parameters)
	.def("get_num_train_data", &PyHPYLM::get_num_train_data)
	.def("get_num_test_data", &PyHPYLM::get_num_test_data)
	.def("get_num_types_of_words", &PyHPYLM::get_num_types_of_words)
	.def("get_num_words", &PyHPYLM::get_num_words)
	.def("sample_hyperparameters", &PyHPYLM::sample_hyperparameters)
	.def("sample_next_token", &PyHPYLM::sample_next_token)
	.def("count_tokens_of_each_depth", &PyHPYLM::count_tokens_of_each_depth)
	.def("compute_log_Pdataset_train", &PyHPYLM::compute_log_Pdataset_train)
	.def("compute_log_Pdataset_test", &PyHPYLM::compute_log_Pdataset_test)
	.def("compute_perplexity_train", &PyHPYLM::compute_perplexity_train)
	.def("compute_perplexity_test", &PyHPYLM::compute_perplexity_test)
	.def("save", &PyHPYLM::save)
	.def("load", &PyHPYLM::load);
}