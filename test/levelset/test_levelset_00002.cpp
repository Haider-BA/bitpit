/*---------------------------------------------------------------------------*\
 *
 *  bitpit
 *
 *  Copyright (C) 2015-2017 OPTIMAD engineering Srl
 *
 *  -------------------------------------------------------------------------
 *  License
 *  This file is part of bitpit.
 *
 *  bitpit is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License v3 (LGPL)
 *  as published by the Free Software Foundation.
 *
 *  bitpit is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 *  License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with bitpit. If not, see <http://www.gnu.org/licenses/>.
 *
\*---------------------------------------------------------------------------*/

/*!
 *	\date			10/jul/2014
 *	\authors		Alessandro Alaia
 *	\authors		Haysam Telib
 *	\authors		Edoardo Lombardi
 *	\version		0.1
 *	\copyright		Copyright 2015 Optimad engineering srl. All rights reserved.
 *	\par			License:\n
 *
 *	\brief Level Set Class Demos
 */

// ========================================================================== //
// INCLUDES                                                                   //
// ========================================================================== //

//Standard Template Library
# include <ctime>
# include <chrono>

#if BITPIT_ENABLE_MPI==1
# include <mpi.h>
#endif

// bitpit
# include "bitpit_IO.hpp"
# include "bitpit_surfunstructured.hpp"
# include "bitpit_volcartesian.hpp"
# include "bitpit_levelset.hpp"

// ========================================================================== //
// NAMESPACES                                                                 //
// ========================================================================== //
using namespace bitpit;

int main( int argc, char *argv[]){

#if BITPIT_ENABLE_MPI==1
    MPI_Init(&argc, &argv);
#endif
    int                    dimensions(3) ;

    // Input geometry
    std::unique_ptr<SurfUnstructured> STL( new SurfUnstructured(0) );

    std::cout << " - Loading stl geometry" << std::endl;

    STL->importSTL("./data/cube.stl", true);

    STL->deleteCoincidentVertices() ;
    STL->buildAdjacencies() ;

    STL->getVTK().setName("geometry_002") ;
    STL->write() ;

    std::cout << "n. vertex: " << STL->getVertexCount() << std::endl;
    std::cout << "n. simplex: " << STL->getCellCount() << std::endl;



    // create cartesian mesh around geometry 
    std::cout << " - Setting mesh" << std::endl;
    std::array<double,3>     meshMin, meshMax, delta ;
    std::array<int,3>        nc = {{64, 64, 64}} ;

    STL->getBoundingBox( meshMin, meshMax ) ;

    delta = meshMax -meshMin ;
    meshMin -=  0.1*delta ;
    meshMax +=  0.1*delta ;

    delta = meshMax -meshMin ;

    VolCartesian mesh( 1, dimensions, meshMin, delta, nc);

    // Compute level set  in narrow band
    LevelSet levelset ;
    int id0;

    std::chrono::time_point<std::chrono::system_clock> start, end;
    int elapsed_seconds;


    levelset.setMesh(&mesh) ;
    id0 = levelset.addObject( std::move(STL), M_PI/3. ) ;

    levelset.setPropagateSign(true) ;
    start = std::chrono::system_clock::now();
    levelset.compute( ) ;
    end = std::chrono::system_clock::now();

    elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
    std::cout << "elapsed time: " << elapsed_seconds << " ms" << std::endl;

    std::cout << " - Exporting data" << std::endl;
    mesh.update() ;
    std::vector<double> LS(mesh.getCellCount() ) ;
    std::vector<std::array<double,3>> LG(mesh.getCellCount() ) ;
    const LevelSetObject &object0 = levelset.getObject(id0);

    std::vector<double>::iterator itLS = LS.begin() ;
    std::vector<std::array<double,3>>::iterator itLG = LG.begin() ;

    for( auto & cell : mesh.getCells() ){
        const long &id = cell.getId() ;
        *itLS = object0.getLS(id) ;
        *itLG = object0.getGradient(id) ;
        ++itLS ;
        ++itLG ;
    };

    mesh.getVTK().addData("ls", VTKFieldType::SCALAR, VTKLocation::CELL, LS) ;
    mesh.getVTK().addData("lg", VTKFieldType::VECTOR, VTKLocation::CELL, LG) ;
    mesh.getVTK().setName("levelset_002") ;
    mesh.write() ;

    std::cout << " - Exported data" << std::endl;

#if BITPIT_ENABLE_MPI==1
    MPI_Finalize();
#endif

    return 0;

};


