#include <vector>
#include <string>
#include "core/vocab.h"
using namespace std;

vector<wstring> split(const wstring &s, char delim){
    vector<wstring> elems;
    wstring item;
    for(char ch: s){
        if (ch == delim){
            if (!item.empty())
                elems.push_back(item);
            item.clear();
        }
        else{
            item += ch;
        }
    }
    if (!item.empty())
        elems.push_back(item);
    return elems;
}

// スペースで分割する
void load_words_in_textfile(string &filename, vector<vector<id>> &dataset, Vocab* &vocab, int padding_bos){
	wifstream ifs(filename.c_str());
	wstring str;
	if (ifs.fail()){
		c_printf("[R]%s", "エラー");
		c_printf("[n] %s%s\n", filename.c_str(), "を開けません.");
		exit(1);
	}
	vocab = new Vocab();
	id bos_id = vocab->add_string(L"<bos>");
	id eos_id = vocab->add_string(L"<eos>");
	id unk_id = vocab->add_string(L"<unk>");
	while (getline(ifs, str) && !str.empty()){
		vector<wstring> words = split(str, ' ');
		vector<id> token_ids;
		// HPYLMでは深さが固定なので先頭にダミーを挿入する
		for(int i = 0;i < padding_bos;i++){
			token_ids.push_back(bos_id);
		}
		for(auto word: words){
			if(word.size() == 0){
				continue;
			}
			id token_id = vocab->add_string(word);
			token_ids.push_back(token_id);
		}
		token_ids.push_back(eos_id);
		if(token_ids.size() > 0){
			dataset.push_back(token_ids);
		}
	}
	cout << filename << "を読み込みました（" << dataset.size() << "行）" << endl;
}

// 文字n-gramの学習
void load_characters_in_textfile(string &filename, vector<vector<id>> &dataset, Vocab* &vocab, int ngram){
	wifstream ifs(filename.c_str());
	wstring str;
	if (ifs.fail()){
		c_printf("[R]%s", "エラー");
		c_printf("[n] %s%s\n", filename.c_str(), "を開けません.");
		exit(1);
	}
	vocab = new Vocab();
	id bos_id = vocab->add_string(L"<bos>");
	id eos_id = vocab->add_string(L"<eos>");
	id unk_id = vocab->add_string(L"<unk>");
	while (getline(ifs, str) && !str.empty()){
		vector<id> token_ids;
		// HPYLMでは深さが固定なので先頭にダミーを挿入する
		for(int i = 0;i < ngram;i++){
			token_ids.push_back(bos_id);
		}
		if(str.size() == 0){
			continue;
		}
		for(int i = 0;i < str.size();i++){
			id token_id = vocab->add_string(wstring(str.begin() + i, str.begin() + i + 1));
			token_ids.push_back(token_id);
		}
		token_ids.push_back(eos_id);
		if(token_ids.size() > 0){
			dataset.push_back(token_ids);
		}
	}
	cout << filename << "を読み込みました（" << dataset.size() << "行）" << endl;
}

void show_progress(int step, int total){
	double progress = step / (double)(total - 1);
	int barWidth = 30;

	cout << "[";
	int pos = barWidth * progress;
	for(int i = 0; i < barWidth; ++i){
		if (i < pos) cout << "=";
		else if (i == pos) cout << ">";
		else cout << " ";
	}
	cout << "] " << int(progress * 100.0) << " %\r";
	cout.flush();
}