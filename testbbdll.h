#ifndef TESTBBDLL_H
#define TESTBBDLL_H

#include "Path.h"

#include <iostream>
#include <list>
#include <vector>
#include <set>
#include <functional>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <cassert>
#include <stdexcept>
#include <concepts>
#include <type_traits>

#define uint unsigned int

template<typename T>
concept ZeroConstructible = requires {
    T{0};
    T(0);
};



template<ZeroConstructible T>
class BBDLL {
	public:
	class Block;
	
	class Node {
		public:
		uint key;
		T val;
		Block *block;
		
		Node(uint k = 0, T v = 0, Block *b = nullptr):
			key(k), val(v), block(b) {}
			
		bool operator < (const Node& t) const { return val < t.val; }
	};
	
	uint counter_ = 0;
	
	class Block {
		public:
		bool area_;// Block 在哪一个 D 中: 0/1 ~ D0/D1
		T bound_;
		uint idx_;
		size_t size_;
		std::list<Node> lst_;
		
		Block(uint counter, T b = 0, bool area = 0):
			bound_(b), idx_(counter), size_(0), area_(area) {}
		Block(uint counter, std::list<Node>&& l, T b, bool area):
			bound_(b), idx_(counter), size_(0), area_(area),
			lst_(std::move(l)) {
				for(auto& node: lst_) {
					size_++;
					node.block = this;
				}
			}

		bool operator < (const Block& b) const {
			return bound_ == b.bound_ ? idx_ < b.idx_ : bound_ < b.bound_;
		}
		
		std::list<Node>::iterator insert(uint k, const T& v) {
			++size_;
			lst_.emplace_back(k, v, this);
//			std::cout << "braek9-2\n";
			return std::prev(lst_.end());
		}
		
		std::unique_ptr<Block> split(uint counter) {
			lst_.sort();
			auto it = lst_.begin();
			std::advance(it, size_+1>>1);
			std::list<Node> nl;
			nl.splice(nl.begin(), lst_, lst_.begin(), it);
			size_ -= size_+1>>1;
			return std::make_unique<Block>(counter, std::move(nl), std::prev(nl.end())->val, area_);
		}
	};
	
	struct BlockCompare {
		using is_transparent = void;
		
		bool operator()(const std::unique_ptr<Block>& a, const std::unique_ptr<Block>& b) const {
			return a->bound_ == b->bound_ ? a->idx_ < b->idx_ : a->bound_ < b->bound_;
	    }
	    
	    bool operator()(const std::unique_ptr<Block>& a, T bound) const {
	        return a->bound_ < bound;
	    }
	    
	    bool operator()(T bound, const std::unique_ptr<Block>& b) const {
	        return bound < b->bound_;
	    }
	    
	    bool operator()(const Block* a, const std::unique_ptr<Block>& b) const {
			return a->bound_ == b->bound_ ? a->idx_ < b->idx_ : a->bound_ < b->bound_;
	    }
	    
		bool operator()(const std::unique_ptr<Block>& a, const Block* b) const {
			return a->bound_ == b->bound_ ? a->idx_ < b->idx_ : a->bound_ < b->bound_;
	    }
	};
	
	public:
	T Bound_;
	size_t Mbs_, size_;
	std::unordered_map<uint,typename std::list<Node>::iterator> hash_;
	std::set<std::unique_ptr<Block>, BlockCompare> D1;
	std::vector<std::unique_ptr<Block> > D0;
	
	private:
	
	void init() {
		D1.emplace(std::make_unique<Block>(counter_++, Bound_, 1));
		size_ = 0;
	}
	
	public:
	
	BBDLL (const T& bound, size_t mbs):
		Bound_(bound), Mbs_(mbs) { init(); }
		
	size_t size() const { return size_; }
	
	void insert(uint k, const T& v) {
		++ size_;
//		std::cout << "braek6\n";
		if(auto it = hash_.find(k); it != hash_.end()) {
//			std::cout << "break7\n";
			// 键已经存在
			if(v >= it->second->val && it->second->block != nullptr) {
				// 原有值不大于插入值，且该键值对存在于DS中
				-- size_;
				return;
			}
			
			if(it->second->block != nullptr) {
				// 原有值大于插入值，且该键值对存在于DS中
				-- size_;
				Block *itb = it->second->block;
				itb->lst_.erase(it->second);// it 已经失效
				if(--itb->size_ == 0 && itb->idx_ > 0 && itb->area_ ==1) {
					
//					std::cout << "waring3 " << itb->idx_ << "\n";
					if(auto dit = D1.find(itb); dit != D1.end()) D1.erase(dit);
				}
			}
		}
//		std::cout << "break8\n";
		// 插入元素
		auto block = D1.lower_bound(v);
		
//		std::cout << block->get()->idx_ << "\n";
		
//		std::cout << "break9\n";
//		auto temp = block->get()->insert(k, v);
//		std::cout << "break9-1\n";
//		hash_[k] = temp;
		hash_[k] = block->get()->insert(k, v);
//		std::cout << "break10\n";
		if(block->get()->size_ > Mbs_)  {
			D1.emplace(std::move(block->get()->split(counter_++)));
		}
//		std::cout << "break11\n";
	}
	
	void batchPrepend(const std::set<std::pair<uint, T> > &data) {
		std::list<Node> lst;
		for(auto [k, v]: data) {
			++ size_;
			if(auto it = hash_.find(k); it != hash_.end()) {
				if(it->second->block != nullptr) {
					-- size_;
					Block *itb = it->second->block;
					itb->lst_.erase(it->second);
					if(--itb->size_ == 0 && itb->area_ == 1 && itb->idx_ > 0) {
//						std::cout << "waring2 " << itb->idx_ << "\n";
						auto dit = D1.find(itb);
						if(dit != D1.end()) D1.erase(dit);
					}
				}
			}
			lst.push_back(Node(k, v));
			hash_[k] = std::prev(lst.end());
		}
		lst.sort();
		
		// 直接分割成块，从后往前处理以保持D0的顺序（上界大的在前）
		size_t remaining = lst.size();
		
		while (remaining > 0) {
			size_t block_size = std::min(Mbs_, remaining);
			std::list<Node> block_lst;
			
			// 从lst末尾往前取block_size个元素
			auto start_it = lst.end();
			std::advance(start_it, -block_size);
			block_lst.splice(block_lst.begin(), lst, start_it, lst.end());
			
			// 创建块，bound是最后一个元素的val（现在是最大的）
			T bound = block_lst.back().val;
			D0.emplace_back(std::make_unique<Block>(counter_++, std::move(block_lst), bound, 0));
			
			remaining -= block_size;
		}
	}
	
	std::pair<T, std::set<uint> > pull() {

		size_t c1 = 0, c0 = 0;
		std::list<Node> nl;
		
		D0.erase(
			std::remove_if(D0.begin(), D0.end(), [](const std::unique_ptr<Block>& ptr) {
				return !ptr || ptr->size_ == 0;
			}),
			D0.end()
		);
		
		for(int i = D0.size()-1; i >= 0; --i) {
			auto& block = D0[i];
			c0 += block->size_;
			for(const auto& cur: block->lst_) {
				nl.emplace_back(cur.key, cur.val);
			}
			if(c0 >= Mbs_) break;
		}
		
		for(const auto& block: D1) {
			c1 += block->size_;
			for(const auto& cur: block->lst_) {
				nl.emplace_back(cur.key, cur.val);
			}
			if(c1 >= Mbs_) break;
		}
		
		if(c0 + c1 == 0) {
			return {0, {}};
		}
		
		nl.sort();
		auto tar = nl.begin();

		std::set<uint> ret1;

		std::advance(tar, std::min(c0 + c1, Mbs_)-1);
		T ret2 = tar->val;

		for(auto it = nl.begin();; ++it) {
			ret1.insert(it->key);
			--size_;
			auto it2 = hash_.find(it->key);
			Block *itb = it2->second->block;
			itb->lst_.erase(it2->second);
			hash_.erase(it2);
			if(--itb->size_ == 0 && itb->area_ == 1 && itb->idx_ > 0) {
//				std::cout << "waring1 " << itb->idx_ << "\n";
				if(auto dit = D1.find(itb); dit != D1.end()) D1.erase(dit);
			}
			if(it == tar) break;
		}

		// 添加极小增量，使得 ret2 > all in ret1 and ret2 <= all not in ret1
		if constexpr(std::is_same_v<T, Path<double> >) {
			ret2.prev++;
		} else
		if constexpr(std::is_same_v<T, double>) {
			ret2 += 1e-5;
		} else
		if constexpr(std::is_same_v<T, int>) {
			ret2 += 1;
		}
		
		return {size_ == 0 ? Bound_ : ret2, ret1};
	}
	
	
	void print() {
		std::cout << Mbs_ << " " << Bound_ << " " << size_ << "\n";
		
		
		std::cout << "visit ele by key:\n";
		for(const auto& it: hash_) {
			std::cout << "<" << it.second->key << ", " << it.second->val << ", " << it.second->block->idx_ << ">\n";
		}
		
		std::cout << "visit block in D1:\n";
		
		for(const auto& b : D1) {
			std::cout << b->idx_ << " " << b->size_ << " " << b->bound_ << " " << b->area_ << "\n";
			for(auto u : b->lst_) {
				std::cout << "<" << u.key << ", " << u.val << ">\n";
			}
		}
		
		std::cout << "visit block in D0:\n";
		
		for(const auto& b : D0) {
			std::cout << b->idx_ << " " << b->size_ << " " << b->bound_ << " " << b->area_ << "\n";
			for(auto u : b->lst_) {
				std::cout << "<" << u.key << ", " << u.val << ">\n";
			}
		}
		
		std::cout << "print over\n";
	}
};


#undef uint
#endif
