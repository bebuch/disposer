//-----------------------------------------------------------------------------
// Copyright (c) 2015-2017 Benjamin Buch
//
// https://github.com/bebuch/disposer
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)
//-----------------------------------------------------------------------------
#ifndef _disposer__tool__input_data__hpp_INCLUDED_
#define _disposer__tool__input_data__hpp_INCLUDED_

#include <vector>


namespace disposer{


	template < typename T, bool ReadOnly >
	class input_data;


	/// \brief Read and move out range view to input data
	template < typename T >
	class input_data< T, false >{
		using container_type = std::vector< T >;
	public:
		/// \brief Move random access iterator
		using iterator =
			std::move_iterator< typename container_type::iterator >;

		/// \brief Constant random access iterator
		using const_iterator =
			typename container_type::const_iterator;

		/// \brief Reverse move random access iterator
		using reverse_iterator =
			std::move_iterator< typename container_type::reverse_iterator >;

		/// \brief Constant reverse random access iterator
		using const_reverse_iterator =
			typename container_type::const_reverse_iterator;

		/// \brief Size type
		using size_type = typename container_type::size_type;


		/// \brief Constructor
		input_data(container_type&& data)
			: data_(std::move(data)) {}

		/// \brief input_data is nighter copy nor movable
		input_data(input_data const&) = delete;

		/// \brief input_data is nighter copy- nor move- assignable
		input_data& operator=(input_data const&) = delete;


		/// \brief Access specified element with bounds checking
		T&& at(size_type pos)&&{
			return std::move(data_.at(pos));
		}

		/// \brief Access specified element with bounds checking
		T& at(size_type pos)&{
			return data_.at(pos);
		}

		/// \brief Access specified element with bounds checking
		T const& at(size_type pos)const&{
			return data_.at(pos);
		}


		/// \brief Access specified element
		T&& operator[](size_type pos)&&
		noexcept(noexcept(std::declval< container_type >()[pos])){
			return std::move(data_[pos]);
		}

		/// \brief Access specified element
		T& operator[](size_type pos)&
		noexcept(noexcept(std::declval< container_type >()[pos])){
			return data_[pos];
		}

		/// \brief Access specified element
		T const& operator[](size_type pos)const&
		noexcept(noexcept(std::declval< container_type const >()[pos])){
			return data_[pos];
		}


		/// \brief Checks whether the container is empty
		bool empty()const noexcept{ return data_.empty(); }

		/// \brief Returns the number of elements
		size_type size()const noexcept{ return data_.size(); }


		/// \brief Returns a move iterator to the beginning
		iterator begin()noexcept{
			return iterator(data_.begin());
		}

		/// \brief Returns a constant iterator to the beginning
		const_iterator begin()const noexcept{
			return data_.begin();
		}

		/// \brief Returns a constant iterator to the beginning
		const_iterator cbegin()const noexcept{
			return data_.cbegin();
		}


		/// \brief Returns a move iterator to the end
		iterator end()noexcept{
			return iterator(data_.end());
		}

		/// \brief Returns a constant iterator to the end
		const_iterator end()const noexcept{
			return data_.end();
		}

		/// \brief Returns a constant iterator to the end
		const_iterator cend()const noexcept{
			return data_.cend();
		}


		/// \brief Returns a reverse move iterator to the beginning
		reverse_iterator rbegin()noexcept{
			return reverse_iterator(data_.rbegin());
		}

		/// \brief Returns a reverse constant iterator to the beginning
		const_reverse_iterator rbegin()const noexcept{
			return data_.rbegin();
		}

		/// \brief Returns a reverse constant iterator to the beginning
		const_reverse_iterator crbegin()const noexcept{
			return data_.crbegin();
		}


		/// \brief Returns a reverse move iterator to the end
		reverse_iterator rend()noexcept{
			return reverse_iterator(data_.rend());
		}

		/// \brief Returns a reverse constant iterator to the end
		const_reverse_iterator rend()const noexcept{
			return data_.rend();
		}

		/// \brief Returns a reverse constant iterator to the end
		const_reverse_iterator crend()const noexcept{
			return data_.crend();
		}


	private:
		/// \brief rvalue reference to the data
		container_type data_;
	};


	/// \brief Read only range view to input data
	template < typename T >
	class input_data< T, true >{
		using container_type = std::vector< T >;
	public:
		/// \brief Move random access iterator
		using iterator =
			typename container_type::const_iterator;

		/// \brief Constant random access iterator
		using const_iterator = iterator;

		/// \brief Reverse move random access iterator
		using reverse_iterator =
			typename container_type::const_reverse_iterator;

		/// \brief Constant reverse random access iterator
		using const_reverse_iterator = reverse_iterator;

		/// \brief Size type
		using size_type = typename container_type::size_type;


		/// \brief Constructor
		input_data(container_type const& data)
			: data_(data) {}

		/// \brief input_data is nighter copy nor movable
		input_data(input_data const&) = delete;

		/// \brief input_data is nighter copy- nor move- assignable
		input_data& operator=(input_data const&) = delete;


		/// \brief Access specified element with bounds checking
		T const& at(size_type pos)const&{
			return data_.at(pos);
		}

		/// \brief Access specified element
		T const& operator[](size_type pos)const&
		noexcept(noexcept(std::declval< container_type const& >()[pos])){
			return data_[pos];
		}


		/// \brief Checks whether the container is empty
		bool empty()const noexcept{ return data_.empty(); }

		/// \brief Returns the number of elements
		size_type size()const noexcept{ return data_.size(); }


		/// \brief Returns a constant iterator to the beginning
		const_iterator begin()const noexcept{
			return data_.begin();
		}

		/// \brief Returns a constant iterator to the beginning
		const_iterator cbegin()const noexcept{
			return data_.cbegin();
		}


		/// \brief Returns a constant iterator to the end
		const_iterator end()const noexcept{
			return data_.end();
		}

		/// \brief Returns a constant iterator to the end
		const_iterator cend()const noexcept{
			return data_.cend();
		}


		/// \brief Returns a reverse constant iterator to the beginning
		const_reverse_iterator rbegin()const noexcept{
			return data_.rbegin();
		}

		/// \brief Returns a reverse constant iterator to the beginning
		const_reverse_iterator crbegin()const noexcept{
			return data_.crbegin();
		}


		/// \brief Returns a reverse constant iterator to the end
		const_reverse_iterator rend()const noexcept{
			return data_.rend();
		}

		/// \brief Returns a reverse constant iterator to the end
		const_reverse_iterator crend()const noexcept{
			return data_.crend();
		}


	private:
		/// \brief rvalue reference to the data
		container_type const& data_;
	};


	/// \brief Range like interface owning the data
	template < typename T >
	using input_data_v = input_data< T, false >;

	/// \brief Range like interface refering to the data
	template < typename T >
	using input_data_r = input_data< T, true >;


}


#endif
