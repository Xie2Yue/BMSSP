#ifndef GRAPH_H
#define GRAPH_H

#include<iostream>
#include<vector>
#include<set>
#include<random>
#include<type_traits>

#define uint unsigned int
#define ull unsigned long long

template<typename T = double>
class Graph {
	private:
		
	uint idx_ = 0;
	
	class Edge {
		public:
		uint to;
		mutable T weight;
		Edge(uint v, T w): to(v), weight(w) {}
		
		template<std::size_t I>
		friend const auto& get(const Edge& e) noexcept;
	};
	
	class Vertex {
		private:
			
		uint idx_;
		std::vector<Edge> to_;
		
		public:
		
		Vertex(uint id): idx_(id) {}
		
		uint getId() const { return idx_; }
		
		void addEdge(uint to, T w = 0) {
			to_.emplace_back(to, w);
		}
		
		const std::vector<Edge>& getEdge() const {
			return to_;
		}
	};
	
	uint E = 0;
	
	std::vector<Vertex>V;
	std::set<uint>S;
	
	public:
	
	const Vertex& getVertex(uint u) const { return V[u]; }
	
	const std::set<uint>& getVertices() const { return S; }
	
	std::pair<uint, uint> size() const { return {V.size(), E}; }
	
	uint addVertex(int t = 1) {
		while(t--) {
			S.insert(idx_);
			V.emplace_back(idx_);
			idx_++;
		}
		return idx_-1;
	}
	
	void addEdge(uint u, uint v, T w = 0) {
		V[u].addEdge(v, w);
		
		++ E;
	}
	
	void clear() {
		S.clear();
		V.clear();
		E = idx_ = 0;
	}
	
	void getRandomGraph(uint n, ull m) {
		
		if(n <= 1) return;
		if(m > n * (n - 1)) m = n * (n - 1);
		if(m < n - 1) m = n - 1 ;
		
		clear();
		
		std::set<ull>M;
		static std::random_device rd;
		static std::mt19937 gen(rd());
		
		for(uint i = 0; i < n; ++i) {
			addVertex();
		}
		
		for(ull i = n * (n - 1) - m; i < n * (n - 1); ++i) {
			if( i <= 1) {
				M.insert(i);
				continue;
			}
			std::uniform_int_distribution<long long> dis(0, i - 1);
			ull val = dis(gen);
			
			if(M.count(val)) {
				M.insert(i);
			} else {
				M.insert(val);
			}
			
		}
		
		for(const auto& pr: M) {
			ull v = pr / (n - 1);
			ull u = pr % (n - 1);
			u += u >= v;
			addEdge(u, v);
			
		}
		
	}
	
	void getRandomGraph2(uint n, ull k) {
		if(n <= 1 || k >= n) return;
		
		clear();
		
		std::set<ull>M;
		static std::random_device rd;
		static std::mt19937 gen(rd());
		
		for(uint i = 0; i < n; i ++) {
			addVertex();
		}
		
		for(uint i = 0; i < n; i ++) {
			M.clear();
			for(uint j = (n-1) - k; j < (n-1); j ++) {
				if(j <= 1) {
					M.insert(i);
					continue;
				}
				std::uniform_int_distribution<long long> dis(0, j - 1);
				ull val = dis(gen);
				
				if(M.count(val)) {
					M.insert(i);
				} else {
					M.insert(val);
				}
			}
			for(auto s: M) {
				addEdge(i, s + (s>=i));
			}
		}
	}

	void getRandomEdgeWeight(T m) {
		static std::random_device rd;
		static std::mt19937 gen(rd());
		if constexpr (std::is_integral_v<T>) {
			std::uniform_int_distribution<ull> dis(1, m);
			for(auto& u: V) {
				for(auto& [v, w]: u.getEdge()) {
					w = dis(gen);
				}
			}
		}
		else if constexpr (std::is_floating_point_v<T>) {
			std::uniform_real_distribution<double> dis(0, m);
			for(auto& u: V) {
				for(auto& [v, w]: u.getEdge()) {
					w = m - dis(gen);
				}
			}
		}
	}
	
	template<typename U>
	friend std::ostream& operator << (std::ostream &out, Graph<U> t) {
		
		out << t.V.size() << " " << t.E << "\n";
		
		for(int i = 0; i < t.V.size(); ++i) {
			for(auto [v, w] : t.V[i].getEdge()) {
				out << t.V[i].getId() << " " << v << " " << w <<"\n";
			}
		}
		
		return out;
		
	}
	
	template<typename U>
	friend std::istream& operator >> (std::istream &in, Graph<U> &t) {
		t.clear();
		
		uint inN = 0, inM = 0, inu = 0, inv = 0;
		double inw = 0;
		
		in >> inN >> inM;
		
		for(int i = 0; i < inN ; ++i) {
			t.addVertex();
		}
		for(int i = 0; i < inM ; ++i) {
			in >> inu >> inv >> inw;
			t.addEdge(inu, inv, inw);
		}
		return in;
	}
};

template<std::size_t I, typename T>
const auto& get(const typename Graph<T>::Edge& e) noexcept {
	if constexpr (I == 0) return e.to;
	if constexpr (I == 1) return e.weight;
}

#undef uint
#undef ull

#endif