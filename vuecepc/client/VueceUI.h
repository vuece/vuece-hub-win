/*
 * VueceUI.h
 *
 *  Created on: Feb 3, 2013
 *      Author: Jingjing Sun
 */

#ifndef VUECEUI_H_
#define VUECEUI_H_

#include <iostream>

#include "VueceConstants.h"

using namespace std;

// Base class
public interface class VueceUI
{
public:
   // pure virtual function providing interface framework.
   void OnUserEvent(int code) = 0;
   void OnRemoteDeviceActivity(VueceStreamingDevice* d) = 0;
   void OnRosterVCardReceived(const std::string&, const std::string&, const std::string&) = 0;
   void OnRosterSubscriptionMsgReceived(VueceRosterSubscriptionMsg* m) = 0;
};


#endif /* VUECEUI_H_ */
