# Cpp Vector Implementation

# Header

## DynamicArray.h

-- This is where the data structure is implemented

## Log.h and Log.cpp

-- This is a logging singleton class built around the **spdlog** library
-- In order to use this you will need to *#define _DEBUG 1* or *0* in 
   visual studio's properties page or before the *#if _DEBUG*
-- You will need to call **reda::Log::Init();** in the entry point of your program to 
   initialize and start using.

# spdlog Library

- GitHub: https://github.com/gabime/spdlog
- License: https://github.com/gabime/spdlog/blob/v1.x/LICENSE