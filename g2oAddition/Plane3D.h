//
// Created by fishmarch on 19-5-28.
//

#ifndef ORB_SLAM2_PLANE3D_H
#define ORB_SLAM2_PLANE3D_H

#include "Thirdparty/g2o/g2o/stuff/misc.h"
#include "Thirdparty/g2o/g2o/core/eigen_types.h"
#include <Eigen/Core>
#include <Eigen/Geometry>

namespace g2o {
    class  Plane3D {
            public:
            EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

            friend Plane3D operator*(const Isometry3D& t, const Plane3D& plane);

            Plane3D(){
                Vector4D v;
                v << 1., 0., 0., -1.;
                fromVector(v);
            }

            Plane3D(const Vector4D& v){
                fromVector(v);
            }


            inline Vector4D toVector() const {
                return _coeffs;
            }

            inline const Vector4D& coeffs() const {return _coeffs;}

            inline void fromVector(const Vector4D& coeffs_) {
                _coeffs=coeffs_;
                normalize(_coeffs);
            }

            static double azimuth(const Vector3D& v) {
                return std::atan2(v(1),v(0));
            }

            static  double elevation(const Vector3D& v) {
                return std::atan2(v(2), v.head<2>().norm());
            }

            double distance() const {
                return -_coeffs(3);
            }

            Vector3D normal() const {
                return _coeffs.head<3>();
            }


            static Matrix3D rotation(const Vector3D& v)  {
                double _azimuth = azimuth(v);
                double _elevation = elevation(v);
                Eigen::AngleAxisd azimuth_v(_azimuth, Vector3D::UnitZ());
                Eigen::AngleAxisd elevation_v(- _elevation, Vector3D::UnitY());
                return (azimuth_v * elevation_v).toRotationMatrix();
            }

            inline void oplus(const Vector3D& v){
                //construct a normal from azimuth and evelation;
                double _azimuth=v[0];
                double _elevation=v[1];
                double s=std::sin(_elevation), c=std::cos(_elevation);
                Vector3D n (c*std::cos(_azimuth), c*std::sin(_azimuth), s) ;

                // rotate the normal
                Matrix3D R=rotation(normal());
                double d=distance()+v[2];
                _coeffs.head<3>() = R*n;
                _coeffs(3) = -d;
                normalize(_coeffs);
            }

            inline Vector3D ominus(const Plane3D& plane){
                //construct the rotation that would bring the plane normal in (1 0 0)
                Matrix3D R=rotation(normal()).transpose();
                Vector3D n=R*plane.normal();
                double d=distance()-plane.distance();
                return Vector3D(azimuth(n), elevation(n), d);
            }

            //protected:

            static inline void normalize(Vector4D& coeffs) {
                double n=coeffs.head<3>().norm();
                coeffs = coeffs * (1./n);
            }

            Vector4D _coeffs;
    };
    // input t : transform matrix applying to the point
    inline Plane3D operator*(const Isometry3D& t, const Plane3D& plane){
        Vector4D v=plane._coeffs;
        Vector4D v2;
        Matrix3D R=t.rotation();
        v2.head<3>() = R*v.head<3>();
        v2(3)=v(3) - t.translation().dot(v2.head<3>());
        return Plane3D(v2);
    };
}


#endif //ORB_SLAM2_PLANE3D_H
