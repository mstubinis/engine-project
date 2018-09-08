#pragma once
#ifndef ENGINE_UTILS_H
#define ENGINE_UTILS_H

#include <algorithm>
#include <vector>
#include <map>
#include <unordered_map>
#include <boost/lexical_cast.hpp>
#include <boost/container/vector.hpp>

typedef unsigned char uchar;
typedef unsigned int uint;

namespace Engine{
	namespace epriv{
		class noncopyable{
			protected:
				noncopyable(){}
				~noncopyable(){}
			private:
				const noncopyable& operator=(const noncopyable&) = delete;// non copyable
				noncopyable(const noncopyable&) = delete;                 // non construction-copyable
				noncopyable(const noncopyable&&) = delete;                // non moveable 1
				noncopyable(noncopyable&&) = delete;                      // non moveable 2
				noncopyable& operator=(const noncopyable&&) = delete;     // non move assignable 1
				noncopyable& operator=(noncopyable&&) = delete;           // non move assignable 2
				//noncopyable& operator=(noncopyable) = delete;           // copy and swap 1
		};
	};
};
template <typename Stream> void readUint32tBigEndian(uint32_t& out,Stream& stream) {
	uint8_t buf[4];
	stream.read((char *)buf, sizeof(buf));
	out = (uint32_t)buf[0] << 24;
	out |= (uint32_t)buf[1] << 16;
	out |= (uint32_t)buf[2] << 8;
	out |= (uint32_t)buf[3];
}
template <typename Stream> void readUint16tBigEndian(uint16_t& out, Stream& stream) {
	uint8_t buf[2];
	stream.read((char *)buf, sizeof(buf));
	out = (uint32_t)buf[0] << 8;
	out |= (uint32_t)buf[1];
}
template <typename Stream> void writeUint32tBigEndian(uint32_t& in, Stream& stream) {
	uint8_t buf[4];
	buf[0] = (in & 0xff000000) >> 24;
	buf[1] = (in & 0x00ff0000) >> 16;
	buf[2] = (in & 0x0000ff00) >> 8;
	buf[3] = (in & 0x000000ff);
	stream.write((char *)buf, sizeof(buf));
}
template <typename Stream> void writeUint16tBigEndian(uint16_t& in, Stream& stream) {
	uint8_t buf[2];
	buf[0] = (in & 0xff00) >> 8;
	buf[1] = (in & 0x00ff);
	stream.write((char *)buf, sizeof(buf));
}


//returns true if the system is little endian, false if it is big endian
constexpr bool isLittleEndian() {
	uint16_t x = 0x0001;
	auto p = reinterpret_cast<uint8_t*>(&x);
	return *p != 0;
}
//swaps the bytes of the parameter
template <class T> void endianSwap(T* objp) {
	uchar *memp = reinterpret_cast<uchar*>(objp);
	std::reverse(memp, memp + sizeof(T));
}


//removes a specific element from a vector
template<typename E,typename B> void removeFromVector(std::vector<B*>& v,E* e){
    for(auto it=v.begin();it!=v.end();){B* c=(*it);if(c==e){it=v.erase(it);}else{++it;}}
}
//removes a specific element from a vector
template<typename E,typename B> void removeFromVector(std::vector<B>& v,E e){
    for(auto it=v.begin();it!=v.end();){B c=(*it);if(c==e){it=v.erase(it);}else{++it;}}
}

//clears a vector, reset its size to zero, and removes the elements from memory. does NOT delete pointer elements
template <typename E> void vector_clear(E& t){ t.clear(); E().swap(t); t.shrink_to_fit(); }

//converts any type to its string representation
template <typename E> std::string to_string(E t){ return boost::lexical_cast<std::string>(t); }
//formats a number to have commas to represent thousandth places
template<typename T> std::string convertNumToNumWithCommas(T n){std::string r=to_string(n);int p=r.length()-3;while(p>0){r.insert(p,",");p-=3;}return r;}

#define SAFE_DELETE_COM(x) { if(x){ x->Release(); x = 0; } } // Convenience macro for releasing a COM object
#define SAFE_DELETE(x) { if(x){ delete x; x = 0;} } // Convenience macro for deleting a pointer
#define SAFE_DELETE_VECTOR(x){ for(auto it:x){ SAFE_DELETE(it); } vector_clear(x); } // Convenience macro for deleting the pointer elements of a vector
#define SAFE_DELETE_MAP(x){ for(auto it:x){ SAFE_DELETE(it.second); } x.clear(); } // Convenience macro for deleting the pointer elements of a map

#endif