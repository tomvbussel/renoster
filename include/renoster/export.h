#ifndef RENOSTER_EXPORT_H_
#define RENOSTER_EXPORT_H_

#if defined(_MSC_VER) || defined(__CYGWIN__)
#define RENO_IMPORT __declspec(dllimport)
#define RENO_EXPORT __declspec(dllexport)
#else
#define RENO_IMPORT __attribute__((visibility("default")))
#define RENO_EXPORT __attribute__((visibility("default")))
#endif

#if defined(LibRenoster_EXPORTS)
#define RENO_API RENO_EXPORT
#else
#define RENO_API RENO_IMPORT
#endif

#endif  // RENOSTER_EXPORT_H_