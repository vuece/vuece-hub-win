/*
 * VueceProgressUI.h
 *
 *  Created on: May 18, 2013
 *      Author: Jingjing Sun
 */

#ifndef VUECEPROGRESSUI_H_
#define VUECEPROGRESSUI_H_

#include <iostream>

using namespace std;

// Base class
public interface class VueceProgressUI
{
public:
	const static int MSG_HINT_SCANNING = 0;
	const static int MSG_HINT_SCAN_DONE= 1;
public:
   // pure virtual function providing interface framework.
   void OnMessage(int hint, const std::string& msg) = 0;
   //error occured: -1
   //no song scanned: 0;
   void OnFinish(int numScannedItem) = 0;
};


#endif /* VUECEPROGRESSUI_H_ */
