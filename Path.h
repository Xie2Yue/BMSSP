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
		uint length, prev;
	public:
		Path(T w): weight(w), length(0), prev(0) {}
		Path(T w, uint p): weight(w), length(1), prev(p) {}
		
		
		bool operator < (const Path& t) const {
			if(weight != t.weight) return weight < t.weight;
			if(length != t.length) return length < t.length;
			return prev < t.prev;
		}
		bool operator > (const Path& t) const {
			if(weight != t.weight) return weight > t.weight;
			if(length != t.length) return length > t.length;
			return prev > t.prev;
		}
		
		bool operator == (const Path& t) const {
			return weight == t.weight && length == t.length && prev == t.prev;
		}
		bool operator != (const Path& t) const {
			return weight != t.weight || length != t.length || prev != t.prev;
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
			ret.prev = t.prev;
			return ret;
		}
		Path& operator += (const Path& t) {
			weight += t.weight;
			length += t.length;
			prev = t.prev;
			return *this;
		}
		
		friend std::ostream& operator << (std::ostream &out, Path t) {
			out << "<" << t.weight << ", " << t.length << ", " << t.prev << ">";
			return out;
		}
};


#endif