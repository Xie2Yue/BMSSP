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
//#define iinf std::numeric_limits<T>::max()


template<typename T = double>
class BMSSPAlgo {
	public:
	Graph<T> G;
	std::vector<Path<T> > d;
	std::vector<uint> cnt, ft;
	uint t, k, l;
	
	auto calc(uint n, uint m, T B, uint s) {
		G.getRandomGraph(n, m);
		G.getRandomEdgeWeight(B);
		
		auto start = std::chrono::steady_clock::now();
		
//		std::ofstream fout("graph.bmssp2");
//		fout << std::fixed << std::setprecision(10) << G << "\n";
//		fout.close();
//		std::ofstream fans("ans.bmssp2");
		
		k = std::max(1u, (uint)floor(pow(log2(n), 1/3.0)));
		t = std::max(1u, (uint)floor(pow(log2(n), 2/3.0)));
		
		d.assign(n, iinf);
		d[s] = 0;
		cnt.assign(n, 0);
		ft.assign(n, 0);
		
		l = ceil(log2(n) / t);
		std::set<uint>S({s});
		
//		std::cout << std::format("calc that k = {}, t = {}, l = {}\n", k, t, l); fmo.dep == 0;
		
		BMSSP(l, iinf, S);
		
		auto end = std::chrono::steady_clock::now();
		
		return std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
		
//		fans << std::fixed << std::setprecision(10);
//		for(int i = 0; i < n; i ++) {
//			fans << i << " " << d[i].weight << "\n";
//		}
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
		
//		std::cout << std::format("{}BMSSP(l = {}, B = {}, S = {}) {{\n", fmo, l, B, S);
//		fmo.dep++;
		
//		std::cout << std::format("{}[P, W] = FindPivots(B = {}, S = {}) {{\n", fmo, B, S);
//		fmo.dep++;
		auto [P, W] = FindPivots(B, S);
//		fmo.dep--;
//		std::cout << std::format("{}}}\n{}so P = {}, W = {}\n", fmo, fmo, P, W);
		
		auto M = 1ll<<((l-1)*t);
//		std::cout << std::format("{}M = {}\n", fmo, M);
		
		BBDLL<Path<T>> D(B, M);
//		std::cout << std::format("{}D = BBDLL(B = {}, M = {})\n", fmo, B, M);
		
//		std::cout << std::format("{}D.insert(P = {}) {{\n", fmo, P);
//		fmo.dep++;
		for(auto x: P) {
			D.insert(x, d[x]);
		}
//		fmo.dep--;
//		std::cout << std::format("{}}} so D = {}\n", fmo, D);
		
		std::set<uint>U;		
		int i = 0;
		Path<T> Bpi = B;
		
//		std::cout << std::format("{}i = {}, U = {}, Bpi = {}\n", fmo, i, U, Bpi);
		
//		std::cout << std::format("{} while(|U| < k*2^(lt) && |D| > 0) {{\n", fmo); fmo.dep++;
		while(U.size() < k * (1ll<<(l*t)) && D.size()>0) {
//			std::cout << std::format("{}because |U| = {} < k*2^(lt) = {} && |D| = {} > 0\n{} so {{\n", fmo, U.size(), k * (1ll<<(l*t)), D.size(), fmo); fmo.dep++;
				
//				std::cout << std::format("{}D = {}", fmo, D);
				auto [Bi, Si] = D.pull();
//				std::cout << std::format("{}[Bi, Si] = D.pull()\n{}Bi = {}, Si = {}\n{}D = {}\n", fmo, fmo, Bi, Si, fmo, D);
				auto [Bipi, Ui] = BMSSP(l-1, Bi, Si);
//				std::cout << std::format("{}[Bipi, Ui] = BMSSP(l-1 = {}, Bi = {}, Si = {}) = [{}, {}]\n", fmo, l-1, Bi, Si, Bipi, Ui);
				
				std::set<std::pair<uint, Path<T> > >K;
//				std::cout << std::format("{}K = {}\n", fmo, K);
				
//				std::cout << std::format("{}for edge[u, v, w] in Ui{{\n", fmo); fmo.dep++;
				for(const auto& u: Ui) {
					U.insert(u);// U cup Ui
					for(const auto& [v, w]: G.getVertex(u).getEdge()) {
//						std::cout << std::format("{}[u, v, w] = [{}, {}, {}]: {{\n", fmo, u, v, w); fmo.dep++;
//						std::cout << std::format("{}d[u] = {},d[u] + Path(w, v) = {}, d[v] = {}\n{}Bi = {}, B = {}\n", fmo, d[u], d[u] + Path(w, v), d[v], fmo, Bi, B);
						if(d[u] + Path(w, v) <= d[v]) {
							d[v] = d[u] + Path(w, v);
							if(d[v] >= Bi && d[v] < B) {
//								std::cout << std::format("{}D.insert(v = {}, d[v] = {})\n", fmo, v, d[v]);
								D.insert(v, d[v]);
							}
							else if(d[v] >= Bipi && d[v] < Bi) {
//								std::cout << std::format("{}K.insert(v = {}, d[v] = {})\n", fmo, v, d[v]);
								K.insert({v, d[v]});
							}
						}
//						fmo.dep--; std::cout << std::format("{}}}\n", fmo);
					}
				}
//				fmo.dep--; std::cout << std::format("{}}}\n", fmo);
				
//				std::cout << std::format("{}K cup= Si(s | Bipi <= d[s] < Bi); Si = {}, K = {}, Bipi = {}, Bi = {}\n", fmo, Si, K, Bipi, Bi); fmo.dep++;
				for(auto s: Si) {
					if(d[s] >= Bipi && d[s] < Bi) {
						K.insert({s, d[s]});
//						std::cout << std::format("{}s = {}, d[s] = {}, Bipi = {}, Bi = {}\n", fmo, s, d[s], Bipi, Bi);
					}
				}
//				fmo.dep--;
				
				D.batchPrepend(K);
//				std::cout << std::format("{}D.batchPrepend(K = {})\n{}so D = {}", fmo, K, fmo, D);
				
				
				Bpi = std::min(Bpi, Bipi);
			
//			fmo.dep--; std::cout << std::format("{}}}\n", fmo);
		}
//		fmo.dep--; std::cout << std::format("{}}}\n", fmo);
		
		for(auto x: W) {
			if(d[x] < Bpi) U.insert(x);
		}
		
//		std::cout << std::format("{}return Bpi = {}, U = {}\n", fmo, Bpi, U);
//		fmo.dep--;
//		std::cout << std::format("{}{{\n", fmo);
		return {Bpi, U};
	}
	
	
	
};

template<std::size_t I, typename T>
const auto& get(const typename BMSSPAlgo<T>::pptui& p) noexcept {
	if constexpr (I == 0) return p.p;
	if constexpr (I == 1) return p.u;
}

#undef uint

#endif