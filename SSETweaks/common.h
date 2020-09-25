#pragma once

#define FN_ESSENTIAL(x) virtual bool IsEssential() const { return x; };
#define FN_PRIO(x) virtual int GetPriority() const { return x; };
#define FN_DRVID(x) virtual int GetID() const { return x; };
