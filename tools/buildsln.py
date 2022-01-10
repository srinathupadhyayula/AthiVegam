import globals
import os, sys, subprocess

CONFIG = "debug"
ret = 0

if globals.IsWindows():
    MS_BUILD_PATH = os.environ["MS_BUILD_PATH"][8:-1]
    MS_BUILD_PATH = "S:\\\\" + MS_BUILD_PATH
    MS_BUILD_PATH = MS_BUILD_PATH.replace("/", "\\\\")

    ret = subprocess.call(["cmd.exe", "/c", MS_BUILD_PATH, "{}.sln".format(globals.ENGINE_NAME), "/property:Configuration={}".format(CONFIG)])
    
if globals.IsLinux():
    ret = subprocess.call(["make", "config={}".format(CONFIG)])

if globals.IsMac():
    ret = subprocess.call(["make", "config={}".format(CONFIG)])

sys.exit(ret)
