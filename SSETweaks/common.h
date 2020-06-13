#pragma once

#define FN_NAMEPROC(x) virtual const char *ModuleName() { return x; };
#define FN_ESSENTIAL(x) virtual bool IsEssential() { return x; };
#define FN_PRIO(x) virtual int GetPriority() { return x; };
#define FN_DRVID(x) virtual int GetID() { return x; };
