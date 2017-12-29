#ifndef RINGBUFFER_H
#define RINGBUFFER_H
template<typename T>
class Ringbuffer {
private:
	T * internalVector;
	int size, pos, actualSize;
	bool firstLoop;
public:
	class iterator;
public:
	Ringbuffer() : pos(0), size(10), actualSize(0), firstLoop(false) {
		internalVector = new T[10];
	}
	Ringbuffer(int size) : pos(0), size(size), actualSize(0), firstLoop(false) {
		internalVector = new T[size];
	}
	~Ringbuffer() {
		delete[] internalVector;
	}
	void setPos(int newPos) {
		if (newPos >= size || newPos < 0) pos = 0;
		else pos = newPos;
	}
	void addElement(T element) {
		if (pos >= size) {
			pos = 0;
			firstLoop = true;
		}
		else if (!firstLoop) {
			actualSize++;
		}
		internalVector[pos++] = element;
	}
	inline T & getElement(int pos) {
		return internalVector[pos];
	}
	inline T & getLastElement() {
		return internalVector[pos - 1];
	}
	inline iterator begin() {
		return iterator(0, *this);
	}
	inline iterator end() {
		return iterator(actualSize, *this);
	}
	inline int getSize() { return actualSize; }
};
template<typename T>
class Ringbuffer<T>::iterator {
private:
	int itPos;
	Ringbuffer & buffer;
public:
	iterator(int pos, Ringbuffer & buffer) : itPos(pos), buffer(buffer) {}
	inline iterator & operator++() {
		itPos++;
		return *this;
	}
	inline iterator & operator++(int) {
		itPos++;
		return *this;
	}
	inline bool operator!=(const iterator & other) {
		return itPos != other.itPos;
	}
	inline T & operator*() {
		return buffer.getElement(itPos);
	}
};
#endif //RINGBUFFER_H


