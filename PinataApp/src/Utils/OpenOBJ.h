#pragma once

#include <fstream>
#include <iostream>
#include <filesystem>
#include "glm/glm.hpp"
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

namespace obj {

    struct MeshData {
		std::vector<glm::vec3> vertexPositions;
        std::vector<uint32_t> indicies;
    };

    inline static MeshData parseObj(const char* path)
    {
        //Timer t("Loading");
        MeshData data;

        std::ifstream stream(path);
        std::string line;

        srand(time(NULL));

        while (std::getline(stream, line)) {
            //if line specifies vertex position eg. "v -1.0 1.0 1.0"
            if (line[0] == 'v' && line[1] == ' ') {
				line = line.substr(2);
				std::stringstream parse(line);
				std::string section;
				glm::vec3 temp;
				parse >> temp.x >> temp.y >> temp.z;
				data.vertexPositions.push_back(temp);
				std::cout << "Vertex: " << temp.x << " " << temp.y << " " << temp.z << std::endl;
            }
            //if line specifies face eg. "f 1/2/3 3/2/4 2/3/2"
            if (line[0] == 'f' && line[1] == ' ') {
                line = line.substr(2);
                std::stringstream parse(line);
                std::string section;
                std::vector<int> temp;
                //only extract first int from each "x/y/z" others don't matter
                while (std::getline(parse, section, ' ')) {
                    temp.push_back(std::stoi(section.substr(0, section.find('/'))) - 1);
                }

                if (temp.size() == 4) {

                    data.indicies.push_back(temp[0]);
                    data.indicies.push_back(temp[1]);
                    data.indicies.push_back(temp[2]);

                    data.indicies.push_back(temp[0]);
                    data.indicies.push_back(temp[2]);
                    data.indicies.push_back(temp[3]);

                }

                else if (temp.size() == 3) {
                    data.indicies.push_back(temp[0]);
                    data.indicies.push_back(temp[1]);
                    data.indicies.push_back(temp[2]);
                }

				std::cout << "Face: " << temp[0] << " " << temp[1] << " " << temp[2] << std::endl;

                //else {
                //    std::cout << "Unkown case: " << temp.size() << std::endl;
                //   // ASSERT(false);
                //}

            }


        }

        return data;
       
    }
			
}