#ifndef _vocab_
#define _vocab_
#include <stdlib.h>
#include <unordered_map>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <boost/serialization/serialization.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>

using namespace std;
using id = unsigned long;

class Vocab{
private:
	// wstring -> characters -> word
	unordered_map<wchar_t, id> char_dict;
	unordered_map<id, wchar_t> char_dict_inv;
	unordered_map<id, vector<id> > word_dict;
	map<vector<id>, id> word_dict_inv;
	vector<id> reusable_ids;
	vector<id> null_word;
	id _auto_increment;
	id bos_id;
	id eos_id;
	
	friend class boost::serialization::access;
	template <class Archive>
	void serialize(Archive& archive, unsigned int version)
	{
		static_cast<void>(version); // No use
		archive & char_dict;
		archive & char_dict_inv;
		archive & word_dict;
		archive & word_dict_inv;
		archive & reusable_ids;
		archive & _auto_increment;
	}

public:
	Vocab(){
		bos_id = 0;
		eos_id = 1;
		char_dict_inv[bos_id] = L'^';
		char_dict_inv[eos_id] = L'$';
		_auto_increment = eos_id;
		null_word = {0};
	}
	id generateId(){
		if(reusable_ids.size() == 0){
			_auto_increment++;
			return _auto_increment;
		}
		id used_id = reusable_ids.back();
		reusable_ids.pop_back();
		return used_id;
	}
	void addCharacter(wchar_t ch){
		if(char_dict.find(ch) == char_dict.end()){
			id gen_id = generateId();
			char_dict[ch] = gen_id;
			char_dict_inv[gen_id] = ch;
			if(word_dict.find(gen_id) == word_dict.end()){
				word_dict[gen_id] = {gen_id};
			}
		}
	}
	id char2id(wchar_t ch){
		auto itr = char_dict.find(ch);
		if(itr == char_dict.end()){
			id gen_id = generateId();
			char_dict[ch] = gen_id;
			char_dict_inv[gen_id] = ch;
			return gen_id;
		}
		return itr->second;
	}
	wchar_t id2char(id id){
		if(char_dict_inv.find(id) == char_dict_inv.end()){
			return L' ';
		}
		return char_dict_inv[id];
	}
	id characters2word(vector<id> &char_ids){
		if(char_ids.size() == 1){
			return char_ids[0];
		}
		auto itr = word_dict_inv.find(char_ids);
		if(itr == word_dict_inv.end()){
			id gen_id = generateId();
			word_dict_inv[char_ids] = gen_id;
			// cout << "word vocab generated " << gen_id << endl;
			word_dict[gen_id] = char_ids;
			return gen_id;
		}
		return itr->second;
	}
	vector<id> &word2characters(id word_id){
		auto itr = word_dict.find(word_id); 
		if (itr == word_dict.end()) {
			return null_word;
		}
		return itr->second;
	}
	wstring word2string(id word_id){
		if(word_dict.find(word_id) == word_dict.end()){
			if(char_dict_inv.find(word_id) == char_dict_inv.end()){
				return L"";
			}
			wstring str;
			str += id2char(word_id);
			return str;
		}
		vector<id> &char_ids = word_dict[word_id];
		return characters2string(char_ids);
	}
	wstring characters2string(vector<id> &char_ids){
		wstring str;
		for(int i = 0;i < char_ids.size();i++){
			str += id2char(char_ids[i]);
		}
		return str;
	}
	void string2characters(wstring str, vector<id> &ids){
		ids.clear();
		for(int i = 0;i < str.length();i++){
			ids.push_back(char2id(str[i]));
		}
	}
	int numWords(){
		return word_dict.size();
	}
	int numCharacters(){
		return char_dict.size();
	}
	id eosId(){
		return eos_id;
	}
	id bosId(){
		return bos_id;
	}
	id autoIncrement(){
		return _auto_increment;
	}
	void save(string dir = "model/"){
		string filename = dir + "vocab";
		std::ofstream ofs(filename);
		boost::archive::binary_oarchive oarchive(ofs);
		oarchive << static_cast<const Vocab&>(*this);
		cout << "saved to " << filename << endl;
		cout << "	num_words: " << numWords() << endl;
		cout << "	num_characters: " << numCharacters() << endl;
		cout << "	auto_increment: " << _auto_increment << endl;
		cout << "	num_reusable_ids: " << reusable_ids.size() << endl;
	}

	void load(string dir = "model/"){
		string filename = dir + "vocab";
		std::ifstream ifs(filename);
		if(ifs.good()){
			cout << "loading " << filename << endl;
			boost::archive::binary_iarchive iarchive(ifs);
			iarchive >> *this;
			cout << "	num_words: " << numWords() << endl;
			cout << "	num_characters: " << numCharacters() << endl;
			cout << "	auto_increment: " << _auto_increment << endl;
			cout << "	num_reusable_ids: " << reusable_ids.size() << endl;
		}
	}
	void dump(){
		for(auto kv : char_dict_inv) {
			int key = kv.first;
			wchar_t charactor = kv.second;
			wcout << key << ": " << charactor << endl;
		} 
	}
	void clean(unordered_map<id, bool> &active_keys){
		unordered_map<id, vector<id> > new_word_dict;
		map<vector<id>, id> new_word_dict_inv;
		
		for(auto elem: word_dict){
			id word_id = elem.first;
			if(active_keys.find(word_id) == active_keys.end()){
				if(char_dict_inv.find(word_id) == char_dict_inv.end()){
					reusable_ids.push_back(word_id);
				}
			}else{
				vector<id> dict = word_dict[word_id];
				new_word_dict[word_id] = dict;
				new_word_dict_inv[dict] = word_id;
			}
		}

		word_dict.clear();
		word_dict_inv.clear();
		word_dict = new_word_dict;
		word_dict_inv = new_word_dict_inv;
	}
};

#endif