#pragma once
#ifndef _DYNAMIC_ARRAY_
#define _DYNAMIC_ARRAY_

/*
 * Always refer to the EASTL library for all the references 
 */

/*
 * The commented code lines in ~Vector() and ReAlloc() actually cause a memory leak since if in the main 
 * function that use of this class was SCOPED and contains a type that includes a pointer of some sort, the 
 * clear function and popback will manually call the destructor of that type thus cleaning it up. However
 * when that scope ends, (and in case clear and popBack were used), the memory has already been cleared by 
 * these two functions. Therefore the program will crash since the destructor of that type will try to 
 * cleanup memory that has already been cleaned..
 * In addition, if a Move Constructor was used to move from one vector to the other, the vector that is the 
 * source of the elements is no longer valid after the move happens and operations on it will maybe crash
 */

////////////////////////////////////////////////////////////////////////////////////////////////
///////// For Development 
////////////////////////////////////////////////////////////////////////////////////////////////


#include <initializer_list>

#include "Log.h"


namespace reda {

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	//////   Vector Iterator Class
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	
	template<typename Vector>
	class VectorIterator
	{
	public:
		using ValueType = typename Vector::ValueType;
		using PointerType = ValueType*;
		using ReferenceType = ValueType&;

	public:
		VectorIterator(PointerType ptr)
			: m_Ptr(ptr) {}

		VectorIterator& operator++()
		{
			++m_Ptr;
			return *this;
		}

		VectorIterator operator++(int)
		{
			VectorIterator iterator = *this;
			++(*this);
			return iterator;
		}

		VectorIterator& operator--()
		{
			--m_Ptr;
			return *this;
		}

		VectorIterator operator--(int)
		{
			VectorIterator iterator= *this;
			--(*this);
			return iterator;
		}

		VectorIterator operator+(int right)
		{
			return m_Ptr + right;
		}

		VectorIterator operator-(int right)
		{
			return m_Ptr - right;
		}

		ReferenceType operator[](int index)
		{
			return *(m_Ptr + index); // Could also be written as m_Ptr[index] since it is a pointer but this 
		}							 // also works...

		bool operator==(const VectorIterator& right) const
		{
			return m_Ptr == right.m_Ptr;
		}

		bool operator!=(const VectorIterator& right) const
		{
			return !(*this == right);
		}

		PointerType operator->() const
		{
			return m_Ptr; // The pointer here is also the position we are at.
		}

		ReferenceType operator*() const
		{
			return *m_Ptr;
		}
	protected:
		PointerType m_Ptr;
	};

	template<typename Vector>
	class ConstVectorIterator : public VectorIterator<Vector>
	{
	public:
		using valueType = typename Vector::ValueType;
		using reference = const valueType&;

		reference operator*() const
		{
			return *(this->m_Ptr);
		}

		ConstVectorIterator& operator++()
		{
			++(this->m_Ptr);
			return *this;
		}

		ConstVectorIterator operator++(int)
		{
			ConstVectorIterator iterator = *this;
			++(*this);
			return iterator;
		}
		ConstVectorIterator& operator--()
		{
			--(this->m_Ptr);
			return *this;
		}

		ConstVectorIterator operator--(int)
		{
			ConstVectorIterator iterator = *this;
			--(*this);
			return iterator;
		}

	};


	/////////////////////////////////////////////////////////////////////////////////////////////////////
	//////   Vector Class
	/////////////////////////////////////////////////////////////////////////////////////////////////////


	template<typename Type>
	class Vector {
	public:
		using ValueType = Type;
		using Iterator = VectorIterator<Vector<Type>>;
		using const_Iterator = ConstVectorIterator<const Vector<Type>>;

	public:
		Vector()
		{							// Normal Constuctor
			// Allocate 2 elements
			ReAlloc(2);
		}

		Vector(const std::initializer_list<Type>& initList)
		{
			ReAlloc(2);

			for (auto& it : initList)
				emplaceBack(it);
		}

		Vector(const Vector& other)			// Copy Constructor
			: m_Size(other.m_Size), m_Capacity(other.m_Size)
		{
			CORE_LOG_INFO("VECTOR COPY CALLED");
			m_Data = CopyAlloc(m_Size, other);
		}
		
		Vector(Vector&& other)				// Move Constructor
			: m_Size(other.m_Size), m_Capacity(other.m_Size)
		{
			CORE_LOG_INFO("VECTOR MOVE CALLED");

			m_Data = MoveAlloc(m_Size, other);

			other.m_Size = 0;
			other.m_Capacity = 0;
			other.m_Data = nullptr;
		}

		~Vector()
		{
			CORE_LOG_ERROR("Vector Deleted!");

			//delete[] m_Data;
			clear();
			::operator delete(m_Data, m_Capacity * sizeof(Type));
		}

		// Reserves the specified number of elements in the vecto. This method is adviced to be used
		// ONLY after the declaration of the object
		void reserve(size_t capacity)
		{
			if (capacity > m_Capacity)
				ReAlloc(capacity);
		}

		// Shrinks the vector's capacity to its size, thus conserving memory. If this was used after 
		// declaration it shrink the vector's capacity down to 0
		void shrinkToSize()
		{
			if (m_Size <= m_Capacity) {
				ReAlloc(m_Size);
				m_Capacity = m_Size;
			}
		}

		// Pushes back an element to the vector
		void pushBack(const Type& val) {
			if (m_Size >= m_Capacity) 
				ReAlloc(m_Capacity + (m_Capacity / 2));

			new((void*)&m_Data[m_Size++]) Type(val); // If this was m_Data[m_Size++] = val, it would crash under
											  // under the error that this is uninitialized memory
											  // therefore we have to use the "Placement new" operator
		}

		// Pushes back an element to the vector
		void pushBack(Type&& val)
		{
			if (m_Size >= m_Capacity)
				ReAlloc(m_Capacity + m_Capacity / 2);

			new((void*)&m_Data[m_Size++]) Type(std::move(val)); // Same comment as the one above ^^^^
		}

		// Emplaces back an element into the vector by MOVING it and not copying it
		template<typename... Args>
		Type& emplaceBack(Args&&... args)
		{
			if (m_Size >= m_Capacity) 
				ReAlloc(m_Capacity + (m_Capacity / 2));

			new((void*)&m_Data[m_Size]) Type(std::forward<Args>(args)...);

			return m_Data[m_Size++];
		}

		// Insert val at specified index by constructing it in its position
		template<typename... Args>
		Type& emplaceAt(uint32_t index, Args&&... args)
		{
			if (index == m_Size - 1) {
				Type temp = emplaceBack(args...);
				return temp;
			}

			if (m_Size >= m_Capacity)
				ReAlloc(m_Capacity + m_Capacity / 2);

			new((void*)&m_Data[m_Size]) Type(std::move(m_Data[m_Size - 1]));

			for (size_t i = m_Size - 1; i > index; i--)
				m_Data[i] = std::move(m_Data[i - 1]);

			new((void*)&m_Data[index]) Type(std::forward<Args>(args)...);

			return m_Data[m_Size++];
		}

		// Insert val at specified index
		void insertAt(uint32_t index, Type&& val)
		{
			if (index == m_Size - 1) {
				pushBack((Type&&)val);
				return;
			}

			if (m_Size >= m_Capacity)
				ReAlloc(m_Capacity + m_Capacity / 2);
			
			// This has to be done since what i was doing first in the for loop was actually trying to use 
			// UnInitialized memory and so it was crashing. This way i am initializing the memory for the extra
			// element that the vector will use and at the same time i was declaring it to be the element 
			// before it. Now in the for loop all i have to do is actually move (shift) the elements forward 
			// by 1 and then setting the specified index to the specified object.
			// ALWAYS REMEMBER THAT YOU NEED TO INITIALIZE THE MEMORY
			new((void*)&m_Data[m_Size]) Type(std::move(m_Data[m_Size - 1]));

			for (size_t i = m_Size - 1; i > index; i--)
				m_Data[i] = std::move(m_Data[i - 1]);

			m_Data[index] = std::move(val);

			m_Size++;
		}

		// Pops the last element in the vector
		void popBack()
		{
			if (m_Size > 0) {
				m_Size--;
				m_Data[m_Size].~Type();
			}
		}

		// Removes the element at the specified index in the vector
		void removeAt(size_t index)
		{
			if (m_Size > 0) {
				if (index == m_Size) {
					popBack();
					return;
				}
				
				m_Size--;
				if (index < m_Size) {
					m_Data[index].~Type();

				for (size_t i = index; i < m_Size; i++)
					m_Data[i] = std::move(m_Data[i + 1]); //m_Data[i] = nullptr;
				}
			}
		}

		// Clears all the elements in the vector
		void clear()
		{
			for (size_t i = 0; i < m_Size; i++)
				m_Data[i].~Type();

			m_Size = 0;
		}

		// Returns the size of the vector
		uint32_t size() const { return m_Size; }

		// Returns True if the vector is empty
		bool isEmpty() const { return (m_Size == 0) ? true : false; }

		// Returns true if the specified value was found in the vector
		bool contains(Type val)
		{
			for (size_t i = 0; i < m_Size; i++) {
				if (m_Data[i] == val)
					return true;
			}

			return false;
		}

		// Returns the index of the first instance of the specified value in the vector, if not found returns -1
		int indexOf(Type val)
		{
			for (size_t i = 0; i < m_Size; i++) {
				if (m_Data[i] == val)
					return i;
			}

			return -1;
		}

#if _DEBUG
		uint32_t capacity() const { return m_Capacity; }
#endif
		////////////////////////////////////////////////////////
		// Operator overloads...
		////////////////////////////////////////////////////////

		Vector& operator=(const Vector& other) // Copy assignment operator
		{
			CORE_LOG_INFO("VECTOR COPY ASSIGNMENT CALLED");

			m_Size = other.m_Size;
			m_Capacity = other.m_Capacity;
			CopyAlloc(m_Size, other);

			return *this;
		}

		Vector& operator=(Vector&& other) // Move assignment operator
		{
			CORE_LOG_INFO("VECTOR MOVE ASSIGNMENT CALLED");

			m_Size = other.m_Size;
			m_Capacity = m_Capacity;

			m_Data = MoveAlloc(m_Size, other);

			other.m_Size = 0;
			other.m_Capacity = 0;
			other.m_Data = nullptr;

			return *this;
		}

		Type& operator[](size_t index)
		{
			return m_Data[index];
		}

		const Type& operator[](size_t index) const
		{
			return m_Data[index];
		}

		////////////////////////////////////////////////////////
		// Iterator functionality...
		////////////////////////////////////////////////////////

		// Returns an iterator pointing to the first element in the vector
		Iterator begin() const
		{
			return Iterator(m_Data);
		}

		// Returns a READ-ONLY iterator pointing to the first element in the vector
		const_Iterator cbegin() const
		{
			return const_Iterator(m_Data);
		}

		// Returns an iterator pointing to the last element in the vector
		Iterator end() const
		{
			return Iterator(m_Data + m_Size);
		}

		// Returns a READ-ONLY iterator pointing to the last element in the vector
		const_Iterator cend() const
		{
			return const_Iterator(m_Data + m_Size);
		}

	private:
		// This function is used to allocate and reallocate memory for a newblock and supports shrinking and 
		// also increasing the size.
		void ReAlloc(size_t newCapacity)
		{
			// 1. allocate a new block of memory
			// 2. copy (try to move) old elements into new block
			// 3. delete old

			CORE_LOG_WARN("REALLOC CALLED");

			Type* newBlock = (Type*)::operator new(newCapacity * sizeof(Type));
			//Type* newBlock = new Type[newCapacity];

			if (newCapacity < m_Size)
				m_Size = newCapacity;

			for (size_t i = 0; i < m_Size; i++)
				new((void*)&newBlock[i]) Type(std::move(m_Data[i]));
			// Can not use memcpy() like i was doing since using it will not call the copy 
			// Constructor of the elements if they were Struct or Class pointers.

			::operator delete(m_Data, m_Capacity * sizeof(Type));
			//delete[] m_Data;
			m_Data = newBlock;
			newBlock = nullptr;
			m_Capacity = newCapacity;
		}

		// Same as the ReAlloc function however this is used only in Copy Constructor and operator
		Type* CopyAlloc(size_t newCapacity, const Vector& other)
		{
			CORE_LOG_WARN("COPYALLOC CALLED");

			m_Data = (Type*)::operator new(newCapacity * sizeof(Type));
			
			for (size_t i = 0; i < other.m_Size; i++)
				new((void*)&m_Data[i]) Type(other.m_Data[i]);

			return m_Data;
		}

		// Same as the CopyAlloc function however this is used only in Move Constructor and operator
		Type* MoveAlloc(size_t newCapacity, const Vector& other)
		{
			CORE_LOG_WARN("MOVEALLOC CALLED");

			m_Data = (Type*)::operator new(newCapacity * sizeof(Type));

			for (size_t i = 0; i < other.m_Size; i++)
				new((void*)&m_Data[i]) Type(std::move(other.m_Data[i]));

			return m_Data;
		}

	private:
		size_t m_Size = 0;
		size_t m_Capacity = 0;
		Type* m_Data = nullptr;
	};
}

#endif