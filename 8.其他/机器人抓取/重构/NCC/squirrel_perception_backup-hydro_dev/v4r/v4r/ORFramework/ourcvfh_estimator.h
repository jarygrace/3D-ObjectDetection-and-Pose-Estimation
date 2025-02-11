/*
 * vfh_estimator.h
 *
 *  Created on: Mar 22, 2012
 *      Author: aitor
 */

#ifndef FAAT_PCL_REC_FRAMEWORK_OURCVFH_ESTIMATOR_H_
#define FAAT_PCL_REC_FRAMEWORK_OURCVFH_ESTIMATOR_H_

#include "global_estimator.h"
#include "normal_estimator.h"
#include <pcl/features/our_cvfh.h>
#include <pcl/surface/mls.h>

namespace faat_pcl
{
  namespace rec_3d_framework
  {
    template<typename PointInT, typename FeatureT>
      class OURCVFHEstimator : public GlobalEstimator<PointInT, FeatureT>
      {

      protected:
        typedef typename pcl::PointCloud<PointInT>::Ptr PointInTPtr;
        using GlobalEstimator<PointInT, FeatureT>::normal_estimator_;
        using GlobalEstimator<PointInT, FeatureT>::normals_;
        float eps_angle_threshold_;
        float curvature_threshold_;
        float cluster_tolerance_factor_;

        std::vector<float> eps_angle_threshold_vector_;
        std::vector<float> curvature_threshold_vector_;
        std::vector<float> cluster_tolerance_vector_;

        bool normalize_bins_;
        bool adaptative_MLS_;
        float refine_factor_;

        std::vector<bool> valid_roll_transforms_;
        std::vector<Eigen::Matrix4f, Eigen::aligned_allocator<Eigen::Matrix4f> > transforms_;
        std::vector<pcl::PointIndices> cluster_indices_;

        float axis_ratio_;
        float min_axis_value_;
        bool organized_data_;

      public:

        OURCVFHEstimator ()
        {
          eps_angle_threshold_ = 0.13f;
          curvature_threshold_ = 0.035f;
          normalize_bins_ = true;
          cluster_tolerance_factor_ = 3.f;
          adaptative_MLS_ = false;
          min_axis_value_ = 0.925f;
          axis_ratio_ = 0.8f;
          organized_data_ = false;
        }

        virtual bool getUsesOrganizedData()
        {
            return organized_data_;
        }

        virtual void
        setNormals(pcl::PointCloud<pcl::Normal>::Ptr & /*normals*/)
        {

        }

        void
        setAxisRatio (float f)
        {
          axis_ratio_ = f;
        }

        void
        setMinAxisValue (float f)
        {
          min_axis_value_ = f;
        }

        void
        setCVFHParams (float p1, float p2, float p3)
        {
          std::cout << "******************* Setting CVFH params ************* and clearing stuff" << std::endl;
          eps_angle_threshold_vector_.clear ();
          curvature_threshold_vector_.clear ();
          cluster_tolerance_vector_.clear ();
          eps_angle_threshold_ = p1;
          curvature_threshold_ = p2;
          cluster_tolerance_factor_ = p3;
        }

        void
        setClusterToleranceVector (std::vector<float> tolerances)
        {
          cluster_tolerance_vector_ = tolerances;
        }

        void
        setEpsAngleThresholdVector (std::vector<float> eps_angle_threshold_vector)
        {
          eps_angle_threshold_vector_ = eps_angle_threshold_vector;
        }

        void
        setCurvatureThresholdVector (std::vector<float> curvature_threshold_vector)
        {
          curvature_threshold_vector_ = curvature_threshold_vector;
        }

        void
        setAdaptativeMLS (bool b)
        {
          adaptative_MLS_ = b;
        }

        void
        setRefineClustersParam (float p4)
        {
          refine_factor_ = p4;
        }

        void
        getValidTransformsVec (std::vector<bool> & valid)
        {
          valid = valid_roll_transforms_;
        }

        void
        getTransformsVec (std::vector<Eigen::Matrix4f, Eigen::aligned_allocator<Eigen::Matrix4f> > & trans)
        {
          trans = transforms_;
        }

        virtual bool
        estimate (const PointInTPtr & in, PointInTPtr & processed, typename pcl::PointCloud<FeatureT>::CloudVectorType & signatures,
                  std::vector<Eigen::Vector3f> & centroids)
        {

          valid_roll_transforms_.clear ();
          transforms_.clear ();

          if (!normal_estimator_)
          {
            PCL_ERROR("This feature needs normals... please provide a normal estimator\n");
            return false;
          }

          pcl::MovingLeastSquares<PointInT, PointInT> mls;
          if (adaptative_MLS_)
          {
            typename pcl::search::KdTree<PointInT>::Ptr tree;
            Eigen::Vector4f centroid_cluster;
            pcl::compute3DCentroid (*in, centroid_cluster);
            float dist_to_sensor = centroid_cluster.norm ();
            float sigma = dist_to_sensor * 0.01f;
            mls.setSearchMethod (tree);
            mls.setSearchRadius (sigma);
            mls.setUpsamplingMethod (mls.SAMPLE_LOCAL_PLANE);
            mls.setUpsamplingRadius (0.002);
            mls.setUpsamplingStepSize (0.001);
          }

          normals_.reset (new pcl::PointCloud<pcl::Normal>);
          {
            normal_estimator_->estimate (in, processed, normals_);
          }

          if (adaptative_MLS_)
          {
            mls.setInputCloud (processed);

            PointInTPtr filtered (new pcl::PointCloud<PointInT>);
            mls.process (*filtered);

            processed.reset (new pcl::PointCloud<PointInT>);
            normals_.reset (new pcl::PointCloud<pcl::Normal>);
            {
              filtered->is_dense = false;
              normal_estimator_->estimate (filtered, processed, normals_);
            }
          }

          /*normals_.reset(new pcl::PointCloud<pcl::Normal>);
           normal_estimator_->estimate (in, processed, normals_);*/

          typedef typename pcl::OURCVFHEstimation<PointInT, pcl::Normal, pcl::VFHSignature308> OURCVFHEstimation;
          pcl::PointCloud<pcl::VFHSignature308> cvfh_signatures;
          typename pcl::search::KdTree<PointInT>::Ptr cvfh_tree (new pcl::search::KdTree<PointInT>);

          if (eps_angle_threshold_vector_.size () == 0)
            eps_angle_threshold_vector_.push_back (eps_angle_threshold_);

          if (curvature_threshold_vector_.size () == 0)
            curvature_threshold_vector_.push_back (curvature_threshold_);

          if (cluster_tolerance_vector_.size () == 0)
            cluster_tolerance_vector_.push_back (cluster_tolerance_factor_);

          //std::cout << eps_angle_threshold_vector_.size() << " " << curvature_threshold_vector_.size() << std::endl;

          for (size_t ei = 0; ei < eps_angle_threshold_vector_.size (); ei++)
          {
            for (size_t ci = 0; ci < curvature_threshold_vector_.size (); ci++)
            {
              for (size_t ti = 0; ti < cluster_tolerance_vector_.size (); ti++)
              {

                OURCVFHEstimation cvfh;
                cvfh.setSearchMethod (cvfh_tree);
                cvfh.setInputCloud (processed);
                cvfh.setInputNormals (normals_);
                cvfh.setEPSAngleThreshold (eps_angle_threshold_vector_[ei]);
                cvfh.setCurvatureThreshold (curvature_threshold_vector_[ci]);
                cvfh.setNormalizeBins (normalize_bins_);
                cvfh.setRefineClusters (refine_factor_);

                float radius = normal_estimator_->normal_radius_;
                float cluster_tolerance_radius = normal_estimator_->grid_resolution_ * cluster_tolerance_vector_[ti];

                if (normal_estimator_->compute_mesh_resolution_)
                {
                  radius = normal_estimator_->mesh_resolution_ * normal_estimator_->factor_normals_;
                  cluster_tolerance_radius = normal_estimator_->mesh_resolution_ * cluster_tolerance_factor_;

                  if (normal_estimator_->do_voxel_grid_)
                  {
                    radius *= normal_estimator_->factor_voxel_grid_;
                    cluster_tolerance_radius *= normal_estimator_->factor_voxel_grid_;
                  }
                }

                cvfh.setClusterTolerance (cluster_tolerance_radius);
                cvfh.setRadiusNormals (radius);
                cvfh.setMinPoints (50);
                cvfh.setAxisRatio (axis_ratio_);
                cvfh.setMinAxisValue (min_axis_value_);
                cvfh.compute (cvfh_signatures);

                //std::cout << "Res:" << normal_estimator_->mesh_resolution_ << " Radius normals:" << radius << " Cluster tolerance:" << cluster_tolerance_radius << " " << eps_angle_threshold_ << " " << curvature_threshold_ << std::endl;

                std::vector<Eigen::Vector3f> centroids_in;
                std::vector<bool> valid_roll_transforms_in;
                std::vector<Eigen::Matrix4f, Eigen::aligned_allocator<Eigen::Matrix4f> > transforms_in;

                cvfh.getCentroidClusters (centroids_in);
                cvfh.getTransforms (transforms_in);
                cvfh.getValidTransformsVec (valid_roll_transforms_in);

                size_t valid = 0;
                for (size_t i = 0; i < valid_roll_transforms_in.size (); i++)
                {
                  if (valid_roll_transforms_in[i])
                  {
                    transforms_in[valid] = transforms_in[i];
                    centroids_in[valid] = centroids_in[i];
                    valid_roll_transforms_in[valid] = valid_roll_transforms_in[i];
                    cvfh_signatures.points[valid] = cvfh_signatures.points[i];
                    valid++;
                  }
                }

                valid_roll_transforms_in.resize (valid);
                centroids_in.resize (valid);
                transforms_in.resize (valid);
                cvfh_signatures.points.resize (valid);

                for(size_t kk=0; kk < centroids_in.size(); kk++) {
                  centroids.push_back(centroids_in[kk]);
                  transforms_.push_back(transforms_in[kk]);
                  valid_roll_transforms_.push_back(valid_roll_transforms_in[kk]);
                }

                for (size_t i = 0; i < cvfh_signatures.points.size (); i++)
                {
                  pcl::PointCloud<FeatureT> vfh_signature;
                  vfh_signature.points.resize (1);
                  vfh_signature.width = vfh_signature.height = 1;
                  for (int d = 0; d < 308; ++d)
                    vfh_signature.points[0].histogram[d] = cvfh_signatures.points[i].histogram[d];

                  signatures.push_back (vfh_signature);
                }
                //cvfh.getClusterIndices (cluster_indices_);
              }
            }
          }

          return true;
        }

        bool
        computedNormals ()
        {
          return true;
        }

        void
        setNormalizeBins (bool b)
        {
          normalize_bins_ = b;
        }

        void getClusterIndices(std::vector<pcl::PointIndices> & indices)
        {
            indices = cluster_indices_;
        }
      };
  }
}

#endif /* REC_FRAMEWORK_OURCVFH_ESTIMATOR_H_ */
