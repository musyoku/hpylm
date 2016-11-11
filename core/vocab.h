#ifndef _vocab_
#define _vocab_
#include <cstdlib>
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
#include "c_printf.h"

using namespace std;
using id = unsigned long long;

class Vocab{
private:
	// wstring -> characters -> word
	unordered_map<wstring, id> _token_id_by_string;
	unordered_map<id, wstring> _string_by_token_id;
	vector<id> _reusable_token_ids;
	id _auto_increment;
	
	friend class boost::serialization::access;
	template <class Archive>
	void serialize(Archive& archive, unsigned int version)
	{
		static_cast<void>(version); // No use
		archive & _token_id_by_string;
		archive & _string_by_token_id;
		archive & _reusable_token_ids;
		archive & _auto_increment;
	}
public:
	Vocab(){
		_auto_increment = 0;
	}
	id generate_token_id(){
		if(_reusable_token_ids.size() == 0){
			_auto_increment++;
			return _auto_increment;
		}
		id token_id = _reusable_token_ids.back();
		_reusable_token_ids.pop_back();
		return token_id;
	}
	id add_string(wstring str){
		auto itr = _token_id_by_string.find(str);
		if(itr == _token_id_by_string.end()){
			id token_id = generate_token_id();
			_token_id_by_string[str] = token_id;
			_string_by_token_id[token_id] = str;
			return token_id;
		}
		return itr->second;
	}
	id string_to_token_id(wstring str){
		auto itr = _token_id_by_string.find(str);
		if(itr == _token_id_by_string.end()){
			id token_id = generate_token_id();
			_token_id_by_string[str] = token_id;
			_string_by_token_id[token_id] = str;
			return token_id;
		}
		return itr->second;
	}
	wstring token_id_to_string(id token_id){
		if(_string_by_token_id.find(token_id) == _string_by_token_id.end()){
			c_printf("[R]%s", "エラー");
			printf(" 存在しないトークンIDです. _string_by_token_id.find(token_id) == _string_by_token_id.end()");
			exit(1);
		}
		return _string_by_token_id[token_id];
	}
	int num_tokens(){
		return _string_by_token_id.size();
	}
	void save(string filename){
		std::ofstream ofs(filename);
		boost::archive::binary_oarchive oarchive(ofs);
		oarchive << static_cast<const Vocab&>(*this);
		// cout << "saved to " << filename << endl;
		// cout << "	num_tokens: " << num_tokens() << endl;
		// cout << "	num_reusable_ids: " << _reusable_token_ids.size() << endl;
	}

	void load(string filename){
		std::ifstream ifs(filename);
		if(ifs.good()){
			// cout << "loading " << filename << endl;
			boost::archive::binary_iarchive iarchive(ifs);
			iarchive >> *this;
			// cout << "	num_tokens: " << num_tokens() << endl;
			// cout << "	num_reusable_ids: " << _reusable_token_ids.size() << endl;
		}
	}
	void dump(){
		for(auto elem : _string_by_token_id) {
			wcout << elem.first << ": " << elem.second << endl;
		} 
	}
	void remove_unused_token_ids(unordered_map<id, bool> &active_token_ids){
		for(auto elem: _string_by_token_id){
			id token_id = elem.first;
			if(active_token_ids.find(token_id) == active_token_ids.end()){
				if(_string_by_token_id.find(token_id) == _string_by_token_id.end()){
					_reusable_token_ids.push_back(token_id);
				}
			}else{

			}
		}
	}
};

#endif