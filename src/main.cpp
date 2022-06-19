#include "DynamicArray.h"
#include "Log.h"
#include <iostream>

class Point
{
private:
	int x, y;
	int* m_MemoryBlock = nullptr;
public:
	Point()
		: x(0), y(0)
	{
		m_MemoryBlock = new int[5];
		for (int i = 0; i < 5; i++)
			m_MemoryBlock[i] = 0;
	}

	Point(int x, int y)
		: x(x), y(y)
	{
		m_MemoryBlock = new int[5];
		for (int i = 0; i < 5; i++)
			m_MemoryBlock[i] = 0;
	}

	Point(const Point& other) // Copy Constructor
	{
		x = other.x;
		y = other.y;

		// This is a way to allocate on the stack not on the heap
		// Just thorwing it out there :p
		/* m_MemoryBlock = (int*)alloca(5 * sizeof(int)); */

		m_MemoryBlock = new int[5];

		memcpy(m_MemoryBlock, other.m_MemoryBlock, 5 * sizeof(int));
	}

	Point(Point&& other) noexcept // Move Constructor
		: x(other.x), y(other.y)
	{
		m_MemoryBlock = other.m_MemoryBlock;
		other.m_MemoryBlock = nullptr;

		LOG_INFO("Point Move");
	}
 
	~Point() { // Destructor
		LOG_ERROR("Point Destroy");
			
		delete[] m_MemoryBlock;
	}

	Point& operator=(const Point& other) // Copy assignment operator
	{
		LOG_INFO("Point Assignment Copy Called");

		x = other.x;
		y = other.y;
		m_MemoryBlock = new int[5];
		memcpy(m_MemoryBlock, other.m_MemoryBlock, 5 * sizeof(int));
		
		return *this;
	}

	Point& operator=(Point&& other) noexcept { // Move assignment operator
		LOG_INFO("Point Assignment Move Called");

		m_MemoryBlock = other.m_MemoryBlock;
		other.m_MemoryBlock = nullptr;
		x = other.x;
		y = other.y;

		other.x = 0;
		other.y = 0;

		return *this;
	}

	friend std::ostream& operator<<(std::ostream& stream, const Point& pnt);
};

std::ostream& operator<<(std::ostream& stream, const Point& pnt) {

	stream << "X: " << pnt.x << "  Y: " << pnt.y;
	return stream;
}

template<typename T>
void printVector(const reda::Vector<T>& vec) {
	for (auto& it : vec) //Using reference after auto will cause the Point to move and not copy
		std::cout << it << std::endl;
	std::cout << "---------------------------------" << std::endl;
}

int main()
{
	reda::Log::Init(); // Initializing the logging system

	{
		reda::Vector<Point> temp{ {5, 5}, {6, 6} };
		temp.emplaceAt(1, 10, 10);
		printVector(temp);
		temp.insertAt(1, { 15, 15 });
		printVector(temp);
		for (reda::Vector<Point>::const_Iterator it = temp.cbegin(); it != temp.cend(); ++it)
			std::cout << *it << std::endl;
	}

	std::cin.get();
}