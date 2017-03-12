#ifndef _vocab_
#define _vocab_
#include <boost/serialization/serialization.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <cstdlib>
#include <unordered_map>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <functional>
#include "c_printf.h"
#include "common.h"

using namespace std;

class Vocab{
private:
	id _auto_increment;
	unordered_map<id, wstring> _string_by_token_id;
	hash<wstring> _hash_func;
	
	friend class boost::serialization::access;
	template <class Archive>
	void serialize(Archive& archive, unsigned int version)
	{
		static_cast<void>(version); // No use
		archive & _string_by_token_id;
		archive & _auto_increment;
	}
public:
	Vocab(){
		_auto_increment = ID_EOS;
		_string_by_token_id[ID_BOS] = L"<bos>";
		_string_by_token_id[ID_EOS] = L"<eos>";
	}
	id add_string(wstring &str){
		id token_id = string_to_token_id(str);
		_string_by_token_id[token_id] = str;
		return token_id;
	}
	id string_to_token_id(wstring &str){
		return (id)_hash_func(str);
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
		oarchive << static_cast<const Vocab&>(*this);
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

#endif