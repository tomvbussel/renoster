#include "util/filesystem.h"

#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>

namespace filesystem = boost::filesystem;

namespace renoster {

bool PathIsAbsolute(const std::string & path)
{
    return filesystem::path(path).is_absolute();
}

bool PathIsRelative(const std::string & path)
{
    return filesystem::path(path).is_relative();
}

std::string ParentPath(const std::string & path)
{
    return filesystem::path(path).parent_path().string();
}

std::string Filename(const std::string & filepath)
{
    return filesystem::path(filepath).filename().string();
}

std::string Stem(const std::string & filepath)
{
    return filesystem::path(filepath).stem().string();
}

std::string Extension(const std::string & filepath)
{
    return filesystem::path(filepath).extension().string();
}

void SplitSearchPath(const std::string & searchpath, std::vector<std::string> & paths)
{
    const boost::char_separator<char> sep(":;");
    boost::tokenizer<boost::char_separator<char>> tokenizer(searchpath, sep);
    for (std::string path : tokenizer)
    {
        paths.push_back(path);   
    }
}

std::string FindSearchPath(const std::string & filename,
        const std::vector<std::string> & dirs,
        bool recursive)
{
    const filesystem::path f(filename);

    // Check if we already have a correct path to the file
    if (filesystem::is_regular_file(f))
    {
        return f.string();
    }

    for (auto && searchdir : dirs)
    {
        const filesystem::path dir(searchdir);

        // Check if the file exists in this subdirectory
        const filesystem::path fp = dir / f;
        if (filesystem::is_regular_file(fp))
        {
            return fp.string();
        }

        if (recursive)
        {
            // List all subdirectories
            std::vector<std::string> subdirs;
            for (auto&& entry : filesystem::directory_iterator(dir))
            {
                if (filesystem::is_directory(entry.path()))
                {
                    subdirs.push_back(entry.path().string());
                }
            }

            // Search in the subdirectories
            std::string result = FindSearchPath(filename, subdirs, recursive);
            if (!result.empty())
            {
                return result;
            }
        }
    }

    // Could not find it
    return std::string();
}

bool PathExists(const std::string & path)
{
    return filesystem::exists(filesystem::path(path));
}

} // namespace renoster
