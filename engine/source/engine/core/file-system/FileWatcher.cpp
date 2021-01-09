#include "engine-precompiled-header.h"
#include "FileWatcher.h"

void longmarch::filewatcher::fileWatcherThread(CFileSystemWatcher& watcherObj)
{
	HANDLE hDir = CreateFile(watcherObj.GetDir().c_str(), // pointer to the file name
		FILE_LIST_DIRECTORY,                // access (read/write) mode
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,  // share mode
		NULL,                               // security descriptor
		OPEN_EXISTING,                      // how to create
		FILE_FLAG_BACKUP_SEMANTICS,         // file attributes
		NULL                                // file with attributes to copy
	);
	wchar_t filename[MAX_PATH];
	FILE_NOTIFY_INFORMATION Buffer[1024];
	DWORD BytesReturned;
	while (watcherObj.Running())
	{
		if (watcherObj.timer.Check(true))
		{
			if (ReadDirectoryChangesW(
				hDir,                                  // handle to directory
				&Buffer,                                    // read results buffer
				sizeof(Buffer),                                // length of buffer
				TRUE,                                 // monitoring option
				FILE_NOTIFY_CHANGE_SECURITY |
				FILE_NOTIFY_CHANGE_CREATION |
				FILE_NOTIFY_CHANGE_LAST_ACCESS |
				FILE_NOTIFY_CHANGE_LAST_WRITE |
				FILE_NOTIFY_CHANGE_SIZE |
				FILE_NOTIFY_CHANGE_ATTRIBUTES |
				FILE_NOTIFY_CHANGE_DIR_NAME |
				FILE_NOTIFY_CHANGE_FILE_NAME,            // filter conditions
				&BytesReturned,              // bytes returned
				NULL,                          // overlapped buffer
				NULL// completion routine
			))
			{
				int offset = 0;
				FILE_NOTIFY_INFORMATION* pNotify;
				pNotify = (FILE_NOTIFY_INFORMATION*)((char*)Buffer + offset);
				wcscpy(filename, L"");

				wcsncpy(filename, pNotify->FileName, pNotify->FileNameLength / 2);

				filename[pNotify->FileNameLength / 2] = NULL;

				switch (Buffer[0].Action)
				{
				case FILE_ACTION_MODIFIED:
					watcherObj.OnFileChange(filename);
					break;
				case FILE_ACTION_ADDED:
					watcherObj.OnFileAdded(filename);
					break;
				case FILE_ACTION_REMOVED:
					watcherObj.OnFileRemoved(filename);
					break;
				case FILE_ACTION_RENAMED_OLD_NAME:
					watcherObj.OnFileRenamed(filename);
					break;
				case FILE_ACTION_RENAMED_NEW_NAME:
					watcherObj.OnFileRenamed(filename);
					break;
				}
			}
		}
	}

	CloseHandle(hDir);
}

longmarch::filewatcher::CFileSystemWatcher::CFileSystemWatcher(const std::wstring& logDir) :m_sDir(logDir), m_bRunning(false)
{
}

void longmarch::filewatcher::CFileSystemWatcher::AddFileChangeListener(IFileWatcherListener* listener)
{
	m_Listeners.push_back(listener);
}

void longmarch::filewatcher::CFileSystemWatcher::OnFileChange(const std::wstring& fileName)
{
	for (auto& listener : m_Listeners) {
		listener->OnFileChange(fileName);
	}
}

void longmarch::filewatcher::CFileSystemWatcher::OnFileAdded(const std::wstring& sFile)
{
	for (auto& listener : m_Listeners) {
		listener->OnFileAdded(sFile);
	}
}

void longmarch::filewatcher::CFileSystemWatcher::OnFileRemoved(const std::wstring& sFile)
{
	for (auto& listener : m_Listeners) {
		listener->OnFileRemoved(sFile);
	}
}

void longmarch::filewatcher::CFileSystemWatcher::OnFileRenamed(const std::wstring& sFile)
{
	for (auto& listener : m_Listeners) {
		listener->OnFileRenamed(sFile);
	}
}

void longmarch::filewatcher::CFileSystemWatcher::Start()
{
	m_bRunning = true;
	m_pFileWatcherThread = std::unique_ptr<std::thread>(new std::thread(fileWatcherThread, std::ref(*this)));
	m_pFileWatcherThread->detach();
}

void longmarch::filewatcher::CFileSystemWatcher::Stop()
{
	m_bRunning = false;
}

longmarch::filewatcher::CFileSystemWatcher::~CFileSystemWatcher()
{
	Stop();
}