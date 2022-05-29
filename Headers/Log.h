#pragma once

// This is a logger class.

#include <iostream>
namespace reda {
	class Logger {
	public:
		Logger(const Logger&) = delete;

		Logger& operator=(const Logger&) = delete;

		static Logger* get() {
			static Logger s_Instance;
			return &s_Instance;
		}

		template<typename T, typename... Args>
		void trace(T msg, Args... args) {
			std::cout << msg << std::endl;

			trace(args...);
		}

		template<typename T>
		void trace(T msg) {
			std::cout << msg << std::endl;
		}
	private:
		Logger() = default;
	};
}