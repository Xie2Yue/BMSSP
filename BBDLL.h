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
	
	class Node{
		public:
		uint key;
		T val;
		Block *block;
		Node *pre, *nxt;
		Node (uint k = 0, T v = 0, Block *b = nullptr, Node *p = nullptr, Node *n = nullptr):
			key(k), val(v), block(b), pre(p), nxt(n) {}
		
		bool operator < (const Node& t) const { return val < t.val; }
	};
	
	class Block {
		public:
		static inline uint counter_ = 0;
		bool mode_;
		T bound_;
		uint idx_;
		uint size_;
		Node *dummy;	
			
		Block(T bound = 0, bool mode = 0) : bound_(bound), idx_(counter_++), size_(0), mode_(mode) {
			dummy = new Node();
			dummy->pre = dummy->nxt = dummy;
		}
		Block(T bound, Node* d, bool mode = 0): bound_(bound), idx_(counter_++), size_(0), dummy(d), mode_(mode) {
			for(Node* cur = dummy->nxt;cur != dummy; cur = cur->nxt) {
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
		
		Node* insert(uint k, const T& v) {
			++size_;
			Node *ins = new Node(k, v, this);
			link(dummy->pre, ins);
			link(ins, dummy);
			return ins;
		}
		Node* insert (Node* ptr, const T& v) {
			++size_;
			ptr->val = v;
			ptr->block = this;
			link(dummy->pre, ptr);
			link(ptr, dummy);
			return ptr;
		}
		std::unique_ptr<Block> split(bool mode = 0) {
			Node* nd = new Node();
			Node* pos = BFPRT(dummy, size_, size_+1>>1);
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
	std::unordered_map<uint, Node*>hash_;
	std::set<std::unique_ptr<Block>, BlockCompare> D1;
	std::vector<std::unique_ptr<Block> > D0;
	private:
	
	void init() {
		D1.emplace(std::make_unique<Block>(Bound_));
		_size = 0;
	}
	
	static void link(Node *a, Node* b){
		a->nxt = b;
		b->pre = a;
	}
	
	static Node* partition(Node* begin, const Node* end, std::function<bool(const Node&)>cmp) {
		Node* pre = begin->pre;
		Node* dummy = new Node();
		dummy->pre = dummy->nxt = dummy;
		for(Node* it = begin; it != end;) {
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
	
	static uint dis(Node* st, const Node* ed) {
		uint ret = 0;
		while(st != ed) {
			++ret;
			st = st->nxt;
		}
		return ret;
	}
	
	static Node* getNext(Node* st, uint k = 1) {
		for(int i = 0; i < k; ++i) {
			st = st->nxt;
		}
		return st;
	}
	
	
	static Node* BFPRT(Node* dummy, uint n, uint k) {
		if(n == 1){ return dummy->nxt; }
		if(k > n) { k = n; }
		Node*nd = new Node();
		nd->pre = nd->nxt = nd;
		Node* cur = dummy->nxt;
		uint m = 1;
		for(int i = 1; i <= n/5; ++i) {
			Node *t = new Node(0, getMidium(
				cur->val, cur->nxt->val, cur->nxt->nxt->val,
				cur->nxt->nxt->nxt->val, cur->nxt->nxt->nxt->nxt->val
			));
			link(nd->pre, t);
			link(t, nd);
//			std::cout << "break4-" << t->val << "\n";
			cur = cur->nxt->nxt->nxt->nxt->nxt;
			++m;
		}
		if(n%5 == 1) {
			Node *t = new Node(0, cur->val);
//			std::cout << "break4-" << t->val << "\n";
			link(nd->pre, t);
			link(t, nd);
		} else if(n%5 == 2) {
			Node *t = new Node(0, std::min(cur->val, cur->nxt->val));
//			std::cout << "break4-" << t->val << "\n";
			link(nd->pre, t);
			link(t, nd);
		} else if(n%5 == 3) {
			Node *t = new Node(0, getMidium(cur->val, cur->nxt->val, cur->nxt->nxt->val));
//			std::cout << "break4-" << t->val << "\n";
			link(nd->pre, t);
			link(t, nd);
		} else if(n%5 == 4) {
			Node *t = new Node(0, getMidium(cur->val, cur->nxt->val,
			cur->nxt->nxt->val, cur->nxt->nxt->nxt->val));
//			std::cout << "break4-" << t->val << "\n";
			link(nd->pre, t);
			link(t, nd);
		} else --m;
		
//		std::cout << "break5-in-" << m << "\n";
		T mid = BFPRT(nd, m, m+1>>1)->val;
//		std::cout << "break5-out-" << mid << "\n";
		
		for(Node* cur = nd->nxt; cur!= nd;) {
			Node* now = cur;
			cur = cur->nxt;
			delete now;
		}
		delete nd;
		
//		for(Node* cur = dummy->nxt; cur != dummy;) {
//			std::cout << "break6-" << cur->key << " " << cur->val << "\n";
//			cur = cur->nxt;
//		}
		
		Node* mid1 = partition(dummy->nxt, dummy, [mid](const Node& x){
			return x.val < mid;
		});
		
//		for(Node* cur = dummy->nxt; cur != dummy;) {
//			std::cout << "break6-" << cur->key << " " << cur->val << "\n";
//			cur = cur->nxt;
//		}
		
		
		if(uint d1 = dis(dummy, mid1) - 1; d1 >= k) {
			Node* sd = new Node();
			link(sd, dummy->nxt);
			link(mid1->pre, sd);
			Node* rit = BFPRT(sd, d1, k);
			link(dummy, sd->nxt);
			link(sd->pre, mid1);
			delete sd;
			return rit;
		} else {
			Node* l2 = mid1->pre;
			Node* mid2 = partition(mid1, dummy, [mid](const Node& x){
				return x.val <= mid;
			});
			mid1 = l2->nxt;
			if(auto d2 = d1 + dis(mid1, mid2); d2 >= k) {
				return getNext(mid1, k -d1 - 1);
			} else {
				Node* sd = new Node();
				Node* m2p = mid2->pre;
				link(sd, mid2);
				link(dummy->pre, sd);
//				for(Node* cur = sd->nxt; cur != sd;) {
//					std::cout << "break7-" << cur->key << " " << cur->val << "\n";
//					cur = cur->nxt;
//				}
				Node* rit = BFPRT(sd, n - d2, k - d2);
//				for(Node* cur = sd->nxt; cur != sd;) {
//					std::cout << "break7-" << cur->key << " " << cur->val << "\n";
//					cur = cur->nxt;
//				}
//				std::cout << "break8-" << rit->key << " " << rit->val << "\n";
				link(m2p, sd->nxt);
				link(sd->pre, dummy);
//				for(Node* cur = dummy->nxt; cur != dummy;) {
//					std::cout << "break9-" << cur->key << " " << cur->val << "\n";
//					cur = cur->nxt;
//				}
				delete(sd);
				return rit;
			}
		}
	}
	
	public:
	BBDLL (const T& bound, uint mbs): Bound_(bound), Mbs_(mbs) { init(); }
	
	~BBDLL() {
		for(auto& it : hash_) {
			delete it.second;
		}
	}
	
	uint size() {
		return _size;
	}
	
	void insert(uint k, const T& v) {
		++ _size;
		if(auto it = hash_.find(k); it != hash_.end()) {
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
			hash_[k] = block->get()->insert(k, v);
			if(block->get()->size_>Mbs_) D1.emplace(std::move(block->get()->split(1)));
		}
	}
	
	void batchPrepend(std::set<std::pair<uint, T> >data) {
		T bound = 0;
		Node *nd = new Node();
		nd->pre = nd->nxt = nd;
		for(auto [k, v] : data) {
			++ _size;
			if(auto it = hash_.find(k); it != hash_.end()) {

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
				Node *node = new Node(k, v);
				link(nd->pre, node);
				link(node, nd);
				hash_[k] = node;
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
		Node* nd = new Node();
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
			for(Node* cur = block->dummy->nxt; cur!= block->dummy;) {
				Node *t = new Node(cur->key, cur->val);
				link(nd->pre, t);
				link(t, nd);
//				std::cout << "break1-" << cur->key << " " << cur->val << "\n";
				cur = cur->nxt;
			}
			if(c1 >= Mbs_) break;
		}
		
		
		for(const auto& block : D1) {
			c2 += block->size_;
			for(Node* cur = block->dummy->nxt; cur!= block->dummy;) {
				Node *t = new Node(cur->key, cur->val);
				link(nd->pre, t);
				link(t, nd);
//				std::cout << "break1-" << cur->key << " " << cur->val << "\n";
				cur = cur->nxt;
			}
			if(c2 >= Mbs_) break;
		}
		
		if(c1 + c2 == 0) {
			delete nd;
			return {0, {}};
		}
//		std::cout << "break3-" << c1 + c2 << "\n";
		
		Node* tar = BFPRT(nd, c1+c2, Mbs_);
		
		for(Node* cur = nd->nxt; cur != nd;) {
//			std::cout << "break2-" << cur->key << " " << cur->val << "\n";
			cur = cur->nxt;
		}
		
		std::set<uint>ret1;
		T ret2 = tar->val;
		bool flag = 1;
		for(Node* cur = nd->nxt; cur != nd;) {
			Node* now = cur;
			if(flag) {
				ret1.insert(cur->key);
				auto it = hash_.find(cur->key);
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
		for(const auto& it : hash_) {
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

//template<typename T>
//struct std::formatter<BBDLL<T> > {
//	constexpr auto parse(std::format_parse_context& ctx) {
//		return ctx.begin();
//    }
//	
//    auto format(const BBDLL<T>& t, std::format_context& ctx) const {
//    	auto out = std::format_to(ctx.out(), "<B = {}, M = {}, [", t.Bound_, t.Mbs_);
//    	for(const auto& it: t.hash_) {
//    		if(it.second->block != nullptr) {
//    			out = std::format_to(out, "<key = {}, val = {}, id = {}> ", it.second->key, it.second->val, it.second->block->idx_);
//			}
//		}
//		return std::format_to(out, "]>");
//    }
//};

#undef uint

#endif