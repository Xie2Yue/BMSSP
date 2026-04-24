#ifndef BMSSP_H
#define BMSSP_H

#include "Graph.h"
#include "Path.h"
#include "BBDLL.h"
#include <queue>
#include <cmath>
#include <limits>
#include <fstream>
#include <format>
#include <iomanip>
#include <chrono>
#include <ctime>

#define uint unsigned int
#define iinf 1e18


template<typename T = double>
class BMSSPAlgo {
	public:
	Graph<T> G;
	std::vector<Path<T> > d;
	std::vector<uint> cnt, ft;
	uint t, k, l, cuv;
	
	auto calc(uint n, uint m, T B, uint s) {
		G.getRandomGraph(n, m);
		G.getRandomEdgeWeight(B);
		cuv = 0;
		
		
//		std::ofstream fout("graph.bmssp2");
//		fout << std::fixed << std::setprecision(10) << G << "\n";
//		fout.close();
//		std::ofstream fans("ans.bmssp2");

		auto start1 = std::chrono::steady_clock::now();
		
		k = std::max(1u, (uint)floor(pow(log2(n), 1/3.0)));
		t = std::max(1u, (uint)floor(pow(log2(n), 2/3.0)));
		
		d.assign(n, iinf);
		d[s] = 0;
		cnt.assign(n, 0);
		ft.assign(n, 0);
		
		l = ceil(log2(n) / t);
		std::set<uint>S({s});
		
		
		BMSSP(l, iinf, S);
		
		auto end1 = std::chrono::steady_clock::now();
		
		
//		fans << std::fixed << std::setprecision(10);
		for(int i = 0; i < n; i ++) {
//			fans << i << " " << d[i].weight << "\n";
			cuv += d[i].weight != iinf;
		}
		auto start2 = std::chrono::steady_clock::now();
		
		d.assign(n, iinf);
		d[s] = 0;
		
		dijkstra(s);
		
		auto end2 = std::chrono::steady_clock::now();
		
//		for(int i = 0; i < n; i ++) {
//			if(d2[i] != d[i]) std::cout << "e";
//		}
		
		return make_tuple(
			std::chrono::duration_cast<std::chrono::microseconds>(end1 - start1),
			std::chrono::duration_cast<std::chrono::microseconds>(end2 - start2), cuv);
	}
	
	uint getfa(int x) {
		if(ft[x] == x) return x;
		return ft[x] = getfa(ft[x]);
	}
	
	std::pair<std::set<uint>, std::set<uint> > FindPivots(const Path<T>& B, const std::set<uint>& S) {
		auto W = S;
		auto W0 = S;
		std::set<uint> W1;
		for(const auto& s : S) {
			ft[s] = s;
			cnt[s] = 0;
		}
		for(uint i = 1; i <= k; ++i) {
			W1.clear();
			for(const auto& u : W0) {
				for(const auto& [v, w] : G.getVertex(u).getEdge()) {
					if(d[u] + Path(w, v) <= d[v]) {
						d[v] = d[u] + Path(w, v);
						ft[v] = u;
						if(d[v] < B) {
							W1.insert(v);
							W.insert(v);
						}
					}
				}
			}
			if(W.size()> k * S.size()) {
				return {S, W};
			}
			W0 = W1;
		}
		std::set<uint> P;
		for(const auto& w : W) {
			if(++cnt[getfa(w)] == k) {
				P.insert(ft[w]);
			}
		}
		return {P, W};
	}
	
	class pptui {
		public:
			
		Path<T> p;
		uint u;
		
		bool operator < (const pptui t)	const {
			return p > t.p;
		}
		
		pptui(const Path<T>& p, uint u): p(p), u(u) {}
		
		template<std::size_t I>
		friend const auto& get(const pptui&) noexcept;
	};
	
	std::pair<Path<T>, std::set<uint> > BaseCase(const Path<T>& B,const std::set<uint>& S) {
		auto U0 = S;
		auto x = *S.begin();
		std::priority_queue<pptui> H;
		H.emplace(d[x], x);
		while( !H.empty() && U0.size() < k+1) {
			auto [dis, u] = H.top();
			H.pop();
			if(d[u] < dis) continue;
			U0.insert(u);
			for(const auto& [v, w]: G.getVertex(u).getEdge()) {
				if(d[u] + Path(w, v) <= d[v] && d[u] + Path(w, v) < B) {
					d[v] = d[u] + Path(w, v);
					H.emplace(d[v], v);
				}
			}
		}
		if(U0.size() <= k) {
			return {B, U0};
		}
		else {
			Path<T> Bpi(0);
			for(const auto& u : U0) {
				Bpi = std::max(Bpi, d[u]);
			}
			std::set<uint> U;
			for(const auto& u : U0) {
				if(d[u] < Bpi) U.insert(u);
			}
			return {Bpi, U};
		}
	}
	
	
	std::pair<Path<T>, std::set<uint> > BMSSP(uint l,const Path<T>& B,const std::set<uint>& S) {
		
		if(l == 0) return BaseCase(B, S);
		
		auto [P, W] = FindPivots(B, S);
		
		auto M = 1ll<<((l-1)*t);
		
		BBDLL<Path<T>> D(B, M);
		
		for(auto x: P) {
			D.insert(x, d[x]);
		}
		
		std::set<uint>U;		
		int i = 0;
		Path<T> Bpi = B;
		
		while(U.size() < k * (1ll<<(l*t)) && D.size()>0) {
				auto [Bi, Si] = D.pull();
				auto [Bipi, Ui] = BMSSP(l-1, Bi, Si);
				
				std::set<std::pair<uint, Path<T> > >K;
				
				for(const auto& u: Ui) {
					U.insert(u);
					for(const auto& [v, w]: G.getVertex(u).getEdge()) {
						if(d[u] + Path(w, v) <= d[v]) {
							d[v] = d[u] + Path(w, v);
							if(d[v] >= Bi && d[v] < B) {
								D.insert(v, d[v]);
							}
							else if(d[v] >= Bipi && d[v] < Bi) {
								K.insert({v, d[v]});
							}
						}
					}
				}
				for(auto s: Si) {
					if(d[s] >= Bipi && d[s] < Bi) {
						K.insert({s, d[s]});
					}
				}
				
				D.batchPrepend(K);
				
				Bpi = std::min(Bpi, Bipi);
		}
		
		for(auto x: W) {
			if(d[x] < Bpi) U.insert(x);
		}
		
		return {Bpi, U};
	}
	
	void dijkstra(uint s) {
		std::priority_queue<pptui> q;
		
		q.emplace(d[s], s);
		while(!q.empty()) {
			auto [dis, u] = q.top();
			q.pop();
			if(d[u] < dis) continue;
			for(auto [v, w]: G.getVertex(u).getEdge()) {
				if(d[u] + Path(w, v) < d[v]) {
					d[v] = d[u] + Path(w, v);
					q.emplace(d[v], v);
				}
			}
		}
	}
	
};

template<std::size_t I, typename T>
const auto& get(const typename BMSSPAlgo<T>::pptui& p) noexcept {
	if constexpr (I == 0) return p.p;
	if constexpr (I == 1) return p.u;
}

#undef uint

#endif