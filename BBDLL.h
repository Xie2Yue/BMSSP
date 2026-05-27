#ifndef BBDLL_H
#define BBDLL_H


#include <iostream>
#include <set>
#include <vector>
#include <functional>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <cassert>
#include <stdexcept>
#include "Path.h"


#define uint unsigned int

template<typename T>
T getMidium(T a, T b, T c, T d, T e) {
	if(a > b) std::swap(a, b);
	if(c > d) std::swap(c, d);
	if(a > c) std::swap(a, c), std::swap(b, d);
	if(e < b) { return c < e ? std::min(e, d) : std::min(b, c); }
	else { return b < c ? std::min(c, e) : std::min(b, d); }
}

template<typename T>
T getMidium(T a, T b, T c, T d) {
	if(a > b) std::swap(a, b);
	if(c > d) std::swap(c, d);
	return std::min(std::max(a, c), std::min(b, d));
}

template<typename T>
T getMidium(T a, T b, T c) {
	if(a > b) std::swap(a, b);
	return c < b ? std::max(a, c) : std::max(a, b);
}

template<typename T = double>
class BBDLL {
	
	private:
	
	class Block;// 前向声明
	
	class Pkv{
		public:
		uint key;
		T val;
		Block *block;
		Pkv *pre, *nxt;
		Pkv (uint k = 0, T v = 0, Block *b = nullptr, Pkv *p = nullptr, Pkv *n = nullptr):
			key(k), val(v), block(b), pre(p), nxt(n) {}
		
		bool operator < (const Pkv& t) const { return val < t.val; }
	};
	
	class Block {
		public:
		static inline uint counter_ = 0;
		bool mode_;
		T bound_;
		uint idx_;
		uint size_;
		Pkv *dummy;	
			
		Block(T bound = 0, bool mode = 0) : bound_(bound), idx_(counter_++), size_(0), mode_(mode) {
			dummy = new Pkv();
			dummy->pre = dummy->nxt = dummy;
		}
		Block(T bound, Pkv* d, bool mode = 0): bound_(bound), idx_(counter_++), size_(0), dummy(d), mode_(mode) {
			for(Pkv* cur = dummy->nxt;cur != dummy; cur = cur->nxt) {
				cur->block = this;
				++size_;
			}
		}
		
		~Block() {
			delete dummy;
		}
		
		bool operator < (const Block& b) const {
			return bound_ == b.bound_ ? idx_ < b.idx_ : bound_ < b.bound_;
		}
		
		Pkv* insert(uint k, const T& v) {
			++size_;
			Pkv *ins = new Pkv(k, v, this);
			link(dummy->pre, ins);
			link(ins, dummy);
			return ins;
		}
		Pkv* insert (Pkv* ptr, const T& v) {
			++size_;
			ptr->val = v;
			ptr->block = this;
			link(dummy->pre, ptr);
			link(ptr, dummy);
			return ptr;
		}
		std::unique_ptr<Block> split(bool mode = 0) {
			Pkv* nd = new Pkv();
			Pkv* pos = BFPRT(dummy, size_, size_+1>>1);
			link(nd, dummy->nxt);
			link(dummy, pos->nxt);
			link(pos, nd);
			size_ -= size_+1>>1;
			return std::make_unique<Block>(pos->val, nd, mode);
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
	uint Mbs_;
	uint _size;
	std::unordered_map<uint, Pkv*>K;
	std::set<std::unique_ptr<Block>, BlockCompare> D1;
	std::vector<std::unique_ptr<Block> > D0;
	private:
	
	void init() {
		D1.emplace(std::make_unique<Block>(Bound_));
		_size = 0;
	}
	
	static void link(Pkv *a, Pkv* b){
		a->nxt = b;
		b->pre = a;
	}
	
	static Pkv* partition(Pkv* begin, const Pkv* end, std::function<bool(const Pkv&)>cmp) {
		Pkv* pre = begin->pre;
		Pkv* dummy = new Pkv();
		dummy->pre = dummy->nxt = dummy;
		for(Pkv* it = begin; it != end;) {
			if(cmp(*it)) {
				if(it == begin) begin = begin->nxt;
				link(it->pre, it->nxt);
				link(dummy->pre, it);
				it = it->nxt;
				link(dummy->pre->nxt, dummy);
			} else it = it->nxt;
		}
		if(dummy->pre == dummy) {
			delete dummy;// 释放链表哨兵
			return begin;
		} else {
			link(dummy->pre, begin);
			link(pre, dummy->nxt);
			delete dummy;// 释放链表哨兵
			return begin;
		}
	}
	
	static uint dis(Pkv* st, const Pkv* ed) {
		uint ret = 0;
		while(st != ed) {
			++ret;
			st = st->nxt;
		}
		return ret;
	}
	
	static Pkv* getNext(Pkv* st, uint k = 1) {
		for(int i = 0; i < k; ++i) {
			st = st->nxt;
		}
		return st;
	}
	
	
	static Pkv* BFPRT(Pkv* dummy, uint n, uint k) {
		if(n == 1){ return dummy->nxt; }
		if(k > n) { k = n; }
		Pkv*nd = new Pkv();
		nd->pre = nd->nxt = nd;
		Pkv* cur = dummy->nxt;
		uint m = 1;
		for(int i = 1; i <= n/5; ++i) {
			Pkv *t = new Pkv(0, getMidium(
				cur->val, cur->nxt->val, cur->nxt->nxt->val,
				cur->nxt->nxt->nxt->val, cur->nxt->nxt->nxt->nxt->val
			));
			link(nd->pre, t);
			link(t, nd);
			cur = cur->nxt->nxt->nxt->nxt->nxt;
			++m;
		}
		if(n%5 == 1) {
			Pkv *t = new Pkv(0, cur->val);
			link(nd->pre, t);
			link(t, nd);
		} else if(n%5 == 2) {
			Pkv *t = new Pkv(0, std::min(cur->val, cur->nxt->val));
			link(nd->pre, t);
			link(t, nd);
		} else if(n%5 == 3) {
			Pkv *t = new Pkv(0, getMidium(cur->val, cur->nxt->val, cur->nxt->nxt->val));
			link(nd->pre, t);
			link(t, nd);
		} else if(n%5 == 4) {
			Pkv *t = new Pkv(0, getMidium(cur->val, cur->nxt->val,
			cur->nxt->nxt->val, cur->nxt->nxt->nxt->val));
			link(nd->pre, t);
			link(t, nd);
		} else --m;
		
		T mid = BFPRT(nd, m, m+1>>1)->val;
		
		for(Pkv* cur = nd->nxt; cur!= nd;) {
			Pkv* now = cur;
			cur = cur->nxt;
			delete now;
		}
		delete nd;
		
		Pkv* mid1 = partition(dummy->nxt, dummy, [mid](const Pkv& x){
			return x.val < mid;
		});
		
		if(uint d1 = dis(dummy, mid1) - 1; d1 >= k) {
			Pkv* sd = new Pkv();
			link(sd, dummy->nxt);
			link(mid1->pre, sd);
			Pkv* rit = BFPRT(sd, d1, k);
			link(dummy, sd->nxt);
			link(sd->pre, mid1);
			delete sd;
			return rit;
		} else {
			Pkv* l2 = mid1->pre;
			Pkv* mid2 = partition(mid1, dummy, [mid](const Pkv& x){
				return x.val <= mid;
			});
			mid1 = l2->nxt;
			if(auto d2 = d1 + dis(mid1, mid2); d2 >= k) {
				return getNext(mid1, k -d1 - 1);
			} else {
				Pkv* sd = new Pkv();
				Pkv* m2p = mid2->pre;
				link(sd, mid2);
				link(dummy->pre, sd);
				Pkv* rit = BFPRT(sd, n - d2, k - d2);
				link(m2p, sd->nxt);
				link(sd->pre, dummy);
				delete(sd);
				return rit;
			}
		}
	}
	
	public:
	BBDLL (const T& bound, uint mbs): Bound_(bound), Mbs_(mbs) { init(); }
	
	~BBDLL() {
		for(auto& it : K) {
			delete it.second;
		}
	}
	
	uint size() {
		return _size;
	}
	
	void insert(uint k, const T& v) {
		++ _size;
		if(auto it = K.find(k); it != K.end()) {
			if(v >= it->second->val && it->second->block != nullptr) {
				-- _size;
				return;
			}
			
			if(it->second->block != nullptr) {
				-- _size;
				link(it->second->pre, it->second->nxt);
				-- it->second->block->size_;
				if(it->second->pre == it->second->nxt) {
					auto db = it->second->block;
					it->second->block = nullptr;
					if(db->idx_ > 0 && db->mode_ == 1) {
						auto dit = D1.find(db);
						if(dit != D1.end()) D1.erase(dit);
					}
				}				
			}

			auto block = D1.lower_bound(v);
			block->get()->insert(it->second, v);
			if(block->get()->size_>Mbs_) D1.emplace(std::move(block->get()->split(1)));
		} else {
			auto block = D1.lower_bound(v);
			K[k] = block->get()->insert(k, v);
			if(block->get()->size_>Mbs_) D1.emplace(std::move(block->get()->split(1)));
		}
	}
	
	void batchPrepend(std::set<std::pair<uint, T> >data) {
		T bound = 0;
		Pkv *nd = new Pkv();
		nd->pre = nd->nxt = nd;
		for(auto [k, v] : data) {
			++ _size;
			if(auto it = K.find(k); it != K.end()) {

				if(it->second->block != nullptr) {
					-- _size;
					link(it->second->pre, it->second->nxt);
					-- it->second->block->size_;
					if(it->second->pre == it->second->nxt) {
						auto db = it->second->block;
						it->second->block = nullptr;
						if(db->idx_ > 0 && db->mode_ == 1) {
							auto dit = D1.find(db);
							if(dit != D1.end()) D1.erase(dit);
						}
					}					
				}

				link(nd->pre, it->second);
				link(it->second, nd);
				it->second->val = v;
			} else {
				Pkv *node = new Pkv(k, v);
				link(nd->pre, node);
				link(node, nd);
				K[k] = node;
			}
			bound = std::max(bound, v);
		}
		
		D0.emplace_back(std::make_unique<Block>(bound, nd));
		
		auto split = [this](Block *b, auto self) {
			if(b->size_ <= Mbs_) return;
			auto nb = b->split();
			self(b, self);
			D0.emplace_back(std::move(nb));
			self(D0.back().get(), self);
		};
		split(D0.back().get(), split);
	}
	
	std::pair<T, std::set<uint> > pull() {
		uint c1 = 0, c2 = 0;
		Pkv* nd = new Pkv();
		nd->pre = nd->nxt = nd;
		
		D0.erase(
			std::remove_if(D0.begin(), D0.end(), [](const std::unique_ptr<Block>& ptr){
				return !ptr || ptr->size_ == 0;
			}),
			D0.end()
		);
		
		for(int i = D0.size() - 1; i >= 0; --i) {
			auto& block = D0[i];
			c1 += block->size_;
			for(Pkv* cur = block->dummy->nxt; cur!= block->dummy;) {
				Pkv *t = new Pkv(cur->key, cur->val);
				link(nd->pre, t);
				link(t, nd);
				cur = cur->nxt;
			}
			if(c1 >= Mbs_) break;
		}
		
		
		for(const auto& block : D1) {
			c2 += block->size_;
			for(Pkv* cur = block->dummy->nxt; cur!= block->dummy;) {
				Pkv *t = new Pkv(cur->key, cur->val);
				link(nd->pre, t);
				link(t, nd);
				cur = cur->nxt;
			}
			if(c2 >= Mbs_) break;
		}
		
		if(c1 + c2 == 0) {
			delete nd;
			return {0, {}};
		}
		
		Pkv* tar = BFPRT(nd, c1+c2, Mbs_);
		
		for(Pkv* cur = nd->nxt; cur != nd;) {
			cur = cur->nxt;
		}
		
		std::set<uint>ret1;
		T ret2 = tar->val;
		bool flag = 1;
		for(Pkv* cur = nd->nxt; cur != nd;) {
			Pkv* now = cur;
			if(flag) {
				ret1.insert(cur->key);
				auto it = K.find(cur->key);
				link(it->second->pre, it->second->nxt);
				-- it->second->block->size_;
				if(it->second->pre == it->second->nxt) {
					auto db = it->second->block;
					it->second->block = nullptr;
					if(db->idx_ > 0 && db->mode_ == 1) {
						auto dit = D1.find(db);
						if(dit != D1.end()) D1.erase(dit);
					}
				} else it->second->block = nullptr;
				
				if(cur == tar) flag = 0;
			}
			cur = cur->nxt;
			delete now;// 释放链表中间
		}
		delete nd;// 释放链表哨兵
		_size -= ret1.size();
		
		
		// 添加极小增量，使得 ret2 > all in ret1 and ret2 <= all not in ret1
		if constexpr(std::is_same_v<T, Path<double> >) {
			ret2.prev++;
		}
		
		return {_size? ret2: Bound_, ret1};
	}
	
	void print() {
		for(const auto& it : K) {
			if(it.second->block == nullptr) std::cout<<"<"<<it.second->key<<" is pulled> ";
			else std::cout<<"<"<<it.second->key<<", "<<it.second->val<<", "<<it.second->block->idx_<<"> ";
		}std::cout<<"\n\n";
		
		D0.erase(
			std::remove_if(D0.begin(), D0.end(), [](const std::unique_ptr<Block>& ptr){
				return ptr->size_ == 0;
			}),
			D0.end()
		);
		
		for(const auto& block : D0) {
			std::cout<<block->idx_<<" "<<block->bound_<<" "<<block->size_<<":\n";
			for(auto cur = block->dummy;;) {
				cur = cur->nxt;
				if(cur == block->dummy)break;
				std::cout<<"<"<<cur->key<<", "<<cur->val<<"> ";
			}
			std::cout<<"\n";
		}
		
		for(const auto& block : D1) {
			std::cout<<block->idx_<<" "<<block->bound_<<" "<<block->size_<<":\n";
			for(auto cur = block->dummy;;) {
				cur = cur->nxt;
				if(cur == block->dummy)break;
				std::cout<<"<"<<cur->key<<", "<<cur->val<<"> ";
			}
			std::cout<<"\n";
		}
	}

};


#undef uint

#endif