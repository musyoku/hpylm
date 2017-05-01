#pragma once
#include <unordered_map>
#include "hashmap.h"
template<class T, class U>
// using hashmap = std::unordered_map<T, U>;
using hashmap = emilib::HashMap<T, U>;

#define HPYLM_INITIAL_D 	0.2
#define HPYLM_INITIAL_THETA 2.0
#define HPYLM_INITIAL_A 	1.0
#define HPYLM_INITIAL_B 	1.0
#define HPYLM_INITIAL_ALPHA 1.0
#define HPYLM_INITIAL_BETA  1.0

using id = size_t;
#define ID_BOS 0
#define ID_EOS 1
