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


#include "AttentionExampleUtils.hpp"

#include <opencv2/opencv.hpp>

#include "v4r/AttentionModule/AttentionModule.hpp"
#include "AttentionExampleUtils.hpp"

// This program shows the use of Surface Height Saliency Map on one image

void printUsage(const char *argv0)
{
  printf(
    "Calculates Surface Height Map\n"
    "usage: %s image.png cloud.pcd type pyramid_type combination_type normalization_type result.png\n"
    "  image.png             ... color image\n"
    "  cloud.pcd             ... point cloud\n"
    "  type                  ... 0 -- distance map; 1 -- tall map; 2 -- short map\n"
    "  pyramid_type          ... 0 -- no pyramid; 1 -- simple pyramid; 2 -- Itti pyramid; 3 -- Frintrop pyramid\n"
    "  combination_type      ... 0 -- SUM; 1 -- MUL; 2 -- MAX\n"
    "  normalization_type    ... 0 -- LIN; 1 -- NMS; 2 -- NLM\n"
    "  result.png            ... output file name\n", argv0);
  printf(" Example: %s image.png cloud.pcd 0 0 0 0\n",argv0);
}

int main(int argc, char** argv)
{
  if(argc != 8)
  {
    printUsage(argv[0]);
    return(0);
  }
  
  // read image
  std::string image_name(argv[1]);
  cv::Mat image = cv::imread(image_name,-1);
    
  pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZRGB>());
  std::string cloud_name(argv[2]);
  if (pcl::io::loadPCDFile<pcl::PointXYZRGB> (cloud_name,*cloud) == -1)
  {
    printf("[ERROR]: Couldn't read point cloud.\n");
    return -1;
  }
  cloud->width = image.cols;
  cloud->height = image.rows;
  
  int type = atoi(argv[3]);
  int pyramid_type = atoi(argv[4]);
  int combination_type = atoi(argv[5]);
  int normalization_type = atoi(argv[6]);
  
  std::string output_name(argv[7]);
  
  pcl::ModelCoefficients::Ptr coefficients(new pcl::ModelCoefficients());
  pcl::PointCloud<pcl::Normal>::Ptr normals(new pcl::PointCloud<pcl::Normal>());
  pcl::PointIndices::Ptr object_indices_in_the_hull(new pcl::PointIndices());
  preparePointCloud(cloud,coefficients,normals,object_indices_in_the_hull);
  
  // start creating parameters
  AttentionModule::SurfaceHeightSaliencyMap surfaceHeightSaliencyMap;
  surfaceHeightSaliencyMap.setWidth(image.cols);
  surfaceHeightSaliencyMap.setHeight(image.rows);
  surfaceHeightSaliencyMap.setCloud(cloud);
  surfaceHeightSaliencyMap.setIndices(object_indices_in_the_hull);
  surfaceHeightSaliencyMap.setNormals(normals);
  surfaceHeightSaliencyMap.setModelCoefficients(coefficients);
  
  printf("Saliency Map Type: ");
  switch(type)
  {
    case(0):
      printf("DISTANCE MAP \n");
      surfaceHeightSaliencyMap.setHeightType(AttentionModule::AM_DISTANCE);
      break;
    case(1):
      printf("TALL MAP \n");
      surfaceHeightSaliencyMap.setHeightType(AttentionModule::AM_TALL);
      break;
    case(2):
      printf("SHORT MAP \n");
      surfaceHeightSaliencyMap.setHeightType(AttentionModule::AM_SHORT);
      break;
    default:
      printf("UNDEFINED -- return \n");
      return(0);
  }
  
  printf("Combination Type: ");
  switch(combination_type)
  {
    case(0):
      printf("SUM \n");
      surfaceHeightSaliencyMap.setCombinationType(AttentionModule::AM_COMB_SUM);
      break;
    case(1):
      printf("MUL \n");
      surfaceHeightSaliencyMap.setCombinationType(AttentionModule::AM_COMB_MUL);
      break;
    case(2):
      printf("MAX \n");
      surfaceHeightSaliencyMap.setCombinationType(AttentionModule::AM_COMB_MAX);
      break;
    default:
      printf("UNDEFINED -- return \n");
      return(0);
  }
  
  printf("Normalization Type: ");
  switch(normalization_type)
  {
    case(0):
      printf("LIN \n");
      surfaceHeightSaliencyMap.setNormalizationType(EPUtils::NT_NONE);
      break;
    case(1):
      printf("NMS \n");
      surfaceHeightSaliencyMap.setNormalizationType(EPUtils::NT_NONMAX);
      break;
    case(2):
      printf("NLM \n");
      surfaceHeightSaliencyMap.setNormalizationType(EPUtils::NT_FRINTROP_NORM);
      break;
    default:
      printf("UNDEFINED -- return \n");
      return(0);
  }
  
  printf("Pyramid Type: ");
  switch(pyramid_type)
  {
    case(0):
      printf("NONE \n");
      surfaceHeightSaliencyMap.calculate();
      break;
    case(1):
      printf("SIMPLE \n");
      surfaceHeightSaliencyMap.calculatePyramid(AttentionModule::SIMPLE_PYRAMID);
      break;
    case(2):
      printf("ITTI \n");
      surfaceHeightSaliencyMap.calculatePyramid(AttentionModule::ITTI_PYRAMID);
      break;
    case(3):
      printf("FRINTROP \n");
      surfaceHeightSaliencyMap.calculatePyramid(AttentionModule::FRINTROP_PYRAMID);
      break;
    default:
      printf("UNDEFINED -- return \n");
      return(0);
  }
    
  cv::Mat map;
  
  if(surfaceHeightSaliencyMap.getMap(map))
  {
    printf("[INFO]: Computation completed.\n");
//     cv::imshow("Surface Distance Map",map);
//     printf("[INFO]: Press any key to continue.\n");
//     cv::waitKey();
    EPUtils::normalize(map);
    cv::Mat temp;
    map.convertTo(temp,CV_8U,255);
    cv::imwrite(output_name,temp);
  }
  else
  {
    printf("[ERROR]: Computation failed.\n");
  }
  
  return(0);
}