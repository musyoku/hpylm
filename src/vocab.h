#pragma once
#include <boost/serialization/serialization.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include "common.h"

using namespace std;

class Vocab{
private:
	unordered_set<id> _token_ids;
	unordered_map<id, wstring> _string_by_token_id;
	hash<wstring> _hash_func;
	
public:
	Vocab(){
		_string_by_token_id[ID_BOS] = L"<bos>";
		_string_by_token_id[ID_EOS] = L"<eos>";
		_token_ids.insert(ID_BOS);
		_token_ids.insert(ID_EOS);
	}
	id add_string(wstring &str){
		id token_id = string_to_token_id(str);
		_string_by_token_id[token_id] = str;
		_token_ids.insert(token_id);
		return token_id;
	}
	id string_to_token_id(wstring &str){
		return (id)_hash_func(str);
	}
	unordered_set<id> & get_all_token_ids(){
		return _token_ids;
	}
	wstring token_id_to_string(id token_id){
		auto itr = _string_by_token_id.find(token_id);
		assert(itr != _string_by_token_id.end());
		return itr->second;
	}
	wstring token_ids_to_sentence(vector<id> &token_ids){
		wstring sentence = L"";
		for(const auto &token_id: token_ids){
			wstring word = token_id_to_string(token_id);
			sentence += word;
			sentence += L" ";
		}
		return sentence;
	}
	int num_tokens(){
		return _string_by_token_id.size();
	}
	void save(string filename = "hpylm.vocab"){
		std::ofstream ofs(filename);
		boost::archive::binary_oarchive oarchive(ofs);
		oarchive << *this;
	}
	template <class Archive>
	void serialize(Archive& archive, unsigned int version)
	{
		static_cast<void>(version); // No use
		archive & _string_by_token_id;
		archive & _token_ids;
	}
	void load(string filename = "hpylm.vocab"){
		std::ifstream ifs(filename);
		if(ifs.good()){
			boost::archive::binary_iarchive iarchive(ifs);
			iarchive >> *this;
		}
	}
	void dump(){
		for(auto elem : _string_by_token_id) {
			wcout << elem.first << ": " << elem.second << endl;
		} 
	}
};