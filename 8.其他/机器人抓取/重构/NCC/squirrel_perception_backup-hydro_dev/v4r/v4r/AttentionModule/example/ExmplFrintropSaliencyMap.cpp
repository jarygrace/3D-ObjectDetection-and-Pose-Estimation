/**
 *  Copyright (C) 2012  
 *    Ekaterina Potapova
 *    Automation and Control Institute
 *    Vienna University of Technology
 *    Gusshausstraße 25-29
 *    1040 Vienna, Austria
 *    potapova(at)acin.tuwien.ac.at
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see http://www.gnu.org/licenses/
 */


#include "v4r/AttentionModule/AttentionModule.hpp"

// This program shows the use of Frintrop Saliency Map on one image

int main(int argc, char** argv)
{
  
  if(argc != 2)
  {
    std::cerr << "Usage: image" << std::endl;
    return(0);
  }
  
  // read image
  std::string image_name(argv[1]);
  cv::Mat image = cv::imread(image_name,-1);
  
  cv::imshow("Original Image",image);
    
  // start creating parameters
  AttentionModule::FrintropSaliencyMap frintropSaliencyMap;
  frintropSaliencyMap.setImage(image);
  frintropSaliencyMap.setNormalizationType(EPUtils::NT_FRINTROP_NORM);
      
  cv::Mat map;
  
  printf("[INFO]: Computing Frintrop Saliency Map.\n");
  
  frintropSaliencyMap.calculate();
  
  if(frintropSaliencyMap.getMap(map))
  {
    printf("[INFO]: Computation completed.\n");
    resize(map,map,cv::Size(image.cols,image.rows));
    cv::imshow("Frintrop Saliency Map",map);
    printf("[INFO]: Press any key to continue.\n");
    cv::waitKey();
  }
  else
  {
    printf("[ERROR]: Computation failed.\n");
  }
    
  return(0);
}