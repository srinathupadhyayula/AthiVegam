ENGINE_NAME = "AthiVegam"
PROJECT_NAME = "Parugu"
TOOLS_DIR = "tools"

V_MAJOR = 0
V_MINOR = 0

import sys, platform

PLATFORM = sys.platform
PLATFORM_WINDOWS = "windows"
PLATFORM_LINUX = "linux"
PLATFORM_MACOS = "darwin"

for x in platform.uname():
	if "microsoft" in x.lower():
		PLATFORM = PLATFORM_WINDOWS
		break

def IsWindows():
	return PLATFORM == PLATFORM_WINDOWS

def IsLinux():
	return PLATFORM == PLATFORM_LINUX

def IsMac():
	return PLATFORM == PLATFORM_MACOS
