/*
 * JILL - C++ framework for JACK
 *
 * Copyright (C) 2010 C Daniel Meliza <dmeliza@uchicago.edu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef _COUNTER_HH
#define _COUNTER_HH

#include <iosfwd>
#include <boost/noncopyable.hpp>
#include <deque>

namespace jill { namespace dsp {

/**
 * @ingroup miscgroup
 * @brief calculate running sum
 *
 * This class is a simple queue-based running counter. The queue has a
 * fixed size and data are pushed onto the queue one by one. The
 * running sum of the values in the queue is maintained by adding the
 * new value and subtracting the last value in the queue, which is
 * then dropped. A comparison is made between the running total and a
 * threshold.
 */
template <class T>
class running_counter : boost::noncopyable {

public:
	/** the data type stored in the counter */
	typedef T data_type;
	/** the data type for size information */
	typedef typename std::deque<data_type>::size_type size_type;

	/** Initialize the counter. @param size  the size of the running sum window */
	explicit running_counter(size_type size=0)
		: _size(size), _running_count(0) {}

	/**
	 * Add a value to the queue.
	 *
	 * @param count          the value to add
	 * @return the value dropped from the end of the queue, or zero
	 *         if the queue is not full
	 */
	data_type push(data_type count) {
		_counts.push_front(count);
		_running_count += count;
		if (_counts.size() <= _size)
			return 0;

		data_type back = _counts.back();
		_running_count -=  back;
		_counts.pop_back();
		return back;
	}

	/**
	 * Return whether the queue is full or not
	 *
	 * @return true iff the queue is at capacity
	 */
	bool is_full() const {
		return _counts.size() == _size;
	}

	/** @return the running total */
	data_type running_count() const { return _running_count; }

        /**
         * Change the number of elements in the running count. If the new size
         * is smaller, then the running total reflects the last @a size elements
         * added to the queue.  If the new size is larger, then the running
         * total remains the same.
         *
         * @param size  the new size of the queue
         */
        void resize(size_type size) {
                _size = size;
                // shrink queue by popping off back
                while (_counts.size() > _size) {
                        _running_count -= _counts.back();
                        _counts.pop_back();
                }
        }

	/** reset the counter */
	void reset() {
		_counts.clear();
		_running_count = 0;
	}

	friend std::ostream& operator<< (std::ostream &os, const running_counter<T> &o) {
		os << o._running_count << " [" << o._counts.size() << '/' << o._size << "] (";
		for (typename std::deque<T>::const_iterator it = o._counts.begin(); it != o._counts.end(); ++it)
			os << *it << ' ';
		return os << ')';
	}

private:
	/// the analysis window
	size_type _size;
	/// count of samples in complete blocks
	std::deque<data_type> _counts;
	/// a running count
	data_type _running_count;
};

}}

#endif
