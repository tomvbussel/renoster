#ifndef RENOSTER_UTIL_FILESYSTEM_H_
#define RENOSTER_UTIL_FILESYSTEM_H_

#include <string>
#include <vector>

namespace renoster {

bool PathIsAbsolute(const std::string & path);

bool PathIsRelative(const std::string & path);

//
std::string ParentPath(const std::string & path);

// Return the filename of a file path
std::string Filename(const std::string & filepath);

// Return the extension of a file path
std::string Extension(const std::string & filepath);

// Return the stem of a file path
std::string Stem(const std::string & filepath);

// Split a search path (paths separated by ':' or ';') into a list of paths.
void SplitSearchPath(const std::string & searchpath, std::vector<std::string> & paths);

// Find the path to the first occurrence of a file in a search path
std::string FindSearchPath(const std::string & filename,
        const std::vector<std::string> & paths,
        bool recursive = true);

bool PathExists(const std::string & path);

} // namespace renoster

#endif // RENOSTER_UTIL_FILESYSTEM_H_
