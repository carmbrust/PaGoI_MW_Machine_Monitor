/*
Filename: debug.h
Author: Chris Armbrust, Marin Digital, (c)2017-2020
Version: 1.0
Date: 31May2018
*/
#include "main.h"

#ifndef Debug_H
 #define Debug_H//------------------------------------------------------------------------------
    #ifndef RELEASED //  this could be DEBUG or DEBUG_PORT
        #define DEBUG_MSG(...) DEBUG_PORT.printf( __VA_ARGS__ )
    #else
        #define DEBUG_MSG(...)
   #endif
#endif
