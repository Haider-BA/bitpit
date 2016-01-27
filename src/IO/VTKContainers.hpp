
#ifndef __VTKCONTAINERS_HH__
#define __VTKCONTAINERS_HH__

#include <boost/mpl/vector.hpp>
#include <boost/mpl/copy.hpp>
#include <boost/variant.hpp>


namespace VTKContainers{

    /*!
     * @ingroup    VTKContainers
     * @typedef    VectorOfVector
     * All supported variants of vector of POD and vector of vector of POD
     */
    typedef boost::mpl::vector<
        std::vector<int8_t>*  ,
        std::vector<int16_t>* ,
        std::vector<int32_t>* ,
        std::vector<int64_t>* ,
        std::vector<uint8_t>* ,
        std::vector<uint16_t>*, 
        std::vector<uint32_t>*, 
        std::vector<uint64_t>*,
    
        std::vector<float>*   ,
        std::vector<double>*  ,
    
        std::vector< std::vector<int8_t> >*  , 
        std::vector< std::vector<int16_t> >*  , 
        std::vector< std::vector<int32_t> >*  , 
        std::vector< std::vector<int64_t> >*  , 
        std::vector< std::vector<uint8_t> >*  , 
        std::vector< std::vector<uint16_t> >* ,
        std::vector< std::vector<uint32_t> >* ,
        std::vector< std::vector<uint64_t> >* ,
    
        std::vector< std::vector<float> >*  , 
        std::vector< std::vector<double> >*
    >::type VectorOfVector;
    
    /*!
     * @ingroup    VTKContainers
     * @typedef    VectorOfArray
     * All supported variants of vector of array of POD
     */
    typedef boost::mpl::vector<
        std::vector< std::array<int16_t,3> >*  , 
        std::vector< std::array<int32_t,3> >*  , 
        std::vector< std::array<int64_t,3> >*  , 
        std::vector< std::array<uint16_t,3> >* ,
        std::vector< std::array<uint32_t,3> >* ,
        std::vector< std::array<uint64_t,3> >* ,
    
        std::vector< std::array<int16_t,4> >*  ,
        std::vector< std::array<int32_t,4> >*  ,
        std::vector< std::array<int64_t,4> >*  ,
        std::vector< std::array<uint16_t,4> >* ,
        std::vector< std::array<uint32_t,4> >* ,
        std::vector< std::array<uint64_t,4> >* ,
    
        std::vector< std::array<int16_t,8> >*  ,
        std::vector< std::array<int32_t,8> >*  ,
        std::vector< std::array<int64_t,8> >*  ,
        std::vector< std::array<uint16_t,8> >* ,
        std::vector< std::array<uint32_t,8> >* ,
        std::vector< std::array<uint64_t,8> >* ,
    
        std::vector< std::array<float,3> >*    ,
        std::vector< std::array<double,3> >*   
    >::type VectorOfArray;
    
    /*!
     * @ingroup    VTKContainers
     * @typedef     VectorContainers
     * All supported containers by VTKUnstructuredVec
     */
    typedef boost::mpl::copy< VectorOfVector::type, boost::mpl::back_inserter<VectorOfArray> > ::type VectorContainers ;
    
    /*!
     * @ingroup    VTKContainers
     * @typedef     Variants
     * Boost variant over VectorContainers
     */
    typedef boost::make_variant_over< VectorContainers >::type  Variants ;

}


#endif