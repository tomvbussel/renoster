#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

#include "renoster/log.h"
#include "renoster/normal.h"
#include "renoster/point.h"
#include "renoster/vector.h"

using namespace renoster;

struct Vertex {
    size_t v = 0;
    size_t vt = 0;
    size_t vn = 0;
};

inline bool operator==(const Vertex & v1, const Vertex & v2)
{
    return v1.v == v2.v && v1.vt == v2.vt && v1.vn == v2.vn;
}

struct VertexHash : std::unary_function<Vertex, size_t> {
    size_t operator()(const Vertex & v) const {
        size_t hash = hashInt(v.v);
        hash += 17 * hashInt(v.vt);
        hash += 23 * hashInt(v.vn);
        return hash;
    }

    std::hash<size_t> hashInt;
};

using VertexMap = std::unordered_map<Vertex, size_t, VertexHash>;

std::vector<Point4f> positions;
std::vector<Point3f> texcoords;
std::vector<Normal3f> normals;

struct Object {
    std::vector<size_t> numVertices;
    std::vector<size_t> indices;
    std::vector<Point4f> positions;
    std::vector<Point3f> texcoords;
    std::vector<Normal3f> normals;
    VertexMap vertices;

    void Clear() {
        numVertices.clear();
        indices.clear();
        positions.clear();
        texcoords.clear();
        normals.clear();
        vertices.clear();
    }
};

Object curObject;

void ReadPosition(std::stringstream & line)
{
    float x, y, z, w;

    if (!(line >> x)) {
        Error("x");
        return;
    }

    if (!(line >> y)) {
        Error("y");
        return;
    }

    if (!(line >> z)) {
        Error("z");
        return;
    }

    if (line >> w) {
        positions.emplace_back(x, y, z, w);
    } else {
        positions.emplace_back(x, y, z, 1.f);
    }

    // TODO
}

void ReadTexCoord(std::stringstream & line)
{
    float u, v, w;

    if (!(line >> u)) {
        Error("u");
        return;
    }

    if (line >> v) {
        if (line >> w) {
            texcoords.emplace_back(u, v, w);
        } else {
            texcoords.emplace_back(u, v, 0.f);
        }
    } else {
        texcoords.emplace_back(u, 0.f, 0.f);
    }

    // TODO: make sure there is not more
}

void ReadNormal(std::stringstream & line)
{
    float x, y, z;

    if (!(line >> x)) {
        Error("x");
        return;
    }

    if (!(line >> x)) {
        Error("y");
        return;
    }

    if (!(line >> x)) {
        Error("z");
        return;
    }

    normals.emplace_back(x, y, z);

    // TODO: make sure there is not more
}

Vertex ReadVertex(const std::string & line)
{
    Vertex vert;

    std::vector<std::string> tokens = Tokenize(line, "/", true);

    if (tokens.size() >= 1 && !tokens[0].empty()) {
        vert.v = std::stoi(tokens[0]);
        if (vert.v < 0) {
            vert.v += positions.size() + 1;
        }
    } else {
        Error("Vertex position is required");
    }

    if (tokens.size() >= 2 && !tokens[1].empty()) {
        vert.vt = std::stoi(tokens[1]);
        if (vert.vt < 0) {
            vert.vt += texcoords.size() + 1;
        }
    }

    if (tokens.size() >= 3 && !tokens[2].empty()) {
        vert.vn = std::stoi(tokens[2]);
        if (vert.vn < 0) {
            vert.vn += normals.size() + 1;
        }
    }

    return vert;
}

bool FindVertex(const Vertex & vert, size_t & index)
{
    auto it = curObject.vertices.find(vert);
    if (it == curObject.vertices.end()) {
        return false;
    }
    index = it->second;
    return true;
}

void ReadFace(std::stringstream & line)
{
    size_t numVertices = 0;
    std::string vertStr;
    while (line >> vertStr) {
        Vertex vert = ReadVertex(vertStr);

        size_t index;
        if (!FindVertex(vert, index)) {
            index = curObject.positions.size();

            curObject.positions.push_back(positions[vert.v - 1]);

            if (vert.vn != 0) {
                curObject.normals.push_back(normals[vert.vn - 1]);
            }

            if (vert.vt != 0) {
                curObject.texcoords.push_back(texcoords[vert.vt - 1]);
            }

            curObject.vertices[vert] = index;
        }

        curObject.indices.push_back(index);

        numVertices++;
    }

    if (numVertices == 0) {
        Error("No vertices");
        return;
    } else {
        curObject.numVertices.push_back(numVertices);
    }
}

void CreateObject(std::ofstream & renoFile)
{
    size_t minNumVertices = -1;
    size_t maxNumVertices = 0;
    for (size_t i = 0; i < curObject.numVertices.size(); ++i) {
        size_t numVertices = curObject.numVertices[i];
        if (numVertices < minNumVertices) {
            minNumVertices = numVertices;
        }
        if (numVertices > maxNumVertices) {
            maxNumVertices = numVertices;
        }
    }

    if (minNumVertices == -1 || maxNumVertices == 0) {
        return;
    }

    if (minNumVertices == maxNumVertices && minNumVertices == 3) {
        // TriangleMesh
        renoFile << "Geometry \"TriangleMesh\"" << std::endl;

        renoFile << "  \"int vertices\" [";
        for (size_t & v : curObject.indices) {
            renoFile << v << "  ";
        }
        renoFile << "]" << std::endl;

        renoFile << "  \"point P\" [";
        for (size_t i = 0; i < curObject.positions.size(); ++i) {
            const Point4f & p = curObject.positions[i];
            renoFile << p.x() << " " << p.y() << " " << p.z();
            if (i + 1 < curObject.positions.size()) {
                renoFile << "  ";
            }
        }
        renoFile << "]" << std::endl;

        if (!curObject.normals.empty()) {
            renoFile << "  \"normal N\" [";
            for (size_t i = 0; i < curObject.normals.size(); ++i) {
                const Normal3f & n = curObject.normals[i];
                renoFile << n.x() << " " << n.y() << " " << n.z();
                if (i + 1 < curObject.normals.size()) {
                    renoFile << "  ";
                }
            }
            renoFile << "]" << std::endl;
        }

        if (!curObject.texcoords.empty()) {
            renoFile << "  \"point2 uv\" [";
            for (size_t i = 0; i < curObject.texcoords.size(); ++i) {
                const Point3f & uv = curObject.texcoords[i];
                renoFile << uv.x() << " " << uv.y();
                if (i + 1 < curObject.texcoords.size()) {
                    renoFile << "  ";
                }
            }
            renoFile << "]" << std::endl;
        }
    } else {
        Error("Unknown mesh type");
    }

    curObject.Clear();
}

void Load(std::ifstream & objFile, std::ofstream & renoFile)
{
    std::string lineStr;
    while (std::getline(objFile, lineStr)) {
        std::stringstream line(lineStr);

        std::string prefix;
        line >> prefix;

        bool newGroup = prefix == "g" || prefix == "o" || prefix == "usemtl";
        if (newGroup) {
            CreateObject(renoFile);
        }

        if (prefix == "v") {
            ReadPosition(line);
        } else if (prefix == "vt") {
            ReadTexCoord(line);
        } else if (prefix == "vn") {
            ReadNormal(line);
        } else if (prefix == "f") {
            ReadFace(line);
        } else {
            Error("Unknown symbol: \"%s\"", prefix);
        }
    }

    CreateObject(renoFile);
}

int main(int argc, char * argv[])
{
    std::string objFilename;
    std::string renoFilename;

    po::options_description generic("Generic options");
    generic.add_options()
        ("version,v", "print version string")
        ("help,h", "produce help message");

    po::options_description hidden("Hidden options");
    hidden.add_options()
        ("objFilename", po::value<std::string>(&objFilename))
        ("renoFilename", po::value<std::string>(&renoFilename));

    po::positional_options_description positionals;
    positionals.add("objFilename", 1);
    positionals.add("renoFilename", 2);

    po::options_description options;
    options.add(generic);
    options.add(hidden);

    po::variables_map vm;

    try
    {
        po::command_line_parser parser(argc, argv);
        parser.options(options);
        parser.positional(positionals);
        po::store(parser.run(), vm);
    }
    catch (po::error & e)
    {
        std::cout << e.what() << std::endl;
        return -1;
    }

    po::notify(vm);

    if (vm.count("help"))
    {
        po::options_description cmdlineOptions;
        cmdlineOptions.add(generic);
        std::cout << cmdlineOptions << std::endl;
        return 1;
    }

    std::ifstream objFile(objFilename);
    std::ofstream renoFile(renoFilename);
    renoFile.setf(std::ios_base::fixed);
    renoFile.precision(6);

    Load(objFile, renoFile);

    return 0;
}
