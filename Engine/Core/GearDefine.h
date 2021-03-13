#pragma once

#ifdef GEAR_EXPORTS
#define GEAR_EXPORT __declspec(dllexport)
#else
#define GEAR_EXPORT __declspec(dllimport)
#endif
