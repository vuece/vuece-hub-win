#pragma once

#include <iostream>
#include <jmutex.h>
#include "talk/base/logging.h"
#include "VueceJingleClientThread.h"
#include "VueceWinUtilities.h"

using namespace std;

#define VUECE_FILE_SYS_WATCHER_TIMER_START_DUE_TIME 3000
#define VUECE_FILE_SYS_WATCHER_TIMER_TIMEOUT_INTERVAL 5000
#define VUECE_FILE_SYS_WATCHER_TIMER_FIRE_THRESHOLD 10000

namespace VueceFileSystem
{
	using namespace System;
	using namespace System::Collections;
	using namespace System::Collections::Generic;
	using namespace System::IO;
	using namespace System::Threading;

  public ref class VueceDirectoryMonitor //: IDirectoryMonitor
  {
    private: System::IO::FileSystemWatcher^ m_fileSystemWatcher;
    private: Dictionary<String^ , DateTime>^ m_pendingEvents;
    private: Timer^ m_timer;
    private: bool m_timerStarted;
    private: TimerCallback^ tcb;
    private: JMutex* mutex;
    private: VueceJingleClientThread* clientWrapperThread;

  public: VueceDirectoryMonitor(String ^ dirPath, VueceJingleClientThread* clientThd)
	{
		m_timerStarted = false;
		m_fileSystemWatcher = gcnew System::IO::FileSystemWatcher();
		m_pendingEvents =  gcnew Dictionary<String^ , DateTime>();
		mutex = new JMutex();
		m_fileSystemWatcher->Path = dirPath;
		m_fileSystemWatcher->IncludeSubdirectories = false;

		clientWrapperThread = clientThd;

		// Create a delegate that invokes methods for the timer.
		tcb = gcnew TimerCallback(this, &VueceDirectoryMonitor::OnTimeout);

		m_fileSystemWatcher->Created += gcnew FileSystemEventHandler(this, &VueceDirectoryMonitor::OnShareFolderChange);
		m_fileSystemWatcher->Changed += gcnew FileSystemEventHandler(this, &VueceDirectoryMonitor::OnShareFolderChange);
		m_fileSystemWatcher->Deleted += gcnew FileSystemEventHandler(this, &VueceDirectoryMonitor::OnShareFolderChange);
		m_fileSystemWatcher->Renamed += gcnew RenamedEventHandler(this, &VueceDirectoryMonitor::OnShareFolderChange_Renamed);
		m_timer = gcnew Timer(tcb, nullptr, Timeout::Infinite, Timeout::Infinite);
    }

  public: void StartMonitor()
    {

		LOG(INFO) << "VueceDirectoryMonitor - Start";
		m_fileSystemWatcher->EnableRaisingEvents = true;
    }


  private: void OnShareFolderChange_Renamed(System::Object^  sender, RenamedEventArgs^  e)
	{
		std::string stdString;

		VueceWinUtilities::MarshalString(e->OldFullPath, stdString);
		LOG(INFO) << "VueceDirectoryMonitor:OnShareFolderChange_Renamed: Old full path: "  << stdString;

		VueceWinUtilities::MarshalString(e->FullPath, stdString);
		LOG(INFO) << "VueceDirectoryMonitor:OnShareFolderChange_Renamed: New full path: "  << stdString;


		//do the same thing as OnShareFolderChange

		mutex->Lock();

        // Save a timestamp for the most recent event for this path
        m_pendingEvents[e->FullPath] = DateTime::Now;

        // Start a timer if not already started
        if (!m_timerStarted)
        {
          m_timer->Change(VUECE_FILE_SYS_WATCHER_TIMER_START_DUE_TIME, VUECE_FILE_SYS_WATCHER_TIMER_TIMEOUT_INTERVAL);
          m_timerStarted = true;
        }

		mutex->Unlock();

	}


  private: void OnShareFolderChange(System::Object^ sender, FileSystemEventArgs^ e)
    {
      // Don't want other threads messing with the pending events right now
		std::string stdString;
		VueceWinUtilities::MarshalString(e->FullPath, stdString);
		LOG(INFO) << "VueceDirectoryMonitor - OnShareFolderChange: " << stdString;

		mutex->Lock();

        // Save a timestamp for the most recent event for this path
        m_pendingEvents[e->FullPath] = DateTime::Now;

        // Start a timer if not already started
        if (!m_timerStarted)
        {
          m_timer->Change(VUECE_FILE_SYS_WATCHER_TIMER_START_DUE_TIME, VUECE_FILE_SYS_WATCHER_TIMER_TIMEOUT_INTERVAL);
          m_timerStarted = true;
        }

		mutex->Unlock();
    }

  private: void OnTimeout(System::Object^ state)
    {
		LOG(INFO) << "VueceDirectoryMonitor - OnTimeout: ";

		List<String^>^ paths;

		// Don't want other threads messing with the pending events right now
		// lock (m_pendingEvents)
		// {
		mutex->Lock();

		// Get a list of all paths that should have events thrown
		paths = FindReadyPaths(m_pendingEvents);

		// Remove paths that are going to be used now

		for each(String^ p in paths )
		{
				m_pendingEvents->Remove(p);
		}

		// Stop the timer if there are no more events pending
		if (m_pendingEvents->Count == 0)
		{
			m_timer->Change(Timeout::Infinite, Timeout::Infinite);
			m_timerStarted = false;
		}
	//  }

		mutex->Unlock();

		// Fire an event for each path that has changed
		for each(String^ p in paths )
		{
				FireEvent(p);
		}
    }

  private: List<String ^>^ FindReadyPaths(Dictionary<String ^, DateTime>^ events)
    {
	  List<String ^>^ results = gcnew List<String ^>();
	  DateTime now = DateTime::Now;

	  for each (KeyValuePair<String ^, DateTime> entry in events)
	  {
		// If the path has not received a new event in the last 75ms
		// an event for the path should be fired
		double diff = now.Subtract(entry.Value).TotalMilliseconds;
		if (diff >= VUECE_FILE_SYS_WATCHER_TIMER_FIRE_THRESHOLD)
		{
			LOG(INFO) << "VueceDirectoryMonitor - FindReadyPaths: Found an event that has not been updated within " <<
					VUECE_FILE_SYS_WATCHER_TIMER_FIRE_THRESHOLD << "ms, it will be fired";
			results->Add(entry.Key);
		}
	  }

	  return results;
    }

  private: void FireEvent(String ^ path)
    {
		std::string stdString;
		VueceWinUtilities::MarshalString(path, stdString);
		LOG(INFO) << "VueceDirectoryMonitor - FireEvent: " << stdString;

		clientWrapperThread->OnFileSystemChanged();
    }
  };

}
