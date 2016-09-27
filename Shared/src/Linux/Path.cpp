#include "stdafx.h"
#include "Path.hpp"
#include "Log.hpp"
#include "File.hpp"
#include "Math.hpp"

/*
	Linux version
*/
#include <unistd.h>
#include <linux/limits.h>
#include <sys/stat.h>
// Convenience
#define MAX_PATH PATH_MAX

char Path::sep = '/';

bool Path::CreateDir(const String& path)
{
	return mkdir(*path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
}
bool Path::Delete(const String& path)
{
    return remove(*path) == 0;
}
bool Path::DeleteDir(const String& path)
{
	if(!ClearDir(path))
		return false;
    return rmdir(*path) == 0;
}
bool Path::Rename(const String& srcFile, const String& dstFile, bool overwrite)
{
	if(FileExists(*dstFile))
	{
		if(!overwrite)
			return false;
		if(Delete(*dstFile))
		{
			Logf("Failed to rename file, overwrite was true but the destination could not be removed", Logger::Warning);
			return false;
		}
	}
	return rename(*srcFile, *dstFile) == 0;
}
bool Path::Copy(const String& srcFile, const String& dstFile, bool overwrite)
{
    File src;
    if(!src.OpenRead(srcFile))
        return false;
    File dst;
    if(!dst.OpenWrite(dstFile))
        return false;

    size_t sizeMax = src.GetSize();
    size_t remaining = sizeMax;
    char buffer[8096];

    // Do a buffered copy
    while(remaining > 0)
    {
        size_t read = Math::Min(remaining, sizeof(buffer));
        if(src.Read(buffer, read) != read)
            return false;
        if(dst.Write(buffer, read) != read)
            return false;
        remaining -= read;
    }
    return true;
}
String Path::GetCurrentPath()
{
	char currDir[MAX_PATH];
	getcwd(currDir, MAX_PATH);
	return currDir;
}
String Path::GetExecutablePath()
{
	char filename[MAX_PATH];

	pid_t pid = getpid();

	// Get name from pid
	char path[MAX_PATH];
	sprintf(path, "/proc/%d/exe", pid);
	int r = readlink(path, filename, PATH_MAX);
	assert(r != -1);

	return String(filename, filename + r);
}
String Path::GetTemporaryPath()
{
	char out[MAX_PATH];
	String curr = GetCurrentPath();
	sprintf(out, "%s/tempXXXXXX", *curr);
	mktemp(out);
	return out;
}
String Path::GetTemporaryFileName(const String& path, const String& prefix)
{
	char out[MAX_PATH];
	sprintf(out, "%s/%sXXXXXX", *path, *prefix);
	mktemp(out);
	return out;
}

bool Path::IsDirectory(const String& path)
{
	struct stat sb;
    if (stat(*path, &sb) == 0 && S_ISDIR(sb.st_mode))
		return true;
	return false;
}

bool Path::FileExists(const String& path)
{
	return access(*path, F_OK) != -1;
}
String Path::Normalize(const String& path)
{
	char out[MAX_PATH];
	realpath(*path, out);
	for(uint32 i = 0; i < MAX_PATH; i++)
	{
		if(out[i] == '\\')
			out[i] = sep;
	}
	return out;
}
bool Path::IsAbsolute(const String& path)
{
	if(path.length() > 1 && path[0] == '/')
		return true;
	return false;
}
