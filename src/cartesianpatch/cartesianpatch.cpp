/*---------------------------------------------------------------------------*\
 *
 *  bitpit
 *
 *  Copyright (C) 2015-2016 OPTIMAD engineering Srl
 *
 *  -------------------------------------------------------------------------
 *  License
 *  This file is part of bitbit.
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

#include <math.h>

#include "bitpit_common.hpp"

#include "cartesianpatch.hpp"

namespace bitpit {

/*!
	\ingroup cartesianpatch
	@{
*/

/*!
	\class CartesianPatch

	\brief The CartesianPatch defines a Cartesian patch.

	CartesianPatch defines a Cartesian patch.
*/

/*!
	Creates a new patch.
*/
CartesianPatch::CartesianPatch(const int &id, const int &dimension,
					 std::array<double, 3> origin, double length, double dh)
	: Patch(id, dimension)
{
	std::cout << ">> Initializing cartesian mesh\n";

	// Info sulle celle
	m_cellSize.resize(dimension);
	m_minCoord.resize(dimension);
	m_nCells1D.resize(dimension);
	for (int n = 0; n < dimension; n++) {
		// Dimensioni della cella
		m_cellSize[n] = dh;

		// Numero di celle
		m_nCells1D[n] = (int) ceil(length / m_cellSize[n]);

		std::cout << "  - Cell count along direction " << n << " : " << m_nCells1D[n] << "\n";

		// Minima coordinata del dominio
		m_minCoord[n] = origin[n] - 0.5 * (m_nCells1D[n] * m_cellSize[n]);
	}

	m_cellVolume = m_cellSize[Vertex::COORD_X] * m_cellSize[Vertex::COORD_Y];
	if (isThreeDimensional()) {
		m_cellVolume *= m_cellSize[Vertex::COORD_Z];
	}

	// Info sui vertici
	m_nVertices1D.resize(dimension);
	for (int n = 0; n < dimension; n++) {
		if (!isThreeDimensional() && n == Vertex::COORD_Z) {
			m_nVertices1D[n] = 0;
		} else {
			m_nVertices1D[n] = m_nCells1D[n] + 1;
		}
	}

	// Info sulle interfacce
	m_nInterfacesX1D.resize(dimension);
	for (int n = 0; n < dimension; n++) {
		m_nInterfacesX1D[n] = m_nCells1D[n];
		if (n == Vertex::COORD_X) {
			m_nInterfacesX1D[n]++;
		}
	}

	m_nInterfacesY1D.resize(dimension);
	for (int n = 0; n < dimension; n++) {
		m_nInterfacesY1D[n] = m_nCells1D[n];
		if (n == Vertex::COORD_Y) {
			m_nInterfacesY1D[n]++;
		}
	}


	if (isThreeDimensional()) {
		m_nInterfacesZ1D.resize(dimension);
		for (int n = 0; n < dimension; n++) {
			m_nInterfacesZ1D[n] = m_nCells1D[n];
			if (n == Vertex::COORD_Z) {
				m_nInterfacesZ1D[n]++;
			}
		}
	}

	for (int n = 0; n < dimension; n++) {
		m_interfaceArea[n] = m_cellVolume / m_cellSize[n];
	}

	for (int i = 0; i < dimension; i++) {
		for (int n = -1; n <= 1; n += 2) {
			std::array<double, 3> normal = {0.0, 0.0, 0.0};
			normal[i] = n;

			m_normals.push_back(normal);
		}
	}
}

/*!
	Destroys the patch.
*/
CartesianPatch::~CartesianPatch()
{
}

/*!
	Evaluates the volume of the specified cell.

	\param id is the id of the cell
	\result The volume of the specified cell.
*/
double CartesianPatch::evalCellVolume(const long &id)
{
	BITPIT_UNUSED(id);

	return m_cellVolume;
}

/*!
	Evaluates the characteristic size of the specified cell.

	\param id is the id of the cell
	\result The characteristic size of the specified cell.
*/
double CartesianPatch::evalCellSize(const long &id)
{
	BITPIT_UNUSED(id);

	return pow(m_cellVolume, 1. / getDimension());
}

/*!
	Evaluates the area of the specified interface.

	\param id is the id of the interface
	\result The area of the specified interface.
*/
double CartesianPatch::evalInterfaceArea(const long &id)
{
	const Interface &interface = getInterface(id);
	int ownerFace = interface.getOwnerFace();
	int direction = std::floor(ownerFace / 2.);

	return m_interfaceArea[direction];
}

/*!
	Evaluates the normal of the specified interface.

	\param id is the id of the interface
	\result The normal of the specified interface.
*/
std::array<double, 3> CartesianPatch::evalInterfaceNormal(const long &id)
{
	const Interface &interface = getInterface(id);
	int ownerFace = interface.getOwnerFace();

	return m_normals[ownerFace];
}

/*!
	Updates the patch.

	\result Returns a vector of Adaption::Info that can be used to track
	the changes done during the update.
*/
const std::vector<Adaption::Info> CartesianPatch::_update(bool trackAdaption)
{
	if (!isDirty()) {
		return std::vector<Adaption::Info>();
	}

	std::cout << ">> Updating cartesian mesh\n";

	// Reset the mesh
	reset();

	// Definition of the mesh
	createVertices();
	createCells();
	createInterfaces();

	// Adaption info
	std::vector<Adaption::Info> adaptionData;
	if (trackAdaption) {
		adaptionData.emplace_back();
		Adaption::Info &adaptionCellInfo = adaptionData.back();
		adaptionCellInfo.type   = Adaption::TYPE_CREATION;
		adaptionCellInfo.entity = Adaption::ENTITY_CELL;
		adaptionCellInfo.current.reserve(m_cells.size());
		for (auto &cell : m_cells) {
			adaptionCellInfo.current.emplace_back();
			unsigned long &cellId = adaptionCellInfo.current.back();
			cellId = cell.get_id();
		}

		adaptionData.emplace_back();
		Adaption::Info &adaptionInterfaceInfo = adaptionData.back();
		adaptionInterfaceInfo.type   = Adaption::TYPE_CREATION;
		adaptionInterfaceInfo.entity = Adaption::ENTITY_INTERFACE;
		adaptionInterfaceInfo.current.reserve(m_interfaces.size());
		for (auto &interface : m_interfaces) {
			adaptionInterfaceInfo.current.emplace_back();
			unsigned long &interfaceId = adaptionInterfaceInfo.current.back();
			interfaceId = interface.get_id();
		}
	} else {
		adaptionData.emplace_back();
	}

	// Done
	return adaptionData;
}

/*!
	Creates the vertices of the patch.
*/
void CartesianPatch::createVertices()
{
	std::cout << "  >> Creating vertices\n";

	// Definition of the vertices
	m_vertexXCoords = std::vector<double>(m_nVertices1D[Vertex::COORD_X]);
	for (int i = 0; i < m_nVertices1D[Vertex::COORD_X]; i++) {
		m_vertexXCoords[i] = m_minCoord[Vertex::COORD_X] + i * m_cellSize[Vertex::COORD_X];
	}

	m_vertexYCoords = std::vector<double>(m_nVertices1D[Vertex::COORD_Y]);
	for (int j = 0; j < m_nVertices1D[Vertex::COORD_Y]; j++) {
		m_vertexYCoords[j] = m_minCoord[Vertex::COORD_Y] + j * m_cellSize[Vertex::COORD_Y];
	}

	if (isThreeDimensional()) {
		m_vertexZCoords = std::vector<double>(m_nVertices1D[Vertex::COORD_Z]);
		for (int k = 0; k < m_nVertices1D[Vertex::COORD_Z]; k++) {
			m_vertexZCoords[k] = m_minCoord[Vertex::COORD_Z] + k * m_cellSize[Vertex::COORD_Z];
		}
	} else {
		m_vertexZCoords = std::vector<double>(0);
	}

	long nTotalVertices = 1;
	for (int n = 0; n < getDimension(); n++) {
		nTotalVertices *= m_nVertices1D[n];
	}

	std::cout << "    - Vertex count: " << nTotalVertices << "\n";

	m_vertices.reserve(nTotalVertices);

	for (int i = 0; i < m_nVertices1D[Vertex::COORD_X]; i++) {
		for (int j = 0; j < m_nVertices1D[Vertex::COORD_Y]; j++) {
			for (int k = 0; (isThreeDimensional()) ? (k < m_nVertices1D[Vertex::COORD_Z]) : (k <= 0); k++) {
				long id_vertex = getVertexLinearId(i, j, k);
				Patch::createVertex(id_vertex);
				Vertex &vertex = m_vertices[id_vertex];

				// Coordinate
				std::array<double, 3> coords;
				coords[Vertex::COORD_X] = m_vertexXCoords[i];
				coords[Vertex::COORD_Y] = m_vertexYCoords[j];
				if (isThreeDimensional()) {
					coords[Vertex::COORD_Z] = m_vertexZCoords[k];
				} else {
					coords[Vertex::COORD_Z] = 0.0;
				}

				vertex.setCoords(coords);
			}
		}
	}
}

/*!
	Creates the cells of the patch.
*/
void CartesianPatch::createCells()
{
	std::cout << "  >> Creating cells\n";

	// Info on the cells
	ElementInfo::Type cellType;
	if (isThreeDimensional()) {
		cellType = ElementInfo::VOXEL;
	} else {
		cellType = ElementInfo::PIXEL;
	}

	const ElementInfo &cellTypeInfo = ElementInfo::getElementInfo(cellType);
	const int &nCellVertices = cellTypeInfo.nVertices;

	// Count the cells
	long nTotalCells = 1;
	for (int n = 0; n < getDimension(); n++) {
		nTotalCells *= m_nCells1D[n];
	}

	std::cout << "    - Cell count: " << nTotalCells << "\n";

	m_cells.reserve(nTotalCells);

	// Create the cells
	std::array<double, 3> centroid = {0.0, 0.0, 0.0};
	for (int i = 0; i < m_nCells1D[Vertex::COORD_X]; i++) {
		centroid[Vertex::COORD_X] = 0.5 * (m_vertexXCoords[i] + m_vertexXCoords[i+1]);
		for (int j = 0; j < m_nCells1D[Vertex::COORD_Y]; j++) {
			centroid[Vertex::COORD_Y] = 0.5 * (m_vertexYCoords[j] + m_vertexYCoords[j+1]);
			for (int k = 0; (isThreeDimensional()) ? (k < m_nCells1D[Vertex::COORD_Z]) : (k <= 0); k++) {
				long id_cell = getCellLinearId(i, j, k);
				Patch::createCell(id_cell);
				Cell &cell = m_cells[id_cell];

				// Initialize the cell
				cell.initialize(cellType, 1);

				// Interior flag
				cell.setInterior(true);

				// Connettività
				cell.setVertex(0, getVertexLinearId(i,     j,     k));
				cell.setVertex(1, getVertexLinearId(i + 1, j,     k));
				cell.setVertex(2, getVertexLinearId(i,     j + 1, k));
				cell.setVertex(3, getVertexLinearId(i + 1, j + 1, k));
				if (isThreeDimensional()) {
					cell.setVertex(4, getVertexLinearId(i,     j,     k + 1));
					cell.setVertex(5, getVertexLinearId(i + 1, j,     k + 1));
					cell.setVertex(6, getVertexLinearId(i,     j + 1, k + 1));
					cell.setVertex(7, getVertexLinearId(i + 1, j + 1, k + 1));
				}
			}
		}
	}
}

/*!
	Creates the interfaces of the patch.
*/
void CartesianPatch::createInterfaces()
{
	std::cout << "  >> Creating interfaces\n";

	// Count the interfaces
	long nTotalInterfaces = 0;
	nTotalInterfaces += countInterfacesDirection(Vertex::COORD_X);
	nTotalInterfaces += countInterfacesDirection(Vertex::COORD_Y);
	if (isThreeDimensional()) {
		nTotalInterfaces += countInterfacesDirection(Vertex::COORD_Z);
	}

	std::cout << "    - Interface count: " << nTotalInterfaces << "\n";

	// Create the interfaces
	m_interfaces.reserve(nTotalInterfaces);

	createInterfacesDirection(Vertex::COORD_X);
	createInterfacesDirection(Vertex::COORD_Y);
	if (isThreeDimensional()) {
		createInterfacesDirection(Vertex::COORD_Z);
	}
}

/*!
	Counts the interfaces normal to the given direction.

	\param direction the method will count the interfaces normal to this
	                 direction
*/
int CartesianPatch::countInterfacesDirection(const Vertex::Coordinate &direction)
{
	std::vector<int> *interfaceCount1D;
	switch (direction)  {

	case Vertex::COORD_X:
		interfaceCount1D = &m_nInterfacesX1D;
		break;

	case Vertex::COORD_Y:
		interfaceCount1D = &m_nInterfacesY1D;
		break;

	case Vertex::COORD_Z:
		interfaceCount1D = &m_nInterfacesZ1D;
		break;

	}

	int nInterfaces = 1;
	for (int n = 0; n < getDimension(); n++) {
		nInterfaces *= (*interfaceCount1D)[n];
	}

	return nInterfaces;
}

/*!
	Creates the interfaces normal to the given direction.

	\param direction the method will creat the interfaces normal to this
	                 direction
*/
void CartesianPatch::createInterfacesDirection(const Vertex::Coordinate &direction)
{
	std::cout << "  >> Creating interfaces normal to direction " << direction << "\n";

	// Info on the interfaces
	ElementInfo::Type interfaceType;
	if (isThreeDimensional()) {
		interfaceType = ElementInfo::PIXEL;
	} else {
		interfaceType = ElementInfo::LINE;
	}

	const ElementInfo &interfaceTypeInfo = ElementInfo::getElementInfo(interfaceType);
	const int nInterfaceVertices = interfaceTypeInfo.nVertices;

	std::vector<int> *interfaceCount1D;
	switch (direction)  {

	case Vertex::COORD_X:
		interfaceCount1D = &m_nInterfacesX1D;
		break;

	case Vertex::COORD_Y:
		interfaceCount1D = &m_nInterfacesY1D;
		break;

	case Vertex::COORD_Z:
		interfaceCount1D = &m_nInterfacesZ1D;
		break;

	}

	// Counters
	std::array<int, 3> counters = {{0, 0, 0}};
	int &i = counters[Vertex::COORD_X];
	int &j = counters[Vertex::COORD_Y];
	int &k = counters[Vertex::COORD_Z];

	// Creation of the interfaces
	for (i = 0; i < (*interfaceCount1D)[Vertex::COORD_X]; i++) {
		for (j = 0; j < (*interfaceCount1D)[Vertex::COORD_Y]; j++) {
			for (k = 0; (isThreeDimensional()) ? (k < (*interfaceCount1D)[Vertex::COORD_Z]) : (k <= 0); k++) {
				long id_interface = getInterfaceLinearId(direction, i, j, k);
				Patch::createInterface(id_interface);
				Interface &interface = m_interfaces[id_interface];

				// Interface type
				if (isThreeDimensional()) {
					interface.setType(ElementInfo::PIXEL);
				} else {
					interface.setType(ElementInfo::LINE);
				}

				// Owner
				std::array<int, 3> ownerIJK;
				for (int n = 0; n < 3; n++) {
					ownerIJK[n] = counters[n];
				}
				if (counters[direction] > 0) {
					ownerIJK[direction] -= 1;
				}
				Cell &owner = m_cells[getCellLinearId(ownerIJK)];

				int ownerFace = 2 * direction;
				if (counters[direction] == 0) {
					ownerFace++;
				}

				interface.setOwner(owner.get_id(), ownerFace);
				owner.setInterface(ownerFace, 0, interface.get_id());

				// Neighbour
				if (counters[direction] != 0 && counters[direction] != (*interfaceCount1D)[direction] - 1) {
					std::array<int, 3> neighIJK;
					for (int n = 0; n < 3; n++) {
						neighIJK[n] = counters[n];
					}

					Cell &neigh = m_cells[getCellLinearId(neighIJK)];

					int neighFace = 2 * direction + 1;

					interface.setNeigh(neigh.get_id(), neighFace);
					neigh.setInterface(neighFace, 0, interface.get_id());
				} else {
					interface.unsetNeigh();
				}

				// Connectivity
				std::unique_ptr<long[]> connect = std::unique_ptr<long[]>(new long[nInterfaceVertices]);
				if (direction == Vertex::COORD_X) {
					connect[0] = getVertexLinearId(i, j,     k);
					connect[1] = getVertexLinearId(i, j + 1, k);
					if (isThreeDimensional()) {
						connect[2] = getVertexLinearId(i, j + 1, k + 1);
						connect[3] = getVertexLinearId(i, j,     k + 1);
					}
				} else if (direction == Vertex::COORD_Y) {
					connect[0] = getVertexLinearId(i,     j,     k);
					connect[1] = getVertexLinearId(i + 1, j,     k);
					if (isThreeDimensional()) {
						connect[2] = getVertexLinearId(i + 1, j, k + 1);
						connect[3] = getVertexLinearId(i,     j, k + 1);
					}
				} else if (direction == Vertex::COORD_Z) {
					connect[0] = getVertexLinearId(i,     j,     k);
					connect[1] = getVertexLinearId(i + 1, j,     k);
					if (isThreeDimensional()) {
						connect[2] = getVertexLinearId(i + 1, j + 1, k);
						connect[3] = getVertexLinearId(i,     j + 1, k);
					}
				}

				interface.setConnect(std::move(connect));
			}
		}
	}
}

/*!
	Marks a cell for refinement.

	This is a void function since mesh refinement is not implemented
	for Cartesian meshes.

	\param id is the id of the cell that needs to be refined
*/
bool CartesianPatch::_markCellForRefinement(const long &id)
{
	BITPIT_UNUSED(id);

	return false;
}

/*!
	Marks a cell for coarsening.

	This is a void function since mesh coarsening is not implemented
	for Cartesian meshes.

	\param id the cell to be refined
*/
bool CartesianPatch::_markCellForCoarsening(const long &id)
{
	BITPIT_UNUSED(id);

	return false;
}

/*!
	Enables cell balancing.

	This is a void function since mesh coarsening is not implemented
	for Cartesian meshes.

	\param id is the id of the cell
	\param enabled defines if enable the balancing for the specified cell
*/
bool CartesianPatch::_enableCellBalancing(const long &id, bool enabled)
{
	BITPIT_UNUSED(id);
	BITPIT_UNUSED(enabled);

	return false;
}

/*!
	Converts the cell cartesian notation to a linear notation
*/
long CartesianPatch::getCellLinearId(const int &i, const int &j, const int &k) const
{
	long id = i;
	id += m_nCells1D[Vertex::COORD_X] * j;
	if (getDimension() == 3) {
		id += m_nCells1D[Vertex::COORD_Y] * m_nCells1D[Vertex::COORD_X] * k;
	}

	return id;
}

/*!
	Converts the cell cartesian notation to a linear notation
*/
long CartesianPatch::getCellLinearId(const std::array<int, 3> &ijk) const
{
	return getCellLinearId(ijk[Vertex::COORD_X], ijk[Vertex::COORD_Y], ijk[Vertex::COORD_Z]);
}


/*!
	Converts the vertex cartesian notation to a linear notation
*/
long CartesianPatch::getVertexLinearId(const int &i, const int &j, const int &k) const
{
	long id = i;
	id += m_nVertices1D[Vertex::COORD_X] * j;
	if (getDimension() == 3) {
		id += m_nVertices1D[Vertex::COORD_Y] * m_nVertices1D[Vertex::COORD_X] * k;
	}

	return id;
}

/*!
	Converts the vertex cartesian notation to a linear notation
*/
long CartesianPatch::getVertexLinearId(const std::array<int, 3> &ijk) const
{
	return getVertexLinearId(ijk[Vertex::COORD_X], ijk[Vertex::COORD_Y], ijk[Vertex::COORD_Z]);
}

/*!
	Converts the interface cartesian notation to a linear notation
*/
long CartesianPatch::getInterfaceLinearId(const int &normal, const int &i, const int &j, const int &k) const
{
	const int *nInterfaces;
	switch (normal) {

	case Vertex::COORD_X:
		nInterfaces = m_nInterfacesX1D.data();
		break;

	case Vertex::COORD_Y:
		nInterfaces = m_nInterfacesY1D.data();
		break;

	case Vertex::COORD_Z:
		nInterfaces = m_nInterfacesZ1D.data();
		break;

	}

	long offset = 0;
	if (normal <=  Vertex::COORD_Y) {
		long offset_y = 1;
		for (int i = 0; i < getDimension(); ++i) {
			offset_y *= m_nInterfacesY1D[i];
		}
		offset += offset_y;
	}
	if (normal <=  Vertex::COORD_X) {
		long offset_x = 1;
		for (int i = 0; i < getDimension(); ++i) {
			offset_x *= m_nInterfacesX1D[i];
		}
		offset += offset_x;
	}

	long id = offset + i;
	id += nInterfaces[Vertex::COORD_X] * j;
	if (isThreeDimensional()) {
		id += nInterfaces[Vertex::COORD_Y] * nInterfaces[Vertex::COORD_X] * k;
	}

	return id;
}

/*!
	@}
*/

}