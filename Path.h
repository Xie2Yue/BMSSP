#ifndef PATH_H
#define PATH_H

#include <iostream>
#include <vector>
#include <format>

#define uint unsigned int

template<typename T = double>
class Path {
	private:
	public:
		T weight;
		uint length;
		std::vector<uint> Vertices;
	public:
		Path(T w): weight(w), length(0) {
			Vertices.clear();
		}
		
		template<typename... Args>
		Path(T w, Args... vertices): weight(w) {
			Vertices.clear();
			((Vertices.push_back(vertices)), ...);
			length = Vertices.size();
		}
		
		bool operator < (const Path& t) const {
			if(weight != t.weight) return weight < t.weight;
			if(length != t.length) return length < t.length;
			for(int i = 0; i < length; i ++) {
				if(Vertices[i] != t.Vertices[i]) return Vertices[i] < t.Vertices[i];
			}
			return 0;
		}
		bool operator > (const Path& t) const {
			if(weight != t.weight) return weight > t.weight;
			if(length != t.length) return length > t.length;
			for(int i = 0; i < length; i ++) {
				if(Vertices[i] != t.Vertices[i]) return Vertices[i] > t.Vertices[i];
			}
			return 0;
		}
		
		bool operator == (const Path& t) const {
			if(weight != t.weight) return 0;
			if(length != t.length) return 0;
			for(int i = 0; i < length; i ++) {
				if(Vertices[i] != t.Vertices[i]) return 0;
			}
			return 1;
		}
		
		bool operator >= (const Path& t) const {
			return !(*this < t);
		}
		bool operator <= (const Path& t) const {
			return !(*this > t);
		}
		
		Path operator + (const Path& t) const {
			Path ret = *this;
			ret.weight += t.weight;
			ret.length += t.length;
			for(auto v: t.Vertices) {
				ret.Vertices.push_back(v);
			}
			return ret;
		}
		Path& operator += (const Path& t) {
			weight += t.weight;
			length += t.length;
			for(auto v: t.Vertices) {
				Vertices.push_back(v);
			}
			return *this;
		}
		
		friend std::ostream& operator << (std::ostream &out, Path t) {
			out << "<" << t.weight << ", " << t.length;
			if(t.length > 0) {
				out << ", [";
				for(int i = 0; i < t.Vertices.size(); i ++) {
					out << t.Vertices[i] << ((i == t.Vertices.size()-1) ? "]": ", ");
				}	
			}
			out << ">";
			return out;
		}
};


//template<typename T>
//struct std::formatter<Path<T> > {
//	constexpr auto parse(std::format_parse_context& ctx) {
//		return ctx.begin();
//    }
//	
//    auto format(const Path<T>& t, std::format_context& ctx) const {
//    	return std::format_to(ctx.out(), "{}", t.weight);
//    	auto out = std::format_to(ctx.out(), "<{}, {}", t.weight, t.length);
//    	if(t.length > 0) {
//    		out = std::format_to(out, ", [");
//    		for(int i = 0; i < t.Vertices.size(); i++) {
//    			out = std::format_to(out, "{}{}", t.Vertices[i], ((i == t.Vertices.size()-1) ? "]": ", "));
//			}
//		}
//		return std::format_to(out, ">");
//    }
//};

#endif