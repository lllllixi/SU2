/*!
 * \file geometry_structure.cpp
 * \brief Main subroutines for creating the primal grid and multigrid structure.
 * \author Current Development: Stanford University.
 *         Original Structure: CADES 1.0 (2009).
 * \version 1.1.
 *
 * Stanford University Unstructured (SU2) Code
 * Copyright (C) 2012 Aerospace Design Laboratory
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../include/geometry_structure.hpp"

CGeometry::CGeometry(void) {
	nEdge = 0;
}

CGeometry::~CGeometry(void) {
	if (nElem_Bound != NULL) delete [] nElem_Bound;
	if (Tag_to_Marker != NULL) delete [] Tag_to_Marker;
	if (nElem_Bound_Storage != NULL) delete [] nElem_Bound_Storage;
	if (nVertex != NULL) delete [] nVertex;

/*	for (unsigned long iElem = 0; iElem < nElem; iElem++)
		elem[iElem]->~CPrimalGrid();
	if (elem != NULL) delete [] elem;

	for (unsigned short iMarker = 0; iMarker < nMarker; iMarker++)
		for (unsigned long iElem_Bound = 0; iElem_Bound < nElem_Bound[iMarker]; iElem_Bound++)
			bound[iMarker][iElem_Bound]->~CPrimalGrid();
	if (bound != NULL) delete [] bound;*/

	for (unsigned long iPoint = 0; iPoint < nPoint; iPoint ++)
		node[iPoint]->~CPoint();
//		node[iPoint]->~CDualGrid();
	if (node != NULL) delete [] node;

	for (unsigned long iEdge = 0; iEdge < nEdge; iEdge ++)
		edge[iEdge]->~CEdge();
//		edge[iEdge]->~CDualGrid();
	if (edge != NULL) delete [] edge;

	for (unsigned short iMarker = 0; iMarker < nMarker; iMarker++)
		for (unsigned long iVertex = 0; iVertex < nVertex[iMarker]; iVertex++)
			vertex[iMarker][iVertex]->~CVertex();
//			vertex[iMarker][iVertex]->~CDualGrid();
	if (vertex != NULL) delete [] vertex;
}

double CGeometry::Point2Plane_Distance(double *Coord, double *iCoord, double *jCoord, double *kCoord) {
	double CrossProduct[3], iVector[3], jVector[3], distance, modulus;
	unsigned short iDim;

	for (iDim = 0; iDim < 3; iDim ++) {
		iVector[iDim] = jCoord[iDim] - iCoord[iDim];
		jVector[iDim] = kCoord[iDim] - iCoord[iDim];
	}
	
	CrossProduct[0] = iVector[1]*jVector[2] - iVector[2]*jVector[1];
	CrossProduct[1] = iVector[2]*jVector[0] - iVector[0]*jVector[2];
	CrossProduct[2] = iVector[0]*jVector[1] - iVector[1]*jVector[0];
	
	modulus = sqrt(CrossProduct[0]*CrossProduct[0]+CrossProduct[1]*CrossProduct[1]+CrossProduct[2]*CrossProduct[2]);
	
	distance = 0.0;
	for (iDim = 0; iDim < 3; iDim ++)
		distance += CrossProduct[iDim]*(Coord[iDim]-iCoord[iDim]);
	distance /= modulus;
	
	return distance;

}

long CGeometry::FindEdge(unsigned long first_point, unsigned long second_point) {
	unsigned long iPoint = 0;
	unsigned short iNode;
	for (iNode = 0; iNode < node[first_point]->GetnPoint(); iNode++) {
		iPoint = node[first_point]->GetPoint(iNode);
		if (iPoint == second_point) break;
	}
	
	if (iPoint == second_point) return node[first_point]->GetEdge(iNode);	
	else {
		cout <<"Ups! I don't find the edge that connect "<< first_point <<" and "<< second_point <<"."<< endl;
		exit(1);
		return -1;
	}
}

void CGeometry::SetEdges(void) {
	unsigned long iPoint, jPoint, iEdge;
	unsigned short jNode, iNode;
	long TestEdge = 0;
	
	nEdge = 0;
	for(iPoint = 0; iPoint < nPoint; iPoint++)
		for(iNode = 0; iNode < node[iPoint]->GetnPoint(); iNode++) {
			jPoint = node[iPoint]->GetPoint(iNode);	
			for(jNode = 0; jNode < node[jPoint]->GetnPoint(); jNode++)
				if (node[jPoint]->GetPoint(jNode) == iPoint) {
					TestEdge = node[jPoint]->GetEdge(jNode); 
					break; 
				}
			if (TestEdge == -1) {
				node[iPoint]->SetEdge(nEdge, iNode);	
				node[jPoint]->SetEdge(nEdge, jNode);
				nEdge++;
			}
		}

	edge = new CEdge*[nEdge];

	for(iPoint = 0; iPoint < nPoint; iPoint++)
		for(iNode = 0; iNode < node[iPoint]->GetnPoint(); iNode++) {
			jPoint = node[iPoint]->GetPoint(iNode);
			iEdge = FindEdge(iPoint, jPoint);
			if (iPoint < jPoint) edge[iEdge] = new CEdge(iPoint,jPoint,nDim);
		}
}

void CGeometry::TestGeometry(void) {
	
	ofstream para_file;
	
	para_file.open("test_geometry.dat", ios::out);
	
	double *Normal = new double[nDim];
	
	for(unsigned long iEdge = 0; iEdge < nEdge; iEdge++) {
		para_file << "Edge index: " << iEdge << endl;
		para_file << "   Point index: " << edge[iEdge]->GetNode(0) << "\t" << edge[iEdge]->GetNode(1) << endl;
		edge[iEdge]->GetNormal(Normal);
		para_file << "      Face normal : ";
		for(unsigned short iDim = 0; iDim < nDim; iDim++)
			para_file << Normal[iDim] << "\t";
		para_file << endl;
	}
	
	para_file << endl;
	para_file << endl;
	para_file << endl;
	para_file << endl;
	
	for(unsigned short iMarker =0; iMarker < nMarker; iMarker++) {
		para_file << "Marker index: " << iMarker << endl;
		for(unsigned long iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
			para_file << "   Vertex index: " << iVertex << endl;
			para_file << "      Point index: " << vertex[iMarker][iVertex]->GetNode() << endl;
			para_file << "      Point coordinates : ";
			for(unsigned short iDim = 0; iDim < nDim; iDim++) {
				para_file << node[vertex[iMarker][iVertex]->GetNode()]->GetCoord(iDim) << "\t";}
			para_file << endl;
			vertex[iMarker][iVertex]->GetNormal(Normal);
			para_file << "         Face normal : ";
			for(unsigned short iDim = 0; iDim < nDim; iDim++)
				para_file << Normal[iDim] << "\t";
			para_file << endl;
		}
	}
	
}

CPhysicalGeometry::CPhysicalGeometry() : CGeometry() {}

CPhysicalGeometry::CPhysicalGeometry(CConfig *config, string val_mesh_filename, unsigned short val_format, unsigned short val_iDomain, unsigned short val_nDomain) : CGeometry() {
  
	/*--- Open File ---*/
//	ofstream Inner_file;
//	ofstream Outer_file;
	
//	Inner_file.open("Inner.dat", ios::out);
//	Outer_file.open("Outer.dat", ios::out);
//	int Counter_Inner = 0.0; int Counter_Outer = 0.0;
	
	string text_line, Marker_Tag;
	ifstream mesh_file;
	unsigned short iNode_Surface, VTK_Type, iMarker, iChar, iCount = 0; 
	unsigned long Point_Surface, iElem_Surface, iElem_Bound = 0, iPoint = 0, ielem_div = 0, ielem = 0, 
	nelem_edge = 0, nelem_triangle = 0, nelem_quad = 0, nelem_tetra = 0, 
	nelem_hexa = 0, nelem_wedge = 0, nelem_pyramid = 0, vnodes_edge[2], vnodes_triangle[3], 
	vnodes_quad[4], vnodes_tetra[4], vnodes_hexa[8], vnodes_wedge[6], vnodes_pyramid[5], dummy, GlobalIndex; 
	char cstr[200];
	double Coord_2D[2], Coord_3D[3], Conversion_Factor = 1.0;
	string::size_type position;
	int rank = MASTER_NODE;
  bool domain_flag = false;
  bool found_transform = false;
  
#ifndef NO_CGNS
  /*--- Local variables which are needed when calling the CGNS mid-level API. ---*/
  unsigned long vnodes_cgns[8];
  double Coord_cgns[3];
  int fn, nbases, nzones, ngrids, ncoords, nsections, file_type;
  int *vertices = NULL, *cells = NULL, nMarkers = 0, *boundVerts = NULL, npe;
  int interiorElems = 0, boundaryElems = 0, totalVerts = 0, prevVerts = 0;
  int cell_dim, phys_dim, nbndry, parent_flag;
  char basename[CGNS_STRING_SIZE], zonename[CGNS_STRING_SIZE]; 
  char coordname[CGNS_STRING_SIZE];
  cgsize_t* cgsize; cgsize = new cgsize_t[3];
  ZoneType_t zonetype;
  DataType_t datatype;
  double** coordArray = NULL;
  double*** gridCoords = NULL;
  ElementType_t elemType;
  cgsize_t range_min, range_max, startE, endE;
  range_min = 1;
  string currentElem;
  int** elemTypeVTK = NULL;
  int** elemIndex = NULL;
  int** nElems = NULL;
  int indexMax, elemMax; indexMax = elemMax = 0;
  cgsize_t**** connElems = NULL;
  cgsize_t* connElemTemp = NULL;
  cgsize_t ElementDataSize = NULL;
  cgsize_t* parentData = NULL;
  int** dataSize = NULL;
  bool** isInternal = NULL;
  char*** sectionNames = NULL;
#endif
  
#ifndef NO_MPI
	unsigned long LocalIndex;
	rank = MPI::COMM_WORLD.Get_rank();	
#endif
	
  switch (val_format) {
    case SU2:
		  
		  /*--- Open grid file ---*/
		  strcpy (cstr, val_mesh_filename.c_str());
		  mesh_file.open(cstr, ios::in);
		  if (mesh_file.fail()) {
			  cout << "There is no geometry file!!" << endl;
			  cout << "Press any key to exit..." << endl;
			  cin.get();
#ifdef NO_MPI
			  exit(1);	
#else
			  MPI::COMM_WORLD.Abort(1);
			  MPI::Finalize();
#endif
		  }
      
      /*--- If more than one, find the domain in the mesh file ---*/
      if (val_nDomain > 1) {
        while (getline (mesh_file,text_line)) {
          /*--- Search for the current domain ---*/
          position = text_line.find ("IDOM=",0);
          if (position != string::npos) {
            text_line.erase (0,5); 
            unsigned short jDomain = atoi(text_line.c_str());
            if (jDomain == val_iDomain) {
              if (rank == MASTER_NODE) cout << "  Domain " << val_iDomain << ":" << endl;
              break;
            }
          }
        }
      }
		  
		  /*--- Read grid file with format SU2 ---*/
		  while (getline (mesh_file,text_line)) {
			  
			  /*--- Read the dimension of the problem ---*/
			  position = text_line.find ("NDIME=",0);
			  if (position != string::npos) {
          if (domain_flag == false) {
            text_line.erase (0,6); nDim = atoi(text_line.c_str());
            if (rank == MASTER_NODE) {
              if (nDim == 2) cout << "Two dimensional problem." << endl;
              if (nDim == 3) cout << "Three dimensional problem." << endl;
            }
            domain_flag = true;
          } else {
            break;
          }
        }
			  
			  /*--- Read the information about inner elements ---*/
			  position = text_line.find ("NELEM=",0);
			  if (position != string::npos) {
				  text_line.erase (0,6); nElem = atoi(text_line.c_str());
				  if (rank == MASTER_NODE)
            cout << nElem << " interior elements." << endl;
				  
				  /*--- Allocate space for elements ---*/
				  if (!config->GetDivide_Element()) elem = new CPrimalGrid*[nElem];
				  else {
					  if (nDim == 2) elem = new CPrimalGrid*[2*nElem];
					  if (nDim == 3) {
						  elem = new CPrimalGrid*[5*nElem];
						  cout << "The grid division only works in 2D!!" << endl;
						  cout << "Press any key to exit..." << endl;
						  cin.get();
					  }
				  }
				  
				  /*--- Loop over all the volumetric elements ---*/
				  while (ielem_div < nElem) {
					  getline(mesh_file,text_line);
					  istringstream elem_line(text_line);
					  
					  elem_line >> VTK_Type;
					  
					  switch(VTK_Type) {
						  case TRIANGLE:	
							  elem_line >> vnodes_triangle[0]; elem_line >> vnodes_triangle[1]; elem_line >> vnodes_triangle[2];
							  elem[ielem] = new CTriangle(vnodes_triangle[0],vnodes_triangle[1],vnodes_triangle[2],nDim);
							  ielem_div++; ielem++; nelem_triangle++; break;
						  case RECTANGLE:
							  elem_line >> vnodes_quad[0]; elem_line >> vnodes_quad[1]; elem_line >> vnodes_quad[2]; elem_line >> vnodes_quad[3];
							  if (!config->GetDivide_Element()) {
								  elem[ielem] = new CRectangle(vnodes_quad[0],vnodes_quad[1],vnodes_quad[2],vnodes_quad[3],nDim);
								  ielem++; nelem_quad++; }
							  else {
								  elem[ielem] = new CTriangle(vnodes_quad[0],vnodes_quad[1],vnodes_quad[2],nDim);
								  ielem++; nelem_triangle++;
								  elem[ielem] = new CTriangle(vnodes_quad[0],vnodes_quad[2],vnodes_quad[3],nDim);
								  ielem++; nelem_triangle++; }
							  ielem_div++;
							  break;
						  case TETRAHEDRON: 
							  elem_line >> vnodes_tetra[0]; elem_line >> vnodes_tetra[1]; elem_line >> vnodes_tetra[2]; elem_line >> vnodes_tetra[3];
							  elem[ielem] = new CTetrahedron(vnodes_tetra[0],vnodes_tetra[1],vnodes_tetra[2],vnodes_tetra[3]);
							  ielem_div++; ielem++; nelem_tetra++; break;
						  case HEXAHEDRON:
							  elem_line >> vnodes_hexa[0]; elem_line >> vnodes_hexa[1]; elem_line >> vnodes_hexa[2];
							  elem_line >> vnodes_hexa[3]; elem_line >> vnodes_hexa[4]; elem_line >> vnodes_hexa[5];
							  elem_line >> vnodes_hexa[6]; elem_line >> vnodes_hexa[7];
							  if (!config->GetDivide_Element()) {
								  elem[ielem] = new CHexahedron(vnodes_hexa[0],vnodes_hexa[1],vnodes_hexa[2],vnodes_hexa[3],vnodes_hexa[4],vnodes_hexa[5],vnodes_hexa[6],vnodes_hexa[7]);
								  ielem++; nelem_hexa++; }
							  else {
								  elem[ielem] = new CTetrahedron(vnodes_hexa[0],vnodes_hexa[1],vnodes_hexa[2],vnodes_hexa[5]);
								  ielem++; nelem_tetra++;
								  elem[ielem] = new CTetrahedron(vnodes_hexa[0],vnodes_hexa[2],vnodes_hexa[7],vnodes_hexa[5]);
								  ielem++; nelem_tetra++;
								  elem[ielem] = new CTetrahedron(vnodes_hexa[0],vnodes_hexa[2],vnodes_hexa[3],vnodes_hexa[7]);
								  ielem++; nelem_tetra++;
								  elem[ielem] = new CTetrahedron(vnodes_hexa[0],vnodes_hexa[5],vnodes_hexa[7],vnodes_hexa[4]);
								  ielem++; nelem_tetra++;
								  elem[ielem] = new CTetrahedron(vnodes_hexa[2],vnodes_hexa[7],vnodes_hexa[5],vnodes_hexa[6]);
								  ielem++; nelem_tetra++; }
							  ielem_div++;
							  break;
						  case WEDGE: 
							  elem_line >> vnodes_wedge[0]; elem_line >> vnodes_wedge[1]; elem_line >> vnodes_wedge[2];
							  elem_line >> vnodes_wedge[3]; elem_line >> vnodes_wedge[4]; elem_line >> vnodes_wedge[5];
							  if (!config->GetDivide_Element()) {
								  elem[ielem] = new CWedge(vnodes_wedge[0],vnodes_wedge[1],vnodes_wedge[2],vnodes_wedge[3],vnodes_wedge[4],vnodes_wedge[5]);
								  ielem++; nelem_wedge++; }
							  else {
								  elem[ielem] = new CTetrahedron(vnodes_wedge[0],vnodes_wedge[1],vnodes_wedge[2],vnodes_wedge[5]);
								  ielem++; nelem_tetra++;
								  elem[ielem] = new CTetrahedron(vnodes_wedge[0],vnodes_wedge[1],vnodes_wedge[5],vnodes_wedge[4]);
								  ielem++; nelem_tetra++;
								  elem[ielem] = new CTetrahedron(vnodes_wedge[0],vnodes_wedge[4],vnodes_wedge[5],vnodes_wedge[3]);
								  ielem++; nelem_tetra++; }
							  ielem_div++; 
							  break;
						  case PYRAMID: 
							  elem_line >> vnodes_pyramid[0]; elem_line >> vnodes_pyramid[1]; elem_line >> vnodes_pyramid[2];
							  elem_line >> vnodes_pyramid[3]; elem_line >> vnodes_pyramid[4];
							  if (!config->GetDivide_Element()) {
								  elem[ielem] = new CPyramid(vnodes_pyramid[0],vnodes_pyramid[1],vnodes_pyramid[2],vnodes_pyramid[3],vnodes_pyramid[4]);
								  ielem++; nelem_pyramid++;
							  }
							  else {
								  elem[ielem] = new CTetrahedron(vnodes_pyramid[0],vnodes_pyramid[1],vnodes_pyramid[2],vnodes_pyramid[4]);
								  ielem++; nelem_tetra++;
								  elem[ielem] = new CTetrahedron(vnodes_pyramid[0],vnodes_pyramid[2],vnodes_pyramid[3],vnodes_pyramid[4]);
								  ielem++; nelem_tetra++; }
							  ielem_div++; 
							  break;	
					  }
				  }
				  nElem_Storage = nelem_triangle*4 + nelem_quad*5 + nelem_tetra*5 + nelem_hexa*9 + nelem_wedge*7 + nelem_pyramid*6;
				  if (config->GetDivide_Element()) nElem = nelem_triangle + nelem_quad + nelem_tetra + nelem_hexa + nelem_wedge + nelem_pyramid;
			  }
			  
			  /*--- Read number of points ---*/
			  position = text_line.find ("NPOIN=",0);
			  if (position != string::npos) {
				  text_line.erase (0,6);
          
          /*--- Check for ghost points. ---*/
          stringstream test_line(text_line);
          while (test_line >> dummy)
            iCount++;
					
          /*--- Now read and store the number of points and possible ghost points. ---*/
          stringstream  stream_line(text_line);
          if (iCount == 2) {
            stream_line >> nPoint;
            stream_line >> nPointDomain;
#ifdef NO_MPI
						if (rank == MASTER_NODE)
              cout << nPoint << " points, and " << nPoint-nPointDomain << " ghost points." << endl;
#else
						unsigned long Local_nPoint = nPoint, Local_nPointDomain = nPointDomain;
						unsigned long Global_nPoint = 0, Global_nPointDomain = 0;
						MPI::COMM_WORLD.Allreduce(&Local_nPoint, &Global_nPoint, 1, MPI::UNSIGNED_LONG, MPI::SUM); 
						MPI::COMM_WORLD.Allreduce(&Local_nPointDomain, &Global_nPointDomain, 1, MPI::UNSIGNED_LONG, MPI::SUM);
						if (rank == MASTER_NODE)
              cout << Global_nPoint << " points, and " << Global_nPoint-Global_nPointDomain << " ghost points." << endl;
#endif
          } 
					else if (iCount == 1) {
            stream_line >> nPoint;
            nPointDomain = nPoint;
            if (rank == MASTER_NODE) cout << nPoint << " points." << endl;
          } 
					else {
            cout << "NPOIN improperly specified!!" << endl;
            cout << "Press any key to exit..." << endl;
            cin.get();
#ifdef NO_MPI
            exit(1);	
#else
            MPI::COMM_WORLD.Abort(1);
            MPI::Finalize();
#endif
          }
          
          /*--- Retrieve grid conversion factor. The conversion is only
           applied for SU2_CFD. All other SU2 components leave the mesh
           as is. ---*/
          if (config->GetKind_SU2() == SU2_CFD)
            Conversion_Factor = config->GetConversion_Factor();
          else
            Conversion_Factor = 1.0;
          
				  node = new CPoint*[nPoint];
				  while (iPoint < nPoint) {
					  getline(mesh_file,text_line);
					  istringstream point_line(text_line);
					  switch(nDim) {
						  case 2:
								GlobalIndex = iPoint;
#ifdef NO_MPI
								point_line >> Coord_2D[0]; point_line >> Coord_2D[1];
#else
								point_line >> Coord_2D[0]; point_line >> Coord_2D[1]; point_line >> LocalIndex; point_line >> GlobalIndex;
#endif
								node[iPoint] = new CPoint(Conversion_Factor*Coord_2D[0], Conversion_Factor*Coord_2D[1], GlobalIndex, config);
							  iPoint++; break;
						  case 3:
								GlobalIndex = iPoint;
#ifdef NO_MPI
							  point_line >> Coord_3D[0]; point_line >> Coord_3D[1]; point_line >> Coord_3D[2];
#else
								point_line >> Coord_3D[0]; point_line >> Coord_3D[1]; point_line >> Coord_3D[2]; point_line >> LocalIndex; point_line >> GlobalIndex;
#endif
							  node[iPoint] = new CPoint(Conversion_Factor*Coord_3D[0], Conversion_Factor*Coord_3D[1], Conversion_Factor*Coord_3D[2], GlobalIndex, config);
								iPoint++; break;
					  }
				  }
			  }			
			  
				 
			  /*--- Read number of markers ---*/
			  position = text_line.find ("NMARK=",0);
			  if (position != string::npos) {
				  text_line.erase (0,6); nMarker = atoi(text_line.c_str());
				  if (rank == MASTER_NODE) cout << nMarker << " surface markers." << endl;
				  config->SetnMarker_All(nMarker);
				  bound = new CPrimalGrid**[nMarker];
				  nElem_Bound = new unsigned long [nMarker];
				  nElem_Bound_Storage = new unsigned long [nMarker];
				  Tag_to_Marker = new string [MAX_INDEX_VALUE];
				  
				  for (iMarker = 0 ; iMarker < nMarker; iMarker++) {
					  getline (mesh_file,text_line);
					  text_line.erase (0,11);
					  string::size_type position;
					  for (iChar = 0; iChar < 20; iChar++) {
						  position = text_line.find( " ", 0 );
						  if(position != string::npos) text_line.erase (position,1);
						  position = text_line.find( "\r", 0 );
						  if(position != string::npos) text_line.erase (position,1);
						  position = text_line.find( "\n", 0 );
						  if(position != string::npos) text_line.erase (position,1);
					  }
					  Marker_Tag = text_line.c_str();	
					  
					  /*--- Physical boundaries definition ---*/
					  if (Marker_Tag != "SEND_RECEIVE") {						
						  getline (mesh_file,text_line);
						  text_line.erase (0,13); nElem_Bound[iMarker] = atoi(text_line.c_str());
						  if (rank == MASTER_NODE)
							  cout << nElem_Bound[iMarker]  << " boundary elements in index "<< iMarker <<" (Marker = " <<Marker_Tag<< ")." << endl;
						  bound[iMarker] = new CPrimalGrid* [nElem_Bound[iMarker]];
						  
						  nelem_edge = 0; nelem_triangle = 0; nelem_quad = 0; ielem = 0;
						  for (iElem_Bound = 0; iElem_Bound < nElem_Bound[iMarker]; iElem_Bound++) {
							  getline(mesh_file,text_line); 
							  istringstream bound_line(text_line);
							  bound_line >> VTK_Type;
							  switch(VTK_Type) {
								  case LINE:
									  bound_line >> vnodes_edge[0]; bound_line >> vnodes_edge[1];
									  bound[iMarker][ielem] = new CLine(vnodes_edge[0],vnodes_edge[1],2);
									  ielem++; nelem_edge++; break;
								  case TRIANGLE:
									  bound_line >> vnodes_triangle[0]; bound_line >> vnodes_triangle[1]; bound_line >> vnodes_triangle[2];
									  bound[iMarker][ielem] = new CTriangle(vnodes_triangle[0],vnodes_triangle[1],vnodes_triangle[2],3);
									  ielem++; nelem_triangle++; break;
								  case RECTANGLE: 
									  bound_line >> vnodes_quad[0]; bound_line >> vnodes_quad[1]; bound_line >> vnodes_quad[2]; bound_line >> vnodes_quad[3];
									  bound[iMarker][ielem] = new CRectangle(vnodes_quad[0],vnodes_quad[1],vnodes_quad[2],vnodes_quad[3],3);
										ielem++; nelem_quad++; break;
							  }
						  }	
						  nElem_Bound_Storage[iMarker] = nelem_edge*3 + nelem_triangle*4 + nelem_quad*5;
														
						  /*--- Update config information storing the boundary information in the right place ---*/
						  Tag_to_Marker[config->GetMarker_Config_Tag(Marker_Tag)] = Marker_Tag;
						  config->SetMarker_All_Tag(iMarker, Marker_Tag);
						  config->SetMarker_All_Boundary(iMarker, config->GetMarker_Config_Boundary(Marker_Tag));
						  config->SetMarker_All_Monitoring(iMarker, config->GetMarker_Config_Monitoring(Marker_Tag));
						  config->SetMarker_All_Plotting(iMarker, config->GetMarker_Config_Plotting(Marker_Tag));
						  config->SetMarker_All_Moving(iMarker, config->GetMarker_Config_Moving(Marker_Tag));
						  config->SetMarker_All_PerBound(iMarker, config->GetMarker_Config_PerBound(Marker_Tag));
						  config->SetMarker_All_SendRecv(iMarker, NONE);
						  
					  }
            
					  /*--- Send-Receive boundaries definition ---*/
					  else {
						  unsigned long nelem_vertex = 0, vnodes_vertex;
						  unsigned short transform;
						  getline (mesh_file,text_line);
						  text_line.erase (0,13); nElem_Bound[iMarker] = atoi(text_line.c_str());
						  bound[iMarker] = new CPrimalGrid* [nElem_Bound[iMarker]];
						  
						  nelem_vertex = 0; ielem = 0;
						  getline (mesh_file,text_line); text_line.erase (0,8);
						  config->SetMarker_All_Boundary(iMarker, SEND_RECEIVE);
						  config->SetMarker_All_SendRecv(iMarker, atoi(text_line.c_str()));
						  
						  for (iElem_Bound = 0; iElem_Bound < nElem_Bound[iMarker]; iElem_Bound++) {
							  getline(mesh_file,text_line);
							  istringstream bound_line(text_line);
							  bound_line >> VTK_Type; bound_line >> vnodes_vertex; bound_line >> transform;
							  
							  bound[iMarker][ielem] = new CVertexMPI(vnodes_vertex, nDim);
							  bound[iMarker][ielem]->SetRotation_Type(transform);
							  ielem++; nelem_vertex++;
							  if (config->GetMarker_All_SendRecv(iMarker) < 0) 
								  node[vnodes_vertex]->SetDomain(false);
							  
						  }
						  
					  }
					  
				  }
			  }
				
        /*--- Read periodic transformation info (center, rotation, translation) ---*/
        position = text_line.find ("NPERIODIC=",0);
        if (position != string::npos) {
          unsigned short nPeriodic, iPeriodic, iIndex;
          
          /*--- Set bool signifying that periodic transormations were found ---*/
          found_transform = true;
          
          /*--- Read and store the number of transformations. ---*/
          text_line.erase (0,10); nPeriodic = atoi(text_line.c_str());
          if (rank == MASTER_NODE)
            cout << nPeriodic - 1 << " periodic transformations." << endl;
          config->SetnPeriodicIndex(nPeriodic);
          
          /*--- Store center, rotation, & translation in that order for each. ---*/
          for (iPeriodic = 0; iPeriodic < nPeriodic; iPeriodic++) {
            getline (mesh_file,text_line);
            position = text_line.find ("PERIODIC_INDEX=",0);
            if (position != string::npos) {
              text_line.erase (0,15); iIndex = atoi(text_line.c_str());
              if (iIndex != iPeriodic) {
                cout << "PERIODIC_INDEX out of order in SU2 file!!" << endl;
                cout << "Press any key to exit..." << endl;
                cin.get();
#ifdef NO_MPI
                exit(1);	
#else
                MPI::COMM_WORLD.Abort(1);
                MPI::Finalize();
#endif
              }
            }
            double* center    = new double[3];
            double* rotation  = new double[3];
            double* translate = new double[3];
            getline (mesh_file,text_line);
            istringstream cent(text_line);
            cent >> center[0]; cent >> center[1]; cent >> center[2];
            config->SetPeriodicCenter(iPeriodic, center);
            getline (mesh_file,text_line);
            istringstream rot(text_line);
            rot >> rotation[0]; rot >> rotation[1]; rot >> rotation[2];
            config->SetPeriodicRotation(iPeriodic, rotation);
            getline (mesh_file,text_line);
            istringstream tran(text_line);
            tran >> translate[0]; tran >> translate[1]; tran >> translate[2];
            config->SetPeriodicTranslate(iPeriodic, translate);
          }
          
        }
				
			}
      
      /*--- If no periodic transormations were found, store default zeros ---*/
      if (!found_transform) {
        unsigned short nPeriodic = 1, iPeriodic = 0;
        config->SetnPeriodicIndex(nPeriodic);
        double* center    = new double[3];
        double* rotation  = new double[3];
        double* translate = new double[3];
        for (unsigned short iDim = 0; iDim < 3; iDim++) {
          center[iDim] = 0.0; rotation[iDim] = 0.0; translate[iDim] = 0.0;
        }
        config->SetPeriodicCenter(iPeriodic, center);
        config->SetPeriodicRotation(iPeriodic, rotation);
        config->SetPeriodicTranslate(iPeriodic, translate);
      }
      
      /*--- Close the input file ---*/
      mesh_file.close();
      
      break;
      
    case CGNS:
			
#ifndef NO_CGNS
      
      /*--- Throw error if not in serial mode. ---*/
#ifndef NO_MPI
      cout << "Parallel support with CGNS format not yet implemented!!" << endl;
      cout << "Press any key to exit..." << endl;
      cin.get();
      MPI::COMM_WORLD.Abort(1);
      MPI::Finalize();
#endif
      
      /*--- Check whether the supplied file is truly a CGNS file. ---*/
      if ( cg_is_cgns(val_mesh_filename.c_str(),&file_type) != CG_OK ) {
        printf( "\n\n   !!! Error !!!\n" );
        printf( " %s is not a CGNS file.\n", val_mesh_filename.c_str());
        printf( " Now exiting...\n\n");
        exit(0);
      }
      
      /*--- Open the CGNS file for reading. The value of fn returned
       is the specific index number for this file and will be 
       repeatedly used in the function calls. ---*/
      if ( cg_open(val_mesh_filename.c_str(),CG_MODE_READ,&fn) ) cg_error_exit();
      cout << "Reading the CGNS file: " << val_mesh_filename.c_str() << endl;
      
      /*--- Get the number of databases. This is the highest node
       in the CGNS heirarchy. ---*/
      if ( cg_nbases(fn, &nbases) ) cg_error_exit();
      cout << "CGNS file contains " << nbases << " database(s)." << endl;
      
      /*--- Check if there is more than one database. Throw an
       error if there is because this reader can currently
       only handle one database. ---*/
      if ( nbases > 1 ) {
        printf("\n\n   !!! Error !!!\n" );
        printf("CGNS reader currently incapable of handling more than 1 database.");
        printf("Now exiting...\n\n");
        exit(0);
      }
      
      /*--- Read the databases. Note that the indexing starts at 1. ---*/
      for ( int i = 1; i <= nbases; i++ ) {
        
        if ( cg_base_read(fn, i, basename, &cell_dim, &phys_dim) ) cg_error_exit();
        
        /*--- Get the number of zones for this base. ---*/
        if ( cg_nzones(fn, i, &nzones) ) cg_error_exit();    
        cout << "Database " << i << ", " << basename << ": " << nzones;
        cout << " zone(s), cell dimension of " << cell_dim << ", physical ";
        cout << "dimension of " << phys_dim << "." << endl;
        
        /*--- Check if there is more than one zone. Throw an
         error if there is, because this reader can currently
         only handle one zone. This can/will be extended in the future. ---*/
        if ( nzones > 1 ) {
          printf("\n\n   !!! Error !!!\n" );
          printf("CGNS reader currently incapable of handling more than 1 zone.");
          printf("Now exiting...\n\n");
          exit(0);
        }
        
        /*--- Initialize some data structures for  all zones. ---*/
        vertices   = new int[nzones];
        cells      = new int[nzones];
        boundVerts = new int[nzones];
        
        coordArray  = new double*[nzones];
        gridCoords  = new double**[nzones];
        elemTypeVTK = new int*[nzones];
        elemIndex   = new int*[nzones];
        nElems      = new int*[nzones];
        dataSize    = new int*[nzones];
        isInternal  = new bool*[nzones];
        nMarkers    = 0;
        
        sectionNames= new char**[nzones];
        connElems = new cgsize_t***[nzones];
        
        /*--- Loop over all zones in this base. Again, indexing starts at 1. ---*/
        for ( int j = 1; j <= nzones; j++ ) {
          
          /*--- Read the basic information for this zone, including
           the name and the number of vertices, cells, and
           boundary cells which are stored in the cgsize variable. ---*/
          if ( cg_zone_read(fn, i, j, zonename, cgsize) ) cg_error_exit();
          
          /*--- Rename the zone size information for clarity. 
           NOTE: The number of cells here may be only the number of
           interior elements or it may be the total. This needs to 
           be counted explicitly later. ---*/
          vertices[j-1]   = cgsize[0];
          cells[j-1]      = cgsize[1];
          boundVerts[j-1] = cgsize[2];
          
          /*--- Increment the total number of vertices from all zones. ---*/
          totalVerts += vertices[j-1];
          
          if ( cg_zone_type(fn, i, j, &zonetype) ) cg_error_exit();
          cout << "Zone " << j << ", " << zonename << ": " << vertices[j-1];
          cout << " vertices, " << cells[j-1] << " cells, " << boundVerts[j-1];
          cout << " boundary vertices." << endl;
          
          /*--- Retrieve the number of grids in this zone.
           For now, we know this is one, but to be more
           general, this will need to check and allow
           for a loop over all grids. ---*/
          
          if ( cg_ngrids(fn, i, j, &ngrids) ) cg_error_exit();
          
          /*--- Check the number of coordinate arrays stored
           in this zone. Should be 2 for 2-D grids and
           3 for 3-D grids. ---*/
          if ( ngrids > 1 ) {
            printf("\n\n   !!! Error !!!\n" );
            printf("CGNS reader currently handles only 1 grid per zone.");
            printf("Now exiting...\n\n");
            exit(0);
          }
          
          if ( cg_ncoords( fn, i, j, &ncoords) ) cg_error_exit();
          cout << "Reading grid coordinates..." << endl;
          cout << "Number of coordinate dimensions is " << ncoords << "." << endl;
          
          /*--- Set the value of range_max to the total number
           of nodes in the unstructured mesh. Also allocate
           memory for the temporary array that will hold
           the grid coordinates as they are extracted. ---*/
          
          range_max       = cgsize[0];
          coordArray[j-1] = new double[range_max];
          
          /*--- Allocate memory for the 2-D array which will
           store the x, y, & z (if required) coordinates
           for writing into the SU2 mesh. ---*/
          
          gridCoords[j-1] = new double*[ncoords];
          for (int ii = 0; ii < ncoords; ii++) {
            *( gridCoords[j-1] + ii ) = new double[range_max];
          }
          
          /*--- Loop over each set of coordinates. Note again
           that the indexing starts at 1. ---*/
          
          for ( int k = 1; k <= ncoords; k++ ) {
            
            /*--- Read the coordinate info. This will retrieve the
             data type (either RealSingle or RealDouble) as
             well as the coordname which will specifiy the
             type of data that it is based in the SIDS convention.
             This might be "CoordinateX," for instance. ---*/
            
            if ( cg_coord_info(fn, i, j, k, &datatype, coordname) ) cg_error_exit();
            cout << "Reading " << coordname << " values from file." << endl;
            
            /*--- Always retrieve the grid coords in double precision. ---*/
            datatype = RealDouble;
            if ( cg_coord_read(fn, i, j, coordname, datatype, &range_min,
                               &range_max, coordArray[j-1]) ) cg_error_exit();
            
            /*--- Copy these coords into the 2-D array for storage until
             writing the SU2 mesh. ---*/
            
            for (int m = 0; m < range_max; m++ ) {
              gridCoords[j-1][k-1][m] = coordArray[j-1][m];
            }
            
          }
          
          /*--- Begin section for retrieving the connectivity info. ---*/
          
          cout << "Reading connectivity information..." << endl;
          
          /*--- First check the number of sections. ---*/
          
          if ( cg_nsections(fn, i, j, &nsections) ) cg_error_exit();
          cout << "Number of connectivity sections is " << nsections << "." << endl;
          
          /*--- Allocate several data structures to hold the various
           pieces of information describing each section. It is
           stored in this manner so that it can be written to 
           SU2 memory later. ---*/
          
          elemTypeVTK[j-1] = new int[nsections];
          elemIndex[j-1]   = new int[nsections];
          nElems[j-1]      = new int[nsections];
          dataSize[j-1]    = new int[nsections];
          isInternal[j-1]  = new bool[nsections];
					//          nMarkers    = 0;
          
          sectionNames[j-1] = new char*[nsections];
          for (int ii = 0; ii < nsections; ii++) {
            sectionNames[j-1][ii]= new char[CGNS_STRING_SIZE];
          }
          
          /*--- Loop over each section. This will include the main
           connectivity information for the grid cells, as well
           as any boundaries which were labeled before export. ---*/
          
          for ( int s = 1; s <= nsections; s++ ) {
            
            /*--- Read the connectivity details for this section.
             Store the total number of elements in this section
             to be used later for memory allocation. ---*/
            
            if ( cg_section_read(fn, i, j, s, sectionNames[j-1][s-1], &elemType, &startE,
                                 &endE, &nbndry, &parent_flag) ) cg_error_exit();
            nElems[j-1][s-1] = (int) (endE-startE+1);
            
            /*--- Read the total number of nodes that will be
             listed when reading this section. ---*/
            
            if ( cg_ElementDataSize(fn, i, j, s, &ElementDataSize) ) 
              cg_error_exit();
            dataSize[j-1][s-1] = ElementDataSize;
            
            /*--- Find the number of nodes required to represent
             this type of element. ---*/
            
            if ( cg_npe(elemType, &npe) ) cg_error_exit();
            elemIndex[j-1][s-1] = npe;
            
            /*--- Need to check the element type and correctly
             specify the VTK identifier for that element.
             SU2 recognizes elements by their VTK number. ---*/
            
            switch (elemType) {
              case NODE:
                currentElem      = "Vertex";
                elemTypeVTK[j-1][s-1] = 1;
                break;
              case BAR_2:
                currentElem      = "Line";
                elemTypeVTK[j-1][s-1] = 3;
                break;
              case BAR_3:
                currentElem      = "Line";
                elemTypeVTK[j-1][s-1] = 3;
                break;
              case TRI_3:
                currentElem      = "Triangle";
                elemTypeVTK[j-1][s-1] = 5;
                break;
              case QUAD_4:
                currentElem      = "Rectangle";
                elemTypeVTK[j-1][s-1] = 9;
                break;
              case TETRA_4:
                currentElem      = "Tetrahedron";
                elemTypeVTK[j-1][s-1] = 10;
                break;
              case HEXA_8:
                currentElem      = "Hexahedron";
                elemTypeVTK[j-1][s-1] = 12;
                break;
              case PENTA_6:
                currentElem      = "Wedge";
                elemTypeVTK[j-1][s-1] = 13;
                break;
              case PYRA_5:
                currentElem      = "Pyramid";
                elemTypeVTK[j-1][s-1] = 14;
                break;
              default:
                printf( "\n\n   !!! Error !!!\n" );
                printf( " Unrecognized element type.\n");
                printf( " Now exiting...\n\n");
                exit(0);
                break;
            }
            
            /*--- Check if the elements in this section are part
             of the internal domain or are part of the boundary
             surfaces. This will be used later to separate the
             internal connectivity from the boundary connectivity. 
             We will check for quad and tri elements for 3-D meshes
             because these will be the boundaries. Similarly, line
             elements will be boundaries to 2-D problems. ---*/
            
            if ( cell_dim == 2 ) {
              /*--- In 2-D check for line elements, VTK type 3. ---*/
              if (elemTypeVTK[j-1][s-1] == 3) {
                isInternal[j-1][s-1] = false;
                nMarkers++;
                boundaryElems += nElems[j-1][s-1];
              } else {
                isInternal[j-1][s-1] = true;
                interiorElems += nElems[j-1][s-1];
              }
            } else {
              /*--- In 3-D check for tri or quad elements, VTK types 5 or 9. ---*/
              if (elemTypeVTK[j-1][s-1] == 5 || elemTypeVTK[j-1][s-1] == 9) {
                isInternal[j-1][s-1] = false;
                nMarkers++;
                boundaryElems += nElems[j-1][s-1];
              } else {
                isInternal[j-1][s-1] = true;
                interiorElems += nElems[j-1][s-1];
              }
            }
            
            /*--- Keep track of the sections with the largest
             number of elements and the number of nodes 
             required to specify an element of a specific
             type. These max values will be used to allocate 
             one large array. ---*/
            
            if ( elemIndex[j-1][s-1] > indexMax || s == 1 ) indexMax = elemIndex[j-1][s-1];
            if ( nElems[j-1][s-1] > elemMax || s == 1 )     elemMax  = nElems[j-1][s-1];
            
            /*--- Print some information to the console. ---*/
            
            cout << "Reading section " << sectionNames[j-1][s-1];
            cout << " of element type " << currentElem << "\n   starting at ";
            cout << startE << " and ending at " << endE << "." << endl;
            
          }
          
          /*--- Allocate memory to store all of the connectivity
           information in one large array. ---*/
          
          connElems[j-1] = new cgsize_t**[nsections];
          for (int ii = 0; ii < nsections; ii++) {
            connElems[j-1][ii] = new cgsize_t*[indexMax];
            for (int jj = 0; jj < indexMax; jj++) {
              connElems[j-1][ii][jj] = new cgsize_t[elemMax];
            }
          }
          
          for ( int s = 1; s <= nsections; s++ ) {
            
            connElemTemp = new cgsize_t[dataSize[j-1][s-1]];
            
            /*--- Retrieve the connectivity information and store. ---*/
            
            if ( cg_elements_read(fn, i, j, s, connElemTemp, parentData) ) 
              cg_error_exit();
            
            /*--- Copy these values into the larger array for
             storage until writing the SU2 file. ---*/
            
            int counter = 0;
            for ( int ii = 0; ii < nElems[j-1][s-1]; ii++ ) {
              for ( int jj = 0; jj < elemIndex[j-1][s-1]; jj++ ) {
                connElems[j-1][s-1][jj][ii] = connElemTemp[counter] + prevVerts;
                counter++;
              }
            }
            delete[] connElemTemp;
          }
          prevVerts += vertices[j-1];
          
        }
      }
      
      /*--- Close the CGNS file. ---*/
      
      if ( cg_close(fn) ) cg_error_exit();
      cout << "Successfully closed the CGNS file." << endl;
      
      /*--- Write a SU2 mesh if requested in the config file. ---*/
      if (config->GetCGNS_To_SU2()) {
        
        string fileNameSU2 = config->GetMesh_Out_FileName();
        cout << "Writing SU2 mesh file: " << fileNameSU2 << "." << endl;
        
        /*--- Open the solution file for writing. ---*/
        
        FILE *SU2File;
        if ( (SU2File = fopen(fileNameSU2.c_str(), "w")) != NULL ) {
          
          /*--- Write the dimension of the problem and the
					 total number of elements first. Note that we
					 need to use the interior elements here (not "cells"). ---*/
          
          fprintf( SU2File, "NDIME= %i\n", cell_dim);
          fprintf( SU2File, "NELEM= %i\n", interiorElems);
          
          /*--- Connectivity info for the internal domain. ---*/
          
          int counter = 0;
          for ( int k = 0; k < nzones; k++ ) {
            for ( int s = 0; s < nsections; s++ ) {
              if ( isInternal[k][s] ) {
                for ( int i = 0; i < nElems[k][s]; i++ ) {
                  fprintf( SU2File, "%2i\t", elemTypeVTK[k][s]);
                  for ( int j = 0; j < elemIndex[k][s]; j++ ) {
                    fprintf( SU2File, "%8i\t", connElems[k][s][j][i] - 1);
                  }
                  fprintf( SU2File, "%d\n", counter);
                  counter++;
                }
              }
            }
          }
          
          /*--- Now write the node coordinates. First write
					 the total number of vertices. Convert the mesh
           if requesed. ---*/
          if (config->GetKind_SU2() == SU2_CFD) {
            if (config->GetWrite_Converted_Mesh()) {
              Conversion_Factor = config->GetConversion_Factor();
              cout << "Converted mesh by a factor of " << Conversion_Factor << endl; 
            } else {
              Conversion_Factor = 1.0;
            }
          } else {
            Conversion_Factor = 1.0;
          }
          fprintf( SU2File, "NPOIN= %i\n", totalVerts);
          counter = 0;
          for ( int k = 0; k < nzones; k ++ ) {
            for ( int i = 0; i < vertices[k]; i++ ) {
              for ( int j = 0; j < cell_dim; j++ ) {
                fprintf( SU2File, "%.16le\t", Conversion_Factor*gridCoords[k][j][i]);
              }
              fprintf( SU2File, "%d\n", counter);
              counter++;
            }
          }
          
          /*--- Lastly write the boundary information.
					 These will write out in the same order
					 that they are stored in the cgns file.
					 Note that the connectivity
					 values are decremented by 1 in order to
					 match the indexing in SU2.              ---*/
          
          fprintf( SU2File, "NMARK= %i\n", nMarkers );
          counter = 0;
          for ( int k = 0; k < nzones; k ++ ) {
            for ( int s = 0; s < nsections; s++ ) {
              if ( !isInternal[k][s] ) {
                counter++;
                fprintf( SU2File, "MARKER_TAG= %s\n", sectionNames[k][s] );
                fprintf( SU2File, "MARKER_ELEMS= %i\n", nElems[k][s] );
                for ( int i = 0; i < nElems[k][s]; i++ ) {
                  fprintf( SU2File, "%2i\t", elemTypeVTK[k][s]);
                  for ( int j = 0; j < elemIndex[k][s]; j++ ) {
                    fprintf( SU2File, "%8i\t", connElems[k][s][j][i] - 1 );
                  }
                  fprintf( SU2File, "\n");
                }
              }
            }
          }
          
        }
        
        /*--- Close the SU2 mesh file. ---*/
        
        fclose( SU2File );
        cout << "Successfully wrote the SU2 mesh file." << endl;
        
      }
      
      /*--- Load the data from the CGNS file into SU2 memory. ---*/
      
      /*--- Read the dimension of the problem ---*/
      nDim = cell_dim;
      if (rank == MASTER_NODE) {
        if (nDim == 2) cout << "Two dimensional problem." << endl;
        if (nDim == 3) cout << "Three dimensional problem." << endl;
      }
      
      /*--- Read the information about inner elements ---*/
      nElem = interiorElems;
      cout << nElem << " inner elements." << endl;
      
      /*--- Allocate space for elements ---*/
      if (!config->GetDivide_Element()) elem = new CPrimalGrid*[nElem];
      else {
        if (nDim == 2) elem = new CPrimalGrid*[2*nElem];
        if (nDim == 3) {
          elem = new CPrimalGrid*[5*nElem];
          cout << "The grid division only works in 2D!!" << endl;
          cout << "Press any key to exit..." << endl;
          cin.get();
        }
      }
      
      /*--- Loop over all the volumetric elements ---*/
      for ( int k = 0; k < nzones; k ++ ) {
        for ( int s = 0; s < nsections; s++ ) {
          if ( isInternal[k][s] ) {
            for ( int i = 0; i < nElems[k][s]; i++ ) {
              
              /*--- Get the VTK type for this element. ---*/
              VTK_Type = elemTypeVTK[k][s];
              
              /*--- Transfer the nodes for this element. ---*/
              for ( int j = 0; j < elemIndex[k][s]; j++ ) {
                vnodes_cgns[j] = connElems[k][s][j][i] - 1;
              }
              
              /* Instantiate this element. */
              switch(VTK_Type) {
                case TRIANGLE:	
                  elem[ielem] = new CTriangle(vnodes_cgns[0],vnodes_cgns[1],vnodes_cgns[2],nDim);
                  ielem_div++; ielem++; nelem_triangle++; break;
                case RECTANGLE:
                  if (!config->GetDivide_Element()) {
                    elem[ielem] = new CRectangle(vnodes_cgns[0],vnodes_cgns[1],vnodes_cgns[2],vnodes_cgns[3],nDim);
                    ielem++; nelem_quad++; }
                  else {
                    elem[ielem] = new CTriangle(vnodes_cgns[0],vnodes_cgns[1],vnodes_cgns[2],nDim);
                    ielem++; nelem_triangle++;
                    elem[ielem] = new CTriangle(vnodes_cgns[0],vnodes_cgns[2],vnodes_cgns[3],nDim);
                    ielem++; nelem_triangle++; }
                  ielem_div++;
                  break;
                case TETRAHEDRON: 
                  elem[ielem] = new CTetrahedron(vnodes_cgns[0],vnodes_cgns[1],vnodes_cgns[2],vnodes_cgns[3]);
                  ielem_div++; ielem++; nelem_tetra++; break;
                case HEXAHEDRON:
                  if (!config->GetDivide_Element()) {
                    elem[ielem] = new CHexahedron(vnodes_cgns[0],vnodes_cgns[1],vnodes_cgns[2],vnodes_cgns[3],vnodes_cgns[4],vnodes_cgns[5],vnodes_cgns[6],vnodes_cgns[7]);
                    ielem++; nelem_hexa++; }
                  else {
                    elem[ielem] = new CTetrahedron(vnodes_cgns[0],vnodes_cgns[1],vnodes_cgns[2],vnodes_cgns[5]);
                    ielem++; nelem_tetra++;
                    elem[ielem] = new CTetrahedron(vnodes_cgns[0],vnodes_cgns[2],vnodes_cgns[7],vnodes_cgns[5]);
                    ielem++; nelem_tetra++;
                    elem[ielem] = new CTetrahedron(vnodes_cgns[0],vnodes_cgns[2],vnodes_cgns[3],vnodes_cgns[7]);
                    ielem++; nelem_tetra++;
                    elem[ielem] = new CTetrahedron(vnodes_cgns[0],vnodes_cgns[5],vnodes_cgns[7],vnodes_cgns[4]);
                    ielem++; nelem_tetra++;
                    elem[ielem] = new CTetrahedron(vnodes_cgns[2],vnodes_cgns[7],vnodes_cgns[5],vnodes_cgns[6]);
                    ielem++; nelem_tetra++; }
                  ielem_div++;
                  break;
                case WEDGE: 
                  if (!config->GetDivide_Element()) {
                    elem[ielem] = new CWedge(vnodes_cgns[0],vnodes_cgns[1],vnodes_cgns[2],vnodes_cgns[3],vnodes_cgns[4],vnodes_cgns[5]);
                    ielem++; nelem_wedge++; }
                  else {
                    elem[ielem] = new CTetrahedron(vnodes_cgns[0],vnodes_cgns[1],vnodes_cgns[2],vnodes_cgns[5]);
                    ielem++; nelem_tetra++;
                    elem[ielem] = new CTetrahedron(vnodes_cgns[0],vnodes_cgns[1],vnodes_cgns[5],vnodes_cgns[4]);
                    ielem++; nelem_tetra++;
                    elem[ielem] = new CTetrahedron(vnodes_cgns[0],vnodes_cgns[4],vnodes_cgns[5],vnodes_cgns[3]);
                    ielem++; nelem_tetra++; }
                  ielem_div++; 
                  break;
                case PYRAMID: 
                  if (!config->GetDivide_Element()) {
                    elem[ielem] = new CPyramid(vnodes_cgns[0],vnodes_cgns[1],vnodes_cgns[2],vnodes_cgns[3],vnodes_cgns[4]);
                    ielem++; nelem_pyramid++;
                  }
                  else {
                    elem[ielem] = new CTetrahedron(vnodes_cgns[0],vnodes_cgns[1],vnodes_cgns[2],vnodes_cgns[4]);
                    ielem++; nelem_tetra++;
                    elem[ielem] = new CTetrahedron(vnodes_cgns[0],vnodes_cgns[2],vnodes_cgns[3],vnodes_cgns[4]);
                    ielem++; nelem_tetra++; }
                  ielem_div++; 
                  break;	
              }
            }
          }
        }
      }
      nElem_Storage = nelem_triangle*4 + nelem_quad*5 + nelem_tetra*5 + nelem_hexa*9 + nelem_wedge*7 + nelem_pyramid*6;
      if (config->GetDivide_Element()) nElem = nelem_triangle + nelem_quad + nelem_tetra + nelem_hexa + nelem_wedge + nelem_pyramid;
      
      /*--- Retrieve grid conversion factor. The conversion is only
       applied for SU2_CFD. All other SU2 components leave the mesh
       as is. ---*/
      if (config->GetKind_SU2() == SU2_CFD)
        Conversion_Factor = config->GetConversion_Factor();
      else
        Conversion_Factor = 1.0;
      
      /*--- Read node coordinates. Note this assumes serial mode. ---*/
      nPoint = totalVerts;
      nPointDomain = nPoint;
      cout << nPoint << " points." << endl;
      node = new CPoint*[nPoint];
      for ( int k = 0; k < nzones; k++ ) {
        for ( int i = 0; i < vertices[k]; i++ ) {
          for ( int j = 0; j < cell_dim; j++ ) {
            Coord_cgns[j] = gridCoords[k][j][i];
          }
          switch(nDim) {
            case 2:
              GlobalIndex = i;
              node[iPoint] = new CPoint(Conversion_Factor*Coord_cgns[0], Conversion_Factor*Coord_cgns[1], GlobalIndex, config);
              iPoint++; break;
            case 3:
              GlobalIndex = i;
              node[iPoint] = new CPoint(Conversion_Factor*Coord_cgns[0], Conversion_Factor*Coord_cgns[1], Conversion_Factor*Coord_cgns[2], GlobalIndex, config);
              iPoint++; break;
          }
        }
      }
      
      /*--- Read number of markers ---*/
      nMarker = nMarkers;
      cout << nMarker << " surface markers." << endl;
      config->SetnMarker_All(nMarker);
      bound = new CPrimalGrid**[nMarker];
      nElem_Bound = new unsigned long [nMarker];
      nElem_Bound_Storage = new unsigned long [nMarker];
      Tag_to_Marker = new string [MAX_INDEX_VALUE];
      
      iMarker = 0;
      for ( int k = 0; k < nzones; k ++ ) {
        for ( int s = 0; s < nsections; s++ ) {
          if ( !isInternal[k][s] ) {
            nelem_edge = 0; nelem_triangle = 0; nelem_quad = 0; ielem = 0;
            Marker_Tag = sectionNames[k][s];
            if (Marker_Tag != "SEND_RECEIVE") {		
              nElem_Bound[iMarker] = nElems[k][s];
              if (rank == MASTER_NODE) 
                cout << nElem_Bound[iMarker]  << " boundary elements in index "<< iMarker <<" (Marker = " <<Marker_Tag<< ")." << endl;
              bound[iMarker] = new CPrimalGrid* [nElem_Bound[iMarker]];
              
              for ( int i = 0; i < nElems[k][s]; i++ ) {
                
                /* Get the VTK type for this element. */
                VTK_Type = elemTypeVTK[k][s];
                
                /* Transfer the nodes for this element. */
                for ( int j = 0; j < elemIndex[k][s]; j++ ) {
                  vnodes_cgns[j] = connElems[k][s][j][i] - 1;
                }
                switch(VTK_Type) {
                  case LINE:
                    bound[iMarker][ielem] = new CLine(vnodes_cgns[0],vnodes_cgns[1],2);
                    ielem++; nelem_edge++; break;
                  case TRIANGLE:
                    bound[iMarker][ielem] = new CTriangle(vnodes_cgns[0],vnodes_cgns[1],vnodes_cgns[2],3);
                    ielem++; nelem_triangle++; break;
                  case RECTANGLE: 
                    bound[iMarker][ielem] = new CRectangle(vnodes_cgns[0],vnodes_cgns[1],vnodes_cgns[2],vnodes_cgns[3],3);
                    ielem++; nelem_quad++; break;
                }
              }
              nElem_Bound_Storage[iMarker] = nelem_edge*3 + nelem_triangle*4 + nelem_quad*5;
              
              /*--- Update config information storing the boundary information in the right place ---*/
              Tag_to_Marker[config->GetMarker_Config_Tag(Marker_Tag)] = Marker_Tag;
              config->SetMarker_All_Tag(iMarker, Marker_Tag);
              config->SetMarker_All_Boundary(iMarker, config->GetMarker_Config_Boundary(Marker_Tag));
              config->SetMarker_All_Monitoring(iMarker, config->GetMarker_Config_Monitoring(Marker_Tag));
              config->SetMarker_All_Plotting(iMarker, config->GetMarker_Config_Plotting(Marker_Tag));
              config->SetMarker_All_Moving(iMarker, config->GetMarker_Config_Moving(Marker_Tag));
              config->SetMarker_All_SendRecv(iMarker, NONE);
            }
#ifndef NO_MPI
            /*--- Send-Receive boundaries definition - could eventually be used for periodic BC ---*/
            /*else {
             unsigned long nelem_vertex = 0, vnodes_vertex;
             getline (mesh_file,text_line);
             text_line.erase (0,13); nElem_Bound[iMarker] = atoi(text_line.c_str());
             bound[iMarker] = new CPrimalGrid* [nElem_Bound[iMarker]];
             nelem_vertex = 0; ielem = 0;
             getline (mesh_file,text_line); text_line.erase (0,8);
             config->SetMarker_All_Boundary(iMarker, SEND_RECEIVE);
             config->SetMarker_All_SendRecv(iMarker,atoi(text_line.c_str()));
             for (iElem_Bound = 0; iElem_Bound < nElem_Bound[iMarker]; iElem_Bound++) {
             getline(mesh_file,text_line);
             istringstream bound_line(text_line);
             bound_line >> VTK_Type; bound_line >> vnodes_vertex;
             bound[iMarker][ielem] = new CVertexMPI(vnodes_vertex,nDim);
             ielem++; nelem_vertex++;
             if (config->GetMarker_All_SendRecv(iMarker) < 0) node[vnodes_vertex]->SetDomain(false);
             }
             } */
#endif
            iMarker++;
          }
        }
      }
      
      /*--- Deallocate temporary memory. ---*/
      delete[] vertices;
      delete[] cells;
      delete[] boundVerts;
      
      for ( int j = 0; j < nzones; j++) {
        delete[] coordArray[j];
        delete[] elemTypeVTK[j];
        delete[] elemIndex[j];
        delete[] nElems[j];
        delete[] dataSize[j];
        delete[] isInternal[j];
        delete[] sectionNames[j];
      }
      delete[] coordArray;
      delete[] elemTypeVTK;
      delete[] elemIndex;
      delete[] nElems;
      delete[] dataSize;
      delete[] isInternal;
      delete[] sectionNames;
      
      for ( int j = 0; j < nzones; j++) {
        for( int i = 0; i < ncoords; i++ ) {
          delete[] gridCoords[j][i];
        }
        delete[] gridCoords[j];
      }
      delete[] gridCoords;
      
      for ( int kk = 0; kk < nzones; kk++) {
        for (int ii = 0; ii < nsections; ii++) {
          for (int jj = 0; jj < indexMax; jj++) {
            delete[] connElems[kk][ii][jj];
          }
          delete connElems[kk][ii];
        }
        delete connElems[kk];
      }
      delete[] connElems;
      
#else
      cout << "SU2 built without CGNS support!!" << endl;
      cout << "To use CGNS, remove the -DNO_CGNS directive ";
      cout << "from the makefile and supply the correct path";
      cout << " to the CGNS library." << endl;
      cout << "Press any key to exit..." << endl;
      cin.get();
#ifdef NO_MPI
      exit(1);	
#else
      MPI::COMM_WORLD.Abort(1);
      MPI::Finalize();
#endif
      
#endif
      
      break;
		  
		  
	  case NETCDF_ASCII:
		  
			
#ifdef NO_MPI
		  unsigned short Marker_Index, VTK_Type, iMarker, 
		  marker, icommas, iDim;
		  unsigned long vnodes_triangle[3], vnodes_quad[4], vnodes_tetra[4], vnodes_hexa[8], 
		  vnodes_wedge[6], nmarker, ielem_triangle, ielem_hexa, ncoord, iSurfElem, ielem_wedge, 
		  ielem_quad, *marker_list, **surf_elem, ielem_surface, *surf_marker, nSurfElem;
		  double coord;
		  string::size_type position, position_;
		  bool stop, add;
		  
		  ielem_surface = 0; nSurfElem = 0;
		  surf_marker = NULL;
		  surf_elem = NULL;
		  
		  
		  nDim = 3; cout << "Three dimensional problem." << endl;
		  
		  /*--- Open grid file ---*/
		  strcpy (cstr, val_mesh_filename.c_str());
		  mesh_file.open(cstr, ios::in);
		  if (mesh_file.fail()) {
			  cout << "There is no geometry file!!" << endl;
			  cout << "Press any key to exit..." << endl;
			  cin.get();
			  exit(1);	
				
		  }
			
		  while (getline (mesh_file, text_line)) {
			  
			  position = text_line.find ("no_of_elements = ",0);
			  if (position != string::npos) {
				  text_line.erase (0,17); nElem = atoi(text_line.c_str());
				  cout << nElem << " inner elements to store." << endl;
				  elem = new CPrimalGrid*[nElem]; }
			  
			  position = text_line.find ("no_of_surfaceelements = ",0);
			  if (position != string::npos) {
				  text_line.erase (0,24); nSurfElem = atoi(text_line.c_str());
				  cout << nSurfElem << " surface elements to store." << endl; 
				  surf_elem = new unsigned long* [nSurfElem];
				  for (ielem_surface = 0; ielem_surface < nSurfElem; ielem_surface++)
					  surf_elem[ielem_surface] = new unsigned long [5];
				  ielem_surface = 0;
			  }
			  
			  position = text_line.find ("no_of_points = ",0);
			  if (position != string::npos) {
				  text_line.erase (0,15); nPoint = atoi(text_line.c_str()); nPointDomain = nPoint;
				  cout << nPoint << " points to store." << endl;
				  node = new CPoint*[nPoint]; }
			  
			  position = text_line.find ("no_of_tetraeders = ",0);
			  if (position != string::npos) {
				  text_line.erase (0,19); nelem_tetra = atoi(text_line.c_str());
				  cout << nelem_tetra << " tetraeders elements to store." << endl; }
			  
			  position = text_line.find ("no_of_prisms = ",0);
			  if (position != string::npos) {
				  text_line.erase (0,15); nelem_wedge = atoi(text_line.c_str());
				  cout << nelem_wedge << " prims elements to store." << endl; }
			  
			  position = text_line.find ("no_of_hexaeders = ",0);
			  if (position != string::npos) {
				  text_line.erase (0,18); nelem_hexa = atoi(text_line.c_str());
				  cout << nelem_hexa << " hexaeders elements to store." << endl; }
			  
			  position = text_line.find ("no_of_surfacetriangles = ",0);
			  if (position != string::npos) {
				  text_line.erase (0,25); nelem_triangle = atoi(text_line.c_str());
				  cout << nelem_triangle << " surface triangle elements to store." << endl; }
			  
			  position = text_line.find ("no_of_surfacequadrilaterals = ",0);
			  if (position != string::npos) {
				  text_line.erase (0,30); nelem_quad = atoi(text_line.c_str());
				  cout << nelem_quad << " surface quadrilaterals elements to store." << endl; }
			  
			  position = text_line.find ("points_of_tetraeders =",0);
			  if (position != string::npos) {
				  for (unsigned long ielem_tetra = 0; ielem_tetra < nelem_tetra; ielem_tetra++) {
					  getline(mesh_file,text_line);
					  for (unsigned short icommas = 0; icommas < 15; icommas++) {
						  position_ = text_line.find( ",", 0 ); if(position_!=string::npos) text_line.erase (position_,1);
						  position_ = text_line.find( ";", 0 ); if(position_!=string::npos) text_line.erase (position_,1);
					  }
					  istringstream elem_line(text_line);
					  VTK_Type = TETRAHEDRON;
					  elem_line >> vnodes_tetra[0]; elem_line >> vnodes_tetra[1]; elem_line >> vnodes_tetra[2]; elem_line >> vnodes_tetra[3];
					  elem[ielem] = new CTetrahedron(vnodes_tetra[0],vnodes_tetra[1],vnodes_tetra[2],vnodes_tetra[3]);
					  ielem++;
				  }
				  cout << "finish tetrahedron element reading" << endl;			
			  }
			  
			  position = text_line.find ("points_of_prisms =",0);
			  if (position != string::npos) {
				  for (ielem_wedge = 0; ielem_wedge < nelem_wedge; ielem_wedge++) {
					  getline(mesh_file,text_line);
					  for (icommas = 0; icommas < 15; icommas++) {
						  position_ = text_line.find( ",", 0 ); if(position_!=string::npos) text_line.erase (position_,1);
						  position_ = text_line.find( ";", 0 ); if(position_!=string::npos) text_line.erase (position_,1);
					  }
					  istringstream elem_line(text_line);
					  VTK_Type = WEDGE;
					  elem_line >> vnodes_wedge[0]; elem_line >> vnodes_wedge[1]; elem_line >> vnodes_wedge[2]; elem_line >> vnodes_wedge[3]; elem_line >> vnodes_wedge[4]; elem_line >> vnodes_wedge[5];
					  elem[ielem] = new CWedge(vnodes_wedge[0],vnodes_wedge[1],vnodes_wedge[2],vnodes_wedge[3],vnodes_wedge[4],vnodes_wedge[5]);
					  ielem++;
				  }
				  cout << "finish prims element reading" << endl;			
			  }
			  
			  position = text_line.find ("points_of_hexaeders =",0);
			  if (position != string::npos) {
				  for (ielem_hexa = 0; ielem_hexa < nelem_hexa; ielem_hexa++) {
					  getline(mesh_file,text_line);
					  for (icommas = 0; icommas < 15; icommas++) {
						  position_ = text_line.find( ",", 0 ); if(position_!=string::npos) text_line.erase (position_,1);
						  position_ = text_line.find( ";", 0 ); if(position_!=string::npos) text_line.erase (position_,1);
					  }
					  istringstream elem_line(text_line);
					  VTK_Type = HEXAHEDRON;
					  elem_line >> vnodes_hexa[0]; elem_line >> vnodes_hexa[1]; elem_line >> vnodes_hexa[2]; elem_line >> vnodes_hexa[3];
					  elem_line >> vnodes_hexa[4]; elem_line >> vnodes_hexa[5]; elem_line >> vnodes_hexa[6]; elem_line >> vnodes_hexa[7];
					  elem[ielem] = new CHexahedron(vnodes_hexa[0],vnodes_hexa[1],vnodes_hexa[2],vnodes_hexa[3],vnodes_hexa[4],vnodes_hexa[5],vnodes_hexa[6],vnodes_hexa[7]);
					  ielem++;
				  }
				  cout << "finish hexaeders element reading" << endl;
			  }		
			  nElem_Storage = nelem_tetra*5 + nelem_wedge*7 + nelem_hexa*9;
			  
			  position = text_line.find ("points_of_surfacetriangles =",0);
			  if (position != string::npos) {
				  for (ielem_triangle = 0; ielem_triangle < nelem_triangle; ielem_triangle++) {
					  getline(mesh_file,text_line);
					  for (icommas = 0; icommas < 15; icommas++) {
						  position_ = text_line.find( ",", 0 ); if(position_!=string::npos) text_line.erase (position_,1);
						  position_ = text_line.find( ";", 0 ); if(position_!=string::npos) text_line.erase (position_,1);
					  }
					  istringstream elem_line(text_line);
					  elem_line >> vnodes_triangle[0]; elem_line >> vnodes_triangle[1]; elem_line >> vnodes_triangle[2];
					  surf_elem[ielem_surface][0]= 3;
					  surf_elem[ielem_surface][1]= vnodes_triangle[0];
					  surf_elem[ielem_surface][2]= vnodes_triangle[1];
					  surf_elem[ielem_surface][3]= vnodes_triangle[2];
					  ielem_surface++;				
				  }
				  cout << "finish surface triangles element reading" << endl;
			  }
			  
			  position = text_line.find ("points_of_surfacequadrilaterals =",0);
			  if (position != string::npos) {
				  for (ielem_quad = 0; ielem_quad < nelem_quad; ielem_quad++) {
					  getline(mesh_file,text_line);
					  for (icommas = 0; icommas < 15; icommas++) {
						  position_ = text_line.find( ",", 0 ); if(position_!=string::npos) text_line.erase (position_,1);
						  position_ = text_line.find( ";", 0 ); if(position_!=string::npos) text_line.erase (position_,1);
					  }
					  istringstream elem_line(text_line);
					  elem_line >> vnodes_quad[0]; elem_line >> vnodes_quad[1]; elem_line >> vnodes_quad[2]; elem_line >> vnodes_quad[3];
					  surf_elem[ielem_surface][0]= 4;
					  surf_elem[ielem_surface][1]= vnodes_quad[0];
					  surf_elem[ielem_surface][2]= vnodes_quad[1];
					  surf_elem[ielem_surface][3]= vnodes_quad[2];				
					  surf_elem[ielem_surface][4]= vnodes_quad[3];
					  ielem_surface++;				
				  }
				  cout << "finish surface quadrilaterals element reading" << endl;			
			  }
			  
			  position = text_line.find ("boundarymarker_of_surfaces =",0);
			  if (position != string::npos) {
				  nmarker=0;
				  stop = false;
				  surf_marker = new unsigned long [nelem_triangle + nelem_quad];
				  
				  text_line.erase (0,29);
				  for (icommas = 0; icommas < 50; icommas++) {
					  position_ = text_line.find( ",", 0 );
					  if(position_!=string::npos) text_line.erase (position_,1);
				  }
				  
				  stringstream  point_line(text_line);
				  while (point_line >> marker,!point_line.eof()) {
					  surf_marker[nmarker] = marker;
					  nmarker++; }
				  
				  while (!stop) {
					  getline(mesh_file,text_line);
					  for (icommas = 0; icommas < 50; icommas++) {
						  position_ = text_line.find( ",", 0 );
						  if(position_!=string::npos) text_line.erase (position_,1);
						  position_ = text_line.find( ";", 0 );
						  if(position_!=string::npos) text_line.erase (position_,1);
					  }
					  stringstream  point_line(text_line);
					  while (point_line>> marker,!point_line.eof()) {
						  surf_marker[nmarker] = marker;
						  if (nmarker == nSurfElem-1) {stop = true; break;}
						  nmarker++;
					  }
				  }
			  }
			  
			  for (iDim = 0; iDim < nDim; iDim++) {
				  ncoord = 0; stop = false;
				  if (iDim == 0) position = text_line.find ("points_xc = ",0);
				  if (iDim == 1) position = text_line.find ("points_yc = ",0);
				  if (iDim == 2) position = text_line.find ("points_zc = ",0);
				  
				  if (position != string::npos) {
					  text_line.erase (0,12);
					  for (icommas = 0; icommas < 50; icommas++) {
						  position_ = text_line.find( ",", 0 );
						  if(position_!=string::npos) text_line.erase (position_,1);
					  }
					  stringstream  point_line(text_line);
					  while (point_line>> coord,!point_line.eof()) {
						  if (iDim==0) node[ncoord] = new CPoint(coord, 0.0, 0.0, ncoord, config);
						  if (iDim==1) node[ncoord]->SetCoord(1, coord);
						  if (iDim==2) node[ncoord]->SetCoord(2, coord);
						  ncoord++; }
					  while (!stop) {
						  getline(mesh_file,text_line);
						  for (icommas = 0; icommas < 50; icommas++) {
							  position_ = text_line.find( ",", 0 );
							  if(position_!=string::npos) text_line.erase (position_,1);
							  position_ = text_line.find( ";", 0 );
							  if(position_!=string::npos) text_line.erase (position_,1);
						  }
						  stringstream  point_line(text_line);
						  while (point_line>> coord,!point_line.eof()) {
							  if (iDim==0) node[ncoord] = new CPoint(coord, 0.0, 0.0, ncoord, config);
							  if (iDim==1) node[ncoord]->SetCoord(1, coord);
							  if (iDim==2) node[ncoord]->SetCoord(2, coord);
							  if (ncoord == nPoint-1) {stop = true; break;}
							  ncoord++;
						  }
					  }
					  if (iDim==0) cout << "finish point xc reading" << endl;
					  if (iDim==1) cout << "finish point yc reading" << endl;
					  if (iDim==2) cout << "finish point zc reading" << endl;
				  }
			  }
		  }
		  
		  
		  /*--- Create a list with all the markers ---*/
		  marker_list = new unsigned long [MAX_NUMBER_MARKER];
		  marker_list[0] = surf_marker[0]; nMarker = 1;
		  for (iSurfElem = 0; iSurfElem < nSurfElem; iSurfElem++) {
			  add = true;
			  for (iMarker = 0; iMarker < nMarker; iMarker++)
				  if (marker_list[iMarker] == surf_marker[iSurfElem]) {
					  add = false; break; }					
			  if (add) {
				  marker_list[nMarker] = surf_marker[iSurfElem];
				  nMarker++;
			  }
		  }
		  
		  nElem_Bound = new unsigned long [nMarker];
		  nElem_Bound_Storage = new unsigned long [nMarker];
		  
		  /*--- Compute the number of element per marker ---*/
		  for (iMarker = 0; iMarker < nMarker; iMarker++) {
			  nElem_Bound[iMarker] = 0;
			  for (iSurfElem = 0; iSurfElem < nSurfElem; iSurfElem++) 
				  if (surf_marker[iSurfElem] == marker_list[iMarker]) 
					  nElem_Bound[iMarker]++;
		  }
		  
			
		  /*--- Realate the marker index with the position in the array of markers ---*/
		  unsigned short *Index_to_Marker;
		  Index_to_Marker = new unsigned short [MAX_INDEX_VALUE];
		  for (iMarker = 0; iMarker < nMarker; iMarker++) {
			  Marker_Index = marker_list[iMarker];
			  Index_to_Marker[Marker_Index] = iMarker;
		  }
		  
		  bound = new CPrimalGrid**[nMarker];
		  
		  for (iMarker = 0; iMarker < nMarker; iMarker++) {
			  Marker_Index = marker_list[iMarker];
			  bound[iMarker] = new CPrimalGrid* [nElem_Bound[iMarker]];
			  ielem_triangle = 0; ielem_quad = 0;
			  for (iSurfElem = 0; iSurfElem < nSurfElem; iSurfElem++)
				  if (surf_marker[iSurfElem] == Marker_Index) {
					  if (surf_elem[iSurfElem][0] == 3) {
						  vnodes_triangle[0] = surf_elem[iSurfElem][1]; vnodes_triangle[1] = surf_elem[iSurfElem][2]; vnodes_triangle[2] = surf_elem[iSurfElem][3];
						  bound[iMarker][ielem_triangle+ielem_quad] = new CTriangle(vnodes_triangle[0],vnodes_triangle[1],vnodes_triangle[2],3);
						  ielem_triangle ++;
					  }
					  if (surf_elem[iSurfElem][0] == 4) {
						  vnodes_quad[0] = surf_elem[iSurfElem][1]; vnodes_quad[1] = surf_elem[iSurfElem][2]; vnodes_quad[2] = surf_elem[iSurfElem][3]; vnodes_quad[3] = surf_elem[iSurfElem][4];					
						  bound[iMarker][ielem_triangle+ielem_quad] = new CRectangle(vnodes_quad[0],vnodes_quad[1],vnodes_quad[2],vnodes_quad[3],3);
						  ielem_quad ++;
					  }
					  nElem_Bound_Storage[iMarker] = ielem_triangle*4 + ielem_quad*5;			
				  }
		  }
		  
			
			
		  /*--- Update config information storing the boundary information in the right place ---*/
		  for (iMarker = 0; iMarker < nMarker; iMarker++) {
			  
			  stringstream out;
			  out << marker_list[iMarker];
			  Marker_Tag = out.str();
			  
			  Tag_to_Marker = new string [MAX_INDEX_VALUE];
			  Tag_to_Marker[config->GetMarker_Config_Tag(Marker_Tag)] = Marker_Tag;
			  config->SetMarker_All_Tag(iMarker, Marker_Tag);
			  config->SetMarker_All_Boundary(iMarker, config->GetMarker_Config_Boundary(Marker_Tag));
			  config->SetMarker_All_Monitoring(iMarker, config->GetMarker_Config_Monitoring(Marker_Tag));
			  config->SetMarker_All_Plotting(iMarker, config->GetMarker_Config_Plotting(Marker_Tag));
			  config->SetMarker_All_Moving(iMarker, config->GetMarker_Config_Moving(Marker_Tag));
			  config->SetMarker_All_SendRecv(iMarker, NONE);
		  }
		  
#else
      cout << "NETCDF mesh format does not have parallel support!!" << endl;
      cout << "Press any key to exit..." << endl;
      cin.get();
      MPI::COMM_WORLD.Abort(1);
      MPI::Finalize();
#endif
      
		  break;
		  
		  
    default:
      cout << "Unrecognized mesh format specified!!" << endl;
      cout << "Press any key to exit..." << endl;
      cin.get();
#ifdef NO_MPI
      exit(1);	
#else
      MPI::COMM_WORLD.Abort(1);
      MPI::Finalize();
#endif
  }
  
	
	/*--- Loop over the surface element to set the boundaries ---*/
	for (iMarker = 0; iMarker < nMarker; iMarker++)
		for (iElem_Surface = 0; iElem_Surface < nElem_Bound[iMarker]; iElem_Surface++)
			for (iNode_Surface = 0; iNode_Surface < bound[iMarker][iElem_Surface]->GetnNodes(); iNode_Surface++) {				
				Point_Surface = bound[iMarker][iElem_Surface]->GetNode(iNode_Surface);
				node[Point_Surface]->SetBoundary(nMarker);
			}
	
	/*--- Loop over the surface element to set the physical boundaries ---*/
	for (iMarker = 0; iMarker < nMarker; iMarker++)
		if (config->GetMarker_All_Boundary(iMarker) != SEND_RECEIVE &&
				config->GetMarker_All_Boundary(iMarker) != NEARFIELD_BOUNDARY && 
				config->GetMarker_All_Boundary(iMarker) != EULER_WALL &&
				config->GetMarker_All_Boundary(iMarker) != SYMMETRY_PLANE && 
				config->GetMarker_All_Boundary(iMarker) != INTERFACE_BOUNDARY)
			for (iElem_Surface = 0; iElem_Surface < nElem_Bound[iMarker]; iElem_Surface++)
				for (iNode_Surface = 0; iNode_Surface < bound[iMarker][iElem_Surface]->GetnNodes(); iNode_Surface++) {				
					Point_Surface = bound[iMarker][iElem_Surface]->GetNode(iNode_Surface);
					node[Point_Surface]->SetBoundary_Physical(true);
				}
	
	/*--- Write a new copy of the grid in meters if requested ---*/
	if (config->GetKind_SU2() == SU2_CFD)
		if (config->GetWrite_Converted_Mesh()) {
			SetMeshFile(config,config->GetMesh_Out_FileName());
			cout.precision(4);
			cout << "Converted mesh by a factor of " << Conversion_Factor << endl;
			cout << "  and wrote to the output file: " << config->GetMesh_Out_FileName() << endl;
		}
	
//	Inner_file.close();
//	Outer_file.close();
//	exit(0);
}

CPhysicalGeometry::~CPhysicalGeometry(void) {}

void CPhysicalGeometry::SetLockheedGrid(CConfig *config) {
	
#ifdef CHECK_QUAD

	unsigned long iPoint, iElem, iElem_Surface;
	unsigned short iMarker;	
	
	bool *PyramidPoint = new bool [nPoint];
	
	for (iPoint = 0; iPoint < nPoint; iPoint++)
		PyramidPoint[iPoint] = false;

	for (iElem = 0; iElem < nElem; iElem++) {
		if (elem[iElem]->GetVTK_Type() == PYRAMID) {
			PyramidPoint[elem[iElem]->GetNode(0)] = true;
			PyramidPoint[elem[iElem]->GetNode(1)] = true;
			PyramidPoint[elem[iElem]->GetNode(2)] = true;
			PyramidPoint[elem[iElem]->GetNode(3)] = true;
			PyramidPoint[elem[iElem]->GetNode(4)] = true;
		}
	}


	for (iMarker = 0; iMarker < nMarker; iMarker++) {
		for (iElem_Surface = 0; iElem_Surface < nElem_Bound[iMarker]; iElem_Surface++) {
			unsigned short Counter = 0;
			if (bound[iMarker][iElem_Surface]->GetVTK_Type() == RECTANGLE) {
				if (PyramidPoint[bound[iMarker][iElem_Surface]->GetNode(0)]) Counter++;
				if (PyramidPoint[bound[iMarker][iElem_Surface]->GetNode(1)]) Counter++;
				if (PyramidPoint[bound[iMarker][iElem_Surface]->GetNode(2)]) Counter++;
				if (PyramidPoint[bound[iMarker][iElem_Surface]->GetNode(3)]) Counter++;
			}
			if (Counter == 4) {
				cout << iElem_Surface <<" "<< Counter <<"---->  "<< 
				bound[iMarker][iElem_Surface]->GetNode(0) <<" "<<
				bound[iMarker][iElem_Surface]->GetNode(1) <<" "<<
				bound[iMarker][iElem_Surface]->GetNode(2) <<" "<<
				bound[iMarker][iElem_Surface]->GetNode(3) <<" "<< endl;
			}
		}
	}
	
	
	for (iMarker = 0; iMarker < nMarker; iMarker++) {
		for (iElem_Surface = 0; iElem_Surface < nElem_Bound[iMarker]; iElem_Surface++) {
			unsigned short Counter = 0;
			if (bound[iMarker][iElem_Surface]->GetVTK_Type() == RECTANGLE) {
				if (fabs(node[bound[iMarker][iElem_Surface]->GetNode(0)]->GetCoord(1)) < 1E-6) Counter++;
				if (fabs(node[bound[iMarker][iElem_Surface]->GetNode(1)]->GetCoord(1)) < 1E-6) Counter++;
				if (fabs(node[bound[iMarker][iElem_Surface]->GetNode(2)]->GetCoord(1)) < 1E-6) Counter++;
				if (fabs(node[bound[iMarker][iElem_Surface]->GetNode(3)]->GetCoord(1)) < 1E-6) Counter++;
			}
			if (Counter == 4) {
				cout << iElem_Surface <<" "<< Counter <<"---->  "<< 
				bound[iMarker][iElem_Surface]->GetNode(0) <<" "<<
				bound[iMarker][iElem_Surface]->GetNode(1) <<" "<<
				bound[iMarker][iElem_Surface]->GetNode(2) <<" "<<
				bound[iMarker][iElem_Surface]->GetNode(3) <<" "<< endl;
			}
		}
	}
	
	for (iMarker = 0; iMarker < nMarker; iMarker++) {
		for (iElem_Surface = 0; iElem_Surface < nElem_Bound[iMarker]; iElem_Surface++) {
			unsigned short Counter = 0;
			if (bound[iMarker][iElem_Surface]->GetVTK_Type() == TRIANGLE) {
				if (node[bound[iMarker][iElem_Surface]->GetNode(0)]->GetCoord(0) < -24.999) Counter++;
				if (node[bound[iMarker][iElem_Surface]->GetNode(1)]->GetCoord(0) < -24.999) Counter++;
				if (node[bound[iMarker][iElem_Surface]->GetNode(2)]->GetCoord(0) < -24.999) Counter++;
				
				if (node[bound[iMarker][iElem_Surface]->GetNode(0)]->GetCoord(0) > 349.9) Counter++;
				if (node[bound[iMarker][iElem_Surface]->GetNode(1)]->GetCoord(0) > 349.9) Counter++;
				if (node[bound[iMarker][iElem_Surface]->GetNode(2)]->GetCoord(0) > 349.9) Counter++;
			}
			if (Counter == 3) {
				cout << iElem_Surface <<" "<< Counter <<"---->  "<< 
				bound[iMarker][iElem_Surface]->GetNode(0) <<" "<<
				bound[iMarker][iElem_Surface]->GetNode(1) <<" "<<
				bound[iMarker][iElem_Surface]->GetNode(2) <<" "<< endl;
			}
		}
	}
	
	for (iMarker = 0; iMarker < nMarker; iMarker++) {
		for (iElem_Surface = 0; iElem_Surface < nElem_Bound[iMarker]; iElem_Surface++) {
			unsigned short Counter = 0;
			if (bound[iMarker][iElem_Surface]->GetVTK_Type() == TRIANGLE) {
				if ( fabs(node[bound[iMarker][iElem_Surface]->GetNode(0)]->GetCoord(1)) < 1E-6) Counter++;
				if ( fabs(node[bound[iMarker][iElem_Surface]->GetNode(1)]->GetCoord(1)) < 1E-6) Counter++;
				if ( fabs(node[bound[iMarker][iElem_Surface]->GetNode(2)]->GetCoord(1)) < 1E-6) Counter++;
				
			}
			if (Counter == 3) {
				cout << iElem_Surface <<" "<< Counter <<"---->  "<< 
				bound[iMarker][iElem_Surface]->GetNode(0) <<" "<<
				bound[iMarker][iElem_Surface]->GetNode(1) <<" "<<
				bound[iMarker][iElem_Surface]->GetNode(2) <<" "<< endl;
			}
		}
	}
	
	bool *PyramidPoint = new bool [nPoint];
	
	for (iPoint = 0; iPoint < nPoint; iPoint++)
		PyramidPoint[iPoint] = false;
	
	for (iElem = 0; iElem < nElem; iElem++) {
		if (elem[iElem]->GetVTK_Type() == PYRAMID) {
			PyramidPoint[elem[iElem]->GetNode(0)] = true;
			PyramidPoint[elem[iElem]->GetNode(1)] = true;
			PyramidPoint[elem[iElem]->GetNode(2)] = true;
			PyramidPoint[elem[iElem]->GetNode(3)] = true;
			PyramidPoint[elem[iElem]->GetNode(4)] = true;
		}
	}

	
	for (iMarker = 0; iMarker < nMarker; iMarker++) {
		for (iElem_Surface = 0; iElem_Surface < nElem_Bound[iMarker]; iElem_Surface++) {
			unsigned short Counter = 0;
			if (bound[iMarker][iElem_Surface]->GetVTK_Type() == TRIANGLE) {
				if ((node[bound[iMarker][iElem_Surface]->GetNode(0)]->GetCoord(0) < -24.999) &&
					(!PyramidPoint[bound[iMarker][iElem_Surface]->GetNode(0)]))
					Counter++;
				if ((node[bound[iMarker][iElem_Surface]->GetNode(1)]->GetCoord(0) < -24.999) &&
					(!PyramidPoint[bound[iMarker][iElem_Surface]->GetNode(1)])) Counter++;
				if ((node[bound[iMarker][iElem_Surface]->GetNode(2)]->GetCoord(0) < -24.999) &&
					(!PyramidPoint[bound[iMarker][iElem_Surface]->GetNode(2)])) Counter++;
				
				if ((node[bound[iMarker][iElem_Surface]->GetNode(0)]->GetCoord(0) > 349.9) &&
					(!PyramidPoint[bound[iMarker][iElem_Surface]->GetNode(0)])) Counter++;
				if ((node[bound[iMarker][iElem_Surface]->GetNode(1)]->GetCoord(0) > 349.9) &&
					(!PyramidPoint[bound[iMarker][iElem_Surface]->GetNode(1)])) Counter++;
				if ((node[bound[iMarker][iElem_Surface]->GetNode(2)]->GetCoord(0) > 349.9) &&
					(!PyramidPoint[bound[iMarker][iElem_Surface]->GetNode(2)])) Counter++;
			}
			if (Counter == 3) {
				cout << iElem_Surface <<" "<< Counter <<"---->  "<< 
				bound[iMarker][iElem_Surface]->GetNode(0) <<" "<<
				bound[iMarker][iElem_Surface]->GetNode(1) <<" "<<
				bound[iMarker][iElem_Surface]->GetNode(2) <<" "<< node[bound[iMarker][iElem_Surface]->GetNode(0)]->GetCoord(0) << endl;
			}
		}
	}
					
	for (iMarker = 0; iMarker < nMarker; iMarker++) 
	if (config->GetMarker_All_Boundary(iMarker) == SYMMETRY_PLANE) {
		for (iElem_Surface = 0; iElem_Surface < nElem_Bound[iMarker]; iElem_Surface++) {
			unsigned short Counter = 0;
			if (bound[iMarker][iElem_Surface]->GetVTK_Type() == TRIANGLE) {
				if ( fabs(node[bound[iMarker][iElem_Surface]->GetNode(0)]->GetCoord(1)) < 1E-6) Counter++;
				if ( fabs(node[bound[iMarker][iElem_Surface]->GetNode(1)]->GetCoord(1)) < 1E-6) Counter++;
				if ( fabs(node[bound[iMarker][iElem_Surface]->GetNode(2)]->GetCoord(1)) < 1E-6) Counter++;
				
			}
			if ((Counter != 3)&& (Counter != 0)) {
				cout << iElem_Surface <<" "<< Counter <<"---->  "<< 
				bound[iMarker][iElem_Surface]->GetNode(0) <<" "<<
				bound[iMarker][iElem_Surface]->GetNode(1) <<" "<<
				bound[iMarker][iElem_Surface]->GetNode(2) <<" "<< endl;
			}
		}
	}
#endif

}

void CPhysicalGeometry::Check_Orientation(CConfig *config) {
	unsigned long Point_1, Point_2, Point_3, Point_4, Point_5, Point_6, 
				  iElem, Point_1_Surface, Point_2_Surface, Point_3_Surface, Point_4_Surface, 
				  iElem_Domain, Point_Domain = 0, Point_Surface, iElem_Surface;
	double test_1, test_2, test_3, test_4, *Coord_1, *Coord_2, *Coord_3, *Coord_4, 
		   *Coord_5, *Coord_6, a[3], b[3], c[3], n[3], test;
	unsigned short iDim, iMarker, iNode_Domain, iNode_Surface;
	bool find;
	
	/*--- Loop over all the elements ---*/
	for (iElem = 0; iElem < nElem; iElem++) {

		/*--- 2D grid, triangle case ---*/
		if (elem[iElem]->GetVTK_Type() == TRIANGLE) {
			
			Point_1 = elem[iElem]->GetNode(0); Coord_1 = node[Point_1]->GetCoord();
			Point_2 = elem[iElem]->GetNode(1); Coord_2 = node[Point_2]->GetCoord();
			Point_3 = elem[iElem]->GetNode(2); Coord_3 = node[Point_3]->GetCoord();

			for(iDim = 0; iDim < nDim; iDim++) {
				a[iDim] = 0.5*(Coord_2[iDim]-Coord_1[iDim]);
				b[iDim] = 0.5*(Coord_3[iDim]-Coord_1[iDim]); }
			test = a[0]*b[1]-b[0]*a[1];
			
			if (test < 0.0) elem[iElem]->Change_Orientation();	
		}

		/*--- 2D grid, rectangle case ---*/
		if (elem[iElem]->GetVTK_Type() == RECTANGLE) {
		
			Point_1 = elem[iElem]->GetNode(0); Coord_1 = node[Point_1]->GetCoord();
			Point_2 = elem[iElem]->GetNode(1); Coord_2 = node[Point_2]->GetCoord();
			Point_3 = elem[iElem]->GetNode(2); Coord_3 = node[Point_3]->GetCoord();
			Point_4 = elem[iElem]->GetNode(3); Coord_4 = node[Point_4]->GetCoord();

			for(iDim = 0; iDim < nDim; iDim++) {
				a[iDim] = 0.5*(Coord_2[iDim]-Coord_1[iDim]);
				b[iDim] = 0.5*(Coord_3[iDim]-Coord_1[iDim]); }
			test_1 = a[0]*b[1]-b[0]*a[1];

			for(iDim = 0; iDim < nDim; iDim++) {
				a[iDim] = 0.5*(Coord_3[iDim]-Coord_2[iDim]);
				b[iDim] = 0.5*(Coord_4[iDim]-Coord_2[iDim]); }
			test_2 = a[0]*b[1]-b[0]*a[1];

			for(iDim = 0; iDim < nDim; iDim++) {
				a[iDim] = 0.5*(Coord_4[iDim]-Coord_3[iDim]);
				b[iDim] = 0.5*(Coord_1[iDim]-Coord_3[iDim]); }
			test_3 = a[0]*b[1]-b[0]*a[1];

			for(iDim = 0; iDim < nDim; iDim++) {
				a[iDim] = 0.5*(Coord_1[iDim]-Coord_4[iDim]);
				b[iDim] = 0.5*(Coord_3[iDim]-Coord_4[iDim]); }
			test_4 = a[0]*b[1]-b[0]*a[1];

			if ((test_1 < 0.0) && (test_2 < 0.0) && (test_3 < 0.0) && (test_4 < 0.0))
				elem[iElem]->Change_Orientation();		
		}
		
		/*--- 3D grid, tetrahedron case ---*/
		if (elem[iElem]->GetVTK_Type() == TETRAHEDRON) {
		
			Point_1 = elem[iElem]->GetNode(0); Coord_1 = node[Point_1]->GetCoord();
			Point_2 = elem[iElem]->GetNode(1); Coord_2 = node[Point_2]->GetCoord();
			Point_3 = elem[iElem]->GetNode(2); Coord_3 = node[Point_3]->GetCoord();
			Point_4 = elem[iElem]->GetNode(3); Coord_4 = node[Point_4]->GetCoord();

			for(iDim = 0; iDim < nDim; iDim++) {
				a[iDim] = 0.5*(Coord_2[iDim]-Coord_1[iDim]);
				b[iDim] = 0.5*(Coord_3[iDim]-Coord_1[iDim]);
				c[iDim] = Coord_4[iDim]-Coord_1[iDim]; }
			n[0] = a[1]*b[2]-b[1]*a[2];
			n[1] = -(a[0]*b[2]-b[0]*a[2]);
			n[2] = a[0]*b[1]-b[0]*a[1];
			
			test = n[0]*c[0]+n[1]*c[1]+n[2]*c[2];
			if (test < 0.0) elem[iElem]->Change_Orientation();
		}

		/*--- 3D grid, wedge case ---*/
		if (elem[iElem]->GetVTK_Type() == WEDGE) {
			
			Point_1 = elem[iElem]->GetNode(0); Coord_1 = node[Point_1]->GetCoord();
			Point_2 = elem[iElem]->GetNode(1); Coord_2 = node[Point_2]->GetCoord();
			Point_3 = elem[iElem]->GetNode(2); Coord_3 = node[Point_3]->GetCoord();
			Point_4 = elem[iElem]->GetNode(3); Coord_4 = node[Point_4]->GetCoord();
			Point_5 = elem[iElem]->GetNode(4); Coord_5 = node[Point_5]->GetCoord();
			Point_6 = elem[iElem]->GetNode(5); Coord_6 = node[Point_6]->GetCoord();

			for(iDim = 0; iDim < nDim; iDim++) {
				a[iDim] = 0.5*(Coord_3[iDim]-Coord_1[iDim]);
				b[iDim] = 0.5*(Coord_2[iDim]-Coord_1[iDim]);
				c[iDim] = (Coord_4[iDim]-Coord_1[iDim])+
							(Coord_5[iDim]-Coord_2[iDim])+
							(Coord_6[iDim]-Coord_3[iDim]); }

			/*--- The normal vector should point to the interior of the element ---*/
			n[0] = a[1]*b[2]-b[1]*a[2];
			n[1] = -(a[0]*b[2]-b[0]*a[2]);
			n[2] = a[0]*b[1]-b[0]*a[1];
			
			test_1 = n[0]*c[0]+n[1]*c[1]+n[2]*c[2];
			
			for(iDim = 0; iDim < nDim; iDim++) {
				a[iDim] = 0.5*(Coord_5[iDim]-Coord_4[iDim]);
				b[iDim] = 0.5*(Coord_6[iDim]-Coord_4[iDim]);
				c[iDim] = (Coord_1[iDim]-Coord_4[iDim])+
				(Coord_2[iDim]-Coord_5[iDim])+
				(Coord_3[iDim]-Coord_6[iDim]); }
			
			/*--- The normal vector should point to the interior of the element ---*/
			n[0] = a[1]*b[2]-b[1]*a[2];
			n[1] = -(a[0]*b[2]-b[0]*a[2]);
			n[2] = a[0]*b[1]-b[0]*a[1];
			
			test_2 = n[0]*c[0]+n[1]*c[1]+n[2]*c[2];
			
			if ((test_1 < 0.0) || (test_2 < 0.0))
				elem[iElem]->Change_Orientation();

		}

		if (elem[iElem]->GetVTK_Type() == HEXAHEDRON) {
			
			Point_1 = elem[iElem]->GetNode(0); Coord_1 = node[Point_1]->GetCoord();
			Point_2 = elem[iElem]->GetNode(1); Coord_2 = node[Point_2]->GetCoord();
			Point_3 = elem[iElem]->GetNode(2); Coord_3 = node[Point_3]->GetCoord();
			Point_4 = elem[iElem]->GetNode(5); Coord_4 = node[Point_4]->GetCoord();
			
			for(iDim = 0; iDim < nDim; iDim++) {
				a[iDim] = 0.5*(Coord_2[iDim]-Coord_1[iDim]);
				b[iDim] = 0.5*(Coord_3[iDim]-Coord_1[iDim]);
				c[iDim] = Coord_4[iDim]-Coord_1[iDim]; }
			n[0] = a[1]*b[2]-b[1]*a[2];
			n[1] = -(a[0]*b[2]-b[0]*a[2]);
			n[2] = a[0]*b[1]-b[0]*a[1];
			
			test_1 = n[0]*c[0]+n[1]*c[1]+n[2]*c[2];
			
			Point_1 = elem[iElem]->GetNode(2); Coord_1 = node[Point_1]->GetCoord();
			Point_2 = elem[iElem]->GetNode(3); Coord_2 = node[Point_2]->GetCoord();
			Point_3 = elem[iElem]->GetNode(0); Coord_3 = node[Point_3]->GetCoord();
			Point_4 = elem[iElem]->GetNode(7); Coord_4 = node[Point_4]->GetCoord();
			
			for(iDim = 0; iDim < nDim; iDim++) {
				a[iDim] = 0.5*(Coord_2[iDim]-Coord_1[iDim]);
				b[iDim] = 0.5*(Coord_3[iDim]-Coord_1[iDim]);
				c[iDim] = Coord_4[iDim]-Coord_1[iDim]; }
			n[0] = a[1]*b[2]-b[1]*a[2];
			n[1] = -(a[0]*b[2]-b[0]*a[2]);
			n[2] = a[0]*b[1]-b[0]*a[1];
			
			test_2 = n[0]*c[0]+n[1]*c[1]+n[2]*c[2];
			
			Point_1 = elem[iElem]->GetNode(1); Coord_1 = node[Point_1]->GetCoord();
			Point_2 = elem[iElem]->GetNode(2); Coord_2 = node[Point_2]->GetCoord();
			Point_3 = elem[iElem]->GetNode(3); Coord_3 = node[Point_3]->GetCoord();
			Point_4 = elem[iElem]->GetNode(6); Coord_4 = node[Point_4]->GetCoord();
			
			for(iDim = 0; iDim < nDim; iDim++) {
				a[iDim] = 0.5*(Coord_2[iDim]-Coord_1[iDim]);
				b[iDim] = 0.5*(Coord_3[iDim]-Coord_1[iDim]);
				c[iDim] = Coord_4[iDim]-Coord_1[iDim]; }
			n[0] = a[1]*b[2]-b[1]*a[2];
			n[1] = -(a[0]*b[2]-b[0]*a[2]);
			n[2] = a[0]*b[1]-b[0]*a[1];
			
			test_3 = n[0]*c[0]+n[1]*c[1]+n[2]*c[2];
			
			Point_1 = elem[iElem]->GetNode(3); Coord_1 = node[Point_1]->GetCoord();
			Point_2 = elem[iElem]->GetNode(0); Coord_2 = node[Point_2]->GetCoord();
			Point_3 = elem[iElem]->GetNode(1); Coord_3 = node[Point_3]->GetCoord();
			Point_4 = elem[iElem]->GetNode(4); Coord_4 = node[Point_4]->GetCoord();
			
			for(iDim = 0; iDim < nDim; iDim++) {
				a[iDim] = 0.5*(Coord_2[iDim]-Coord_1[iDim]);
				b[iDim] = 0.5*(Coord_3[iDim]-Coord_1[iDim]);
				c[iDim] = Coord_4[iDim]-Coord_1[iDim]; }
			n[0] = a[1]*b[2]-b[1]*a[2];
			n[1] = -(a[0]*b[2]-b[0]*a[2]);
			n[2] = a[0]*b[1]-b[0]*a[1];
			
			test_4 = n[0]*c[0]+n[1]*c[1]+n[2]*c[2];
			
			if ((test_1 < 0.0) || (test_2 < 0.0) || (test_3 < 0.0) 
					|| (test_4 < 0.0)) elem[iElem]->Change_Orientation();
			
		}
		
		if (elem[iElem]->GetVTK_Type() == PYRAMID) {
			
			Point_1 = elem[iElem]->GetNode(0); Coord_1 = node[Point_1]->GetCoord();
			Point_2 = elem[iElem]->GetNode(1); Coord_2 = node[Point_2]->GetCoord();
			Point_3 = elem[iElem]->GetNode(2); Coord_3 = node[Point_3]->GetCoord();
			Point_4 = elem[iElem]->GetNode(4); Coord_4 = node[Point_4]->GetCoord();
			
			for(iDim = 0; iDim < nDim; iDim++) {
				a[iDim] = 0.5*(Coord_2[iDim]-Coord_1[iDim]);
				b[iDim] = 0.5*(Coord_3[iDim]-Coord_1[iDim]);
				c[iDim] = Coord_4[iDim]-Coord_1[iDim]; }
			n[0] = a[1]*b[2]-b[1]*a[2];
			n[1] = -(a[0]*b[2]-b[0]*a[2]);
			n[2] = a[0]*b[1]-b[0]*a[1];
			
			test_1 = n[0]*c[0]+n[1]*c[1]+n[2]*c[2];
			
			Point_1 = elem[iElem]->GetNode(2); Coord_1 = node[Point_1]->GetCoord();
			Point_2 = elem[iElem]->GetNode(3); Coord_2 = node[Point_2]->GetCoord();
			Point_3 = elem[iElem]->GetNode(0); Coord_3 = node[Point_3]->GetCoord();
			Point_4 = elem[iElem]->GetNode(4); Coord_4 = node[Point_4]->GetCoord();
			
			for(iDim = 0; iDim < nDim; iDim++) {
				a[iDim] = 0.5*(Coord_2[iDim]-Coord_1[iDim]);
				b[iDim] = 0.5*(Coord_3[iDim]-Coord_1[iDim]);
				c[iDim] = Coord_4[iDim]-Coord_1[iDim]; }
			n[0] = a[1]*b[2]-b[1]*a[2];
			n[1] = -(a[0]*b[2]-b[0]*a[2]);
			n[2] = a[0]*b[1]-b[0]*a[1];
			
			test_2 = n[0]*c[0]+n[1]*c[1]+n[2]*c[2];
			
			if ((test_1 < 0.0) || (test_2 < 0.0)) 
				elem[iElem]->Change_Orientation();
			
		}
		
	}	

	/*--- Surface elements ---*/
	for (iMarker = 0; iMarker < nMarker; iMarker++)
		for (iElem_Surface = 0; iElem_Surface < nElem_Bound[iMarker]; iElem_Surface++) {
			
			iElem_Domain = bound[iMarker][iElem_Surface]->GetDomainElement();
			for (iNode_Domain = 0; iNode_Domain < elem[iElem_Domain]->GetnNodes(); iNode_Domain++) {
				Point_Domain = elem[iElem_Domain]->GetNode(iNode_Domain);
				find = false;				
				for (iNode_Surface = 0; iNode_Surface < bound[iMarker][iElem_Surface]->GetnNodes(); iNode_Surface++) {				
					Point_Surface = bound[iMarker][iElem_Surface]->GetNode(iNode_Surface);
					if (Point_Surface == Point_Domain) {find = true; break;}
				}
				if (!find) break;
			}
			
			/*--- 2D grid, line case ---*/
			if (bound[iMarker][iElem_Surface]->GetVTK_Type() == LINE) {
				
				Point_1_Surface = bound[iMarker][iElem_Surface]->GetNode(0); Coord_1 = node[Point_1_Surface]->GetCoord();
				Point_2_Surface = bound[iMarker][iElem_Surface]->GetNode(1); Coord_2 = node[Point_2_Surface]->GetCoord();
				Coord_3 = node[Point_Domain]->GetCoord();
				
				for(iDim = 0; iDim < nDim; iDim++) {
					a[iDim] = 0.5*(Coord_2[iDim]-Coord_1[iDim]);
					b[iDim] = 0.5*(Coord_3[iDim]-Coord_1[iDim]); }
				test = a[0]*b[1]-b[0]*a[1];

				if (test < 0.0) bound[iMarker][iElem_Surface]->Change_Orientation();
			}
			
			/*--- 3D grid, triangle case ---*/
			if (bound[iMarker][iElem_Surface]->GetVTK_Type() == TRIANGLE) {
				
				Point_1_Surface = bound[iMarker][iElem_Surface]->GetNode(0); Coord_1 = node[Point_1_Surface]->GetCoord();
				Point_2_Surface = bound[iMarker][iElem_Surface]->GetNode(1); Coord_2 = node[Point_2_Surface]->GetCoord();
				Point_3_Surface = bound[iMarker][iElem_Surface]->GetNode(2); Coord_3 = node[Point_3_Surface]->GetCoord();
				Coord_4 = node[Point_Domain]->GetCoord();
				
				for(iDim = 0; iDim < nDim; iDim++) {
					a[iDim] = 0.5*(Coord_2[iDim]-Coord_1[iDim]);
					b[iDim] = 0.5*(Coord_3[iDim]-Coord_1[iDim]);
					c[iDim] = Coord_4[iDim]-Coord_1[iDim]; }
				n[0] = a[1]*b[2]-b[1]*a[2];
				n[1] = -(a[0]*b[2]-b[0]*a[2]);
				n[2] = a[0]*b[1]-b[0]*a[1];
			
				test = n[0]*c[0]+n[1]*c[1]+n[2]*c[2];
				if (test < 0.0) bound[iMarker][iElem_Surface]->Change_Orientation();
			}

			if (bound[iMarker][iElem_Surface]->GetVTK_Type() == RECTANGLE) {
				
				Point_1_Surface = bound[iMarker][iElem_Surface]->GetNode(0); Coord_1 = node[Point_1_Surface]->GetCoord();
				Point_2_Surface = bound[iMarker][iElem_Surface]->GetNode(1); Coord_2 = node[Point_2_Surface]->GetCoord();
				Point_3_Surface = bound[iMarker][iElem_Surface]->GetNode(2); Coord_3 = node[Point_3_Surface]->GetCoord();
				Point_4_Surface = bound[iMarker][iElem_Surface]->GetNode(3); Coord_4 = node[Point_4_Surface]->GetCoord();
				Coord_5 = node[Point_Domain]->GetCoord();
				
				for(iDim = 0; iDim < nDim; iDim++) {
					a[iDim] = 0.5*(Coord_2[iDim]-Coord_1[iDim]);
					b[iDim] = 0.5*(Coord_3[iDim]-Coord_1[iDim]);
					c[iDim] = Coord_5[iDim]-Coord_1[iDim]; }
				n[0] = a[1]*b[2]-b[1]*a[2];
				n[1] = -(a[0]*b[2]-b[0]*a[2]);
				n[2] = a[0]*b[1]-b[0]*a[1];
				test_1 = n[0]*c[0]+n[1]*c[1]+n[2]*c[2];
				
				for(iDim = 0; iDim < nDim; iDim++) {
					a[iDim] = 0.5*(Coord_3[iDim]-Coord_2[iDim]);
					b[iDim] = 0.5*(Coord_4[iDim]-Coord_2[iDim]);
					c[iDim] = Coord_5[iDim]-Coord_2[iDim]; }
				n[0] = a[1]*b[2]-b[1]*a[2];
				n[1] = -(a[0]*b[2]-b[0]*a[2]);
				n[2] = a[0]*b[1]-b[0]*a[1];
				test_2 = n[0]*c[0]+n[1]*c[1]+n[2]*c[2];
				
				for(iDim = 0; iDim < nDim; iDim++) {
					a[iDim] = 0.5*(Coord_4[iDim]-Coord_3[iDim]);
					b[iDim] = 0.5*(Coord_1[iDim]-Coord_3[iDim]);
					c[iDim] = Coord_5[iDim]-Coord_3[iDim]; }
				n[0] = a[1]*b[2]-b[1]*a[2];
				n[1] = -(a[0]*b[2]-b[0]*a[2]);
				n[2] = a[0]*b[1]-b[0]*a[1];
				test_3 = n[0]*c[0]+n[1]*c[1]+n[2]*c[2];
				
				for(iDim = 0; iDim < nDim; iDim++) {
					a[iDim] = 0.5*(Coord_1[iDim]-Coord_4[iDim]);
					b[iDim] = 0.5*(Coord_3[iDim]-Coord_4[iDim]);
					c[iDim] = Coord_5[iDim]-Coord_4[iDim]; }
				n[0] = a[1]*b[2]-b[1]*a[2];
				n[1] = -(a[0]*b[2]-b[0]*a[2]);
				n[2] = a[0]*b[1]-b[0]*a[1];
				test_4 = n[0]*c[0]+n[1]*c[1]+n[2]*c[2];
				
				if ((test_1 < 0.0) && (test_2 < 0.0) && (test_3 < 0.0) && (test_4 < 0.0))
					bound[iMarker][iElem_Surface]->Change_Orientation();
			}
		}
}

void CPhysicalGeometry::SetEsuP(void) {
	unsigned long iPoint, iElem;
	unsigned short iNode;

	/*--- Loop over all the elements ---*/
	for(iElem = 0; iElem < nElem; iElem++)
		/*--- Loop over all the nodes of an element ---*/
		for(iNode = 0; iNode < elem[iElem]->GetnNodes(); iNode++) {  
			iPoint = elem[iElem]->GetNode(iNode);
			/*--- Store the element into the point ---*/
			node[iPoint]->SetElem(iElem);
		}
}

void CPhysicalGeometry::SetWall_Distance(CConfig *config) {
	double *coord, dist2, dist;
	unsigned short iDim, iMarker;
	unsigned long iPoint, iVertex, nVertex_NS;
	
#ifdef NO_MPI

	/*--- identification of the wall points and coordinates ---*/
	nVertex_NS = 0;
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if ((config->GetMarker_All_Boundary(iMarker) == NO_SLIP_WALL) || 
			(config->GetMarker_All_Boundary(iMarker) == EULER_WALL))
			nVertex_NS += GetnVertex(iMarker);
	
	/*--- Allocate vector of boundary coordinates ---*/
	double **Coord_bound;
	Coord_bound = new double* [nVertex_NS];
	for (iVertex = 0; iVertex < nVertex_NS; iVertex++)
		Coord_bound[iVertex] = new double [nDim];
	
	/*--- Get coordinates of the points of the surface ---*/
	nVertex_NS = 0;
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if ((config->GetMarker_All_Boundary(iMarker) == NO_SLIP_WALL) || 
			(config->GetMarker_All_Boundary(iMarker) == EULER_WALL))
			for (iVertex = 0; iVertex < GetnVertex(iMarker); iVertex++) {
				iPoint = vertex[iMarker][iVertex]->GetNode();
				for (iDim = 0; iDim < nDim; iDim++)
					Coord_bound[nVertex_NS][iDim] = node[iPoint]->GetCoord(iDim);
				nVertex_NS++;
			}
	
	/*--- Get coordinates of the points and compute distances to the surface ---*/
	for (iPoint = 0; iPoint < GetnPoint(); iPoint++) {
			coord = node[iPoint]->GetCoord();
			/*--- Compute the squared distance to the rest of points, and get the minimum ---*/
			dist = 1E20;
			for (iVertex = 0; iVertex < nVertex_NS; iVertex++) {
				dist2 = 0.0;
				for (iDim = 0; iDim < nDim; iDim++)
					dist2 += (coord[iDim]-Coord_bound[iVertex][iDim])*(coord[iDim]-Coord_bound[iVertex][iDim]);
				if (dist2 < dist) dist = dist2;
			}
			node[iPoint]->SetWallDistance(sqrt(dist));
		}
	
	/*--- Deallocate vector of boundary coordinates ---*/
	for (iVertex = 0; iVertex < nVertex_NS; iVertex++)
		delete [] Coord_bound[iVertex];
	delete [] Coord_bound;
	
	cout << "Wall distance computation." << endl;
	
#else 
	int iProcessor;
	
	/*--- Count the number of wall nodes in the whole mesh ---*/
	unsigned long nLocalVertex_NS = 0, nGlobalVertex_NS = 0, MaxLocalVertex_NS = 0;
	
	int nProcessor = MPI::COMM_WORLD.Get_size();
	
	unsigned long *Buffer_Send_nVertex = new unsigned long [1];
	unsigned long *Buffer_Receive_nVertex = new unsigned long [nProcessor];

	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if ((config->GetMarker_All_Boundary(iMarker) == NO_SLIP_WALL) || 
				(config->GetMarker_All_Boundary(iMarker) == EULER_WALL))
			nLocalVertex_NS += GetnVertex(iMarker);
	
	Buffer_Send_nVertex[0] = nLocalVertex_NS;	
	
	MPI::COMM_WORLD.Allreduce(&nLocalVertex_NS, &nGlobalVertex_NS, 1, MPI::UNSIGNED_LONG, MPI::SUM); 	
	MPI::COMM_WORLD.Allreduce(&nLocalVertex_NS, &MaxLocalVertex_NS, 1, MPI::UNSIGNED_LONG, MPI::MAX); 	
	MPI::COMM_WORLD.Allgather(Buffer_Send_nVertex, 1, MPI::UNSIGNED_LONG, Buffer_Receive_nVertex, 1, MPI::UNSIGNED_LONG);
	
	double *Buffer_Send_Coord = new double [MaxLocalVertex_NS*nDim];
	double *Buffer_Receive_Coord = new double [nProcessor*MaxLocalVertex_NS*nDim];
	unsigned long nBuffer = MaxLocalVertex_NS*nDim;
	
	for (iVertex = 0; iVertex < MaxLocalVertex_NS; iVertex++)
		for (iDim = 0; iDim < nDim; iDim++)
			Buffer_Send_Coord[iVertex*nDim+iDim] = 0.0;

	nVertex_NS = 0;
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if ((config->GetMarker_All_Boundary(iMarker) == NO_SLIP_WALL) || 
				(config->GetMarker_All_Boundary(iMarker) == EULER_WALL))
			for (iVertex = 0; iVertex < GetnVertex(iMarker); iVertex++) {
				iPoint = vertex[iMarker][iVertex]->GetNode();
				for (iDim = 0; iDim < nDim; iDim++)
					Buffer_Send_Coord[nVertex_NS*nDim+iDim] = node[iPoint]->GetCoord(iDim);
				nVertex_NS++;
			}

	MPI::COMM_WORLD.Allgather(Buffer_Send_Coord, nBuffer, MPI::DOUBLE, Buffer_Receive_Coord, nBuffer, MPI::DOUBLE);

	/*--- Get coordinates of the points and compute distances to the surface ---*/
	for (iPoint = 0; iPoint < GetnPointDomain(); iPoint++) {
		coord = node[iPoint]->GetCoord();
		
		/*--- Compute the squared distance and get the minimum ---*/
		dist = 1E20;
		for (iProcessor = 0; iProcessor < nProcessor; iProcessor++)
			for (iVertex = 0; iVertex < Buffer_Receive_nVertex[iProcessor]; iVertex++) {
				dist2 = 0.0;
				for (iDim = 0; iDim < nDim; iDim++)
					dist2 += (coord[iDim]-Buffer_Receive_Coord[(iProcessor*MaxLocalVertex_NS+iVertex)*nDim+iDim])*
					(coord[iDim]-Buffer_Receive_Coord[(iProcessor*MaxLocalVertex_NS+iVertex)*nDim+iDim]);
				if (dist2 < dist) dist = dist2;
			}
		node[iPoint]->SetWallDistance(sqrt(dist));
	}

	delete [] Buffer_Send_Coord;
	delete [] Buffer_Receive_Coord;
	delete [] Buffer_Send_nVertex;
	delete [] Buffer_Receive_nVertex;
	
	int rank = MPI::COMM_WORLD.Get_rank();

	if (rank == MASTER_NODE)
		cout << "Wall distance computation." << endl;

#endif
	
}

void CPhysicalGeometry::SetPositive_ZArea(CConfig *config) {
	unsigned short iMarker, Boundary, Monitoring;
	unsigned long iVertex, iPoint;
	double *Normal, PositiveZArea;
	int rank = MASTER_NODE;

#ifdef NO_MPI
	
	PositiveZArea = 0.0;
	for (iMarker = 0; iMarker < nMarker; iMarker++) {
		Boundary = config->GetMarker_All_Boundary(iMarker);
		Monitoring = config->GetMarker_All_Monitoring(iMarker);
		
		if (((Boundary == EULER_WALL) || (Boundary == NO_SLIP_WALL)) && (Monitoring == YES))
			for(iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
				iPoint = vertex[iMarker][iVertex]->GetNode();
				if (node[iPoint]->GetDomain()) {
					Normal = vertex[iMarker][iVertex]->GetNormal();
					if (Normal[nDim-1] < 0) PositiveZArea -= Normal[nDim-1];
				}	
			}
	}
	
#else
	
	double TotalPositiveZArea;
	
	rank = MPI::COMM_WORLD.Get_rank();
	
	PositiveZArea = 0.0;
	for (iMarker = 0; iMarker < nMarker; iMarker++) {
		Boundary = config->GetMarker_All_Boundary(iMarker);
		Monitoring = config->GetMarker_All_Monitoring(iMarker);
		
		if (((Boundary == EULER_WALL) || (Boundary == NO_SLIP_WALL)) && (Monitoring == YES))
			for(iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
				iPoint = vertex[iMarker][iVertex]->GetNode();
				if (node[iPoint]->GetDomain()) {
					Normal = vertex[iMarker][iVertex]->GetNormal();
					if (Normal[nDim-1] < 0) PositiveZArea -= Normal[nDim-1];
				}	
			}
	}

	MPI::COMM_WORLD.Reduce(&PositiveZArea, &TotalPositiveZArea, 1, MPI::DOUBLE, MPI::SUM, MASTER_NODE);
	if (MPI::COMM_WORLD.Get_rank() == MASTER_NODE) PositiveZArea = TotalPositiveZArea;
	MPI::COMM_WORLD.Bcast (&PositiveZArea, 1, MPI::DOUBLE, MASTER_NODE);
	
#endif
	
	if (config->GetRefAreaCoeff() == 0.0)
		config->SetRefAreaCoeff(PositiveZArea);

	if (rank == MASTER_NODE) {
		if (nDim == 2) cout << "Area projection in the y-plane = "<< PositiveZArea << "." << endl;
		else cout << "Area projection in the z-plane = "<< PositiveZArea << "." << endl;
	}

}


void CPhysicalGeometry::SetPsuP(void) {
	unsigned short Node_Neighbor, iElem, iNode, iNeighbor;
	unsigned long jElem, Point_Neighbor, iPoint;

	/*--- Loop over all the points ---*/
	for(iPoint = 0; iPoint < nPoint; iPoint++)
		/*--- Loop over all elements shared by the point ---*/
		for(iElem = 0; iElem < node[iPoint]->GetnElem(); iElem++) {  
			jElem = node[iPoint]->GetElem(iElem);
			/*--- If we find the point iPoint in the surronding element ---*/
			for(iNode = 0; iNode < elem[jElem]->GetnNodes(); iNode++)
				if (elem[jElem]->GetNode(iNode) == iPoint)
					/*--- Localize the local index of the neighbor of iPoint in the element ---*/
					for(iNeighbor = 0; iNeighbor < elem[jElem]->GetnNeighbor_Nodes(iNode); iNeighbor++) { 
						Node_Neighbor = elem[jElem]->GetNeighbor_Nodes(iNode,iNeighbor);
						Point_Neighbor = elem[jElem]->GetNode(Node_Neighbor);
						 /*--- Store the point into the point ---*/
						node[iPoint]->SetPoint(Point_Neighbor);
					}	
		}		
}

void CPhysicalGeometry::SetEsuE(void) {
	unsigned short first_elem_face, second_elem_face, iFace, iNode, jElem;
	unsigned long face_point, Test_Elem, iElem;

	/*--- Loop over all the elements, faces and nodes ---*/
	for(iElem = 0; iElem < nElem; iElem++)
		for (iFace = 0; iFace < elem[iElem]->GetnFaces(); iFace++)
			for (iNode = 0; iNode < elem[iElem]->GetnNodesFace(iFace); iNode++) {
				face_point = elem[iElem]->GetNode(elem[iElem]->GetFaces(iFace,iNode));
				/*--- Loop over all elements sharing the face point ---*/
				for(jElem = 0; jElem < node[face_point]->GetnElem(); jElem++) {
					Test_Elem = node[face_point]->GetElem(jElem);
					/*--- If it is a new element in this face ---*/
					if ((elem[iElem]->GetNeighbor_Elements(iFace) == -1) && (iElem < Test_Elem))
						if (FindFace(iElem,Test_Elem,first_elem_face,second_elem_face)) { 
							/*--- Localice which faces are sharing both elements ---*/
							elem[iElem]->SetNeighbor_Elements(Test_Elem,first_elem_face);
							/*--- Store the element for both elements ---*/ 
							elem[Test_Elem]->SetNeighbor_Elements(iElem,second_elem_face);
						}
				}
			}
}

void CPhysicalGeometry::SetBoundVolume(void) {
	unsigned short cont, iMarker, iElem, iNode_Domain, iNode_Surface;
	unsigned long Point_Domain, Point_Surface, Point, iElem_Surface, iElem_Domain;

	for (iMarker = 0; iMarker < nMarker; iMarker++)
		for (iElem_Surface = 0; iElem_Surface < nElem_Bound[iMarker]; iElem_Surface++) {
			/*--- Choose and arbitrary point from the surface --*/
			Point = bound[iMarker][iElem_Surface]->GetNode(0);
			for (iElem = 0; iElem < node[Point]->GetnElem(); iElem++) {
				/*--- Look for elements surronding that point --*/
				cont = 0; iElem_Domain = node[Point]->GetElem(iElem);		
				for (iNode_Domain = 0; iNode_Domain < elem[iElem_Domain]->GetnNodes(); iNode_Domain++) {
					Point_Domain = elem[iElem_Domain]->GetNode(iNode_Domain);
					for (iNode_Surface = 0; iNode_Surface < bound[iMarker][iElem_Surface]->GetnNodes(); iNode_Surface++) {
						Point_Surface = bound[iMarker][iElem_Surface]->GetNode(iNode_Surface);
						if (Point_Surface == Point_Domain) cont++;
						if (cont == bound[iMarker][iElem_Surface]->GetnNodes()) break;
					}
					if (cont ==  bound[iMarker][iElem_Surface]->GetnNodes()) break;
				}
				if (cont ==  bound[iMarker][iElem_Surface]->GetnNodes()) {
					bound[iMarker][iElem_Surface]->SetDomainElement(iElem_Domain);
					break;
				}
			}
		}
}

void CPhysicalGeometry::SetVertex(CConfig *config) {
	unsigned long  iPoint, iVertex, iElem;
	unsigned short iMarker, iNode;

	/*--- Initialize the Vertex vector for each node of the grid ---*/
	for (iPoint = 0; iPoint < nPoint; iPoint++)
		for (iMarker = 0; iMarker < nMarker; iMarker++)
			node[iPoint]->SetVertex(-1,iMarker); 
	
	/*--- Create and compute the vector with the number of vertex per marker ---*/
	nVertex = new unsigned long [nMarker];
	for (iMarker = 0; iMarker < nMarker; iMarker++) {
		/*--- Initialize the number of Bound Vertex for each Marker ---*/
		nVertex[iMarker] = 0;	
		for (iElem = 0; iElem < nElem_Bound[iMarker]; iElem++)
			for(iNode = 0; iNode < bound[iMarker][iElem]->GetnNodes(); iNode++) {
				iPoint = bound[iMarker][iElem]->GetNode(iNode);
				 /*--- Set the vertex in the node information ---*/
				if ((node[iPoint]->GetVertex(iMarker) == -1) || (config->GetMarker_All_Boundary(iMarker) == SEND_RECEIVE)) {
					iVertex = nVertex[iMarker];
					node[iPoint]->SetVertex(nVertex[iMarker],iMarker);
					nVertex[iMarker]++;
				}
			}
	}
	
	/*--- Initialize the Vertex vector for each node, the previous result is deleted ---*/
	for (iPoint = 0; iPoint < nPoint; iPoint++)
		for (iMarker = 0; iMarker < nMarker; iMarker++)
			node[iPoint]->SetVertex(-1,iMarker); 
	
	/*--- Create the bound vertex structure, note that the order 
	 is the same as in the input file, this is important for Send/Receive part ---*/
	vertex = new CVertex**[nMarker]; 
	for (iMarker = 0; iMarker < nMarker; iMarker++) {
		vertex[iMarker] = new CVertex* [nVertex[iMarker]];
		nVertex[iMarker] = 0;	
		
		/*--- Initialize the number of Bound Vertex for each Marker ---*/
		for (iElem = 0; iElem < nElem_Bound[iMarker]; iElem++)
			for(iNode = 0; iNode < bound[iMarker][iElem]->GetnNodes(); iNode++) {
				iPoint = bound[iMarker][iElem]->GetNode(iNode);
				/*--- Set the vertex in the node information ---*/
				if ((node[iPoint]->GetVertex(iMarker) == -1) || (config->GetMarker_All_Boundary(iMarker) == SEND_RECEIVE)){
					iVertex = nVertex[iMarker];
					vertex[iMarker][iVertex] = new CVertex(iPoint, nDim);
					
					if (config->GetMarker_All_Boundary(iMarker) == SEND_RECEIVE)
						vertex[iMarker][iVertex]->SetRotation_Type(bound[iMarker][iElem]->GetRotation_Type());
					
					node[iPoint]->SetVertex(nVertex[iMarker],iMarker);
					nVertex[iMarker]++;
				}
			}
	}
}
			
void CPhysicalGeometry::SetCG(void) {
	unsigned short nNode, iDim, iMarker, iNode;
	unsigned long elem_poin, edge_poin, iElem, iEdge;
	double **Coord;

	/*--- Compute the center of gravity for elements ---*/
	for(iElem = 0; iElem<nElem; iElem++) { 
		nNode = elem[iElem]->GetnNodes();
		Coord = new double* [nNode];
		/*--- Store the coordinates for all the element nodes ---*/ 
		for (iNode = 0; iNode < nNode; iNode++) {
			elem_poin = elem[iElem]->GetNode(iNode);
			Coord[iNode] = new double [nDim]; 
			for (iDim = 0; iDim < nDim; iDim++)
				Coord[iNode][iDim]=node[elem_poin]->GetCoord(iDim);
		}
		/*--- Compute the element CG coordinates ---*/
		elem[iElem]->SetCG(Coord);
	
		for (iNode = 0; iNode < nNode; iNode++)
			if (Coord[iNode] != NULL) delete [] Coord[iNode];
		if (Coord != NULL) delete [] Coord;		
	}

	/*--- Center of gravity for face elements ---*/
	for(iMarker = 0; iMarker < nMarker; iMarker++)
		for(iElem = 0; iElem < nElem_Bound[iMarker]; iElem++) {
			nNode = bound[iMarker][iElem]->GetnNodes();
			Coord = new double* [nNode];
			/*--- Store the coordinates for all the element nodes ---*/ 
			for (iNode = 0; iNode < nNode; iNode++) {
				elem_poin = bound[iMarker][iElem]->GetNode(iNode);
				Coord[iNode] = new double [nDim]; 
				for (iDim = 0; iDim < nDim; iDim++)
					Coord[iNode][iDim]=node[elem_poin]->GetCoord(iDim);
			}
			/*--- Compute the element CG coordinates ---*/
			bound[iMarker][iElem]->SetCG(Coord);
			for (iNode=0; iNode < nNode; iNode++)
				if (Coord[iNode] != NULL) delete [] Coord[iNode];
			if (Coord != NULL) delete [] Coord;
		}

	/*--- Center of gravity for edges ---*/
	for (iEdge = 0; iEdge < nEdge; iEdge++) {
		nNode = edge[iEdge]->GetnNodes();
		Coord = new double* [nNode];
		/*--- Store the coordinates for all the element nodes ---*/
		for (iNode = 0; iNode < nNode; iNode++) {
			edge_poin=edge[iEdge]->GetNode(iNode);
			Coord[iNode] = new double [nDim]; 
			for (iDim = 0; iDim<nDim; iDim++)
				Coord[iNode][iDim]=node[edge_poin]->GetCoord(iDim);
		}
		/*--- Compute the edge CG coordinates ---*/
		edge[iEdge]->SetCG(Coord);
		
		for (iNode=0; iNode < nNode; iNode++)
			if (Coord[iNode] != NULL) delete [] Coord[iNode];
		if (Coord != NULL) delete [] Coord;		
	}
}

void CPhysicalGeometry::SetBoundControlVolume(CConfig *config, unsigned short action) {
	unsigned short Neighbor_Node, iMarker, iNode, iNeighbor_Nodes, iDim;
	unsigned long Neighbor_Point, iVertex, iEdge, iPoint, iElem;
	
	/*--- Update values of faces of the edge ---*/
	if (action != ALLOCATE)
		for (iMarker = 0; iMarker < nMarker; iMarker++)
			for (iVertex = 0; iVertex < nVertex[iMarker]; iVertex++)
					vertex[iMarker][iVertex]->SetZeroValues();

	double *Coord_Edge_CG = new double [nDim];
	double *Coord_Elem_CG = new double [nDim];
	double *Coord_Vertex = new double [nDim];
	
	/*--- Loop over all the markers ---*/
	for (iMarker = 0; iMarker < nMarker; iMarker++)
		/*--- Loop over all the boundary elements ---*/
		for (iElem = 0; iElem < nElem_Bound[iMarker]; iElem++)
			/*--- Loop over all the nodes of the boundary ---*/
			for(iNode = 0; iNode < bound[iMarker][iElem]->GetnNodes(); iNode++) { 
				iPoint = bound[iMarker][iElem]->GetNode(iNode);
				iVertex = node[iPoint]->GetVertex(iMarker);
				/*--- Loop over the neighbor nodes, there is a face for each one ---*/
				for(iNeighbor_Nodes = 0; iNeighbor_Nodes < bound[iMarker][iElem]->GetnNeighbor_Nodes(iNode); iNeighbor_Nodes++) {
					Neighbor_Node = bound[iMarker][iElem]->GetNeighbor_Nodes(iNode,iNeighbor_Nodes);
					Neighbor_Point = bound[iMarker][iElem]->GetNode(Neighbor_Node);
					/*--- Shared edge by the Neighbor Point and the point ---*/
					iEdge = FindEdge(iPoint, Neighbor_Point);
					for (iDim = 0; iDim < nDim; iDim++) {
						Coord_Edge_CG[iDim] = edge[iEdge]->GetCG(iDim);
						Coord_Elem_CG[iDim] = bound[iMarker][iElem]->GetCG(iDim);
						Coord_Vertex[iDim] = node[iPoint]->GetCoord(iDim);
					}
					switch (nDim) {
						case 2:
							/*--- Store the 2D face (ojo hay cambio de sentido para ajustarse al sentido del contorno de nodo 0 al 1) ---*/
							if (iNode == 0) vertex[iMarker][iVertex]->SetNodes_Coord(Coord_Elem_CG, Coord_Vertex, config); 
							if (iNode == 1) vertex[iMarker][iVertex]->SetNodes_Coord(Coord_Vertex, Coord_Elem_CG, config);
							break;
						case 3:  
							/*--- Store the 3D face (ojo hay cambio de sentido para ajustarse al sentido del contorno de nodo 0 al 1) ---*/
							if (iNeighbor_Nodes == 0) vertex[iMarker][iVertex]->SetNodes_Coord(Coord_Elem_CG, Coord_Edge_CG, Coord_Vertex, config);
							if (iNeighbor_Nodes == 1) vertex[iMarker][iVertex]->SetNodes_Coord(Coord_Edge_CG, Coord_Elem_CG, Coord_Vertex, config);
					}
				}
			}

	delete [] Coord_Edge_CG;
	delete [] Coord_Elem_CG;
	delete [] Coord_Vertex;
}

void CPhysicalGeometry::MachNearField(CConfig *config) {
	double epsilon = 1e-1;
	
#ifdef NO_MPI
	
	unsigned short iMarker, jMarker;
	unsigned long iVertex, iPoint, jVertex, jPoint = 0, pPoint = 0;
	double *Coord_i, *Coord_j, dist = 0.0, mindist, maxdist;
	
	cout << "Set Near-Field boundary conditions (if any). " <<endl; 

	maxdist = 0.0;
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if (config->GetMarker_All_Boundary(iMarker) == NEARFIELD_BOUNDARY) {
			for (iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
				iPoint = vertex[iMarker][iVertex]->GetNode();
				Coord_i = node[iPoint]->GetCoord();
								
				mindist = 1e10;
				for (jMarker = 0; jMarker < config->GetnMarker_All(); jMarker++)
					if ((config->GetMarker_All_Boundary(jMarker) == NEARFIELD_BOUNDARY) && (iMarker != jMarker))
						for (jVertex = 0; jVertex < nVertex[jMarker]; jVertex++) {
							jPoint = vertex[jMarker][jVertex]->GetNode();
							Coord_j = node[jPoint]->GetCoord();
							if (nDim == 2) dist = sqrt(pow(Coord_j[0]-Coord_i[0],2.0) + pow(Coord_j[1]-Coord_i[1],2.0));
							if (nDim == 3) dist = sqrt(pow(Coord_j[0]-Coord_i[0],2.0) + pow(Coord_j[1]-Coord_i[1],2.0) + pow(Coord_j[2]-Coord_i[2],2.0));
							if (dist < mindist) { mindist = dist; pPoint = jPoint; }
						}
				maxdist = max(maxdist, mindist);
				vertex[iMarker][iVertex]->SetPeriodicPoint(pPoint);
				
				if (mindist > epsilon) {
					cout.precision(10);
					cout << endl;
					cout << "   Bad match for point " << iPoint << ".\tNearest";
					cout << " donor distance: " << scientific << mindist << ".";
					vertex[iMarker][iVertex]->SetPeriodicPoint(iPoint);
					maxdist = min(maxdist, 0.0);
				}
			}
			cout <<"The max distance between points is: " << maxdist <<"."<< endl;
		}
	
#else
	
	MPI::COMM_WORLD.Barrier();
	
	unsigned short iMarker, iDim;
	unsigned long iVertex, iPoint, pPoint = 0, jVertex, jPoint;
	double *Coord_i, Coord_j[3], dist = 0.0, mindist, maxdist;
	int iProcessor, pProcessor = 0;
	unsigned long nLocalVertex_NearField = 0, nGlobalVertex_NearField = 0, MaxLocalVertex_NearField = 0;
	
	int rank = MPI::COMM_WORLD.Get_rank();
	int nProcessor = MPI::COMM_WORLD.Get_size();
	
	unsigned long *Buffer_Send_nVertex = new unsigned long [1];
	unsigned long *Buffer_Receive_nVertex = new unsigned long [nProcessor];

	if (rank == MASTER_NODE) cout << "Set Near-Field boundary conditions (if any)." <<endl; 
	
	/*--- Compute the number of vertex that have nearfield boundary condition
	 without including the ghost nodes ---*/
	nLocalVertex_NearField = 0;
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if (config->GetMarker_All_Boundary(iMarker) == NEARFIELD_BOUNDARY)
			for (iVertex = 0; iVertex < GetnVertex(iMarker); iVertex++) {
				iPoint = vertex[iMarker][iVertex]->GetNode();
				if (node[iPoint]->GetDomain()) nLocalVertex_NearField ++;
			}

	Buffer_Send_nVertex[0] = nLocalVertex_NearField;
	
	/*--- Send Near-Field vertex information --*/
	MPI::COMM_WORLD.Allreduce(&nLocalVertex_NearField, &nGlobalVertex_NearField, 1, MPI::UNSIGNED_LONG, MPI::SUM); 	
	MPI::COMM_WORLD.Allreduce(&nLocalVertex_NearField, &MaxLocalVertex_NearField, 1, MPI::UNSIGNED_LONG, MPI::MAX); 	
	MPI::COMM_WORLD.Allgather(Buffer_Send_nVertex, 1, MPI::UNSIGNED_LONG, Buffer_Receive_nVertex, 1, MPI::UNSIGNED_LONG);
	
	double *Buffer_Send_Coord = new double [MaxLocalVertex_NearField*nDim];
	unsigned long *Buffer_Send_Point = new unsigned long [MaxLocalVertex_NearField];
	
	double *Buffer_Receive_Coord = new double [nProcessor*MaxLocalVertex_NearField*nDim];
	unsigned long *Buffer_Receive_Point = new unsigned long [nProcessor*MaxLocalVertex_NearField];

	unsigned long nBuffer_Coord = MaxLocalVertex_NearField*nDim;
	unsigned long nBuffer_Point = MaxLocalVertex_NearField;
	
	for (iVertex = 0; iVertex < MaxLocalVertex_NearField; iVertex++) {
		Buffer_Send_Point[iVertex] = 0;
		for (iDim = 0; iDim < nDim; iDim++)
			Buffer_Send_Coord[iVertex*nDim+iDim] = 0.0;
	}
	
	/*--- Copy coordinates and point to the auxiliar vector --*/
	nLocalVertex_NearField = 0;
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if (config->GetMarker_All_Boundary(iMarker) == NEARFIELD_BOUNDARY)
			for (iVertex = 0; iVertex < GetnVertex(iMarker); iVertex++) {
				iPoint = vertex[iMarker][iVertex]->GetNode();
				if (node[iPoint]->GetDomain()) {
					Buffer_Send_Point[nLocalVertex_NearField] = iPoint;
					for (iDim = 0; iDim < nDim; iDim++)
						Buffer_Send_Coord[nLocalVertex_NearField*nDim+iDim] = node[iPoint]->GetCoord(iDim);
					nLocalVertex_NearField++;
				}
			}
	
	MPI::COMM_WORLD.Allgather(Buffer_Send_Coord, nBuffer_Coord, MPI::DOUBLE, Buffer_Receive_Coord, nBuffer_Coord, MPI::DOUBLE);
	MPI::COMM_WORLD.Allgather(Buffer_Send_Point, nBuffer_Point, MPI::UNSIGNED_LONG, Buffer_Receive_Point, nBuffer_Point, MPI::UNSIGNED_LONG);
	
	/*--- Compute the closest point to a Near-Field boundary point ---*/
	maxdist = 0.0;
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if (config->GetMarker_All_Boundary(iMarker) == NEARFIELD_BOUNDARY) {
			for (iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
				iPoint = vertex[iMarker][iVertex]->GetNode();
				if (node[iPoint]->GetDomain()) {
					
					/*--- Coordinates of the boundary point ---*/
					Coord_i = node[iPoint]->GetCoord(); mindist = 1E6; pProcessor = 0; pPoint = 0;
					
					/*--- Loop over all the boundaries to find the pair ---*/
					for (iProcessor = 0; iProcessor < nProcessor; iProcessor++)
						for (jVertex = 0; jVertex < Buffer_Receive_nVertex[iProcessor]; jVertex++) {
							jPoint = Buffer_Receive_Point[iProcessor*MaxLocalVertex_NearField+jVertex];
							
							/*--- Compute the distance ---*/
							dist = 0.0; for (iDim = 0; iDim < nDim; iDim++) {
								Coord_j[iDim] = Buffer_Receive_Coord[(iProcessor*MaxLocalVertex_NearField+jVertex)*nDim+iDim];
								dist += pow(Coord_j[iDim]-Coord_i[iDim],2.0);
							} dist = sqrt(dist);
							
							if (((dist < mindist) && (iProcessor != rank)) ||
									((dist < mindist) && (iProcessor == rank) && (jPoint != iPoint))) {
								mindist = dist; pProcessor = iProcessor; pPoint = jPoint; 
							}
						}
					
					/*--- Store the value of the pair ---*/
					maxdist = max(maxdist, mindist);
					vertex[iMarker][iVertex]->SetPeriodicPoint(pPoint, pProcessor);
					
					if (mindist > epsilon) {
						cout.precision(10);
						cout << endl;
						cout << "   Bad match for point " << iPoint << ".\tNearest";
						cout << " donor distance: " << scientific << mindist << ".";
						vertex[iMarker][iVertex]->SetPeriodicPoint(iPoint);
						maxdist = min(maxdist, 0.0);
					}
					
				}
			}
			cout <<"Node rank: "<<rank<<". The max distance between points is: " << maxdist <<"."<< endl;
		}
	
	delete [] Buffer_Send_Coord;
	delete [] Buffer_Send_Point;
	
	delete [] Buffer_Receive_Coord;
	delete [] Buffer_Receive_Point;
	
	delete [] Buffer_Send_nVertex;
	delete [] Buffer_Receive_nVertex;


	MPI::COMM_WORLD.Barrier();
	
#endif

}

void CPhysicalGeometry::MachInterface(CConfig *config) {
	double epsilon = 1.5e-1;

#ifdef NO_MPI
	
	unsigned short iMarker, jMarker;
	unsigned long iVertex, iPoint, jVertex, jPoint = 0, pPoint = 0;
	double *Coord_i, *Coord_j, dist = 0.0, mindist, maxdist;
	
	cout << "Set Interface boundary conditions (if any)." << endl; 
	
	maxdist = 0.0;
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if (config->GetMarker_All_Boundary(iMarker) == INTERFACE_BOUNDARY) {
			for (iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
				iPoint = vertex[iMarker][iVertex]->GetNode();
				Coord_i = node[iPoint]->GetCoord();
				
				mindist = 1E6;
				for (jMarker = 0; jMarker < config->GetnMarker_All(); jMarker++)
					if ((config->GetMarker_All_Boundary(jMarker) == INTERFACE_BOUNDARY) && (iMarker != jMarker))
						for (jVertex = 0; jVertex < nVertex[jMarker]; jVertex++) {
							jPoint = vertex[jMarker][jVertex]->GetNode();
							Coord_j = node[jPoint]->GetCoord();
							if (nDim == 2) dist = sqrt(pow(Coord_j[0]-Coord_i[0],2.0) + pow(Coord_j[1]-Coord_i[1],2.0));
							if (nDim == 3) dist = sqrt(pow(Coord_j[0]-Coord_i[0],2.0) + pow(Coord_j[1]-Coord_i[1],2.0) + pow(Coord_j[2]-Coord_i[2],2.0));
							if (dist < mindist) {mindist = dist; pPoint = jPoint;}
						}
				maxdist = max(maxdist, mindist);
				vertex[iMarker][iVertex]->SetPeriodicPoint(pPoint);
				
				if (mindist > epsilon) {
					cout.precision(10);
					cout << endl;
					cout << "   Bad match for point " << iPoint << ".\tNearest";
					cout << " donor distance: " << scientific << mindist << ".";
					vertex[iMarker][iVertex]->SetPeriodicPoint(iPoint);
					maxdist = min(maxdist, 0.0);
				}
				
			}
		cout <<"The max distance between points is: " << maxdist <<"."<< endl;
	}

#else
	
	MPI::COMM_WORLD.Barrier();
	
	unsigned short iMarker, iDim;
	unsigned long iVertex, iPoint, pPoint = 0, jVertex, jPoint;
	double *Coord_i, Coord_j[3], dist = 0.0, mindist, maxdist;
	int iProcessor, pProcessor = 0;
	unsigned long nLocalVertex_Interface = 0, nGlobalVertex_Interface = 0, MaxLocalVertex_Interface = 0;
	
	int rank = MPI::COMM_WORLD.Get_rank();
	int nProcessor = MPI::COMM_WORLD.Get_size();
	
	unsigned long *Buffer_Send_nVertex = new unsigned long [1];
	unsigned long *Buffer_Receive_nVertex = new unsigned long [nProcessor];
	
	if (rank == MASTER_NODE) cout << "Set Interface boundary conditions (if any)." <<endl; 
	
	/*--- Compute the number of vertex that have nearfield boundary condition
	 without including the ghost nodes ---*/
	nLocalVertex_Interface = 0;
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if (config->GetMarker_All_Boundary(iMarker) == INTERFACE_BOUNDARY)
			for (iVertex = 0; iVertex < GetnVertex(iMarker); iVertex++) {
				iPoint = vertex[iMarker][iVertex]->GetNode();
				if (node[iPoint]->GetDomain()) nLocalVertex_Interface ++;
			}
	
	Buffer_Send_nVertex[0] = nLocalVertex_Interface;
	
	/*--- Send Interface vertex information --*/
	MPI::COMM_WORLD.Allreduce(&nLocalVertex_Interface, &nGlobalVertex_Interface, 1, MPI::UNSIGNED_LONG, MPI::SUM); 	
	MPI::COMM_WORLD.Allreduce(&nLocalVertex_Interface, &MaxLocalVertex_Interface, 1, MPI::UNSIGNED_LONG, MPI::MAX); 	
	MPI::COMM_WORLD.Allgather(Buffer_Send_nVertex, 1, MPI::UNSIGNED_LONG, Buffer_Receive_nVertex, 1, MPI::UNSIGNED_LONG);
	
	double *Buffer_Send_Coord = new double [MaxLocalVertex_Interface*nDim];
	unsigned long *Buffer_Send_Point = new unsigned long [MaxLocalVertex_Interface];
	
	double *Buffer_Receive_Coord = new double [nProcessor*MaxLocalVertex_Interface*nDim];
	unsigned long *Buffer_Receive_Point = new unsigned long [nProcessor*MaxLocalVertex_Interface];

	unsigned long nBuffer_Coord = MaxLocalVertex_Interface*nDim;
	unsigned long nBuffer_Point = MaxLocalVertex_Interface;

	for (iVertex = 0; iVertex < MaxLocalVertex_Interface; iVertex++) {
		Buffer_Send_Point[iVertex] = 0;
		for (iDim = 0; iDim < nDim; iDim++)
			Buffer_Send_Coord[iVertex*nDim+iDim] = 0.0;
	}
	
	/*--- Copy coordinates and point to the auxiliar vector --*/
	nLocalVertex_Interface = 0;
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if (config->GetMarker_All_Boundary(iMarker) == INTERFACE_BOUNDARY)
			for (iVertex = 0; iVertex < GetnVertex(iMarker); iVertex++) {
				iPoint = vertex[iMarker][iVertex]->GetNode();
				if (node[iPoint]->GetDomain()) {
					Buffer_Send_Point[nLocalVertex_Interface] = iPoint;
					for (iDim = 0; iDim < nDim; iDim++)
						Buffer_Send_Coord[nLocalVertex_Interface*nDim+iDim] = node[iPoint]->GetCoord(iDim);
					nLocalVertex_Interface++;
				}
			}
	
	MPI::COMM_WORLD.Allgather(Buffer_Send_Coord, nBuffer_Coord, MPI::DOUBLE, Buffer_Receive_Coord, nBuffer_Coord, MPI::DOUBLE);
	MPI::COMM_WORLD.Allgather(Buffer_Send_Point, nBuffer_Point, MPI::UNSIGNED_LONG, Buffer_Receive_Point, nBuffer_Point, MPI::UNSIGNED_LONG);

	/*--- Compute the closest point to a Near-Field boundary point ---*/
	maxdist = 0.0;
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if (config->GetMarker_All_Boundary(iMarker) == INTERFACE_BOUNDARY) {
			for (iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
				iPoint = vertex[iMarker][iVertex]->GetNode();
				if (node[iPoint]->GetDomain()) {
					
					/*--- Coordinates of the boundary point ---*/
					Coord_i = node[iPoint]->GetCoord(); mindist = 1E6; pProcessor = 0; pPoint = 0;
					
					/*--- Loop over all the boundaries to find the pair ---*/
					for (iProcessor = 0; iProcessor < nProcessor; iProcessor++)
						for (jVertex = 0; jVertex < Buffer_Receive_nVertex[iProcessor]; jVertex++) {
							jPoint = Buffer_Receive_Point[iProcessor*MaxLocalVertex_Interface+jVertex];

							/*--- Compute the distance ---*/
							dist = 0.0; for (iDim = 0; iDim < nDim; iDim++) {
								Coord_j[iDim] = Buffer_Receive_Coord[(iProcessor*MaxLocalVertex_Interface+jVertex)*nDim+iDim];
								dist += pow(Coord_j[iDim]-Coord_i[iDim],2.0);
							} dist = sqrt(dist);
							
							if (((dist < mindist) && (iProcessor != rank)) ||
									((dist < mindist) && (iProcessor == rank) && (jPoint != iPoint))) {
								mindist = dist; pProcessor = iProcessor; pPoint = jPoint; 
							}
						}
					
					/*--- Store the value of the pair ---*/
					maxdist = max(maxdist, mindist);
					vertex[iMarker][iVertex]->SetPeriodicPoint(pPoint, pProcessor);
					
					if (mindist > epsilon) {
						cout.precision(10);
						cout << endl;
						cout << "   Bad match for point " << iPoint << ".\tNearest";
						cout << " donor distance: " << scientific << mindist << ".";
						vertex[iMarker][iVertex]->SetPeriodicPoint(iPoint);
						maxdist = min(maxdist, 0.0);
					}
					
				}
			}
			cout << "Node rank: "<< rank << ". The max distance between points is: " << maxdist <<"."<< endl;
		}

	delete [] Buffer_Send_Coord;
	delete [] Buffer_Send_Point;
	
	delete [] Buffer_Receive_Coord;
	delete [] Buffer_Receive_Point;
	
	delete [] Buffer_Send_nVertex;
	delete [] Buffer_Receive_nVertex;
	
	
	MPI::COMM_WORLD.Barrier();

#endif
	
}


void CPhysicalGeometry::SetControlVolume(CConfig *config, unsigned short action) {
	unsigned long face_iPoint = 0, face_jPoint = 0, iEdge, iPoint, iElem;
	unsigned short nEdgesFace = 1, iFace, iEdgesFace, iDim;
	double *Coord_Edge_CG, *Coord_FaceElem_CG, *Coord_Elem_CG, *Coord_FaceiPoint, *Coord_FacejPoint, Area, 
	Volume, DomainVolume, my_DomainVolume;
	bool change_face_orientation;
	
#ifdef NO_MPI
	int rank = MASTER_NODE;
#else
	int rank = MPI::COMM_WORLD.Get_rank();
#endif
	
	/*--- Update values of faces of the edge ---*/
	if (action != ALLOCATE) {
		for(iEdge = 0; iEdge < nEdge; iEdge++)
			edge[iEdge]->SetZeroValues();
		for(iPoint = 0; iPoint < nPoint; iPoint++)
			node[iPoint]->SetVolume (0.0);
	}
	
	Coord_Edge_CG = new double [nDim];
	Coord_FaceElem_CG = new double [nDim];
	Coord_Elem_CG = new double [nDim];
	Coord_FaceiPoint = new double [nDim];
	Coord_FacejPoint = new double [nDim];
	
	my_DomainVolume = 0.0;
	for(iElem = 0; iElem < nElem; iElem++)
		for (iFace = 0; iFace < elem[iElem]->GetnFaces(); iFace++) {
			
			/*--- In 2D all the faces have only one edge ---*/
			if (nDim == 2) nEdgesFace = 1;
			/*--- In 3D the number of edges per face is the same as the number of point per face ---*/
			if (nDim == 3) nEdgesFace = elem[iElem]->GetnNodesFace(iFace);
			
			/*--- Bucle over the edges of a face ---*/
			for (iEdgesFace = 0; iEdgesFace < nEdgesFace; iEdgesFace++) {
				
				/*--- In 2D only one edge (two points) per edge ---*/
				if (nDim == 2) { 
					face_iPoint = elem[iElem]->GetNode(elem[iElem]->GetFaces(iFace,0));
					face_jPoint = elem[iElem]->GetNode(elem[iElem]->GetFaces(iFace,1));
				}

				/*--- In 3D there are several edges in each face ---*/
				if (nDim == 3) {
					face_iPoint = elem[iElem]->GetNode(elem[iElem]->GetFaces(iFace,iEdgesFace));
					if (iEdgesFace != nEdgesFace-1)
						face_jPoint = elem[iElem]->GetNode(elem[iElem]->GetFaces(iFace,iEdgesFace+1));
					else
						face_jPoint = elem[iElem]->GetNode(elem[iElem]->GetFaces(iFace,0));
				}
				
				/*--- We define a direction (from the smalest index to the greatest) --*/
				change_face_orientation = false;
				if (face_iPoint > face_jPoint) change_face_orientation = true;
				iEdge = FindEdge(face_iPoint, face_jPoint);
				
				for (iDim = 0; iDim < nDim; iDim++) {
					Coord_Edge_CG[iDim] = edge[iEdge]->GetCG(iDim);
					Coord_Elem_CG[iDim] = elem[iElem]->GetCG(iDim);
					Coord_FaceElem_CG[iDim] = elem[iElem]->GetFaceCG(iFace,iDim);
					Coord_FaceiPoint[iDim] = node[face_iPoint]->GetCoord(iDim);
					Coord_FacejPoint[iDim] = node[face_jPoint]->GetCoord(iDim);
				}
				
				switch (nDim) {
					case 2: 
						/*--- Two dimensional problem ---*/
						if (change_face_orientation) edge[iEdge]->SetNodes_Coord(Coord_Elem_CG, Coord_Edge_CG, config);
						else edge[iEdge]->SetNodes_Coord(Coord_Edge_CG,Coord_Elem_CG, config);
						Area = edge[iEdge]->GetVolume(Coord_FaceiPoint,Coord_Edge_CG,Coord_Elem_CG);
						node[face_iPoint]->AddVolume(Area); my_DomainVolume +=Area;
						Area = edge[iEdge]->GetVolume(Coord_FacejPoint,Coord_Edge_CG,Coord_Elem_CG);
						node[face_jPoint]->AddVolume(Area); my_DomainVolume +=Area;
						break;
					case 3: 
						/*--- Three dimensional problem ---*/
						if (change_face_orientation) edge[iEdge]->SetNodes_Coord(Coord_FaceElem_CG,Coord_Edge_CG,Coord_Elem_CG, config);
						else edge[iEdge]->SetNodes_Coord(Coord_Edge_CG,Coord_FaceElem_CG,Coord_Elem_CG, config);
						Volume = edge[iEdge]->GetVolume(Coord_FaceiPoint,Coord_Edge_CG,Coord_FaceElem_CG, Coord_Elem_CG);
						node[face_iPoint]->AddVolume(Volume); my_DomainVolume +=Volume;						
						Volume = edge[iEdge]->GetVolume(Coord_FacejPoint,Coord_Edge_CG,Coord_FaceElem_CG, Coord_Elem_CG);
						node[face_jPoint]->AddVolume(Volume); my_DomainVolume +=Volume;	
						break;
				}
			}
		}
		
	/*--- Set the volume for the iterations n and n-1 (dual time stteping with grid movement) ---*/
	for (iPoint = 0; iPoint < nPoint; iPoint++) {		
		node[iPoint]->SetVolume_n();
		node[iPoint]->SetVolume_n1();
	}
	
#ifndef NO_MPI
	MPI::COMM_WORLD.Allreduce(&my_DomainVolume, &DomainVolume, 1, MPI::DOUBLE, MPI::SUM); 	
#else
	DomainVolume = my_DomainVolume;
#endif

	if (rank == MASTER_NODE) {
		if (nDim == 2) cout <<"Area of the computational grid: "<< DomainVolume <<"."<<endl;
		if (nDim == 3) cout <<"Volume of the computational grid: "<< DomainVolume <<"."<<endl;
	}
	
	config->SetDomainVolume(DomainVolume);

	delete [] Coord_Edge_CG;
	delete [] Coord_FaceElem_CG;
	delete [] Coord_Elem_CG;
	delete [] Coord_FaceiPoint;
	delete [] Coord_FacejPoint;
}

void CPhysicalGeometry::SetMeshFile (CConfig *config, string val_mesh_out_filename) {
	unsigned long iElem, iPoint, iElem_Bound;
	unsigned short iMarker, iNodes, iDim;
  unsigned short iPeriodic, nPeriodic = 0;
	ofstream output_file;
	string Grid_Marker;
	char *cstr;
	double *center, *angles, *transl;
  
	cstr = new char [val_mesh_out_filename.size()+1];
	strcpy (cstr, val_mesh_out_filename.c_str());

	/*--- Open .su2 grid file ---*/
  output_file.precision(15);
	output_file.open(cstr, ios::out);

	/*--- Write dimension, number of elements and number of points ---*/
	output_file << "NDIME= " << nDim << endl;
	output_file << "NELEM= " << nElem << endl;
	for (iElem = 0; iElem < nElem; iElem++) {
		output_file << elem[iElem]->GetVTK_Type();
		for (iNodes = 0; iNodes < elem[iElem]->GetnNodes(); iNodes++)
			output_file << "\t" << elem[iElem]->GetNode(iNodes);
		output_file << "\t"<<iElem<<endl;	
	}

  /*--- Write the node coordinates ---*/
	output_file << "NPOIN= " << nPoint << "\t" << nPointDomain << endl;
	output_file.precision(15);
	for (iPoint = 0; iPoint < nPoint; iPoint++) {
		for (iDim = 0; iDim < nDim; iDim++)
			output_file << scientific << "\t" << node[iPoint]->GetCoord(iDim) ;
#ifdef NO_MPI
		output_file << "\t" << iPoint << endl;
#else
		output_file << "\t" << iPoint << "\t" << node[iPoint]->GetGlobalIndex() << endl;
#endif

	}

  /*--- Loop through and write the boundary info ---*/
	output_file << "NMARK= " << nMarker << endl;
	for (iMarker = 0; iMarker < nMarker; iMarker++) {
    
    /*--- Ignore SEND_RECEIVE for the moment ---*/
    if (bound[iMarker][0]->GetVTK_Type() != VERTEX) {
          
		Grid_Marker = config->GetMarker_All_Tag(iMarker);
		output_file << "MARKER_TAG= " << Grid_Marker <<endl;
		output_file << "MARKER_ELEMS= " << nElem_Bound[iMarker]<< endl;
		
		if (nDim == 2) {
			for (iElem_Bound = 0; iElem_Bound < nElem_Bound[iMarker]; iElem_Bound++) {
				output_file << bound[iMarker][iElem_Bound]->GetVTK_Type() << "\t" ;
				for (iNodes = 0; iNodes < bound[iMarker][iElem_Bound]->GetnNodes(); iNodes++)
					output_file << bound[iMarker][iElem_Bound]->GetNode(iNodes) << "\t" ;
				output_file	<< iElem_Bound << endl;
			}
		}
	
		if (nDim == 3) {
			for (iElem_Bound = 0; iElem_Bound < nElem_Bound[iMarker]; iElem_Bound++) {
				output_file << bound[iMarker][iElem_Bound]->GetVTK_Type() << "\t" ;
				for (iNodes = 0; iNodes < bound[iMarker][iElem_Bound]->GetnNodes(); iNodes++)
					output_file << bound[iMarker][iElem_Bound]->GetNode(iNodes) << "\t" ;
				output_file	<< iElem_Bound << endl;
			}
		}
		
    } else if (bound[iMarker][0]->GetVTK_Type() == VERTEX) {
			output_file << "MARKER_TAG= SEND_RECEIVE" << endl;
			output_file << "MARKER_ELEMS= " << nElem_Bound[iMarker]<< endl;
			if (config->GetMarker_All_SendRecv(iMarker) > 0) output_file << "SEND_TO= " << config->GetMarker_All_SendRecv(iMarker) << endl;
			if (config->GetMarker_All_SendRecv(iMarker) < 0) output_file << "SEND_TO= " << config->GetMarker_All_SendRecv(iMarker) << endl;
			
			for (iElem_Bound = 0; iElem_Bound < nElem_Bound[iMarker]; iElem_Bound++) {
				output_file << bound[iMarker][iElem_Bound]->GetVTK_Type() << "\t" << 
				bound[iMarker][iElem_Bound]->GetNode(0) << "\t" << 
				bound[iMarker][iElem_Bound]->GetRotation_Type() << endl;
			}
      
		}
  }
    
  /*--- Get the total number of periodic transformations ---*/
  nPeriodic = config->GetnPeriodicIndex();
	output_file << "NPERIODIC= " << nPeriodic << endl;
  
	/*--- From iPeriodic obtain the iMarker ---*/
	for (iPeriodic = 0; iPeriodic < nPeriodic; iPeriodic++) {
		
		/*--- Retrieve the supplied periodic information. ---*/
		center = config->GetPeriodicCenter(iPeriodic);
		angles = config->GetPeriodicRotation(iPeriodic);
		transl = config->GetPeriodicTranslate(iPeriodic);		
		
		output_file << "PERIODIC_INDEX= " << iPeriodic << endl;
		output_file << center[0] << "\t" << center[1] << "\t" << center[2] << endl;
		output_file << angles[0] << "\t" << angles[1] << "\t" << angles[2] << endl;
		output_file << transl[0] << "\t" << transl[1] << "\t" << transl[2] << endl;
    
  }
    
    
	output_file.close();
}

void CPhysicalGeometry::SetMeshFile_IntSurface(CConfig *config, string val_mesh_out_filename) {
	unsigned long iElem, iPoint, iElem_Bound, TestPoint;
	unsigned short iMarker, iNodes, iDim, NearFieldMarker = 0;
	ofstream output_file;
	string Grid_Marker;
	char *cstr;
	
	/*--- Identify the surface that must be doubled ---*/
	if (config->GetKind_Adaptation() != DOUBLE_SURFACE)
		NearFieldMarker = nMarker-1;
	else {
		for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
			if (config->GetMarker_All_Boundary(iMarker) == NEARFIELD_BOUNDARY)
				NearFieldMarker = iMarker;		
	}
	
	cstr = new char [val_mesh_out_filename.size()+1];
	strcpy (cstr, val_mesh_out_filename.c_str());
	
	/*--- Open .su2 grid file ---*/
	output_file.open(cstr, ios::out);
	
	/*--- Create a list that for each point of the inner surface gives you the 
	 equivalent point from the outer surface ---*/
	long *LowerPoint = new long[nPoint];
	for (iPoint = 0; iPoint < nPoint; iPoint++) LowerPoint[iPoint] = -1;

	for (iElem_Bound = 0; iElem_Bound < nElem_Bound[NearFieldMarker]; iElem_Bound++)
		for (iNodes = 0; iNodes < bound[NearFieldMarker][iElem_Bound]->GetnNodes(); iNodes++) {
			iPoint = bound[NearFieldMarker][iElem_Bound]->GetNode(iNodes);
			if (LowerPoint[iPoint] == -1) {
				LowerPoint[iPoint] = 0;
			}
		}
	
	/*--- Write dimension, number of elements and number of points ---*/
	output_file << "NDIME=" << nDim << endl;
	
	bool *InnerElement = new bool[nElem];
	for (iElem = 0; iElem < nElem; iElem++)
		InnerElement[iElem] = false;		
	
	output_file << "NELEM=" << nElem << endl;	
	for (iElem = 0; iElem < nElem; iElem++) {
		output_file << elem[iElem]->GetVTK_Type();
		
		/*--- Test to identify inner points from outer points ---*/
		bool InnerSide = true;
		
		/*--- Identification of the inner, and outer domain ---*/
		for (iNodes = 0; iNodes < elem[iElem]->GetnNodes(); iNodes++) {
			TestPoint = elem[iElem]->GetNode(iNodes);
			if (LowerPoint[TestPoint] != -1) {
				/*--- There are two combinations (0,1,4,5) and (2,3,6,7) ---*/
				if (iNodes == 2) InnerSide = false;
			}
		}
		
		if (InnerSide) InnerElement[iElem] = true;
		
		for (iNodes = 0; iNodes < elem[iElem]->GetnNodes(); iNodes++)
			output_file << "\t" << elem[iElem]->GetNode(iNodes);
		output_file << "\t"<<iElem<<endl;			
	}
	
	output_file << "NPOIN=" << nPoint << endl;
	output_file.precision(15);
	for (iPoint = 0; iPoint < nPoint; iPoint++) {
		for (iDim = 0; iDim < nDim; iDim++)
			output_file << scientific << "\t" << node[iPoint]->GetCoord(iDim) ;
		output_file << "\t" << iPoint << endl;
	}
	
	/*--- Write the boundary elements ---*/
	output_file << "NMARK=" << nMarker+1 << endl;
	
	for (iMarker = 0; iMarker < nMarker-1; iMarker++) {
		Grid_Marker = config->GetMarker_All_Tag(iMarker);
		output_file << "MARKER_TAG=" << Grid_Marker <<endl;
		output_file << "MARKER_ELEMS=" << nElem_Bound[iMarker]<< endl;
		
		for (iElem_Bound = 0; iElem_Bound < nElem_Bound[iMarker]; iElem_Bound++) {
			output_file << bound[iMarker][iElem_Bound]->GetVTK_Type() << "\t" ;
			for (iNodes = 0; iNodes < bound[iMarker][iElem_Bound]->GetnNodes(); iNodes++)
				output_file << bound[iMarker][iElem_Bound]->GetNode(iNodes) << "\t" ;
			output_file	<< endl;
		}
	}
	
	/*--- We add the boundary that corresponf with the lower side ---*/
	output_file << "MARKER_TAG=" << config->GetPlaneTag(0) << endl;
	output_file << "MARKER_ELEMS=" << int(nElem_Bound[NearFieldMarker]/2)<< endl;
	for (iElem_Bound = 0; iElem_Bound < nElem_Bound[NearFieldMarker]; iElem_Bound++) {
		unsigned long DomainElement = bound[NearFieldMarker][iElem_Bound]->GetDomainElement();
		if (InnerElement[DomainElement]) {		
			output_file << bound[NearFieldMarker][iElem_Bound]->GetVTK_Type() << "\t" ;
			for (iNodes = 0; iNodes < bound[iMarker][iElem_Bound]->GetnNodes(); iNodes++)
				output_file << bound[iMarker][iElem_Bound]->GetNode(iNodes) << "\t" ;
			output_file	<< endl;
		}
	}
	
	output_file << "MARKER_TAG=" << config->GetPlaneTag(1) << endl;
	output_file << "MARKER_ELEMS=" << int(nElem_Bound[NearFieldMarker]/2)<< endl;
	for (iElem_Bound = 0; iElem_Bound < nElem_Bound[NearFieldMarker]; iElem_Bound++) {
		unsigned long DomainElement = bound[NearFieldMarker][iElem_Bound]->GetDomainElement();
		if (!InnerElement[DomainElement]) {		
			output_file << bound[NearFieldMarker][iElem_Bound]->GetVTK_Type() << "\t" ;
			for (iNodes = 0; iNodes < bound[iMarker][iElem_Bound]->GetnNodes(); iNodes++)
				output_file << bound[iMarker][iElem_Bound]->GetNode(iNodes) << "\t" ;
			output_file	<< endl;
		}
	}
	
	output_file.close();
}

void CPhysicalGeometry::SetParaView(char mesh_filename[200]) {
	unsigned long iElem;
	unsigned short iNode;
	ofstream para_file;
	
	para_file.open(mesh_filename, ios::out);

	para_file << "# vtk DataFile Version 2.0" << endl;
	para_file << "Visualization of the volumetric grid" << endl;
	para_file << "ASCII" << endl;
	para_file << "DATASET UNSTRUCTURED_GRID" << endl;
	para_file << "POINTS " << nPoint << " float" << endl;

	para_file.precision(15);

	for(unsigned long iPoint = 0; iPoint < nPoint; iPoint++) {
		for(unsigned short iDim = 0; iDim < nDim; iDim++)
			para_file << scientific << node[iPoint]->GetCoord(iDim) << "\t";
		if (nDim == 2) para_file << "0.0";
		para_file << "\n";
	}	 

	para_file << "CELLS " << nElem << "\t" << nElem_Storage << endl;
	for(iElem = 0; iElem < nElem; iElem++) {
		para_file << elem[iElem]->GetnNodes() << "\t";
		for(iNode = 0; iNode < elem[iElem]->GetnNodes(); iNode++) {
			if (iNode == elem[iElem]->GetnNodes()-1) para_file << elem[iElem]->GetNode(iNode) << "\n";			
			else para_file << elem[iElem]->GetNode(iNode) << "\t";			
		}
	}	

	para_file << "CELL_TYPES " << nElem << endl;
	for(unsigned long iElem=0; iElem < nElem; iElem++) {
		para_file << elem[iElem]->GetVTK_Type() << endl;
	}
	para_file.close();
}

void CPhysicalGeometry::SetTecPlot(char mesh_filename[200]) {
	unsigned long iElem, iPoint;
	unsigned short iDim;
	ofstream Tecplot_File;

	Tecplot_File.open(mesh_filename, ios::out);
	Tecplot_File << "TITLE = \"Visualization of the volumetric grid\"" << endl;

	if (nDim == 2) {
		Tecplot_File << "VARIABLES = \"x\",\"y\" " << endl;
		Tecplot_File << "ZONE NODES= "<< nPoint <<", ELEMENTS= "<< nElem <<", DATAPACKING=POINT, ZONETYPE=FEQUADRILATERAL"<< endl;
	}
	if (nDim == 3) {
		Tecplot_File << "VARIABLES = \"x\",\"y\",\"z\" " << endl;	
		Tecplot_File << "ZONE NODES= "<< nPoint <<", ELEMENTS= "<< nElem <<", DATAPACKING=POINT, ZONETYPE=FEBRICK"<< endl;
	}
		
	for(iPoint = 0; iPoint < nPoint; iPoint++) {
		for(iDim = 0; iDim < nDim; iDim++)
			Tecplot_File << scientific << node[iPoint]->GetCoord(iDim) << "\t";
		Tecplot_File << "\n";
	}	 
	
	for(iElem = 0; iElem < nElem; iElem++) {
		if (elem[iElem]->GetVTK_Type() == TRIANGLE) {
			Tecplot_File <<
			elem[iElem]->GetNode(0)+1 <<" "<< elem[iElem]->GetNode(1)+1 <<" "<<
			elem[iElem]->GetNode(2)+1 <<" "<< elem[iElem]->GetNode(2)+1 << endl;
		}
		if (elem[iElem]->GetVTK_Type() == RECTANGLE) {
			Tecplot_File <<
			elem[iElem]->GetNode(0)+1 <<" "<< elem[iElem]->GetNode(1)+1 <<" "<<
			elem[iElem]->GetNode(2)+1 <<" "<< elem[iElem]->GetNode(3)+1 << endl;
		}
		if (elem[iElem]->GetVTK_Type() == TETRAHEDRON) {
			Tecplot_File <<
			elem[iElem]->GetNode(0)+1 <<" "<< elem[iElem]->GetNode(1)+1 <<" "<<
			elem[iElem]->GetNode(2)+1 <<" "<< elem[iElem]->GetNode(2)+1 <<" "<<
			elem[iElem]->GetNode(3)+1 <<" "<< elem[iElem]->GetNode(3)+1 <<" "<<
			elem[iElem]->GetNode(3)+1 <<" "<< elem[iElem]->GetNode(3)+1 << endl;
		}
		if (elem[iElem]->GetVTK_Type() == HEXAHEDRON) {
			Tecplot_File <<
			elem[iElem]->GetNode(0)+1 <<" "<< elem[iElem]->GetNode(1)+1 <<" "<<
			elem[iElem]->GetNode(2)+1 <<" "<< elem[iElem]->GetNode(3)+1 <<" "<<
			elem[iElem]->GetNode(4)+1 <<" "<< elem[iElem]->GetNode(5)+1 <<" "<<
			elem[iElem]->GetNode(6)+1 <<" "<< elem[iElem]->GetNode(7)+1 << endl;
		}
		if (elem[iElem]->GetVTK_Type() == PYRAMID) {
			Tecplot_File <<
			elem[iElem]->GetNode(0)+1 <<" "<< elem[iElem]->GetNode(1)+1 <<" "<<
			elem[iElem]->GetNode(2)+1 <<" "<< elem[iElem]->GetNode(3)+1 <<" "<<
			elem[iElem]->GetNode(4)+1 <<" "<< elem[iElem]->GetNode(4)+1 <<" "<<
			elem[iElem]->GetNode(4)+1 <<" "<< elem[iElem]->GetNode(4)+1 << endl;
		}
		if (elem[iElem]->GetVTK_Type() == WEDGE) {
			Tecplot_File <<
			elem[iElem]->GetNode(0)+1 <<" "<< elem[iElem]->GetNode(1)+1 <<" "<<
			elem[iElem]->GetNode(1)+1 <<" "<< elem[iElem]->GetNode(2)+1 <<" "<<
			elem[iElem]->GetNode(3)+1 <<" "<< elem[iElem]->GetNode(4)+1 <<" "<<
			elem[iElem]->GetNode(4)+1 <<" "<< elem[iElem]->GetNode(5)+1 << endl;
		}
	}
	
	Tecplot_File.close();
}

void CPhysicalGeometry::SetCoord_Smoothing (unsigned short val_nSmooth, double val_smooth_coeff, CConfig *config) {
	unsigned short iSmooth, nneigh, iMarker;
	double *Coord_Old, *Coord_Sum, *Coord, *Coord_i, *Coord_j, Position_Plane = 0.0;
	unsigned long iEdge, iPoint, jPoint, iVertex;
	double eps = 1E-6;
	bool NearField = false;
	
	if (config->GetKind_Adaptation() == HORIZONTAL_PLANE) {
		NearField = true;
		Position_Plane = config->GetPosition_Plane();
	}
	
	Coord = new double [nDim];

	for (iPoint = 0; iPoint < GetnPoint(); iPoint++) {
		double *Coord = node[iPoint]->GetCoord();
		node[iPoint]->SetCoord_Old(Coord);
	}

	/*--- Jacobi iterations ---*/
	for (iSmooth = 0; iSmooth < val_nSmooth; iSmooth++) {

		for (iPoint = 0; iPoint < nPoint; iPoint++)
			node[iPoint]->SetCoord_SumZero();

		
		/*--- Loop over Interior edges ---*/
		for(iEdge = 0; iEdge < nEdge; iEdge++) {	
			iPoint = edge[iEdge]->GetNode(0);
			Coord_i = node[iPoint]->GetCoord();

			jPoint = edge[iEdge]->GetNode(1);
			Coord_j = node[jPoint]->GetCoord();

			/*--- Accumulate nearest neighbor Coord to Res_sum for each variable ---*/
			node[iPoint]->AddCoord_Sum(Coord_j);
			node[jPoint]->AddCoord_Sum(Coord_i);
			
		}

		/*--- Loop over all mesh points (Update Coords with averaged sum) ---*/
		for (iPoint = 0; iPoint < nPoint; iPoint++) {
			nneigh = node[iPoint]->GetnPoint();
			Coord_Sum = node[iPoint]->GetCoord_Sum();
			Coord_Old = node[iPoint]->GetCoord_Old();
			
			if (nDim == 2) {
				Coord[0] =(Coord_Old[0] + val_smooth_coeff*Coord_Sum[0]) /(1.0 + val_smooth_coeff*double(nneigh));
				Coord[1] =(Coord_Old[1] + val_smooth_coeff*Coord_Sum[1]) /(1.0 + val_smooth_coeff*double(nneigh));
				if ((NearField) && ((Coord_Old[1] > Position_Plane-eps) && (Coord_Old[1] < Position_Plane+eps)))
					Coord[1] = Coord_Old[1];
			}
			
			if (nDim == 3) {
				Coord[0] =(Coord_Old[0] + val_smooth_coeff*Coord_Sum[0]) /(1.0 + val_smooth_coeff*double(nneigh));
				Coord[1] =(Coord_Old[1] + val_smooth_coeff*Coord_Sum[1]) /(1.0 + val_smooth_coeff*double(nneigh));
				Coord[2] =(Coord_Old[2] + val_smooth_coeff*Coord_Sum[2]) /(1.0 + val_smooth_coeff*double(nneigh));
				if ((NearField) && ((Coord_Old[2] > Position_Plane-eps) && (Coord_Old[2] < Position_Plane+eps)))
					Coord[2] = Coord_Old[2];
			}
				
			node[iPoint]->SetCoord(Coord);
		}

		/*--- Copy boundary values ---*/
		for (iMarker = 0; iMarker < nMarker; iMarker++)
			for (iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
				iPoint = vertex[iMarker][iVertex]->GetNode();
				Coord_Old = node[iPoint]->GetCoord_Old();
				node[iPoint]->SetCoord(Coord_Old);
			}
	}

	delete [] Coord;
}

bool CPhysicalGeometry::FindFace(unsigned long first_elem, unsigned long second_elem,
										   unsigned short &face_first_elem, 
										   unsigned short &face_second_elem) {
	
	/*--- Find repeated nodes between two elements to identify the common face ---*/
	unsigned long face[4], face_poin[4], iPoint = 0, jPoint = 0;
	unsigned short face_node, iFace, iNode, jNode, kNode, nNodesFace;
	bool face_found;
	
	face_found = false;
	
	if (first_elem == second_elem) return 0;

	kNode = 0;
	for (iNode = 0; iNode < elem[first_elem]->GetnNodes(); iNode++) {
		iPoint = elem[first_elem]->GetNode(iNode);
		for (jNode = 0; jNode < elem[second_elem]->GetnNodes(); jNode++) {
			jPoint = elem[second_elem]->GetNode(jNode);
			if (iPoint == jPoint) { 
				face[kNode] = iPoint;
				kNode++; break; }
		}
	}

	/*--- Sort elements in face ---*/
	sort(face, face+kNode);

	/*--- Busca la secuencia de la cara en las caras del primer elemento ---*/
	for (iFace = 0; iFace < elem[first_elem]->GetnFaces(); iFace++) {
		nNodesFace = elem[first_elem]->GetnNodesFace(iFace);
		for (iNode = 0; iNode < nNodesFace; iNode++) {
			face_node = elem[first_elem]->GetFaces(iFace,iNode);
			face_poin[iNode] = elem[first_elem]->GetNode(face_node);
		}
		/*--- Sort face_poin to perform comparison ---*/
		sort(face_poin, face_poin+nNodesFace);

		/*--- List comparison ---*/
		if (nNodesFace == kNode)
			if (equal (face_poin, face_poin+nNodesFace, face)) { 
				face_first_elem = iFace;
				face_found = true;
				break;
			}
	}

	/*--- Busca la secuencia de la cara en las caras del segundo ---*/
	for (iFace = 0; iFace < elem[second_elem]->GetnFaces(); iFace++) {
		nNodesFace = elem[second_elem]->GetnNodesFace(iFace);
		for (iNode = 0; iNode < nNodesFace; iNode++) {
			face_node = elem[second_elem]->GetFaces(iFace,iNode);
			face_poin[iNode] = elem[second_elem]->GetNode(face_node);
		}
		/*--- Sort face_poin to perform comparison ---*/
		sort(face_poin, face_poin+nNodesFace);
		
		/*--- List comparison ---*/
		if (nNodesFace == kNode)
			if (equal (face_poin, face_poin+nNodesFace, face)) { 
				face_second_elem = iFace;
				face_found = (face_found && true);
				break;
			}
	}

	if (face_found) return true;
	else return false;

}

void CPhysicalGeometry::SetBoundParaView (CConfig *config, char mesh_filename[200]) {
	ofstream Paraview_File;
	unsigned long iPoint, Total_nElem_Bound, Total_nElem_Bound_Storage, iElem, *PointSurface = NULL, nPointSurface = 0;
	unsigned short Coord_i, iMarker, iNode;

	/*--- It is important to do a renumering to don't add points 
	 that do not belong to the surfaces ---*/
	PointSurface = new unsigned long[nPoint];
	for (iPoint = 0; iPoint < nPoint; iPoint++)
		if (node[iPoint]->GetBoundary()) {
			PointSurface[iPoint] = nPointSurface;
			nPointSurface++;
		}
			
	/*--- Open the paraview file ---*/
	Paraview_File.open(mesh_filename, ios::out);

	/*--- Write the header of the file ---*/
	Paraview_File << "# vtk DataFile Version 2.0" << endl;
	Paraview_File << "Visualization of the surface grid (using local numbering)" << endl;
	Paraview_File << "ASCII" << endl;
	Paraview_File << "DATASET UNSTRUCTURED_GRID" << endl;
	Paraview_File << "POINTS " << nPointSurface << " float" << endl;

	/*--- Only write the coordiantes of the points that are on the surfaces ---*/
	if (nDim == 3) {
		for(iPoint = 0; iPoint < nPoint; iPoint++)
			if (node[iPoint]->GetBoundary()) {
				for(Coord_i = 0; Coord_i < nDim-1; Coord_i++)
					Paraview_File << node[iPoint]->GetCoord(Coord_i) << "\t";
				Paraview_File << node[iPoint]->GetCoord(nDim-1) << "\n";
			}
	}
	else {
		for(iPoint = 0; iPoint < nPoint; iPoint++)
			if (node[iPoint]->GetBoundary()){
				for(Coord_i = 0; Coord_i < nDim; Coord_i++)
					Paraview_File << node[iPoint]->GetCoord(Coord_i) << "\t";
				Paraview_File << "0.0\n";
			}
	}
	
	/*--- Compute the total number of elements ---*/
	Total_nElem_Bound = 0; Total_nElem_Bound_Storage = 0;
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if (config->GetMarker_All_Plotting(iMarker) == YES) {
			Total_nElem_Bound += nElem_Bound[iMarker];
			Total_nElem_Bound_Storage += nElem_Bound_Storage[iMarker];
		}
	
	/*--- Write the cells using the new numbering ---*/
	Paraview_File << "CELLS " << Total_nElem_Bound << "\t" << Total_nElem_Bound_Storage << endl;
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++) 
		if (config->GetMarker_All_Plotting(iMarker) == YES) 
			for(iElem = 0; iElem < nElem_Bound[iMarker]; iElem++) {
				Paraview_File << bound[iMarker][iElem]->GetnNodes() << "\t";
				for(iNode = 0; iNode < bound[iMarker][iElem]->GetnNodes()-1; iNode++)
					Paraview_File << PointSurface[bound[iMarker][iElem]->GetNode(iNode)] << "\t";			
				Paraview_File << PointSurface[bound[iMarker][iElem]->GetNode(bound[iMarker][iElem]->GetnNodes()-1)] << "\n";
			}
		
	Paraview_File << "CELL_TYPES " << Total_nElem_Bound << endl;
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++) 
		if (config->GetMarker_All_Plotting(iMarker) == YES) 
			for(iElem = 0; iElem < nElem_Bound[iMarker]; iElem++)
				Paraview_File << bound[iMarker][iElem]->GetVTK_Type() << endl;

	Paraview_File << "POINT_DATA " << nPointSurface << endl;

	/*--- Dealocate memory and close the file ---*/
	delete [] PointSurface;
	Paraview_File.close();
}

void CPhysicalGeometry::SetBoundTecPlot (CConfig *config, char mesh_filename[200]) {
	ofstream Tecplot_File;
	unsigned long iPoint, Total_nElem_Bound, iElem, *PointSurface = NULL, nPointSurface = 0;
	unsigned short Coord_i, iMarker;
	
	/*--- It is important to do a renumering to don't add points 
	 that do not belong to the surfaces ---*/
	PointSurface = new unsigned long[nPoint];
	for (iPoint = 0; iPoint < nPoint; iPoint++)
		if (node[iPoint]->GetBoundary()) {
			PointSurface[iPoint] = nPointSurface;
			nPointSurface++;
		}
	
	/*--- Compute the total number of elements ---*/
	Total_nElem_Bound = 0;
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if (config->GetMarker_All_Plotting(iMarker) == YES) {
			Total_nElem_Bound += nElem_Bound[iMarker];
		}
	
	/*--- Open the paraview file ---*/
	Tecplot_File.open(mesh_filename, ios::out);
	Tecplot_File << "TITLE = \"Visualization of the surface grid\"" << endl;

	/*--- Write the header of the file ---*/
	if (nDim == 2) {
		Tecplot_File << "VARIABLES = \"x\",\"y\" " << endl;
		Tecplot_File << "ZONE NODES= "<< nPointSurface <<", ELEMENTS= "<< Total_nElem_Bound <<", DATAPACKING=POINT, ZONETYPE=FELINESEG"<< endl;
	}
	if (nDim == 3) {
		Tecplot_File << "VARIABLES = \"x\",\"y\",\"z\" " << endl;	
		Tecplot_File << "ZONE NODES= "<< nPointSurface <<", ELEMENTS= "<< Total_nElem_Bound <<", DATAPACKING=POINT, ZONETYPE=FEQUADRILATERAL"<< endl;
	}
		
	/*--- Only write the coordiantes of the points that are on the surfaces ---*/
	if (nDim == 3) {
		for(iPoint = 0; iPoint < nPoint; iPoint++)
			if (node[iPoint]->GetBoundary()) {
				for(Coord_i = 0; Coord_i < nDim-1; Coord_i++)
					Tecplot_File << node[iPoint]->GetCoord(Coord_i) << "\t";
				Tecplot_File << node[iPoint]->GetCoord(nDim-1) << "\n";
			}
	}
	else {
		for(iPoint = 0; iPoint < nPoint; iPoint++)
			if (node[iPoint]->GetBoundary()){
				for(Coord_i = 0; Coord_i < nDim; Coord_i++)
					Tecplot_File << node[iPoint]->GetCoord(Coord_i) << "\t";
				Tecplot_File << "\n";
			}
	}
	
	/*--- Write the cells using the new numbering ---*/
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++) 
		if (config->GetMarker_All_Plotting(iMarker) == YES) 
			for(iElem = 0; iElem < nElem_Bound[iMarker]; iElem++) {
				if (nDim == 2) {
						Tecplot_File << PointSurface[bound[iMarker][iElem]->GetNode(0)]+1 << "\t" 
						<< PointSurface[bound[iMarker][iElem]->GetNode(1)]+1 << endl; 
				}
				if (nDim == 3) {
					if (bound[iMarker][iElem]->GetnNodes() == 3) {
						Tecplot_File << PointSurface[bound[iMarker][iElem]->GetNode(0)]+1 << "\t" 
						<< PointSurface[bound[iMarker][iElem]->GetNode(1)]+1 << "\t" 
						<< PointSurface[bound[iMarker][iElem]->GetNode(2)]+1 << "\t" 
						<< PointSurface[bound[iMarker][iElem]->GetNode(2)]+1 << endl; 
					}
					if (bound[iMarker][iElem]->GetnNodes() == 4) {
						Tecplot_File << PointSurface[bound[iMarker][iElem]->GetNode(0)]+1 << "\t" 
						<< PointSurface[bound[iMarker][iElem]->GetNode(1)]+1 << "\t" 
						<< PointSurface[bound[iMarker][iElem]->GetNode(2)]+1 << "\t" 
						<< PointSurface[bound[iMarker][iElem]->GetNode(3)]+1 << endl; 
					}
				}
			}
	
	/*--- Dealocate memory and close the file ---*/
	delete [] PointSurface;
	Tecplot_File.close();
}

void CPhysicalGeometry::SetBoundSTL (CConfig *config, char mesh_filename[200]) {
	ofstream STL_File;
	unsigned long this_node, iNode, nNode, iElem;
	unsigned short iDim, iMarker;
	double p[3], u[3], v[3], n[3], a;

	/*---	STL format:
			solid NAME
			  ...
			  facet normal 0.00 0.00 1.00
				outer loop
				  vertex  2.00  2.00  0.00
				  vertex -1.00  1.00  0.00
				  vertex  0.00 -1.00  0.00
				endloop
			  endfacet
			  ...
			end solid    
	--- */
		
	/*--- Open the STL file ---*/
	STL_File.open(mesh_filename, ios::out);
	
	/*--- Write the header of the file ---*/
	STL_File << "solid surface_mesh" << endl;
	
	/*--- Write facets of surface markers ---*/
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++) 
		if (config->GetMarker_All_Plotting(iMarker) == YES) 
			for(iElem = 0; iElem < nElem_Bound[iMarker]; iElem++) {
				
				/*--- number of nodes for this elemnt ---*/
				nNode = bound[iMarker][iElem]->GetnNodes();

				/*--- Calculate Normal Vector ---*/
				for (iDim=0; iDim<nDim; iDim++){
					p[0] = node[bound[iMarker][iElem]->GetNode(0)]      ->GetCoord(iDim);
					p[1] = node[bound[iMarker][iElem]->GetNode(1)]      ->GetCoord(iDim);
					p[2] = node[bound[iMarker][iElem]->GetNode(nNode-1)]->GetCoord(iDim);
					/*cout << p[0] <<endl;
					cout << p[1] <<endl;
					cout << p[2] <<endl;*/
					u[iDim] = p[1]-p[0];
					v[iDim] = p[2]-p[0];
				}

				n[0] = u[1]*v[2]-u[2]*v[1];
				n[1] = u[2]*v[0]-u[0]*v[2];
				n[2] = u[0]*v[1]-u[1]*v[0];
				a = sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
				/*cout << n[0] <<endl;
				cout << n[1] <<endl;
				cout << n[2] <<endl;
				cout << a << endl;*/

				/*--- Print normal vector ---*/
				STL_File << "  facet normal ";
				for (iDim=0; iDim<nDim; iDim++){
					STL_File << n[iDim]/a << " "; 
				}
				STL_File << endl;

				/*--- STL Facet Loop --*/
				STL_File << "    outer loop" << endl;

				/*--- Print Nodes for Facet ---*/
				for(iNode=0; iNode<nNode; iNode++) {
					this_node = bound[iMarker][iElem]->GetNode(iNode);
					STL_File << "      vertex ";
					for (iDim = 0; iDim < nDim; iDim++)
                        STL_File << node[this_node]->GetCoord(iDim) << " ";
					if (nDim==2)
						STL_File << 0.0 << " ";
                    STL_File <<  endl;
				}
				STL_File << "    endloop" << endl;
				STL_File << "  endfacet" << endl;
			}

	/*--- Done with Surface Mesh ---*/
	STL_File << "endsolid" << endl;
	
	/*--- Close the file ---*/
	STL_File.close();
}

void CPhysicalGeometry::SetColorGrid(CConfig *config, unsigned short val_ndomain) {
	
#ifndef NO_METIS
	
	unsigned long iPoint, iVertex, iElem, iElem_Triangle, iElem_Tetrahedron;
  unsigned long nElem_Triangle, nElem_Tetrahedron, kPoint, jPoint;
	unsigned short iMarker, iMaxColor = 0, iColor, MaxColor = 0, iNode, jNode;
	int ne = 0, nn, *elmnts = NULL, etype, *epart = NULL;
  int *npart = NULL, numflag, nparts, edgecut, *eptr;
	unsigned long *RepColor = new unsigned long[val_ndomain];

  cout << endl <<"---------------------- Performing Mesh Partitioning ---------------------" << endl;
  
	nElem_Triangle = 0;
	nElem_Tetrahedron = 0;
	for (iElem = 0; iElem < GetnElem(); iElem++) {
		if (elem[iElem]->GetVTK_Type() == TRIANGLE) nElem_Triangle = nElem_Triangle + 1;
		if (elem[iElem]->GetVTK_Type() == RECTANGLE) nElem_Triangle = nElem_Triangle + 2;
		if (elem[iElem]->GetVTK_Type() == TETRAHEDRON) nElem_Tetrahedron = nElem_Tetrahedron + 1;
		if (elem[iElem]->GetVTK_Type() == HEXAHEDRON) nElem_Tetrahedron = nElem_Tetrahedron + 5;
		if (elem[iElem]->GetVTK_Type() == PYRAMID) nElem_Tetrahedron = nElem_Tetrahedron + 2; 
		if (elem[iElem]->GetVTK_Type() == WEDGE) nElem_Tetrahedron = nElem_Tetrahedron + 3; 
	}
	
	if (GetnDim() == 2) {
		ne = nElem_Triangle;
		elmnts = new int [ne*3]; 
		etype = 1;
	}
	if (GetnDim() == 3) {
		ne = nElem_Tetrahedron;
		elmnts = new int [ne*4];
		etype = 2;
	}
	
	nn = nPoint;
	numflag = 0; 
	nparts = val_ndomain;
	epart = new int [ne];
	npart = new int [nn];
  eptr  = new int[ne+1];
	if (nparts < 2) {
        cout << "The number of domains must be greater than 1!!" << endl;
        cout << "Press any key to exit..." << endl;
        cin.get(); exit(1);	
	}
	
	iElem_Triangle = 0; iElem_Tetrahedron = 0;
	for (iElem = 0; iElem < GetnElem(); iElem++) {
		if (elem[iElem]->GetVTK_Type() == TRIANGLE) {
			elmnts[3*iElem_Triangle+0]= elem[iElem]->GetNode(0);
			elmnts[3*iElem_Triangle+1]= elem[iElem]->GetNode(1);
			elmnts[3*iElem_Triangle+2]= elem[iElem]->GetNode(2);
      eptr[iElem_Triangle] = 3*iElem_Triangle;
			iElem_Triangle++;
		}
		if (elem[iElem]->GetVTK_Type() == RECTANGLE) {
			elmnts[3*iElem_Triangle+0]= elem[iElem]->GetNode(0);
			elmnts[3*iElem_Triangle+1]= elem[iElem]->GetNode(1);
			elmnts[3*iElem_Triangle+2]= elem[iElem]->GetNode(2);
      eptr[iElem_Triangle] = 3*iElem_Triangle;
			iElem_Triangle++;
			elmnts[3*iElem_Triangle+0]= elem[iElem]->GetNode(0);
			elmnts[3*iElem_Triangle+1]= elem[iElem]->GetNode(2);
			elmnts[3*iElem_Triangle+2]= elem[iElem]->GetNode(3);
      eptr[iElem_Triangle] = 3*iElem_Triangle;
			iElem_Triangle++;
		}
		if (elem[iElem]->GetVTK_Type() == TETRAHEDRON) {
			elmnts[4*iElem_Tetrahedron+0]= elem[iElem]->GetNode(0);
			elmnts[4*iElem_Tetrahedron+1]= elem[iElem]->GetNode(1);
			elmnts[4*iElem_Tetrahedron+2]= elem[iElem]->GetNode(2);
			elmnts[4*iElem_Tetrahedron+3]= elem[iElem]->GetNode(3);
      eptr[iElem_Tetrahedron] = 4*iElem_Tetrahedron;
			iElem_Tetrahedron++;
		}
		if (elem[iElem]->GetVTK_Type() == HEXAHEDRON) {
			elmnts[4*iElem_Tetrahedron+0]= elem[iElem]->GetNode(0);
			elmnts[4*iElem_Tetrahedron+1]= elem[iElem]->GetNode(1);
			elmnts[4*iElem_Tetrahedron+2]= elem[iElem]->GetNode(2);
			elmnts[4*iElem_Tetrahedron+3]= elem[iElem]->GetNode(5);
      eptr[iElem_Tetrahedron] = 4*iElem_Tetrahedron;
			iElem_Tetrahedron++;
			elmnts[4*iElem_Tetrahedron+0]= elem[iElem]->GetNode(0);
			elmnts[4*iElem_Tetrahedron+1]= elem[iElem]->GetNode(2);
			elmnts[4*iElem_Tetrahedron+2]= elem[iElem]->GetNode(3);
			elmnts[4*iElem_Tetrahedron+3]= elem[iElem]->GetNode(7);
      eptr[iElem_Tetrahedron] = 4*iElem_Tetrahedron;
			iElem_Tetrahedron++;
			elmnts[4*iElem_Tetrahedron+0]= elem[iElem]->GetNode(0);
			elmnts[4*iElem_Tetrahedron+1]= elem[iElem]->GetNode(5);
			elmnts[4*iElem_Tetrahedron+2]= elem[iElem]->GetNode(7);
			elmnts[4*iElem_Tetrahedron+3]= elem[iElem]->GetNode(4);
      eptr[iElem_Tetrahedron] = 4*iElem_Tetrahedron;
			iElem_Tetrahedron++;
			elmnts[4*iElem_Tetrahedron+0]= elem[iElem]->GetNode(5);
			elmnts[4*iElem_Tetrahedron+1]= elem[iElem]->GetNode(2);
			elmnts[4*iElem_Tetrahedron+2]= elem[iElem]->GetNode(7);
			elmnts[4*iElem_Tetrahedron+3]= elem[iElem]->GetNode(6);
      eptr[iElem_Tetrahedron] = 4*iElem_Tetrahedron;
			iElem_Tetrahedron++;
			elmnts[4*iElem_Tetrahedron+0]= elem[iElem]->GetNode(0);
			elmnts[4*iElem_Tetrahedron+1]= elem[iElem]->GetNode(2);
			elmnts[4*iElem_Tetrahedron+2]= elem[iElem]->GetNode(5);
			elmnts[4*iElem_Tetrahedron+3]= elem[iElem]->GetNode(7);
      eptr[iElem_Tetrahedron] = 4*iElem_Tetrahedron;
			iElem_Tetrahedron++;
		}
		if (elem[iElem]->GetVTK_Type() == PYRAMID) {
			elmnts[4*iElem_Tetrahedron+0]= elem[iElem]->GetNode(0);
			elmnts[4*iElem_Tetrahedron+1]= elem[iElem]->GetNode(1);
			elmnts[4*iElem_Tetrahedron+2]= elem[iElem]->GetNode(2);
			elmnts[4*iElem_Tetrahedron+3]= elem[iElem]->GetNode(4);
      eptr[iElem_Tetrahedron] = 4*iElem_Tetrahedron;
			iElem_Tetrahedron++;
			elmnts[4*iElem_Tetrahedron+0]= elem[iElem]->GetNode(0);
			elmnts[4*iElem_Tetrahedron+1]= elem[iElem]->GetNode(2);
			elmnts[4*iElem_Tetrahedron+2]= elem[iElem]->GetNode(3);
			elmnts[4*iElem_Tetrahedron+3]= elem[iElem]->GetNode(4);
      eptr[iElem_Tetrahedron] = 4*iElem_Tetrahedron;
			iElem_Tetrahedron++;
		}
		if (elem[iElem]->GetVTK_Type() == WEDGE) {
			elmnts[4*iElem_Tetrahedron+0]= elem[iElem]->GetNode(0);
			elmnts[4*iElem_Tetrahedron+1]= elem[iElem]->GetNode(1);
			elmnts[4*iElem_Tetrahedron+2]= elem[iElem]->GetNode(4);
			elmnts[4*iElem_Tetrahedron+3]= elem[iElem]->GetNode(2);
      eptr[iElem_Tetrahedron] = 4*iElem_Tetrahedron;
			iElem_Tetrahedron++;
			elmnts[4*iElem_Tetrahedron+0]= elem[iElem]->GetNode(0);
			elmnts[4*iElem_Tetrahedron+1]= elem[iElem]->GetNode(2);
			elmnts[4*iElem_Tetrahedron+2]= elem[iElem]->GetNode(3);
			elmnts[4*iElem_Tetrahedron+3]= elem[iElem]->GetNode(4);
      eptr[iElem_Tetrahedron] = 4*iElem_Tetrahedron;
			iElem_Tetrahedron++;
			elmnts[4*iElem_Tetrahedron+0]= elem[iElem]->GetNode(3);
			elmnts[4*iElem_Tetrahedron+1]= elem[iElem]->GetNode(4);
			elmnts[4*iElem_Tetrahedron+2]= elem[iElem]->GetNode(5);
			elmnts[4*iElem_Tetrahedron+3]= elem[iElem]->GetNode(2);
      eptr[iElem_Tetrahedron] = 4*iElem_Tetrahedron;
			iElem_Tetrahedron++;
		}
	}
  /*--- Add final value to element pointer array ---*/
  if (GetnDim() == 2) {
    eptr[ne] = 3*ne;
  } else {
    eptr[ne] = 4*ne;
  }
  
#ifdef METIS_5
	/*--- Calling METIS 5.0.2 ---*/
  int options[METIS_NOPTIONS];
  METIS_SetDefaultOptions(options);
  options[METIS_OPTION_OBJTYPE] = METIS_OBJTYPE_CUT;
  METIS_PartMeshNodal(&ne, &nn, eptr, elmnts, NULL, NULL, &nparts, NULL, NULL, &edgecut, epart, npart);
  cout << "Finished partitioning using METIS 5.0.2." << endl;
  cout << edgecut << " edge cuts." << endl;
#else
  /*--- Calling METIS 4.0.3 ---*/
  METIS_PartMeshNodal(&ne, &nn, elmnts, &etype, &numflag, &nparts, &edgecut, epart, npart);
  cout << "Finished partitioning using METIS 4.0.3." << endl;
  cout << edgecut << " edge cuts." << endl;
#endif
  
	for (iPoint = 0; iPoint < nPoint; iPoint++)
		node[iPoint]->SetColor(npart[iPoint]);
	
	/*--- Dealing with periodic boundary conditions or nearfield, an special 
	 partitioning is done ---*/
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++) {
		
		for (iColor = 0; iColor < val_ndomain; iColor++)
			RepColor[iColor] = 0;
		MaxColor = 0;
		
		if (config->GetMarker_All_Boundary(iMarker) == PERIODIC_BOUNDARY){
			
			for (iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
				iPoint = vertex[iMarker][iVertex]->GetNode();
				RepColor[int(node[iPoint]->GetColor())]++;
				for (iNode = 0; iNode < node[iPoint]->GetnPoint(); iNode++) {
					jPoint = node[iPoint]->GetPoint(iNode);
					RepColor[int(node[jPoint]->GetColor())]++;
					for (jNode = 0; jNode < node[jPoint]->GetnPoint(); jNode++) {
						kPoint = node[jPoint]->GetPoint(jNode);
						RepColor[int(node[kPoint]->GetColor())]++;
					}
				}
			}
			
			for (iColor = 0; iColor < val_ndomain; iColor++)
				if (RepColor[iColor] > MaxColor) {
					MaxColor = RepColor[iColor];
					iMaxColor = iColor;
				}

			for (iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
				iPoint = vertex[iMarker][iVertex]->GetNode();
				node[iPoint]->SetColor(iMaxColor);
				for (iNode = 0; iNode < node[iPoint]->GetnPoint(); iNode++) {
					jPoint = node[iPoint]->GetPoint(iNode);
					node[jPoint]->SetColor(iMaxColor);
					for (jNode = 0; jNode < node[jPoint]->GetnPoint(); jNode++) {
						kPoint = node[jPoint]->GetPoint(jNode);
						node[kPoint]->SetColor(iMaxColor);
					}
				}
			}
		}
	}
	
	delete [] epart;
	delete [] npart;
	delete [] RepColor;
#endif
}

void CPhysicalGeometry::SetSendReceive(CConfig *config, unsigned short val_ndomain) {
	unsigned long iElem, iPoint, jPoint, iVertex;
	unsigned short iNode, base_color, iDomain, jNode, jDomain;
	unsigned short iMarker, jMarker, dMarker = 0;
	bool ElemBound;
	vector<unsigned long>::iterator it[MAX_NUMBER_DOMAIN][MAX_NUMBER_DOMAIN];

	/*--- Bucle over all the elements of the full domain ---*/
	for (iElem = 0; iElem < nElem; iElem++) {
		
		/*--- Identify if an element has different colours ---*/
		ElemBound = false;
		base_color = node[elem[iElem]->GetNode(0)]->GetColor();
		for (iNode = 1; iNode < elem[iElem]->GetnNodes(); iNode++) {
			iPoint = elem[iElem]->GetNode(iNode);
			if ( node[iPoint]->GetColor() != base_color ) {
				ElemBound = true; break;
			}
		}
		
		/*--- Identify the points that must be sended between domains ---*/
		if (ElemBound) {
			/*--- Loop over all the points of the marker element ---*/
			for (iNode = 0; iNode < elem[iElem]->GetnNodes(); iNode++) {
				iPoint = elem[iElem]->GetNode(iNode); 
				iDomain = node[iPoint]->GetColor();
				
				/*--- Loop over the all the points of the element 
				 to find the points with different colours ---*/
				for(jNode = 0; jNode < elem[iElem]->GetnNodes(); jNode++) {
					jPoint = elem[iElem]->GetNode(jNode);
					jDomain = node[jPoint]->GetColor();
					if (iDomain != jDomain) {
						
						/*--- We send from iDomain to jDomain the value of iPoint ---*/
						SendDomain[iDomain][jDomain].push_back(iPoint);
						
						/*--- We send from jDomain to iDomain the value of jPoint ---*/
						SendDomain[jDomain][iDomain].push_back(jPoint); 
					}
				}
			}
		}
	}
	
	/*--- Sort the points that must be sended and delete repeated points ---*/
	for (iDomain = 0; iDomain < val_ndomain; iDomain++)
		for (jDomain = 0; jDomain < val_ndomain; jDomain++) {
			sort( SendDomain[iDomain][jDomain].begin(), SendDomain[iDomain][jDomain].end());
			it[iDomain][jDomain] = unique( SendDomain[iDomain][jDomain].begin(), SendDomain[iDomain][jDomain].end());
			SendDomain[iDomain][jDomain].resize( it[iDomain][jDomain] - SendDomain[iDomain][jDomain].begin() );
		}
	
	/*--- Loop through all SEND points thus far and set their transformation
	 to zero, as these are all purely MPI points, not periodic. ---*/
	for (iDomain = 0; iDomain < val_ndomain; iDomain++)
		for (jDomain = 0; jDomain < val_ndomain; jDomain++)
			for (iNode = 0; iNode < SendDomain[iDomain][jDomain].size(); iNode++)
				SendTransf[iDomain][jDomain].push_back(0);
	
	PeriodicDomainIndex = new long [nPoint];
	for (iPoint = 0; iPoint < nPoint; iPoint++)
		PeriodicDomainIndex[iPoint] = -1;

	/*--- Check for any points which are part of a preexisting periodic
	 boundary condition. These points already utilize the SEND_RECEIVE
	 subroutines for passing their data. In this case, these points
	 must be added. ---*/
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++) {
		if (config->GetMarker_All_Boundary(iMarker) == SEND_RECEIVE) {
			short SendRecv = config->GetMarker_All_SendRecv(iMarker);
			
			/*--- Only for recive ---*/
			if (SendRecv < 0) {
				
				unsigned short receive_from = abs(SendRecv);
				
				/*--- Retrieve donor marker. ---*/
				for (jMarker = 0; jMarker < config->GetnMarker_All(); jMarker++){
					if (config->GetMarker_All_SendRecv(jMarker) == receive_from) dMarker = jMarker;
				}
				
				/*--- Get the color of the current node and also the send node. ---*/
				for (iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
					
					/*--- Retrieve node information for this boundary point. ---*/
					iPoint = vertex[iMarker][iVertex]->GetNode();
					jPoint = vertex[dMarker][iVertex]->GetNode();
					
					iDomain = node[iPoint]->GetColor();
					jDomain = node[jPoint]->GetColor();
					
					/*--- Store the donor point (for sending). ---*/
					SendDomain[jDomain][iDomain].push_back(jPoint);
					
					/*--- Store the node that recive the information. ---*/
					PeriodicDomainIndex[jPoint] = iPoint;
					
					/*--- Retrieve and store the type of transformation. ---*/
					SendTransf[jDomain][iDomain].push_back(bound[dMarker][iVertex]->GetRotation_Type());
					
				}
			}
		}
	}
	
}

void CPhysicalGeometry::Set3D_to_2D (CConfig *config, char mesh_vtk[200], char mesh_su2[200], 
									 unsigned short nslices) {
	
	// nslices: number of (identical) slices in the original 3D model (usually nslices == 1)
	cout << "CPhysicalGeometry::Set3D_to_2D >> Writing 2D meshes... " ;
	
	ofstream para_file, su2_file;
	
	para_file.open(mesh_vtk, ios::out);
	para_file.precision(15);
	su2_file.open(mesh_su2, ios::out);
	su2_file.precision(15);
	
	double coord[3];
	const unsigned long nPoint_2D = nPoint/(nslices+1);
	const unsigned long nElem_2D = nElem/nslices;
	unsigned short nNodes_2D, nMarker_2D = 0;
	unsigned short idim, iMarker, Boundary;
	unsigned long ipoint, ielem;
	
	para_file << "# vtk DataFile Version 2.0" << endl;
	para_file << "Visualization of the volumetric grid" << endl;
	para_file << "ASCII" << endl;
	para_file << "DATASET UNSTRUCTURED_GRID" << endl;
	para_file << "POINTS " << nPoint_2D << " float" << endl; 
	su2_file << "NDIME=2" << endl;
	su2_file << "NPOIN=" << nPoint_2D << endl;
	
	for (ipoint = 0; ipoint<nPoint_2D; ipoint++) {
		for (idim = 0; idim < 2; idim++)
			coord[idim]=node[ipoint]->GetCoord(2*idim);
		coord[2] = 0.0;
		for (idim = 0; idim < 3; idim++) {
			para_file << coord[idim] << "\t";
			su2_file << coord[idim] << "\t";
		}
		para_file << endl;
		su2_file << ipoint << endl;
	}
	
	para_file << "CELLS " << nElem_2D << "\t" << 
	(nElem_Storage-nElem)/(nslices+1)+nElem_2D << endl;
	su2_file << "NELEM=" << nElem_2D << endl;
	for (ielem = 0; ielem < nElem_2D; ielem++) {
		nNodes_2D = elem[ielem]->GetnNodes()/2;
		para_file << nNodes_2D << "\t";
		
		if (elem[ielem]->GetnNodes()==6) su2_file << "5" << "\t";
		if (elem[ielem]->GetnNodes()==8) su2_file << "9" << "\t";
		
		if (elem[ielem]->GetNode(0) < nPoint_2D) su2_file << elem[ielem]->GetNode(0) << "\t";
		if (elem[ielem]->GetNode(1) < nPoint_2D) su2_file << elem[ielem]->GetNode(1) << "\t";
		if (elem[ielem]->GetNode(2) < nPoint_2D) su2_file << elem[ielem]->GetNode(2) << "\t";
		if (elem[ielem]->GetNode(3) < nPoint_2D) su2_file << elem[ielem]->GetNode(3) << "\t";
		if (elem[ielem]->GetNode(4) < nPoint_2D) su2_file << elem[ielem]->GetNode(4) << "\t";
		if (elem[ielem]->GetNode(5) < nPoint_2D) su2_file << elem[ielem]->GetNode(5) << "\t";
		if (elem[ielem]->GetNode(6) < nPoint_2D) su2_file << elem[ielem]->GetNode(6) << "\t";
		if (elem[ielem]->GetNode(7) < nPoint_2D) su2_file << elem[ielem]->GetNode(7) << "\t";

		para_file << endl;
		su2_file << endl;
	}	
	
	para_file << "CELL_TYPES " << nElem_2D << endl;
	for (ielem = 0; ielem < nElem_2D; ielem++) {
		switch (elem[ielem]->GetnNodes()/2) {
			case 3: para_file << "5" << endl; break;
			case 4: para_file << "9" << endl; break;
		}
	}
	
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++) {
		Boundary = config->GetMarker_All_Boundary(iMarker);
		switch(Boundary) {
			case (SYMMETRY_PLANE):
				break;
			default:
				nMarker_2D++;
				break;
		}	
	}
	
	su2_file << "NMARK=" << nMarker_2D << endl;
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++) {
		Boundary = config->GetMarker_All_Boundary(iMarker);
		switch(Boundary) {
			case (SYMMETRY_PLANE):
				break;
			default:
				su2_file << "MARKER_TAG=" << config->GetMarker_All_Tag(iMarker) << endl;
				su2_file << "MARKER_ELEMS=" << nElem_Bound[iMarker] << endl;
				for (ielem = 0; ielem < nElem_Bound[iMarker]; ielem++) {
					su2_file << "3" << "\t";
					if (bound[iMarker][ielem]->GetNode(0) < nPoint_2D) su2_file << bound[iMarker][ielem]->GetNode(0) << "\t";
					if (bound[iMarker][ielem]->GetNode(1) < nPoint_2D) su2_file << bound[iMarker][ielem]->GetNode(1) << "\t";
					if (bound[iMarker][ielem]->GetNode(2) < nPoint_2D) su2_file << bound[iMarker][ielem]->GetNode(2) << "\t";
					if (bound[iMarker][ielem]->GetNode(3) < nPoint_2D) su2_file << bound[iMarker][ielem]->GetNode(3) << "\t";
					
					su2_file << endl;
				}
				break;
		}	
	}
	
	para_file.close();
	su2_file.close();
	
	cout << "Completed." << endl;
}


void CPhysicalGeometry::GetQualityStatistics(double *statistics) {
	unsigned long jPoint, Point_2, Point_3, iElem;
	double *Coord_j, *Coord_2, *Coord_3;
	unsigned short iDim;

	statistics[0] = 1e06;
	statistics[1] = 0;

	/*--- Loop interior edges ---*/
	for (iElem = 0; iElem < this->GetnElem(); iElem++) {

		if ((this->GetnDim() == 2) && (elem[iElem]->GetVTK_Type() == TRIANGLE)) {
		
			jPoint = elem[iElem]->GetNode(0); Coord_j = node[jPoint]->GetCoord();
			Point_2 = elem[iElem]->GetNode(1); Coord_2 = node[Point_2]->GetCoord();
			Point_3 = elem[iElem]->GetNode(2); Coord_3 = node[Point_3]->GetCoord();

			/*--- Compute sides of the triangle ---*/
			double a = 0, b = 0, c = 0;
			for (iDim = 0; iDim < nDim; iDim++) {
				a += (Coord_2[iDim]-Coord_j[iDim])*(Coord_2[iDim]-Coord_j[iDim]);
				b += (Coord_3[iDim]-Coord_j[iDim])*(Coord_3[iDim]-Coord_j[iDim]);
				c += (Coord_3[iDim]-Coord_2[iDim])*(Coord_3[iDim]-Coord_2[iDim]);
			}
			a = sqrt(a); b = sqrt(b); c = sqrt(c);

			/*--- Compute semiperimeter (s) and area ---*/
			double s = 0.5*(a + b + c);
			double Area = sqrt(s*(s-a)*(s-b)*(s-c));

			/*--- Compute radius of the circumcircle (R) and of the incircle (r) ---*/
			double R = (a*b*c) / (4.0*Area);
			double r = Area / s;
			double roR = r / R;

			/*--- Update statistics ---*/
			if (roR < statistics[0])
				statistics[0] = roR;
			statistics[1] += roR;

		}
	}
	statistics[1] /= this->GetnElem();

}

void CPhysicalGeometry::SetRotationalVelocity(CConfig *config) {
  
  unsigned long iPoint;
  double RotVel[3], Distance[3], *Coord, *Axis, *Omega, L_Ref;
  
  /*--- Loop over all points and set rotational velocity.
        Note that this only need be done once for steady rotation. ---*/
  for (iPoint = 0; iPoint < nPoint; iPoint++) {
    
    /*--- Get values for this node ---*/
    Coord = node[iPoint]->GetCoord();
    Axis  = config->GetRotAxisOrigin();
    Omega = config->GetOmega_FreeStreamND();
    L_Ref = config->GetLength_Ref(); // should always be 1
    
    /*--- Calculate non-dim distance fron rotation center ---*/
    Distance[0] = (Coord[0]-Axis[0])/L_Ref;
    Distance[1] = (Coord[1]-Axis[1])/L_Ref;
    Distance[2] = (Coord[2]-Axis[2])/L_Ref;
    
    /*--- Calculate the angular velocity as omega X r ---*/
    RotVel[0] = Omega[1]*(Distance[2]) - Omega[2]*(Distance[1]);
    RotVel[1] = Omega[2]*(Distance[0]) - Omega[0]*(Distance[2]);
    RotVel[2] = Omega[0]*(Distance[1]) - Omega[1]*(Distance[0]);
    
    node[iPoint]->SetRotVel(RotVel);
    
  }
	
}


void CPhysicalGeometry::SetPeriodicBoundary(CConfig *config) {
  
	unsigned short iMarker, jMarker, kMarker = 0, iPeriodic, iDim, nPeriodic = 0, VTK_Type;
	unsigned long iNode, iIndex, iVertex, iPoint, iElem, kElem;
	unsigned long jElem, kPoint = 0, jVertex = 0, jPoint = 0, pPoint = 0, nPointPeriodic, newNodes[4];
	vector<unsigned long>::iterator IterElem[MAX_NUMBER_PERIODIC], IterPoint[MAX_NUMBER_PERIODIC][2], IterNewElem[MAX_NUMBER_MARKER];
	double *center, *angles, rotMatrix[3][3] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, 
	translation[3], *trans, theta, phi, psi, cosTheta, sinTheta, cosPhi, sinPhi, cosPsi, sinPsi, 
	dx, dy, dz, rotCoord[3], epsilon = 1e-10, mindist = 1e6, *Coord_i, *Coord_j, dist = 0.0;
	bool isBadMatch = false;
	

	/*--- Send an initial message to the console. ---*/
	cout << "Setting the periodic boundary conditions." <<endl; 
	
	/*--- Loop through each marker to find any periodic boundaries. ---*/
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if (config->GetMarker_All_Boundary(iMarker) == PERIODIC_BOUNDARY) {
			
			/*--- Evaluate the number of periodic boundary conditions defined 
            in the geometry file ---*/
			nPeriodic++;
			
			/*--- Get marker index of the periodic donor boundary. ---*/
			jMarker = config->GetMarker_Periodic_Donor(config->GetMarker_All_Tag(iMarker));
			
			/*--- Write some info to the console. ---*/
			cout << "Checking " << config->GetMarker_All_Tag(iMarker);
			cout << " boundary against periodic donor, " << config->GetMarker_All_Tag(jMarker) << ". ";
			
			/*--- Retrieve the supplied periodic information. ---*/
			center = config->GetPeriodicRotCenter(config->GetMarker_All_Tag(iMarker));
			angles = config->GetPeriodicRotAngles(config->GetMarker_All_Tag(iMarker));
			trans  = config->GetPeriodicTranslation(config->GetMarker_All_Tag(iMarker));
			
			/*--- Store (center+trans) as it is constant and will be added on. ---*/
			translation[0] = center[0] + trans[0];
			translation[1] = center[1] + trans[1];
			translation[2] = center[2] + trans[2];
			
			/*--- Store angles separately for clarity. Compute sines/cosines. ---*/
			theta = angles[0];   
			phi   = angles[1]; 
			psi   = angles[2];
			
			cosTheta = cos(theta);  cosPhi = cos(phi);  cosPsi = cos(psi);
			sinTheta = sin(theta);  sinPhi = sin(phi);  sinPsi = sin(psi);
			
			/*--- Compute the rotation matrix. Note that the implicit
            ordering is rotation about the x-axis, y-axis, then z-axis. ---*/
			rotMatrix[0][0] = cosPhi*cosPsi;
			rotMatrix[1][0] = cosPhi*sinPsi;
			rotMatrix[2][0] = -sinPhi;
			
			rotMatrix[0][1] = sinTheta*sinPhi*cosPsi - cosTheta*sinPsi;
			rotMatrix[1][1] = sinTheta*sinPhi*sinPsi + cosTheta*cosPsi;
			rotMatrix[2][1] = sinTheta*cosPhi;
			
			rotMatrix[0][2] = cosTheta*sinPhi*cosPsi + sinTheta*sinPsi;
			rotMatrix[1][2] = cosTheta*sinPhi*sinPsi - sinTheta*cosPsi;
			rotMatrix[2][2] = cosTheta*cosPhi;
			
			/*--- Loop through all vertices and find/set the periodic point. ---*/
			for (iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
				
				/*--- Retrieve node information for this boundary point. ---*/
				iPoint  = vertex[iMarker][iVertex]->GetNode();
				Coord_i = node[iPoint]->GetCoord();
				
				/*--- Get the position vector from rot center to point. ---*/
				dx = Coord_i[0] - center[0]; 
				dy = Coord_i[1] - center[1]; 
        if (nDim == 3) {
          dz = Coord_i[2] - center[2];
        } else {
          dz = 0.0;
        }
				
				/*--- Compute transformed point coordinates. ---*/
				rotCoord[0] = rotMatrix[0][0]*dx 
                    + rotMatrix[0][1]*dy 
                    + rotMatrix[0][2]*dz + translation[0];
        
				rotCoord[1] = rotMatrix[1][0]*dx 
                    + rotMatrix[1][1]*dy 
                    + rotMatrix[1][2]*dz + translation[1];
        
				rotCoord[2] = rotMatrix[2][0]*dx 
                    + rotMatrix[2][1]*dy 
                    + rotMatrix[2][2]*dz + translation[2];
				
				/*--- Perform a search to find the closest donor point. ---*/
				mindist = 1e10;
				for (jVertex = 0; jVertex < nVertex[jMarker]; jVertex++) {
					
					/*--- Retrieve information for this jPoint. ---*/
					jPoint = vertex[jMarker][jVertex]->GetNode();
					Coord_j = node[jPoint]->GetCoord();
					
					/*--- Check the distance between the computed periodic
					 location and this jPoint. ---*/
          dist = 0.0;
          for (iDim = 0; iDim < nDim; iDim++){
            dist += (Coord_j[iDim]-rotCoord[iDim])*(Coord_j[iDim]-rotCoord[iDim]);
          }
					dist = sqrt(dist);
					
					/*---  Store vertex information if this is the closest
					 point found thus far. ---*/
					if (dist < mindist) { mindist = dist; pPoint = jPoint; }
				}
				
				/*--- Set the periodic point for this iPoint. ---*/
				vertex[iMarker][iVertex]->SetPeriodicPoint(pPoint);
				
				/*--- Print warning if the nearest point was not within
              the specified tolerance. Computation will continue. ---*/
				if (mindist > epsilon) {
					isBadMatch = true;
					cout.precision(10);
					cout << endl;
					cout << "   Bad match for point " << iPoint << ".\tNearest";
					cout << " donor distance: " << scientific << mindist << ".";
				}
			}
			
			/*--- Print final warning when finding bad matches. ---*/
			if (isBadMatch) {
				cout << endl;
        cout << "\n !!! Warning !!!" << endl;
        cout << "Bad matches found. Computation will continue, but be cautious.\n";
			}
			cout << endl;
			isBadMatch = false;
			
		}

	/*--- Create a vector to identify the points that belong to each periodic boundary condtion ---*/
	bool *PeriodicBC = new bool [nPoint];
	for (iPoint = 0; iPoint < nPoint; iPoint++) PeriodicBC[iPoint] = false;
	
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if (config->GetMarker_All_Boundary(iMarker) == PERIODIC_BOUNDARY)
			for (iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
				iPoint = vertex[iMarker][iVertex]->GetNode();
				PeriodicBC[iPoint] = true;
			}
	
	/*--- Determine the new points that must be added to each periodic boundary ---*/
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if (config->GetMarker_All_Boundary(iMarker) == PERIODIC_BOUNDARY) {
			
			/*--- An integer identify the periodic boundary condition ---*/
			iPeriodic = config->GetMarker_All_PerBound(iMarker);
			
			for (iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
				
				/*--- iPoint is the original point on the surface and jPoint is the 
				 equivalent point in the other periodic surface ---*/
				iPoint = vertex[iMarker][iVertex]->GetNode();
				jPoint = vertex[iMarker][iVertex]->GetPeriodicPoint();

				/*--- Now we must determine the neighbor points to the periodic points 
				 and store all the information (in this list we do not include the points 
				 that already belong to the periodic boundary ---*/
				for (iIndex = 0; iIndex < node[jPoint]->GetnPoint(); iIndex++) {
					kPoint = node[jPoint]->GetPoint(iIndex);

					/*--- We only add those points that do not belong to any 
					 periodic boundary condition ---*/
					if (!PeriodicBC[kPoint]) PeriodicPoint[iPeriodic][0].push_back(kPoint);
				}
				
				/*--- Now we add the elements that share a point with the periodic 
				 boundary condition ---*/
				for (iIndex = 0; iIndex < node[jPoint]->GetnElem(); iIndex++) {
					iElem = node[jPoint]->GetElem(iIndex);
					PeriodicElem[iPeriodic].push_back(iElem);
				}
			}
		}
	
	/*--- Sort the points that must be sended and delete repeated points ---*/
	for (iPeriodic = 1; iPeriodic <= nPeriodic; iPeriodic++) {
		sort( PeriodicPoint[iPeriodic][0].begin(), PeriodicPoint[iPeriodic][0].end());
		IterPoint[iPeriodic][0] = unique( PeriodicPoint[iPeriodic][0].begin(), PeriodicPoint[iPeriodic][0].end());
		PeriodicPoint[iPeriodic][0].resize( IterPoint[iPeriodic][0] - PeriodicPoint[iPeriodic][0].begin() );
	}
		
	/*--- Create a list of the points that recieve the values (new points) ---*/
	nPointPeriodic = nPoint;
	for (iPeriodic = 1; iPeriodic <= nPeriodic; iPeriodic++)
		for (iPoint = 0; iPoint < PeriodicPoint[iPeriodic][0].size(); iPoint++) {
			PeriodicPoint[iPeriodic][1].push_back(nPointPeriodic);
			nPointPeriodic++;
		}

	/*--- Sort the elements that must be replicated in the periodic boundary 
	 and delete the repeated elements ---*/
	for (iPeriodic = 1; iPeriodic <= nPeriodic; iPeriodic++) {
		sort( PeriodicElem[iPeriodic].begin(), PeriodicElem[iPeriodic].end());
		IterElem[iPeriodic] = unique( PeriodicElem[iPeriodic].begin(), PeriodicElem[iPeriodic].end());
		PeriodicElem[iPeriodic].resize( IterElem[iPeriodic] - PeriodicElem[iPeriodic].begin() );
	}
	
  /*--- Check all SEND points to see if they also lie on another boundary. ---*/
  for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++) {
    for (iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
      
      /*--- iPoint is a node that lies on the current marker. ---*/
      iPoint = vertex[iMarker][iVertex]->GetNode();
      
      /*--- Search through SEND points to check for iPoint. ---*/
      for (iPeriodic = 1; iPeriodic <= nPeriodic; iPeriodic++) {
        
        /*--- jPoint is the SEND point. ---*/
        for (iElem = 0; iElem < PeriodicPoint[iPeriodic][0].size(); iElem++) {
          jPoint = PeriodicPoint[iPeriodic][0][iElem];
          
          /*--- If the two match, then jPoint lies on this boundary.
                However, we are concerned with the new points, so we
                will store kPoint instead. ---*/
          if (iPoint == jPoint) {
            kPoint = PeriodicPoint[iPeriodic][1][iElem];
            
            /*--- We also want the type of boundary element that this point
                  was within, so that we know what type of element to add
                  built from the new points. ---*/
            bool isJPoint, isPeriodic;
            for(jElem = 0; jElem < nElem_Bound[iMarker]; jElem++) {
              isJPoint = false; isPeriodic = false;
              for (iNode = 0; iNode < bound[iMarker][jElem]->GetnNodes(); iNode++) {
                if (bound[iMarker][jElem]->GetNode(iNode) == jPoint) isJPoint = true;
                if (PeriodicBC[bound[iMarker][jElem]->GetNode(iNode)]) isPeriodic = true;
              }
              
              /*--- If both points were found, store this element. ---*/
              if (isJPoint && isPeriodic) {
                OldBoundaryElems[iMarker].push_back(jElem);                
              }
              
            }
            
          }
        }
      }
    }
	}
  
  /*--- Sort the elements that must be added and remove duplicates. ---*/
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++) {
		sort( OldBoundaryElems[iMarker].begin(), OldBoundaryElems[iMarker].end());
		IterNewElem[iMarker] = unique( OldBoundaryElems[iMarker].begin(), OldBoundaryElems[iMarker].end());
		OldBoundaryElems[iMarker].resize( IterNewElem[iMarker] - OldBoundaryElems[iMarker].begin() );
	}
  
  /*--- Create the new boundary elements. Points making up these new
        elements must either be SEND/RECEIVE or periodic points. ---*/
  nNewElem_Bound = new unsigned long[nMarker];
  newBound = new CPrimalGrid**[nMarker];
  for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++) {
    nNewElem_Bound[iMarker] = OldBoundaryElems[iMarker].size();
    newBound[iMarker]       = new CPrimalGrid*[nNewElem_Bound[iMarker]];

    /*--- Loop through all new elements to be added. ---*/
    for (iElem = 0; iElem < nNewElem_Bound[iMarker]; iElem++) {
      jElem = OldBoundaryElems[iMarker][iElem];
      
      /*--- Loop through all nodes of this element. ---*/
      for (iNode = 0; iNode < bound[iMarker][jElem]->GetnNodes(); iNode++) {
        pPoint = bound[iMarker][jElem]->GetNode(iNode);
        
        /*--- Check if this node is a send point. If so, the corresponding
              receive point will be used in making the new boundary element. ---*/
        for (iPeriodic = 1; iPeriodic <= nPeriodic; iPeriodic++) {
          for (kElem = 0; kElem < PeriodicPoint[iPeriodic][0].size(); kElem++) {
            if (pPoint == PeriodicPoint[iPeriodic][0][kElem]) newNodes[iNode] = PeriodicPoint[iPeriodic][1][kElem];
          }
        }
        
        /*--- Check if this node is a periodic point. If so, the corresponding
              periodic point will be used in making the new boundary element. ---*/
        if (PeriodicBC[pPoint]) {

          /*--- Find the corresponding periodic point. ---*/
          for (jMarker = 0; jMarker < config->GetnMarker_All(); jMarker++) {
            if (config->GetMarker_All_Boundary(jMarker) == PERIODIC_BOUNDARY) {
              for (iVertex = 0; iVertex < nVertex[jMarker]; iVertex++) {
                if (pPoint == vertex[jMarker][iVertex]->GetNode()) {kMarker = jMarker; jVertex = iVertex;}
              }
            }
          }
          newNodes[iNode] = vertex[kMarker][jVertex]->GetPeriodicPoint();          
        }
      }
      
      /*--- Now instantiate the new element. ---*/
      VTK_Type = bound[iMarker][jElem]->GetVTK_Type();
      switch(VTK_Type) {
        case LINE:
          newBound[iMarker][iElem] = new CLine(newNodes[0],newNodes[1],2);
          break;
        case TRIANGLE:
          newBound[iMarker][iElem] = new CTriangle(newNodes[0],newNodes[1],newNodes[2],3);
          break;
        case RECTANGLE: 
          newBound[iMarker][iElem] = new CRectangle(newNodes[0],newNodes[1],newNodes[2],newNodes[3],3);
          break;
      }
      
    }
  }
  
	delete [] PeriodicBC;
	
}

void CPhysicalGeometry::FindSharpEdges(CConfig *config) {
  
	unsigned short iMarker, iNeigh, iDim, Neighbor_Counter = 0;
	unsigned long Neighbor_Point, iVertex, iPoint;  
  double dot_product, dihedral_angle, avg_dihedral;
  double Coord_Vertex_i[3], Coord_Vertex_j[3], Unit_Normal[2][3], area;
	
  /*--- IMPORTANT: Sharp corner angle threshold as a multiple of the average ---*/
  double angle_threshold = 10.0;
  
  if (nDim == 2) {
    
    /*--- Loop over all the markers ---*/
    for (iMarker = 0; iMarker < nMarker; iMarker++) {
      
      avg_dihedral = 0.0;
      
      /*--- Create a vector to identify the points on this marker ---*/
      bool *Surface_Node = new bool[nPoint];
      for (iPoint = 0; iPoint < nPoint; iPoint++) Surface_Node[iPoint] = false;
      
      /*--- Loop through and flag all global nodes on this marker ---*/
      for (iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
				iPoint  = vertex[iMarker][iVertex]->GetNode();
        Surface_Node[iPoint] = true;
      } 
      
      /*--- Now loop through all marker vertices again, this time also 
       finding the neighbors of each node that share this marker.---*/
      for (iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
        iPoint  = vertex[iMarker][iVertex]->GetNode();
        
        /*--- Loop through neighbors. In 2-D, there should be 2 nodes on either
         side of this vertex that lie on the same surface. ---*/
        Neighbor_Counter = 0;
        for (iNeigh = 0; iNeigh < node[iPoint]->GetnPoint(); iNeigh++) {
          Neighbor_Point = node[iPoint]->GetPoint(iNeigh);
          
          /*--- Check if this neighbor lies on the surface. If so, compute
           the surface normal for the edge that the nodes share. ---*/
          if (Surface_Node[Neighbor_Point]) {
            for (iDim = 0; iDim < nDim; iDim++) {
              Coord_Vertex_i[iDim]  = node[iPoint]->GetCoord(iDim);
              Coord_Vertex_j[iDim]  = node[Neighbor_Point]->GetCoord(iDim);
            }
            
            /*--- The order of the two points matters when computing the normal ---*/
            if (Neighbor_Counter == 0) {
              Unit_Normal[Neighbor_Counter][0] = Coord_Vertex_i[1]-Coord_Vertex_j[1];
              Unit_Normal[Neighbor_Counter][1] = -(Coord_Vertex_i[0]-Coord_Vertex_j[0]);
            } else if (Neighbor_Counter == 1) {
              Unit_Normal[Neighbor_Counter][0] = Coord_Vertex_j[1]-Coord_Vertex_i[1];
              Unit_Normal[Neighbor_Counter][1] = -(Coord_Vertex_j[0]-Coord_Vertex_i[0]);
            }
            
            /*--- Store as a unit normal ---*/
            area = 0.0;
            for (iDim = 0; iDim < nDim; iDim++) 
              area += Unit_Normal[Neighbor_Counter][iDim]*Unit_Normal[Neighbor_Counter][iDim];
            area = sqrt(area);
            for (iDim = 0; iDim < nDim; iDim++) Unit_Normal[Neighbor_Counter][iDim] /= area;
            
            /*--- Increment neighbor counter ---*/
            Neighbor_Counter++;
          }
        }
        
        /*--- Now we have the two edge normals that we need to compute the
         dihedral angle about this vertex. ---*/
        dot_product = 0.0;
        for (iDim = 0; iDim < nDim; iDim++)
          dot_product += Unit_Normal[0][iDim]*Unit_Normal[1][iDim];
        dihedral_angle = acos(dot_product);
        vertex[iMarker][iVertex]->SetAuxVar(dihedral_angle);
        avg_dihedral += dihedral_angle/(double)nVertex[iMarker];
      }
      
      /*--- Check criteria and set sharp corner boolean for each vertex ---*/
      for (iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
        if (vertex[iMarker][iVertex]->GetAuxVar() > angle_threshold*avg_dihedral) {
          iPoint  = vertex[iMarker][iVertex]->GetNode();
          for (iDim = 0; iDim < nDim; iDim++)
            Coord_Vertex_i[iDim] = node[iPoint]->GetCoord(iDim);
          vertex[iMarker][iVertex]->SetSharp_Corner(true);
          cout.precision(6);
          cout << "  Found a sharp corner at point (" << Coord_Vertex_i[0];
          cout << ", " << Coord_Vertex_i[1] << ")" << endl;
        }
      }
      
      delete [] Surface_Node;
    }
    
  } else {
    /*--- Do nothing in 3-D at the moment. ---*/
  }
  
}

void CPhysicalGeometry::FindClosestNeighbor(CConfig *config) {
	
	unsigned short iMarker, iDim;
	unsigned long iPoint, iVertex;
	
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if (config->GetMarker_All_Boundary(iMarker) != SEND_RECEIVE &&
				config->GetMarker_All_Boundary(iMarker) != INTERFACE_BOUNDARY &&
				config->GetMarker_All_Boundary(iMarker) != NEARFIELD_BOUNDARY )			
			for (iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
				
				iPoint = vertex[iMarker][iVertex]->GetNode();
				
				/*--- If the node belong to the domain ---*/
				if (node[iPoint]->GetDomain()) {
					
					/*--- Compute closest normal neighbor ---*/
					double cos_max, scalar_prod, norm_vect, norm_Normal, cos_alpha, diff_coord;
					unsigned long Point_Normal = 0, jPoint;
					unsigned short iNeigh;
					double *Normal = vertex[iMarker][iVertex]->GetNormal();
					cos_max = -1.0;
					for (iNeigh = 0; iNeigh < node[iPoint]->GetnPoint(); iNeigh++) {
						jPoint = node[iPoint]->GetPoint(iNeigh);
						scalar_prod = 0.0; norm_vect = 0.0; norm_Normal = 0.0;
						for(iDim = 0; iDim < GetnDim(); iDim++) {
							diff_coord = node[jPoint]->GetCoord(iDim)-node[iPoint]->GetCoord(iDim);
							scalar_prod += diff_coord*Normal[iDim];
							norm_vect += diff_coord*diff_coord;
							norm_Normal += Normal[iDim]*Normal[iDim];
						}
						norm_vect = sqrt(norm_vect);
						norm_Normal = sqrt(norm_Normal);
						cos_alpha = scalar_prod/(norm_vect*norm_Normal);
						/*--- Get maximum cosine (not minimum because normals are oriented inwards) ---*/
						if (cos_alpha >= cos_max) {
							Point_Normal = jPoint;
							cos_max = cos_alpha;
						}
					}
					vertex[iMarker][iVertex]->SetClosest_Neighbor(Point_Normal);				
				}
			}
}

CMultiGridGeometry::CMultiGridGeometry(CGeometry *fine_grid, CConfig *config, unsigned short iMesh) : CGeometry() {
	
	unsigned long iPoint, Index_CoarseCV, CVPoint, jPoint, iElem, iVertex;
	bool agglomerate_seed = true, agglomerate_CV = true, Stretching = false;
	unsigned short nChildren, iNode, counter, iMarker, jMarker, iDim, Marker_Boundary;
	short marker_seed;	
	
	double max_dimension = 1.0/config->GetMaxDimension();
	unsigned short max_children = config->GetMaxChildren();

	
	/*--- Determine if the indirect agglomeration is allowed, or not ---*/
	if (iMesh == 1) {
		for (iPoint = 0; iPoint < fine_grid->GetnPoint(); iPoint ++)
			fine_grid->node[iPoint]->SetAgglomerate_Indirect(false);		
		for (iElem = 0; iElem < fine_grid->GetnElem(); iElem++)
			if ((fine_grid->elem[iElem]->GetVTK_Type() == HEXAHEDRON) || (fine_grid->elem[iElem]->GetVTK_Type() == RECTANGLE))
				for (iNode = 0; iNode < fine_grid->elem[iElem]->GetnNodes(); iNode++) {
					iPoint = fine_grid->elem[iElem]->GetNode(iNode);
					fine_grid->node[iPoint]->SetAgglomerate_Indirect(true);
				}
	}
	
	/*--- Write the number of dimensions of the coarse grid ---*/
	nDim = fine_grid->GetnDim();
	
	/*--- Write the number of dimensions of the coarse grid, note that the first point is to create an
	 arry of pointers, and then the structure is created ---*/
	node = new CPoint*[fine_grid->GetnPoint()];
	for (iPoint = 0; iPoint < fine_grid->GetnPoint(); iPoint ++) {
		node[iPoint] = new CPoint(nDim, config);
		
		/*--- Set the indirect agglomeration to false ---*/
		node[iPoint]->SetAgglomerate_Indirect(false);	
	}
		
	Index_CoarseCV = 0;
	
	/*--- The first step is the boundary agglomeration. ---*/
	for (iMarker = 0; iMarker < fine_grid->GetnMarker(); iMarker++) {
		Marker_Boundary = config->GetMarker_All_Boundary(iMarker);
		
		for (iVertex = 0; iVertex < fine_grid->GetnVertex(iMarker); iVertex++) {
			iPoint = fine_grid->vertex[iMarker][iVertex]->GetNode();
			
			/*--- Evaluate the size of the element ---*/ 
			bool check_volume = true;
			double ratio = pow(fine_grid->node[iPoint]->GetVolume(), 1.0/double(nDim))*max_dimension;
			double limit = pow(config->GetDomainVolume(), 1.0/double(nDim));
			if ( ratio > limit ) check_volume = false;

			/*--- Evaluate the streching of the element ---*/ 
			double *Coord_i = fine_grid->node[iPoint]->GetCoord();
			double max_dist = 0.0 ; double min_dist = 1E20;
			Stretching = false;
			for (iNode = 0; iNode <	fine_grid->node[iPoint]->GetnPoint(); iNode ++) {
				jPoint = fine_grid->node[iPoint]->GetPoint(iNode);
				double *Coord_j = fine_grid->node[jPoint]->GetCoord();
				double distance = 0.0;
				for (iDim = 0; iDim < nDim; iDim++)
					distance += (Coord_j[iDim]-Coord_i[iDim])*(Coord_j[iDim]-Coord_i[iDim]);
				distance = sqrt(distance);
				max_dist = max(distance, max_dist);
				min_dist = min(distance, min_dist);
			}
			if ( max_dist/min_dist > 100.0 ) Stretching = true;
			
			/*--- If the element has not being previously agglomerated and it belongs to the physical domain, 
			 then the agglomeration is studied ---*/
			if ((fine_grid->node[iPoint]->GetAgglomerate() == false) &&  (fine_grid->node[iPoint]->GetDomain()) && (check_volume)) {
				
				nChildren = 1;
				
				/*--- We set an index for the parent control volume ---*/
				fine_grid->node[iPoint]->SetParent_CV(Index_CoarseCV);
				
				/*--- We add the seed point (child) to the parent control volume ---*/
				node[Index_CoarseCV]->SetChildren_CV(0,iPoint);			
				agglomerate_seed = true; counter = 0; marker_seed = iMarker;
				
				/*--- For a particular point in the fine grid we save all the markers that are in that point ---*/
				unsigned short copy_marker[MAX_NUMBER_MARKER];
				for (jMarker = 0; jMarker < fine_grid->GetnMarker(); jMarker ++)
					if (fine_grid->node[iPoint]->GetVertex(jMarker) != -1) {
						copy_marker[counter] = jMarker;
						counter++;
					}
				
				/*--- To aglomerate a vertex it must have only one physical bc!! 
				 This can be improved. ---*/
				
				/*--- If there is only a marker, it is a good candidate for agglomeration ---*/
				if (counter == 1) agglomerate_seed = true;
				
				/*--- If there are two markers, we will aglomerate if one of the marker is SEND_RECEIVE ---*/
				if (counter == 2) {
					if ((config->GetMarker_All_Boundary(copy_marker[0]) == SEND_RECEIVE) || 
						(config->GetMarker_All_Boundary(copy_marker[1]) == SEND_RECEIVE)) agglomerate_seed = true;  
					else agglomerate_seed = false;  
				}
				
				/*--- If there are more than 2 markers on the , the aglomeration will be discarted ---*/
				if (counter > 2) agglomerate_seed = false;	
				
				/*--- If the seed can be agglomerated, we try to agglomerate more points ---*/
				if (agglomerate_seed) {
					
					/*--- Now we do a sweep over all the nodes that surround the seed point ---*/
					for (iNode = 0; iNode <	fine_grid->node[iPoint]->GetnPoint(); iNode ++) {
						
						agglomerate_CV = false;
						CVPoint = fine_grid->node[iPoint]->GetPoint(iNode);
						
						/*--- Determine if the CVPoint can be agglomerated ---*/
						agglomerate_CV = SetBoundAgglomeration(CVPoint, marker_seed, fine_grid, config);
						
						/*--- The new point can be agglomerated ---*/
						if (agglomerate_CV && (nChildren < max_children))  {
							
							/*--- We set the value of the parent ---*/
							fine_grid->node[CVPoint]->SetParent_CV(Index_CoarseCV);
							
							/*--- We set the value of the child ---*/
							node[Index_CoarseCV]->SetChildren_CV(nChildren,CVPoint); 
							nChildren++;
						}
					}
					
					vector<unsigned long> Suitable_Indirect_Neighbors;
					if (fine_grid->node[iPoint]->GetAgglomerate_Indirect())
						SetSuitableNeighbors(&Suitable_Indirect_Neighbors, iPoint, Index_CoarseCV, fine_grid);
					
					/*--- Now we do a sweep over all the indirect nodes that can be added ---*/			
					for (iNode = 0; iNode <	Suitable_Indirect_Neighbors.size(); iNode ++) {	
						agglomerate_CV = false;
						CVPoint = Suitable_Indirect_Neighbors[iNode];
						
						/*--- Determine if the CVPoint can be agglomerated ---*/
						agglomerate_CV = SetBoundAgglomeration(CVPoint, marker_seed, fine_grid, config);
						
						/*--- The new point can be agglomerated ---*/
						if (agglomerate_CV && (nChildren < max_children))  {
							
							/*--- We set the value of the parent ---*/
							fine_grid->node[CVPoint]->SetParent_CV(Index_CoarseCV);
							
							/*--- We set the indirect agglomeration information ---*/
							if (fine_grid->node[CVPoint]->GetAgglomerate_Indirect()) 
								node[Index_CoarseCV]->SetAgglomerate_Indirect(true);
							
							/*--- We set the value of the child ---*/
							node[Index_CoarseCV]->SetChildren_CV(nChildren,CVPoint); 
							nChildren++;
						}
					}
				}
				
				/*--- Update the number of child of the control volume ---*/
				node[Index_CoarseCV]->SetnChildren_CV(nChildren);	
				Index_CoarseCV++;
			}
		}
	}
	
	/*--- Agglomerate all the nodes that have more than one physical boundary condition,
	 Maybe here we can add the posibility of merging the vertex that have the same number, 
	 and kind  of markers---*/
	for (iMarker = 0; iMarker < fine_grid->GetnMarker(); iMarker++)
		for (iVertex = 0; iVertex < fine_grid->GetnVertex(iMarker); iVertex++) {
			iPoint = fine_grid->vertex[iMarker][iVertex]->GetNode();
			if ((fine_grid->node[iPoint]->GetAgglomerate() == false) && 
				(fine_grid->node[iPoint]->GetDomain())) {	
				fine_grid->node[iPoint]->SetParent_CV(Index_CoarseCV);
				node[Index_CoarseCV]->SetChildren_CV(0,iPoint);
				node[Index_CoarseCV]->SetnChildren_CV(1);	
				Index_CoarseCV++;				
			}	
		}
	
	
	/*--- Create an advancing fromt for the agglomeration, 
	 0 means that the point is not going to be agglomerated
	 1 means that the point has been previously agglomerated
	 2 means that the point should be adapted 
	 ---*/
	
/*	unsigned short (*PointStatus)[2], jNode;
	unsigned long kPoint;
	PointStatus = new unsigned short[fine_grid->GetnPoint()][2];
	
	for (iPoint = 0; iPoint < fine_grid->GetnPoint(); iPoint ++) {
		PointStatus[iPoint][0] = 0;
		PointStatus[iPoint][1] = 0;
	}*/
	
	/*--- Identify the front to be adapted, starting from the boundaries ---*/
/*	for (iPoint = 0; iPoint < fine_grid->GetnPoint(); iPoint ++)
		if (fine_grid->node[iPoint]->GetAgglomerate() == true) {
			PointStatus[iPoint][0] = 1;
			for (iNode = 0; iNode <	fine_grid->node[iPoint]->GetnPoint(); iNode ++) {	
				jPoint = fine_grid->node[iPoint]->GetPoint(iNode);
				if (PointStatus[jPoint][0] == 0) { 
					PointStatus[jPoint][0] = 2;
					for (jNode = 0; jNode <	fine_grid->node[jPoint]->GetnPoint(); jNode ++) {
						kPoint = fine_grid->node[jPoint]->GetPoint(jNode);
						if (fine_grid->node[kPoint]->GetAgglomerate() == false)
							PointStatus[jPoint][1]++;
					}
				}
			}
		}
	
	
	bool NewPoint = true;
	unsigned long iPointMin;*/
	
	/*--- Agglomerate the domain nodes ---*/
	for (iPoint = 0; iPoint < fine_grid->GetnPoint(); iPoint ++) {

/*	while (NewPoint) {

		iPointMin = fine_grid->GetnPoint();
		unsigned short MinNeighbors = 100;
		for (iPoint = 0; iPoint < fine_grid->GetnPoint(); iPoint ++) {*/

			/*--- If the point is in the adaptation front ---*/
//			if (PointStatus[iPoint][0] == 2) {
				
				/*--- Compute the number of neighbors without adaptation ---*/
/*				PointStatus[iPoint][1] = 0;
				for (jNode = 0; jNode <	fine_grid->node[iPoint]->GetnPoint(); jNode ++) {
					jPoint = fine_grid->node[iPoint]->GetPoint(jNode);
					if (fine_grid->node[jPoint]->GetAgglomerate() == false)
						PointStatus[iPoint][1]++;
				}
				
				if (PointStatus[iPoint][1] < MinNeighbors) {
					MinNeighbors = PointStatus[iPoint][1];
					iPointMin = iPoint;
				}
			}
		}*/
		
		/*--- Set that the point is going to be adapted ---*/
/*		iPoint = iPointMin;
		PointStatus[iPoint][0] = 1;
		
		if (iPoint == fine_grid->GetnPoint()) break;*/
		
		/*--- Evaluate the size of the element ---*/ 
		bool check_volume = true;
		double ratio = pow(fine_grid->node[iPoint]->GetVolume(), 1.0/double(nDim))*max_dimension;
		double limit = pow(config->GetDomainVolume(), 1.0/double(nDim));
		if ( ratio > limit ) check_volume = false;
		
		/*--- Evaluate the streching of the element ---*/ 
		double *Coord_i = fine_grid->node[iPoint]->GetCoord();
		double max_dist = 0.0 ; double min_dist = 1E20;
		Stretching = false;
		for (iNode = 0; iNode <	fine_grid->node[iPoint]->GetnPoint(); iNode ++) {
			jPoint = fine_grid->node[iPoint]->GetPoint(iNode);
			double *Coord_j = fine_grid->node[jPoint]->GetCoord();
			double distance = 0.0;
			for (iDim = 0; iDim < nDim; iDim++)
				distance += (Coord_j[iDim]-Coord_i[iDim])*(Coord_j[iDim]-Coord_i[iDim]);
			distance = sqrt(distance);
			max_dist = max(distance, max_dist);
			min_dist = min(distance, min_dist);
		}
		if ( max_dist/min_dist > 100.0 ) Stretching = true;
		
		/*--- If the element has not being previously agglomerated and it belongs to the physical domain, 
		 then the agglomeration is studied ---*/
		if ((fine_grid->node[iPoint]->GetAgglomerate() == false) && (fine_grid->node[iPoint]->GetDomain()) && (check_volume)) { 
			
			nChildren = 1;
			
			/*--- We set an index for the parent control volume ---*/
			fine_grid->node[iPoint]->SetParent_CV(Index_CoarseCV); 
			
			/*--- We add the seed point (child) to the parent control volume ---*/
			node[Index_CoarseCV]->SetChildren_CV(0,iPoint);			
			
			/*--- Now we do a sweep over all the nodes that surround the seed point ---*/
			for (iNode = 0; iNode <	fine_grid->node[iPoint]->GetnPoint(); iNode ++) {	

				agglomerate_CV = false;
				CVPoint = fine_grid->node[iPoint]->GetPoint(iNode);				
				
				/*--- Determine if the CVPoint can be agglomerated ---*/
				if ((fine_grid->node[CVPoint]->GetAgglomerate() == false) && (fine_grid->node[CVPoint]->GetDomain())) 
					agglomerate_CV = true;
				
				/*--- The new point can be agglomerated ---*/
				if (agglomerate_CV && (nChildren < max_children))  {
					
					/*--- We set the value of the parent ---*/
					fine_grid->node[CVPoint]->SetParent_CV(Index_CoarseCV);
					
					/*--- The point has been  agglomerated, all the neighbors of this 
					 point are now in the adaptation front ---*/
/*					PointStatus[CVPoint][0] = 1;
					for (jNode = 0; jNode <	fine_grid->node[CVPoint]->GetnPoint(); jNode ++) {
						jPoint = fine_grid->node[CVPoint]->GetPoint(jNode);
						if (PointStatus[jPoint][0] == 0) PointStatus[jPoint][0] = 2;
					}*/
				
					/*--- We set the value of the child ---*/
					node[Index_CoarseCV]->SetChildren_CV(nChildren,CVPoint); 
					nChildren++;
				}
			}
			
		
			/*--- Subrotuine to identify the indirect neighbors ---*/
			vector<unsigned long> Suitable_Indirect_Neighbors;			
			if (fine_grid->node[iPoint]->GetAgglomerate_Indirect())
				SetSuitableNeighbors(&Suitable_Indirect_Neighbors, iPoint, Index_CoarseCV, fine_grid);

			/*--- Now we do a sweep over all the indirect nodes that can be added ---*/			
			for (iNode = 0; iNode <	Suitable_Indirect_Neighbors.size(); iNode ++) {	
				agglomerate_CV = false;
				CVPoint = Suitable_Indirect_Neighbors[iNode];				
				
				/*--- Determine if the CVPoint can be agglomerated ---*/
				if ((fine_grid->node[CVPoint]->GetAgglomerate() == false) && (fine_grid->node[CVPoint]->GetDomain())) 
					agglomerate_CV = true;
				
				/*--- The new point can be agglomerated ---*/
				if ((agglomerate_CV) && (nChildren < max_children))  {
					
					/*--- We set the value of the parent ---*/
					fine_grid->node[CVPoint]->SetParent_CV(Index_CoarseCV);
					
					/*--- We set the indirect agglomeration information ---*/
					if (fine_grid->node[CVPoint]->GetAgglomerate_Indirect()) 
						node[Index_CoarseCV]->SetAgglomerate_Indirect(true);
					
					/*--- We set the value of the child ---*/
					node[Index_CoarseCV]->SetChildren_CV(nChildren,CVPoint); 
					nChildren++;
				}
			}
			
			/*--- Update the number of control of childrens ---*/
			node[Index_CoarseCV]->SetnChildren_CV(nChildren);	
			Index_CoarseCV++;
		}		
	}
	
	/*--- Add all the elements that have not being agglomerated, in the previous stage ---*/
	for (iPoint = 0; iPoint < fine_grid->GetnPoint(); iPoint ++) {
		if ((fine_grid->node[iPoint]->GetAgglomerate() == false) && 
			(fine_grid->node[iPoint]->GetDomain())) { 
			nChildren = 1;
			fine_grid->node[iPoint]->SetParent_CV(Index_CoarseCV); 
			if (fine_grid->node[iPoint]->GetAgglomerate_Indirect()) 
				node[Index_CoarseCV]->SetAgglomerate_Indirect(true);
			node[Index_CoarseCV]->SetChildren_CV(0,iPoint);
			node[Index_CoarseCV]->SetnChildren_CV(nChildren);
			Index_CoarseCV++;
		}
	}
	
	
	/*--- Dealing with MPI parallelization, the objective is that the received nodes must be agglomerated 
	 in the same way as the donor nodes. ---*/
	
	nPointDomain = Index_CoarseCV;

	unsigned long jVertex;
	short Send_Recv;

	/*--- Send the node agglomeration information of the donor 
	 (parent and children), Sending only occurs with MPI ---*/
	
#ifndef NO_MPI
	
	
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++) {
		
		Marker_Boundary = config->GetMarker_All_Boundary(iMarker);
		Send_Recv = config->GetMarker_All_SendRecv(iMarker);
		
		if ((Marker_Boundary == SEND_RECEIVE) && (Send_Recv > 0)) {	
			
			unsigned long nBuffer = fine_grid->nVertex[iMarker];

			unsigned long *Buffer_Send_Parent = new unsigned long[nBuffer];
			unsigned long *Buffer_Send_Children = new unsigned long[nBuffer];
			unsigned short *Buffer_Send_Repetition = new unsigned short[nBuffer];

			unsigned short index = 1;
			for (iVertex = 0; iVertex < fine_grid->nVertex[iMarker]; iVertex++) {
				Buffer_Send_Repetition[iVertex] = 0;
				iPoint = fine_grid->vertex[iMarker][iVertex]->GetNode();
				Buffer_Send_Children[iVertex] = iPoint;
				Buffer_Send_Parent[iVertex] = fine_grid->node[iPoint]->GetParent_CV();
				
				for (jVertex = 0; jVertex < fine_grid->GetnVertex(iMarker); jVertex++) {
					iPoint = fine_grid->vertex[iMarker][iVertex]->GetNode();
					jPoint = fine_grid->vertex[iMarker][jVertex]->GetNode();
					if ((iPoint == jPoint) && (iVertex != jVertex)) {
						Buffer_Send_Repetition[iVertex] = index;
						index++;
					}
				}
				
			}
			
			int send_to = Send_Recv - 1;

			MPI::COMM_WORLD.Bsend(Buffer_Send_Children, nBuffer, MPI::UNSIGNED_LONG, send_to, 1);
			MPI::COMM_WORLD.Bsend(Buffer_Send_Parent, nBuffer, MPI::UNSIGNED_LONG, send_to, 0);
			MPI::COMM_WORLD.Bsend(Buffer_Send_Repetition, nBuffer, MPI::UNSIGNED_SHORT, send_to, 2);
			
			delete[] Buffer_Send_Parent;
			delete[] Buffer_Send_Children;
			delete[] Buffer_Send_Repetition;			

		}
	}
	
#endif
	
	/*--- Receive the donor agglomeration (parent and children) ---*/
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++) {
		Marker_Boundary = config->GetMarker_All_Boundary(iMarker);
		Send_Recv = config->GetMarker_All_SendRecv(iMarker);
		
		if ((Marker_Boundary == SEND_RECEIVE) && (Send_Recv < 0)) {

			int receive_from = abs(Send_Recv)-1;
			unsigned long nBuffer = fine_grid->nVertex[iMarker];

			unsigned long *Buffer_Receive_Parent = new unsigned long [nBuffer];
			unsigned long *Buffer_Receive_Children = new unsigned long [nBuffer];
			unsigned short *Buffer_Receive_Repetition = new unsigned short [nBuffer];
			
			
#ifndef NO_MPI
			MPI::COMM_WORLD.Recv(Buffer_Receive_Children, nBuffer, MPI::UNSIGNED_LONG, receive_from, 1);
			MPI::COMM_WORLD.Recv(Buffer_Receive_Parent, nBuffer, MPI::UNSIGNED_LONG, receive_from, 0);
			MPI::COMM_WORLD.Recv(Buffer_Receive_Repetition, nBuffer, MPI::UNSIGNED_SHORT, receive_from, 2);
#else
			/*--- Retrieve the donor information from the matching marker ---*/
			unsigned short donor_marker = 1;
			unsigned long donorPoint;
			for (unsigned short iMark = 0; iMark < config->GetnMarker_All(); iMark++){
				if (config->GetMarker_All_SendRecv(iMark)-1 == receive_from) donor_marker = iMark;
			}
			
			/*--- Get the information from the donor directly. This is a serial
			 computation with access to all nodes. Note that there is an
			 implicit ordering in the list. ---*/
			for (iVertex = 0; iVertex < fine_grid->nVertex[iMarker]; iVertex++) {
				donorPoint = fine_grid->vertex[donor_marker][iVertex]->GetNode();
				Buffer_Receive_Children[iVertex] = donorPoint;
				Buffer_Receive_Parent[iVertex] = fine_grid->node[donorPoint]->GetParent_CV();
			}
#endif

			/*--- Create a list of the parent nodes without repeated parents ---*/
			vector<unsigned long>::iterator it;
			vector<unsigned long> Aux_Parent;
			
			for (iVertex = 0; iVertex < fine_grid->nVertex[iMarker]; iVertex++)
				Aux_Parent.push_back (Buffer_Receive_Parent[iVertex]);
			
			sort(Aux_Parent.begin(), Aux_Parent.end());
			it = unique(Aux_Parent.begin(), Aux_Parent.end());
			Aux_Parent.resize(it - Aux_Parent.begin());
			
			unsigned long *Parent_Remote = new unsigned long[fine_grid->nVertex[iMarker]];
			unsigned long *Children_Remote = new unsigned long[fine_grid->nVertex[iMarker]];
			unsigned long *Parent_Local = new unsigned long[fine_grid->nVertex[iMarker]];
			unsigned long *Children_Local = new unsigned long[fine_grid->nVertex[iMarker]];
			unsigned long *Repetition_Local = new unsigned long[fine_grid->nVertex[iMarker]];
			unsigned long *Repetition_Remote = new unsigned long[fine_grid->nVertex[iMarker]];
			
			
			/*--- Create the local vector and remote for the parents and the children ---*/
			for (iVertex = 0; iVertex < fine_grid->nVertex[iMarker]; iVertex++) {
				Parent_Remote[iVertex] = Buffer_Receive_Parent[iVertex];
				
				/*--- We use the same sortering as in the donor domain ---*/
				for (jVertex = 0; jVertex < Aux_Parent.size(); jVertex++) {
					if (Parent_Remote[iVertex] == Aux_Parent[jVertex]) {
						Parent_Local[iVertex] = jVertex + Index_CoarseCV;
						break;
					}
				}
				
				Children_Remote[iVertex] = Buffer_Receive_Children[iVertex];
				Children_Local[iVertex] = fine_grid->vertex[iMarker][iVertex]->GetNode();
				
				Repetition_Remote[iVertex] = Buffer_Receive_Repetition[iVertex];
				Repetition_Local[iVertex] = Buffer_Receive_Repetition[iVertex];
			}
			
			
			Index_CoarseCV += Aux_Parent.size();
			
			unsigned short *nChildren_ = new unsigned short [Index_CoarseCV];
			for (unsigned long iParent = 0; iParent < Index_CoarseCV; iParent++) 
				nChildren_[iParent] = 0;
			
			/*--- Create the final structure ---*/
			for (iVertex = 0; iVertex < fine_grid->nVertex[iMarker]; iVertex++) {
				/*--- Be careful, it is possible that a node change the agglomeration configuration, the priority 
				 is always, when receive the information ---*/
				fine_grid->node[Children_Local[iVertex]]->SetParent_CV(Parent_Local[iVertex]);				
				node[Parent_Local[iVertex]]->SetChildren_CV(nChildren_[Parent_Local[iVertex]],Children_Local[iVertex]);
				nChildren_[Parent_Local[iVertex]]++;
				node[Parent_Local[iVertex]]->SetnChildren_CV(nChildren_[Parent_Local[iVertex]]);
				node[Parent_Local[iVertex]]->SetDomain(false);
			}
			
			delete [] nChildren_;
			delete [] Buffer_Receive_Parent;
			delete [] Buffer_Receive_Children;
			delete [] Buffer_Receive_Repetition;
			delete [] Parent_Remote;
			delete [] Children_Remote;
			delete [] Parent_Local;
			delete [] Children_Local;
			delete [] Repetition_Local;
			delete [] Repetition_Remote;
			
		}
	}
	
	nPoint = Index_CoarseCV;
	
#ifdef NO_MPI
	nPointDomain = nPoint;
	cout << "CVs of the MG level: " << nPoint << ". Agglom. rate 1/" << double(fine_grid->GetnPoint())/double(nPoint) <<"."<<endl;
#else
	int rank = MPI::COMM_WORLD.Get_rank();
	cout << "Node rank: "<<rank <<". CVs of the MG level: " << nPoint << ". Agglom. rate 1/" << double(fine_grid->GetnPoint())/double(nPoint) <<"."<<endl;
#endif
  
}

CMultiGridGeometry::~CMultiGridGeometry(void) {
	delete[] node;
}

bool CMultiGridGeometry::SetBoundAgglomeration(unsigned long CVPoint, short marker_seed, CGeometry *fine_grid, CConfig *config) {
	
	bool agglomerate_CV = false;
	unsigned short counter, jMarker;
	
	/*--- Basic condition, the element has not being previously agglomerated and, it belong to the domain ---*/
	if ((fine_grid->node[CVPoint]->GetAgglomerate() == false) && (fine_grid->node[CVPoint]->GetDomain())) {
		
		/*--- If the element belong to the boundary, we must be careful ---*/
		if (fine_grid->node[CVPoint]->GetBoundary()) {
			
			/*--- Identify the markers of the vertex that we whant to agglomerate ---*/
			counter = 0;
			unsigned short copy_marker[MAX_NUMBER_MARKER];
			for (jMarker = 0; jMarker < fine_grid->GetnMarker(); jMarker ++)
				if (fine_grid->node[CVPoint]->GetVertex(jMarker) != -1) { 
					copy_marker[counter] = jMarker;
					counter++; 
				}
			
			/*--- The basic condition is that the aglomerated vertex must have the same physical marker, 
			 but eventually a send-receive condition ---*/
			
			/*--- Only one marker in the vertex that is going to be aglomerated ---*/
			if (counter == 1) {
				
				/*--- We agglomerate if there is only a marker and is the same marker as the seed marker ---*/
				if (copy_marker[0] == marker_seed) 
					agglomerate_CV = true;
				
				/*--- If there is only a marker, but the marker is the SEND_RECEIVE ---*/
				if (config->GetMarker_All_Boundary(copy_marker[0]) == SEND_RECEIVE) 
					agglomerate_CV = true;
			}
			
			/*--- If there are two markers in the vertex that is going to be aglomerated ---*/
			if (counter == 2) {
				
				/*--- First we verify that the seed is a physical boundary ---*/
				if (config->GetMarker_All_Boundary(marker_seed) != SEND_RECEIVE) {
					
					/*--- Then we check that one of the marker is equal to the seed marker, and the other is send/receive ---*/
					if (((copy_marker[0] == marker_seed) && (config->GetMarker_All_Boundary(copy_marker[1]) == SEND_RECEIVE)) ||
						((config->GetMarker_All_Boundary(copy_marker[0]) == SEND_RECEIVE) && (copy_marker[1] == marker_seed)))
						agglomerate_CV = true;
				}
			}
			
		}
		
		/*--- If the element belong to the domain, it is allways aglomerated ---*/
		else {
			agglomerate_CV = true; 
		} 
	}
	
	return agglomerate_CV;
}



void CMultiGridGeometry::SetSuitableNeighbors(vector<unsigned long> *Suitable_Indirect_Neighbors, unsigned long iPoint, 
											  unsigned long Index_CoarseCV, CGeometry *fine_grid) {
	
	unsigned long jPoint, kPoint,  iOriginNeighbor, lPoint;
	unsigned short iNode, jNode, iNeighbor, jNeighbor, kNode;
	bool SecondNeighborSeed, check_1, ThirdNeighborSeed;
	
	/*--- Create a list with the first neighbors, including the seed ---*/
	vector<unsigned long> First_Neighbor_Points;
	First_Neighbor_Points.push_back(iPoint);
	for (iNode = 0; iNode <	fine_grid->node[iPoint]->GetnPoint(); iNode ++) {
		jPoint = fine_grid->node[iPoint]->GetPoint(iNode);
		First_Neighbor_Points.push_back(jPoint);
	}
	
	/*--- Create a list with the second neighbors, without first, and seed neighbors ---*/
	vector<unsigned long> Second_Neighbor_Points, Second_Origin_Points, Suitable_Second_Neighbors;
	
	for (iNode = 0; iNode <	fine_grid->node[iPoint]->GetnPoint(); iNode ++) {
		jPoint = fine_grid->node[iPoint]->GetPoint(iNode);
		for (jNode = 0; jNode <	fine_grid->node[jPoint]->GetnPoint(); jNode ++) {
			kPoint = fine_grid->node[jPoint]->GetPoint(jNode);				

			/*--- Check that the second neighbor do not belong to the first neighbor or the seed ---*/
			SecondNeighborSeed = true;
			for (iNeighbor = 0; iNeighbor <	First_Neighbor_Points.size(); iNeighbor ++)
				if (kPoint == First_Neighbor_Points[iNeighbor]) {
					SecondNeighborSeed = false;
					break;
				}
			
			if (SecondNeighborSeed) {
				Second_Neighbor_Points.push_back(kPoint);
				Second_Origin_Points.push_back(jPoint);
			}
			
		}
	}
	
	/*---  Identify those second neighbors that are repeated (candidate to be added) ---*/
	for (iNeighbor = 0; iNeighbor <	Second_Neighbor_Points.size(); iNeighbor ++)
		for (jNeighbor = 0; jNeighbor <	Second_Neighbor_Points.size(); jNeighbor ++)
			
			/*--- Repeated second neighbor with different origin ---*/
			if ((Second_Neighbor_Points[iNeighbor] == Second_Neighbor_Points[jNeighbor]) &&
				(Second_Origin_Points[iNeighbor] != Second_Origin_Points[jNeighbor]) &&
				(iNeighbor < jNeighbor)) {
				
				/*--- Check that the origin nodes are not neighbor ---*/
				check_1 = true;
				for (iNode = 0; iNode <	fine_grid->node[Second_Origin_Points[iNeighbor]]->GetnPoint(); iNode ++) {
					iOriginNeighbor = fine_grid->node[Second_Origin_Points[iNeighbor]]->GetPoint(iNode);
					if (iOriginNeighbor == Second_Origin_Points[jNeighbor]) {
						check_1 = false;
						break;
					}
				}

				if (check_1) {
					Suitable_Indirect_Neighbors->push_back(Second_Neighbor_Points[iNeighbor]);
					
					/*--- Create alist with the suitable second neighbor, that we will use 
					 to compute the third neighbors --*/
					Suitable_Second_Neighbors.push_back(Second_Neighbor_Points[iNeighbor]);
				}
			}
	
	vector<unsigned long>::iterator it;
	
	/*--- Remove repeated from the suitable second neighbors ---*/
	sort(Suitable_Second_Neighbors.begin(), Suitable_Second_Neighbors.end());
	it = unique(Suitable_Second_Neighbors.begin(), Suitable_Second_Neighbors.end());
	Suitable_Second_Neighbors.resize(it - Suitable_Second_Neighbors.begin());
	
	/*--- Remove repeated from first neighbors ---*/
	sort(First_Neighbor_Points.begin(), First_Neighbor_Points.end());
	it = unique(First_Neighbor_Points.begin(), First_Neighbor_Points.end());
	First_Neighbor_Points.resize(it - First_Neighbor_Points.begin());
	
	/*--- Create a list with the third neighbors, without first, second, and seed neighbors ---*/
	vector<unsigned long> Third_Neighbor_Points, Third_Origin_Points;
	
	for (jNode = 0; jNode <	Suitable_Second_Neighbors.size(); jNode ++) {
		kPoint = Suitable_Second_Neighbors[jNode];
		
		for (kNode = 0; kNode <	fine_grid->node[kPoint]->GetnPoint(); kNode ++) {
			lPoint = fine_grid->node[kPoint]->GetPoint(kNode);		
			
			/*--- Check that the third neighbor do not belong to the first neighbors or the seed ---*/
			ThirdNeighborSeed = true;
			
			for (iNeighbor = 0; iNeighbor <	First_Neighbor_Points.size(); iNeighbor ++)
				if (lPoint == First_Neighbor_Points[iNeighbor]) {
					ThirdNeighborSeed = false;
					break;
				}
			
			/*--- Check that the third neighbor do not belong to the second neighbors ---*/
			for (iNeighbor = 0; iNeighbor <	Suitable_Second_Neighbors.size(); iNeighbor ++)
				if (lPoint == Suitable_Second_Neighbors[iNeighbor]) {
					ThirdNeighborSeed = false;
					break;
				}
			
			if (ThirdNeighborSeed) {
				Third_Neighbor_Points.push_back(lPoint);
				Third_Origin_Points.push_back(kPoint);
			}
			
		}
	}

	/*---  Identify those third neighbors that are repeated (candidate to be added) ---*/
	for (iNeighbor = 0; iNeighbor <	Third_Neighbor_Points.size(); iNeighbor ++)
		for (jNeighbor = 0; jNeighbor <	Third_Neighbor_Points.size(); jNeighbor ++)
			
			/*--- Repeated second neighbor with different origin ---*/
			if ((Third_Neighbor_Points[iNeighbor] == Third_Neighbor_Points[jNeighbor]) &&
				(Third_Origin_Points[iNeighbor] != Third_Origin_Points[jNeighbor]) &&
				(iNeighbor < jNeighbor)) {
				
				/*--- Check that the origin nodes are not neighbor ---*/
				check_1 = true;
				for (iNode = 0; iNode <	fine_grid->node[Third_Origin_Points[iNeighbor]]->GetnPoint(); iNode ++) {
					iOriginNeighbor = fine_grid->node[Third_Origin_Points[iNeighbor]]->GetPoint(iNode);
					if (iOriginNeighbor == Third_Origin_Points[jNeighbor]) {
						check_1 = false;
						break;
					}
				}
				
				if (check_1)
					Suitable_Indirect_Neighbors->push_back(Third_Neighbor_Points[iNeighbor]);
				
			}

	/*--- Remove repeated from Suitable Indirect Neighbors List ---*/
	sort(Suitable_Indirect_Neighbors->begin(), Suitable_Indirect_Neighbors->end());
	it = unique(Suitable_Indirect_Neighbors->begin(), Suitable_Indirect_Neighbors->end());
	Suitable_Indirect_Neighbors->resize(it - Suitable_Indirect_Neighbors->begin());

}



void CMultiGridGeometry::SetPsuP(CGeometry *fine_grid) {
	unsigned long iFinePoint, iFinePoint_Neighbor, iParent, iCoarsePoint;
	unsigned short iChildren, iNode;
	
	for (iCoarsePoint = 0; iCoarsePoint < nPoint; iCoarsePoint ++)
		for (iChildren = 0; iChildren <  node[iCoarsePoint]->GetnChildren_CV(); iChildren ++) {
			iFinePoint = node[iCoarsePoint]->GetChildren_CV(iChildren);
			for (iNode = 0; iNode < fine_grid->node[iFinePoint]->GetnPoint(); iNode ++) {
				iFinePoint_Neighbor = fine_grid->node[iFinePoint]->GetPoint(iNode);
				iParent = fine_grid->node[iFinePoint_Neighbor]->GetParent_CV();
				if (iParent != iCoarsePoint) node[iCoarsePoint]->SetPoint(iParent);
			}
		}
}

void CMultiGridGeometry::SetVertex(CGeometry *fine_grid, CConfig *config) {
	unsigned long  iVertex, iFinePoint, iCoarsePoint;
	unsigned short iMarker, iMarker_Tag, iChildren;

	nMarker = fine_grid->GetnMarker();	

	/*--- If any children node belong to the boundary then the entire control 
	 volume will belong to the boundary ---*/
	for (iCoarsePoint = 0; iCoarsePoint < nPoint; iCoarsePoint ++)
		for (iChildren = 0; iChildren <	node[iCoarsePoint]->GetnChildren_CV(); iChildren ++) {
			iFinePoint = node[iCoarsePoint]->GetChildren_CV(iChildren);
			if (fine_grid->node[iFinePoint]->GetBoundary()) {
				node[iCoarsePoint]->SetBoundary(nMarker);
				break;
			}
		}
	
	vertex = new CVertex**[nMarker];
	nVertex = new unsigned long [nMarker];

	Tag_to_Marker = new string [MAX_INDEX_VALUE];
	for (iMarker_Tag = 0; iMarker_Tag < MAX_INDEX_VALUE; iMarker_Tag++)
		Tag_to_Marker[iMarker_Tag] = fine_grid->GetMarker_Tag(iMarker_Tag);

	/*--- Compute the number of vertices to do the dimensionalization ---*/
	for (iMarker = 0; iMarker < nMarker; iMarker++) nVertex[iMarker] = 0;
	

	for (iCoarsePoint = 0; iCoarsePoint < nPoint; iCoarsePoint ++) {
		if (node[iCoarsePoint]->GetBoundary()) {
			for (iChildren = 0; iChildren <	node[iCoarsePoint]->GetnChildren_CV(); iChildren ++) {
				iFinePoint = node[iCoarsePoint]->GetChildren_CV(iChildren);
					for (iMarker = 0; iMarker < nMarker; iMarker ++) {
						if ((fine_grid->node[iFinePoint]->GetVertex(iMarker) != -1) && (node[iCoarsePoint]->GetVertex(iMarker) == -1)) {
							iVertex = nVertex[iMarker];
							node[iCoarsePoint]->SetVertex(iVertex,iMarker);
							nVertex[iMarker]++;
						}
					}
			}
		}
	}

	for (iMarker = 0; iMarker < nMarker; iMarker++) {
		vertex[iMarker] = new CVertex* [fine_grid->GetnVertex(iMarker)+1];
		nVertex[iMarker] = 0;
	}
	
	for (iCoarsePoint = 0; iCoarsePoint < nPoint; iCoarsePoint ++)
		if (node[iCoarsePoint]->GetBoundary())
			for (iMarker = 0; iMarker < nMarker; iMarker ++)
				node[iCoarsePoint]->SetVertex(-1,iMarker);
 
	for (iMarker = 0; iMarker < nMarker; iMarker++) nVertex[iMarker] = 0;
	
	for (iCoarsePoint = 0; iCoarsePoint < nPoint; iCoarsePoint ++)
		if (node[iCoarsePoint]->GetBoundary())
			for (iChildren = 0; iChildren <	node[iCoarsePoint]->GetnChildren_CV(); iChildren ++) {
				iFinePoint = node[iCoarsePoint]->GetChildren_CV(iChildren);
					for (iMarker = 0; iMarker < fine_grid->GetnMarker(); iMarker ++) {
						if ((fine_grid->node[iFinePoint]->GetVertex(iMarker) != -1) && (node[iCoarsePoint]->GetVertex(iMarker) == -1)) {
							iVertex = nVertex[iMarker];
							vertex[iMarker][iVertex] = new CVertex(iCoarsePoint, nDim);
							node[iCoarsePoint]->SetVertex(iVertex,iMarker);
							
							/*--- If the vertex do not belong to the domain, set the transformation to apply ---*/
							if (config->GetMarker_All_Boundary(iMarker) != SEND_RECEIVE &&
									config->GetMarker_All_Boundary(iMarker) != NEARFIELD_BOUNDARY && 
									config->GetMarker_All_Boundary(iMarker) != EULER_WALL &&
									config->GetMarker_All_Boundary(iMarker) != SYMMETRY_PLANE && 
									config->GetMarker_All_Boundary(iMarker) != INTERFACE_BOUNDARY) {
									node[iCoarsePoint]->SetBoundary_Physical(true);
							}
							else {
								unsigned long ChildVertex = fine_grid->node[iFinePoint]->GetVertex(iMarker);
								unsigned short RotationKind = fine_grid->vertex[iMarker][ChildVertex]->GetRotation_Type();
								vertex[iMarker][iVertex]->SetRotation_Type(RotationKind);
							}
							
							nVertex[iMarker]++;
						}
					}
			}
}

void CMultiGridGeometry::MachNearField(CConfig *config) {
	
#ifdef NO_MPI

	unsigned short iMarker;
	unsigned long iVertex, iPoint;
	
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if (config->GetMarker_All_Boundary(iMarker) == NEARFIELD_BOUNDARY)
			for (iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
				iPoint = vertex[iMarker][iVertex]->GetNode();
				vertex[iMarker][iVertex]->SetPeriodicPoint(iPoint);
			}
	
#else
	
	unsigned short iMarker;
	unsigned long iVertex, iPoint;
	int rank = MPI::COMM_WORLD.Get_rank();
	
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if (config->GetMarker_All_Boundary(iMarker) == NEARFIELD_BOUNDARY)
			for (iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
				iPoint = vertex[iMarker][iVertex]->GetNode();
				if (node[iPoint]->GetDomain()) {
					vertex[iMarker][iVertex]->SetPeriodicPoint(iPoint, rank);
				}
			}
	
#endif
	
}

void CMultiGridGeometry::MachInterface(CConfig *config) {
	
#ifdef NO_MPI
	
	unsigned short iMarker;
	unsigned long iVertex, iPoint;
	
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if (config->GetMarker_All_Boundary(iMarker) == INTERFACE_BOUNDARY)
			for (iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
				iPoint = vertex[iMarker][iVertex]->GetNode();
				vertex[iMarker][iVertex]->SetPeriodicPoint(iPoint);
			}
	
#else
	
	unsigned short iMarker;
	unsigned long iVertex, iPoint;
	int rank = MPI::COMM_WORLD.Get_rank();
	
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if (config->GetMarker_All_Boundary(iMarker) == INTERFACE_BOUNDARY)
			for (iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
				iPoint = vertex[iMarker][iVertex]->GetNode();
				if (node[iPoint]->GetDomain()) {
					vertex[iMarker][iVertex]->SetPeriodicPoint(iPoint, rank);
				}
			}
	
#endif
	
}


void CMultiGridGeometry::SetControlVolume(CConfig *config, CGeometry *fine_grid, unsigned short action) {

	unsigned long iFinePoint,iFinePoint_Neighbor, iCoarsePoint, iEdge, iParent, FineEdge, CoarseEdge, iPoint;
	unsigned short iChildren, iNode, iDim;
	bool change_face_orientation;
	double *Normal, Coarse_Volume;
	Normal = new double [nDim];
  bool rotating_frame = config->GetRotating_Frame();
  
	/*--- Compute the area of the coarse volume ---*/
	for (iCoarsePoint = 0; iCoarsePoint < nPoint; iCoarsePoint ++) {
		node[iCoarsePoint]->SetVolume(0.0);
		Coarse_Volume = 0.0;
		for (iChildren = 0; iChildren < node[iCoarsePoint]->GetnChildren_CV(); iChildren ++) {
			iFinePoint = node[iCoarsePoint]->GetChildren_CV(iChildren);
			Coarse_Volume += fine_grid->node[iFinePoint]->GetVolume();
		}
		node[iCoarsePoint]->SetVolume(Coarse_Volume);
	}

// Update or not the values of faces at the edge	
	if (action != ALLOCATE) {	
		for(iEdge=0; iEdge < nEdge; iEdge++)
			edge[iEdge]->SetZeroValues();
	}

	for (iCoarsePoint = 0; iCoarsePoint < nPoint; iCoarsePoint ++)
		for (iChildren = 0; iChildren < node[iCoarsePoint]->GetnChildren_CV(); iChildren ++) {
			iFinePoint = node[iCoarsePoint]->GetChildren_CV(iChildren);
			for (iNode = 0; iNode < fine_grid->node[iFinePoint]->GetnPoint(); iNode ++) {
				iFinePoint_Neighbor = fine_grid->node[iFinePoint]->GetPoint(iNode);
				iParent = fine_grid->node[iFinePoint_Neighbor]->GetParent_CV();
				if ((iParent != iCoarsePoint) && (iParent < iCoarsePoint)) {
				// Localiza la arista que une el nodo del VC (Fine_Mesh_Point) con el VC contiguo (Neighbor_Point)	
					FineEdge = fine_grid->FindEdge(iFinePoint, iFinePoint_Neighbor);

					// Definimos una orientacion... del Indice menor al mayor
					// necesario para orientar las caras de una arista,
					// y debe ser coherente con que hemos seleccionado
					// los casos donde iParent < iCoarsePoint
					change_face_orientation = false;
					if (iFinePoint < iFinePoint_Neighbor) change_face_orientation = true;

					// Localiza la arista que une los volumenes de control en la malla basta
					CoarseEdge = FindEdge(iParent, iCoarsePoint);

					// Copia los volumenes de control de la arista fina a la arista basta, 
					//	por cada cara de la arista en la malla fina
					// Leemos las caracterIsticas de cada cara, sin utilizar las coordenadas
					// que definen las caras
					fine_grid->edge[FineEdge]->GetNormal(Normal);
						
					if (change_face_orientation) {
						for (iDim = 0; iDim < nDim; iDim++) Normal[iDim] = -Normal[iDim];
						edge[CoarseEdge]->AddNormal(Normal);
            /*--- Add contribution for the rotating volume flux if necessary ---*/
            if (rotating_frame)
              edge[CoarseEdge]->AddRotFlux(-fine_grid->edge[FineEdge]->GetRotFlux());
					} else {
            edge[CoarseEdge]->AddNormal(Normal);
            /*--- Add contribution for the rotating volume flux if necessary ---*/
            if (rotating_frame)
              edge[CoarseEdge]->AddRotFlux(fine_grid->edge[FineEdge]->GetRotFlux());
          }
				}
			}
		}
	delete [] Normal;
	
	/*--- Set the volume for the iterations n and n-1 (dual time stteping with grid movement) ---*/
	for (iPoint = 0; iPoint < nPoint; iPoint++) {		
		node[iPoint]->SetVolume_n();
		node[iPoint]->SetVolume_n1();
	}

}

void CMultiGridGeometry::SetBoundControlVolume(CConfig *config, CGeometry *fine_grid, unsigned short action) {
	unsigned long iCoarsePoint, iFinePoint, FineVertex, iVertex;
	unsigned short iMarker, iChildren;
	double *Normal;
	Normal = new double [nDim];
  bool rotating_frame = config->GetRotating_Frame();
  
	if (action != ALLOCATE) {
		for (iMarker = 0; iMarker < nMarker; iMarker++)
			for (iVertex = 0; iVertex < nVertex[iMarker]; iVertex++)
					vertex[iMarker][iVertex]->SetZeroValues();
	}

	for (iMarker = 0; iMarker < nMarker; iMarker ++)
		for(iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
			iCoarsePoint = vertex[iMarker][iVertex]->GetNode();
			for (iChildren = 0; iChildren < node[iCoarsePoint]->GetnChildren_CV(); iChildren ++) {
				iFinePoint = node[iCoarsePoint]->GetChildren_CV(iChildren);
				if (fine_grid->node[iFinePoint]->GetVertex(iMarker)!=-1) {
					FineVertex = fine_grid->node[iFinePoint]->GetVertex(iMarker);
					fine_grid->vertex[iMarker][FineVertex]->GetNormal(Normal);
					vertex[iMarker][iVertex]->AddNormal(Normal);
          /*--- Add contribution for the rotating volume flux if necessary ---*/
          if (rotating_frame)
            vertex[iMarker][iVertex]->AddRotFlux(fine_grid->vertex[iMarker][FineVertex]->GetRotFlux());
				}
			}
		}
	
	delete [] Normal;
}

void CMultiGridGeometry::SetCoord(CGeometry *geometry) {
	unsigned long Point_Fine, Point_Coarse;
	unsigned short iChildren, iDim;
	double Area_Parent, Area_Children;
	double *Coordinates_Fine, *Coordinates;
	Coordinates = new double[nDim];
	
	for (Point_Coarse = 0; Point_Coarse < GetnPoint(); Point_Coarse++) {
		Area_Parent = node[Point_Coarse]->GetVolume();
		for (iDim = 0; iDim < nDim; iDim++) Coordinates[iDim] = 0.0;
		for (iChildren = 0; iChildren < node[Point_Coarse]->GetnChildren_CV(); iChildren++) {
			Point_Fine = node[Point_Coarse]->GetChildren_CV(iChildren);
			Area_Children = geometry->node[Point_Fine]->GetVolume();
			Coordinates_Fine = geometry->node[Point_Fine]->GetCoord();
			for (iDim = 0; iDim < nDim; iDim++)
				Coordinates[iDim] += Coordinates_Fine[iDim]*Area_Children/Area_Parent; 
		}
		for (iDim = 0; iDim < nDim; iDim++)
			node[Point_Coarse]->SetCoord(iDim,Coordinates[iDim]);
	}
	delete [] Coordinates;
}

void CMultiGridGeometry::SetRotationalVelocity(CConfig *config) {
  
  unsigned short Point_Coarse;
  double RotVel[3], Distance[3], *Coord, *Axis, *Omega, Length_Ref;
  
  /*--- Loop over all points and set rotational velocity.
   Note that this only need be done once for steady rotation. ---*/
  for (Point_Coarse = 0; Point_Coarse < GetnPoint(); Point_Coarse++) {
    
    /*--- Get values for this node ---*/
    Coord     = node[Point_Coarse]->GetCoord();
    Axis      = config->GetRotAxisOrigin();
    Omega     = config->GetOmega_FreeStreamND();
    Length_Ref = config->GetLength_Ref();
    
    /*--- Calculate non-dim distance fron rotation center ---*/
    Distance[0] = (Coord[0]-Axis[0])/Length_Ref;
    Distance[1] = (Coord[1]-Axis[1])/Length_Ref;
    Distance[2] = (Coord[2]-Axis[2])/Length_Ref;
    
    /*--- Calculate the angular velocity as omega X r ---*/
    RotVel[0] = Omega[1]*(Distance[2]) - Omega[2]*(Distance[1]);
    RotVel[1] = Omega[2]*(Distance[0]) - Omega[0]*(Distance[2]);
    RotVel[2] = Omega[0]*(Distance[1]) - Omega[1]*(Distance[0]);
    
    node[Point_Coarse]->SetRotVel(RotVel);
    
  }
}

void CMultiGridGeometry::FindClosestNeighbor(CConfig *config) {
	
	unsigned short iMarker, iDim;
	unsigned long iPoint, iVertex;
	
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if (config->GetMarker_All_Boundary(iMarker) != SEND_RECEIVE &&
				config->GetMarker_All_Boundary(iMarker) != INTERFACE_BOUNDARY &&
				config->GetMarker_All_Boundary(iMarker) != NEARFIELD_BOUNDARY )			
			for (iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
				
				iPoint = vertex[iMarker][iVertex]->GetNode();
				
				/*--- If the node belong to the domain ---*/
				if (node[iPoint]->GetDomain()) {
					
					/*--- Compute closest normal neighbor ---*/
					double cos_max, scalar_prod, norm_vect, norm_Normal, cos_alpha, diff_coord;
					unsigned long Point_Normal = 0, jPoint;
					unsigned short iNeigh;
					double *Normal = vertex[iMarker][iVertex]->GetNormal();
					cos_max = -1.0;
					for (iNeigh = 0; iNeigh < node[iPoint]->GetnPoint(); iNeigh++) {
						jPoint = node[iPoint]->GetPoint(iNeigh);
						scalar_prod = 0.0; norm_vect = 0.0; norm_Normal = 0.0;
						for(iDim = 0; iDim < GetnDim(); iDim++) {
							diff_coord = node[jPoint]->GetCoord(iDim)-node[iPoint]->GetCoord(iDim);
							scalar_prod += diff_coord*Normal[iDim];
							norm_vect += diff_coord*diff_coord;
							norm_Normal += Normal[iDim]*Normal[iDim];
						}
						norm_vect = sqrt(norm_vect);
						norm_Normal = sqrt(norm_Normal);
						cos_alpha = scalar_prod/(norm_vect*norm_Normal);
						/*--- Get maximum cosine (not minimum because normals are oriented inwards) ---*/
						if (cos_alpha >= cos_max) {
							Point_Normal = jPoint;
							cos_max = cos_alpha;
						}
					}
					vertex[iMarker][iVertex]->SetClosest_Neighbor(Point_Normal);				
				}
			}
}

CBoundaryGeometry::CBoundaryGeometry(CConfig *config, string val_mesh_filename, unsigned short val_format) : CGeometry() {
  
	string text_line;
	ifstream mesh_file;
	unsigned short iNode_Surface, VTK_Type, iMarker, iChar, iCount = 0;
	unsigned long Point_Surface, iElem_Surface, iElem_Bound = 0, iPoint = 0, iElem = 0, ielem = 0, 
	nelem_edge = 0, nelem_triangle = 0, nelem_quad = 0, 
	vnodes_edge[2], vnodes_triangle[3], 
	vnodes_quad[4], dummy, GlobalIndex;
	string Marker_Tag;
	char cstr[200];
	int rank = MASTER_NODE;
	double Coord_2D[2], Coord_3D[3];
	string::size_type position;
  
#ifndef NO_MPI
	unsigned long LocalIndex;
	rank = MPI::COMM_WORLD.Get_rank();	
#endif
	
	if(rank == MASTER_NODE)
		cout << endl <<"---------------------- Read grid file information -----------------------" << endl;
  
	/*--- Open grid file ---*/
	strcpy (cstr, val_mesh_filename.c_str());
	mesh_file.open(cstr, ios::in);
	if (mesh_file.fail()) {
		cout << "There is no geometry file!!" << endl;
		cout << "Press any key to exit..." << endl;
		cin.get();
#ifdef NO_MPI
		exit(1);	
#else
		MPI::COMM_WORLD.Abort(1);
		MPI::Finalize();
#endif
	}
	
  /*--- Read grid file with format SU2 ---*/
  while (getline (mesh_file,text_line)) {
    
    /*--- Read the dimension of the problem ---*/
    position = text_line.find ("NDIME=",0);
    if (position != string::npos) {
      text_line.erase (0,6); nDim = atoi(text_line.c_str());
			if (rank == MASTER_NODE) {
				if (nDim == 2) cout << "Two dimensional problem." << endl;
				if (nDim == 3) cout << "Three dimensional problem." << endl;
			}
    }
    
		/*--- Read the information about inner elements ---*/
		position = text_line.find ("NELEM=",0);
		if (position != string::npos) {
			text_line.erase (0,6); nElem = atoi(text_line.c_str());
			if (rank == MASTER_NODE)
				cout << nElem << " interior elements." << endl;
			while (iElem < nElem) {
				getline(mesh_file,text_line);
				iElem++; 
			}
		}
			
    /*--- Read number of points ---*/
    position = text_line.find ("NPOIN=",0);
    if (position != string::npos) {
      text_line.erase (0,6);
      
      /*--- Check for ghost points. ---*/
      stringstream test_line(text_line);
      while (test_line >> dummy)
        iCount++;
      
      /*--- Now read and store the number of points and possible ghost points. ---*/
      stringstream  stream_line(text_line);
      if (iCount == 2) {
        stream_line >> nPoint;
        stream_line >> nPointDomain;
#ifdef NO_MPI
				if (rank == MASTER_NODE)
					cout << nPoint << " points, and " << nPoint-nPointDomain << " ghost points." << endl;
#else
				unsigned long Local_nPoint = nPoint, Local_nPointDomain = nPointDomain;
				unsigned long Global_nPoint = 0, Global_nPointDomain = 0;
				MPI::COMM_WORLD.Allreduce(&Local_nPoint, &Global_nPoint, 1, MPI::UNSIGNED_LONG, MPI::SUM); 
				MPI::COMM_WORLD.Allreduce(&Local_nPointDomain, &Global_nPointDomain, 1, MPI::UNSIGNED_LONG, MPI::SUM);
				if (rank == MASTER_NODE)
					cout << Global_nPoint << " points, and " << Global_nPoint-Global_nPointDomain << " ghost points." << endl;
#endif
      } 
			else if (iCount == 1) {
        stream_line >> nPoint;
        nPointDomain = nPoint;
        if (rank == MASTER_NODE) cout << nPoint << " points." << endl;
      } 
			else {
        cout << "NPOIN improperly specified!!" << endl;
        cout << "Press any key to exit..." << endl;
        cin.get();
#ifdef NO_MPI
				exit(1);	
#else
				MPI::COMM_WORLD.Abort(1);
				MPI::Finalize();
#endif
      }
      
      node = new CPoint*[nPoint];
      while (iPoint < nPoint) {
        getline(mesh_file,text_line);
        istringstream point_line(text_line);
        switch(nDim) {
          case 2:
            GlobalIndex = iPoint;
#ifdef NO_MPI
						point_line >> Coord_2D[0]; point_line >> Coord_2D[1];
#else
						point_line >> Coord_2D[0]; point_line >> Coord_2D[1]; point_line >> LocalIndex; point_line >> GlobalIndex;
#endif						
            node[iPoint] = new CPoint(Coord_2D[0], Coord_2D[1], GlobalIndex, config);
            iPoint++; break;
          case 3:
            GlobalIndex = iPoint;
#ifdef NO_MPI
						point_line >> Coord_3D[0]; point_line >> Coord_3D[1]; point_line >> Coord_3D[2];
#else
						point_line >> Coord_3D[0]; point_line >> Coord_3D[1]; point_line >> Coord_3D[2]; point_line >> LocalIndex; point_line >> GlobalIndex;
#endif
            node[iPoint] = new CPoint(Coord_3D[0], Coord_3D[1], Coord_3D[2], GlobalIndex, config);
            iPoint++; break;
        }
      }
    }			
    
    /*--- Read number of markers ---*/
    position = text_line.find ("NMARK=",0);
    if (position != string::npos) {
      text_line.erase (0,6); nMarker = atoi(text_line.c_str());
      if (rank == MASTER_NODE) cout << nMarker << " surface markers." << endl;
      config->SetnMarker_All(nMarker);
      bound = new CPrimalGrid**[nMarker];
      nElem_Bound = new unsigned long [nMarker];
      nElem_Bound_Storage = new unsigned long [nMarker];
      Tag_to_Marker = new string [MAX_INDEX_VALUE];
      
      for (iMarker = 0 ; iMarker < nMarker; iMarker++) {
        getline (mesh_file,text_line);
        text_line.erase (0,11);
        string::size_type position;
        for (iChar = 0; iChar < 20; iChar++) {
          position = text_line.find( " ", 0 );
          if(position != string::npos) text_line.erase (position,1);
          position = text_line.find( "\r", 0 );
          if(position != string::npos) text_line.erase (position,1);
          position = text_line.find( "\n", 0 );
          if(position != string::npos) text_line.erase (position,1);
        }
        Marker_Tag = text_line.c_str();	
        
        /*--- Physical boundaries definition ---*/
        if (Marker_Tag != "SEND_RECEIVE") {						
          getline (mesh_file,text_line);
          text_line.erase (0,13); nElem_Bound[iMarker] = atoi(text_line.c_str());
					if (rank == MASTER_NODE)
						cout << nElem_Bound[iMarker]  << " boundary elements in index "<< iMarker <<" (Marker = " <<Marker_Tag<< ")." << endl;
          bound[iMarker] = new CPrimalGrid* [nElem_Bound[iMarker]];
          
          nelem_edge = 0; nelem_triangle = 0; nelem_quad = 0; ielem = 0;
          for (iElem_Bound = 0; iElem_Bound < nElem_Bound[iMarker]; iElem_Bound++) {
            getline(mesh_file,text_line); 
            istringstream bound_line(text_line);
            bound_line >> VTK_Type;
            switch(VTK_Type) {
              case LINE:
                bound_line >> vnodes_edge[0]; bound_line >> vnodes_edge[1];
                bound[iMarker][ielem] = new CLine(vnodes_edge[0],vnodes_edge[1],2);
                ielem++; nelem_edge++; break;
              case TRIANGLE:
                bound_line >> vnodes_triangle[0]; bound_line >> vnodes_triangle[1]; bound_line >> vnodes_triangle[2];
                bound[iMarker][ielem] = new CTriangle(vnodes_triangle[0],vnodes_triangle[1],vnodes_triangle[2],3);
                ielem++; nelem_triangle++; break;
              case RECTANGLE: 
                bound_line >> vnodes_quad[0]; bound_line >> vnodes_quad[1]; bound_line >> vnodes_quad[2]; bound_line >> vnodes_quad[3];
                bound[iMarker][ielem] = new CRectangle(vnodes_quad[0],vnodes_quad[1],vnodes_quad[2],vnodes_quad[3],3);
                ielem++; nelem_quad++; break;
            }
          }	
          nElem_Bound_Storage[iMarker] = nelem_edge*3 + nelem_triangle*4 + nelem_quad*5;
          
          /*--- Update config information storing the boundary information in the right place ---*/
          Tag_to_Marker[config->GetMarker_Config_Tag(Marker_Tag)] = Marker_Tag;
          config->SetMarker_All_Tag(iMarker, Marker_Tag);
          config->SetMarker_All_Boundary(iMarker, config->GetMarker_Config_Boundary(Marker_Tag));
          config->SetMarker_All_Monitoring(iMarker, config->GetMarker_Config_Monitoring(Marker_Tag));
          config->SetMarker_All_Plotting(iMarker, config->GetMarker_Config_Plotting(Marker_Tag));
          config->SetMarker_All_Moving(iMarker, config->GetMarker_Config_Moving(Marker_Tag));
          config->SetMarker_All_PerBound(iMarker, config->GetMarker_Config_PerBound(Marker_Tag));
          config->SetMarker_All_SendRecv(iMarker, NONE);
          
        }
        
        /*--- Send-Receive boundaries definition ---*/
        else {
          unsigned long nelem_vertex = 0, vnodes_vertex;
          unsigned short transform;
          getline (mesh_file,text_line);
          text_line.erase (0,13); nElem_Bound[iMarker] = atoi(text_line.c_str());
          bound[iMarker] = new CPrimalGrid* [nElem_Bound[iMarker]];
          
          nelem_vertex = 0; ielem = 0;
          getline (mesh_file,text_line); text_line.erase (0,8);
          config->SetMarker_All_Boundary(iMarker, SEND_RECEIVE);
          config->SetMarker_All_SendRecv(iMarker, atoi(text_line.c_str()));
          
          for (iElem_Bound = 0; iElem_Bound < nElem_Bound[iMarker]; iElem_Bound++) {
            getline(mesh_file,text_line);
            istringstream bound_line(text_line);
            bound_line >> VTK_Type; bound_line >> vnodes_vertex; bound_line >> transform;
            
            bound[iMarker][ielem] = new CVertexMPI(vnodes_vertex, nDim);
            bound[iMarker][ielem]->SetRotation_Type(transform);
            ielem++; nelem_vertex++;
            if (config->GetMarker_All_SendRecv(iMarker) < 0) 
              node[vnodes_vertex]->SetDomain(false);
            
          }
          
        }
        
      }
    }
    
    /*--- Read periodic transformation info (center, rotation, translation) ---*/
    position = text_line.find ("NPERIODIC=",0);
    if (position != string::npos) {
      unsigned short nPeriodic, iPeriodic, iIndex;
      
      /*--- Read and store the number of transformations. ---*/
      text_line.erase (0,10); nPeriodic = atoi(text_line.c_str());
			if (rank == MASTER_NODE)
				cout << nPeriodic - 1 << " periodic transformations." << endl;
      config->SetnPeriodicIndex(nPeriodic);
      
      /*--- Store center, rotation, & translation in that order for each. ---*/
      for (iPeriodic = 0; iPeriodic < nPeriodic; iPeriodic++) {
        getline (mesh_file,text_line);
        position = text_line.find ("PERIODIC_INDEX=",0);
        if (position != string::npos) {
          text_line.erase (0,15); iIndex = atoi(text_line.c_str());
          if (iIndex != iPeriodic) {
            cout << "PERIODIC_INDEX out of order in SU2 file!!" << endl;
            cout << "Press any key to exit..." << endl;
						cin.get();
#ifdef NO_MPI
						exit(1);	
#else
						MPI::COMM_WORLD.Abort(1);
						MPI::Finalize();
#endif	
          }
        }
        double* center    = new double[3];
        double* rotation  = new double[3];
        double* translate = new double[3];
        getline (mesh_file,text_line);
        istringstream cent(text_line);
        cent >> center[0]; cent >> center[1]; cent >> center[2];
        config->SetPeriodicCenter(iPeriodic, center);
        getline (mesh_file,text_line);
        istringstream rot(text_line);
        rot >> rotation[0]; rot >> rotation[1]; rot >> rotation[2];
        config->SetPeriodicRotation(iPeriodic, rotation);
        getline (mesh_file,text_line);
        istringstream tran(text_line);
        tran >> translate[0]; tran >> translate[1]; tran >> translate[2];
        config->SetPeriodicTranslate(iPeriodic, translate);
      }
      
    }
    
  }
  
  
  /*--- Loop over the surface element to set the boundaries ---*/
	for (iMarker = 0; iMarker < nMarker; iMarker++)
		for (iElem_Surface = 0; iElem_Surface < nElem_Bound[iMarker]; iElem_Surface++)
			for (iNode_Surface = 0; iNode_Surface < bound[iMarker][iElem_Surface]->GetnNodes(); iNode_Surface++) {				
				Point_Surface = bound[iMarker][iElem_Surface]->GetNode(iNode_Surface);
				node[Point_Surface]->SetBoundary(nMarker);
			}
	
	/*--- Loop over the surface element to set the physical boundaries ---*/
	for (iMarker = 0; iMarker < nMarker; iMarker++)
		if (config->GetMarker_All_Boundary(iMarker) != SEND_RECEIVE)
			if (config->GetMarker_All_Boundary(iMarker) != SEND_RECEIVE &&
					config->GetMarker_All_Boundary(iMarker) != NEARFIELD_BOUNDARY && 
					config->GetMarker_All_Boundary(iMarker) != EULER_WALL &&
					config->GetMarker_All_Boundary(iMarker) != SYMMETRY_PLANE && 
					config->GetMarker_All_Boundary(iMarker) != INTERFACE_BOUNDARY)
        for (iElem_Surface = 0; iElem_Surface < nElem_Bound[iMarker]; iElem_Surface++)
          for (iNode_Surface = 0; iNode_Surface < bound[iMarker][iElem_Surface]->GetnNodes(); iNode_Surface++) {				
            Point_Surface = bound[iMarker][iElem_Surface]->GetNode(iNode_Surface);
            node[Point_Surface]->SetBoundary_Physical(true);
          }
  
}


CBoundaryGeometry::~CBoundaryGeometry(void) {
	if (nElem_Bound != NULL) delete [] nElem_Bound;
	if (Tag_to_Marker != NULL) delete [] Tag_to_Marker;
	if (nVertex != NULL) delete [] nVertex;

/*	for (unsigned short iMarker = 0; iMarker < nMarker; iMarker++)
		for (unsigned long iElem_Bound = 0; iElem_Bound < nElem_Bound[iMarker]; iElem_Bound++)
			bound[iMarker][iElem_Bound]->~CPrimalGrid();
	if (bound != NULL) delete [] bound;*/

	for (unsigned short iMarker = 0; iMarker < nMarker; iMarker++)
		for (unsigned long iVertex = 0; iVertex < nVertex[iMarker]; iVertex++)
			vertex[iMarker][iVertex]->~CVertex();
//			vertex[iMarker][iVertex]->~CDualGrid();
	if (vertex != NULL) delete [] vertex;
}

void CBoundaryGeometry::SetVertex(void) {
	unsigned long  iPoint, iVertex, iElem;
	unsigned short iMarker, iNode;

	/*--- Initialize the Vertex vector for each node of the grid ---*/
	for (iPoint = 0; iPoint < nPoint; iPoint++)
		for (iMarker = 0; iMarker < nMarker; iMarker++)
			node[iPoint]->SetVertex(-1,iMarker); 
	
	/*--- Create and compute the vector with the number of vertex per marker ---*/
	nVertex = new unsigned long [nMarker];
	for (iMarker = 0; iMarker < nMarker; iMarker++) {
		/*--- Initialize the number of Bound Vertex for each Marker ---*/
		nVertex[iMarker] = 0;	
		for (iElem = 0; iElem < nElem_Bound[iMarker]; iElem++)
			for(iNode = 0; iNode < bound[iMarker][iElem]->GetnNodes(); iNode++) {
				iPoint = bound[iMarker][iElem]->GetNode(iNode);
				 /*--- Set the vertex in the node information ---*/
				if (node[iPoint]->GetVertex(iMarker) == -1) {
					iVertex = nVertex[iMarker];
					node[iPoint]->SetVertex(nVertex[iMarker],iMarker);
					nVertex[iMarker]++;
				}
			}
	}
	
	/*--- Initialize the Vertex vector for each node, the previous result is deleted ---*/
	for (iPoint = 0; iPoint < nPoint; iPoint++)
		for (iMarker = 0; iMarker < nMarker; iMarker++)
			node[iPoint]->SetVertex(-1,iMarker); 
	
	/*--- Create the bound vertex structure, note that the order 
	 is the same as in the input file, this is important for Send/Receive part ---*/
	vertex = new CVertex**[nMarker]; 
	for (iMarker = 0; iMarker < nMarker; iMarker++) {
		vertex[iMarker] = new CVertex* [nVertex[iMarker]];
		nVertex[iMarker] = 0;	
		/*--- Initialize the number of Bound Vertex for each Marker ---*/
		for (iElem = 0; iElem < nElem_Bound[iMarker]; iElem++)
			for(iNode = 0; iNode < bound[iMarker][iElem]->GetnNodes(); iNode++) {
				iPoint = bound[iMarker][iElem]->GetNode(iNode);
				/*--- Set the vertex in the node information ---*/
				if (node[iPoint]->GetVertex(iMarker) == -1) {
					iVertex = nVertex[iMarker];
					vertex[iMarker][iVertex] = new CVertex(iPoint, nDim);
					node[iPoint]->SetVertex(nVertex[iMarker],iMarker);
					nVertex[iMarker]++;
				}
			}
	}
}

void CBoundaryGeometry::SetBoundControlVolume(CConfig *config, unsigned short action) {
	unsigned short Neighbor_Node, iMarker, iNode, iNeighbor_Nodes, iDim;
	unsigned long Neighbor_Point, iVertex, iPoint, iElem;
	double **Coord;
	unsigned short nNode;
	unsigned long elem_poin;

	/*--- Center of gravity for face elements ---*/
	for(iMarker = 0; iMarker < nMarker; iMarker++)
		for(iElem = 0; iElem < nElem_Bound[iMarker]; iElem++) {
			nNode = bound[iMarker][iElem]->GetnNodes();
			Coord = new double* [nNode];
			/*--- Store the coordinates for all the element nodes ---*/ 
			for (iNode = 0; iNode < nNode; iNode++) {
				elem_poin = bound[iMarker][iElem]->GetNode(iNode);
				Coord[iNode] = new double [nDim]; 
				for (iDim = 0; iDim < nDim; iDim++)
					Coord[iNode][iDim] = node[elem_poin]->GetCoord(iDim);
			}
			/*--- Compute the element CG coordinates ---*/
			bound[iMarker][iElem]->SetCG(Coord);
			for (iNode=0; iNode < nNode; iNode++)
				if (Coord[iNode] != NULL) delete [] Coord[iNode];
			if (Coord != NULL) delete [] Coord;
		}

	double *Coord_Edge_CG = new double [nDim];
	double *Coord_Elem_CG = new double [nDim];
	double *Coord_Vertex = new double [nDim];
	
	/*--- Loop over all the markers ---*/
	for (iMarker = 0; iMarker < nMarker; iMarker++)
		/*--- Loop over all the boundary elements ---*/
		for (iElem = 0; iElem < nElem_Bound[iMarker]; iElem++)
			/*--- Loop over all the nodes of the boundary ---*/
			for(iNode = 0; iNode < bound[iMarker][iElem]->GetnNodes(); iNode++) { 
				iPoint = bound[iMarker][iElem]->GetNode(iNode);
				iVertex = node[iPoint]->GetVertex(iMarker);
				/*--- Loop over the neighbor nodes, there is a face for each one ---*/
				for(iNeighbor_Nodes = 0; iNeighbor_Nodes < bound[iMarker][iElem]->GetnNeighbor_Nodes(iNode); iNeighbor_Nodes++) {
					Neighbor_Node  = bound[iMarker][iElem]->GetNeighbor_Nodes(iNode,iNeighbor_Nodes);
					Neighbor_Point = bound[iMarker][iElem]->GetNode(Neighbor_Node);
					/*--- Shared edge by the Neighbor Point and the point ---*/
					for (iDim = 0; iDim < nDim; iDim++) {
						Coord_Edge_CG[iDim] = 0.5*(node[iPoint]->GetCoord(iDim) + node[Neighbor_Point]->GetCoord(iDim));
						Coord_Elem_CG[iDim] = bound[iMarker][iElem]->GetCG(iDim);
						Coord_Vertex[iDim]  = node[iPoint]->GetCoord(iDim);
					}
					
					vertex[iMarker][iVertex]->SetCoord(Coord_Vertex);
					
					switch (nDim) {
						case 2:
							/*--- Store the 2D face (ojo hay cambio de sentido para ajustarse al sentido del contorno de nodo 0 al 1) ---*/
							if (iNode == 0) vertex[iMarker][iVertex]->SetNodes_Coord(Coord_Elem_CG, Coord_Vertex, config); 
							if (iNode == 1) vertex[iMarker][iVertex]->SetNodes_Coord(Coord_Vertex, Coord_Elem_CG, config);
							break;
						case 3:  
							/*--- Store the 3D face (ojo hay cambio de sentido para ajustarse al sentido del contorno de nodo 0 al 1) ---*/
							if (iNeighbor_Nodes == 0) vertex[iMarker][iVertex]->SetNodes_Coord(Coord_Elem_CG, Coord_Edge_CG, Coord_Vertex, config);
							if (iNeighbor_Nodes == 1) vertex[iMarker][iVertex]->SetNodes_Coord(Coord_Edge_CG, Coord_Elem_CG, Coord_Vertex, config);
					}
				}
			}

	delete [] Coord_Edge_CG;
	delete [] Coord_Elem_CG;
	delete [] Coord_Vertex;
}

void CBoundaryGeometry::SetBoundSensitivity(CConfig *config) {
	unsigned short iMarker, icommas;
	unsigned long iVertex, iPoint;
	double Sensitivity;
	char cstr[200];
	unsigned long (*Point2Vertex)[2];
	unsigned long nPointLocal = nPoint;
	unsigned long nPointGlobal = 0;

#ifndef NO_MPI
	MPI::COMM_WORLD.Allreduce(&nPointLocal, &nPointGlobal, 1, MPI::UNSIGNED_LONG, MPI::SUM); 
#else
	nPointGlobal = nPointLocal;
#endif
	
	Point2Vertex = new unsigned long[nPointGlobal][2];
	
	for (iMarker = 0; iMarker < nMarker; iMarker++)
		for (iVertex = 0; iVertex < nVertex[iMarker]; iVertex++) {
			
			/*--- The sensitivity file use the global numbering ---*/
#ifdef NO_MPI
			iPoint = vertex[iMarker][iVertex]->GetNode();
#else
			unsigned long LocalPoint = vertex[iMarker][iVertex]->GetNode();
			iPoint = node[LocalPoint]->GetGlobalIndex();
#endif
			Point2Vertex[iPoint][0] = iMarker;
			Point2Vertex[iPoint][1] = iVertex;
		}	
	
	string text_line;
	ifstream Surface_file;

#ifdef NO_MPI
	char buffer[50];
	strcpy (cstr, config->GetSurfAdjCoeff_FileName().c_str());
	sprintf (buffer, ".csv");
	strcat (cstr, buffer);
#else
	strcpy (cstr, config->GetSurfAdjCSV_FileName().c_str());
#endif

	string::size_type position;

	Surface_file.open(cstr, ios::in);
	getline(Surface_file,text_line);
	
	while (getline(Surface_file,text_line)) {
		for (icommas = 0; icommas < 50; icommas++) {
			position = text_line.find( ",", 0 );
			if(position!=string::npos) text_line.erase (position,1);
		}
		stringstream  point_line(text_line);
		point_line >> iPoint >> Sensitivity;
					
		/*--- Find the vertex for the Point and Marker ---*/
		iMarker = Point2Vertex[iPoint][0];
		iVertex = Point2Vertex[iPoint][1];
		
		vertex[iMarker][iVertex]->SetAuxVar(Sensitivity);	
		
	}
	Surface_file.close();
	
	delete [] Point2Vertex;
}

void CBoundaryGeometry::SetBoundParaView (CConfig *config, char mesh_filename[200]) {
	ofstream Paraview_File;
	unsigned long iPoint, Total_nElem_Bound, Total_nElem_Bound_Storage, iElem;
	unsigned short Coord_i, iMarker, iNode;
	
	Paraview_File.open(mesh_filename, ios::out);

	Paraview_File << "# vtk DataFile Version 2.0" << endl;
	Paraview_File << "Visualization of the surface grid" << endl;
	Paraview_File << "ASCII" << endl;
	Paraview_File << "DATASET UNSTRUCTURED_GRID" << endl;
	Paraview_File << "POINTS " << nPoint << " float" << endl;

	if (nDim == 3) {
		for(iPoint = 0; iPoint < nPoint; iPoint++) {
			for(Coord_i = 0; Coord_i < nDim-1; Coord_i++)
				Paraview_File << node[iPoint]->GetCoord(Coord_i) << "\t";
			Paraview_File << node[iPoint]->GetCoord(nDim-1) << "\n";
		}
	}
	else {
		for(iPoint = 0; iPoint < nPoint; iPoint++) {
			for(Coord_i = 0; Coord_i < nDim; Coord_i++)
				Paraview_File << node[iPoint]->GetCoord(Coord_i) << "\t";
			Paraview_File << "0.0\n";
		}
	}
	
	
	Total_nElem_Bound = 0; Total_nElem_Bound_Storage = 0;
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if (config->GetMarker_All_Plotting(iMarker) == YES) {
			Total_nElem_Bound += nElem_Bound[iMarker];
			Total_nElem_Bound_Storage += nElem_Bound_Storage[iMarker];
		}
	
	Paraview_File << "CELLS " << Total_nElem_Bound << "\t" << Total_nElem_Bound_Storage << endl;
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++) 
		if (config->GetMarker_All_Plotting(iMarker) == YES) 
			for(iElem = 0; iElem<nElem_Bound[iMarker]; iElem++) {
				Paraview_File << bound[iMarker][iElem]->GetnNodes() << "\t";
				for(iNode = 0; iNode < bound[iMarker][iElem]->GetnNodes()-1; iNode++)
					Paraview_File << bound[iMarker][iElem]->GetNode(iNode) << "\t";			
				Paraview_File << bound[iMarker][iElem]->GetNode(bound[iMarker][iElem]->GetnNodes()-1) << "\n";
			}
		
	Paraview_File << "CELL_TYPES " << Total_nElem_Bound << endl;
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++) 
		if (config->GetMarker_All_Plotting(iMarker) == YES) 
			for(iElem = 0; iElem < nElem_Bound[iMarker]; iElem++) 
				Paraview_File << bound[iMarker][iElem]->GetVTK_Type() << endl;

	Paraview_File.close();
}

CDomainGeometry::CDomainGeometry(CGeometry *geometry, CConfig *config, unsigned short val_domain) {
	
	unsigned long iElemDomain, iPointDomain, iPointGhost, iPointReal, 
	nVertexDomain[MAX_NUMBER_MARKER], iPoint, iElem, iVertex, nElem_Storage = 0, 
	nelem_edge = 0, nelem_triangle = 0, nelem_quad = 0, nelem_tetra = 0, 
	nelem_hexa = 0, nelem_wedge = 0, nelem_pyramid = 0;
	long vnodes_global[8], vnodes_local[8];
	unsigned short iNode, iDim, iMarker, nMarkerDomain, nDomain, iDomain;
	double coord[3];
	bool ElemIn, VertexIn, MarkerIn;
	
	/*--- Auxiliar vector to change the numbering ---*/
	Global_to_Local_Point =  new long[geometry->GetnPoint()];
	for (iPoint = 0; iPoint < geometry->GetnPoint(); iPoint++) 
		Global_to_Local_Point[iPoint] = -1;
	
	/*--- Set the number of dimensions of the grid ---*/
	nDim = geometry->GetnDim();
	
	/*--- Bucle over the original grid to perform the dimensionalizaton of the new vectors ---*/
	nElem = 0; nPoint = 0; nPointGhost = 0; nPointDomain = 0;
	for (iElem = 0; iElem < geometry->GetnElem(); iElem++) {
		
		/*--- Check if the element belong to the domain ---*/
		ElemIn = false;
		for (iNode = 0; iNode < geometry->elem[iElem]->GetnNodes(); iNode++) {
			iPoint = geometry->elem[iElem]->GetNode(iNode);
			if ( geometry->node[iPoint]->GetColor() == val_domain ) {
				ElemIn = true; break; 
			}
		}
		
		/*--- If an element belong to the domain (at least one point belong 
		 to the boundary)---*/ 
		if (ElemIn) {
			for (iNode = 0; iNode < geometry->elem[iElem]->GetnNodes(); iNode++) {
				iPoint = geometry->elem[iElem]->GetNode(iNode);
				if (Global_to_Local_Point[iPoint] == -1) {
					Global_to_Local_Point[iPoint] = 1;
					nPoint++;
					if ( geometry->node[iPoint]->GetColor() != val_domain ) nPointGhost++;
					else nPointDomain++;
				}
			}
			nElem++;
		}
	}
	
	/*--- Auxiliar vector to store the local to global index ---*/
	Local_to_Global_Point =  new unsigned long[nPoint];
	
	/*--- Dimensionate the number of elements and nodes of the domain ---*/
	elem = new CPrimalGrid*[nElem];
	node = new CPoint*[nPoint];
	
	/*--- Reset auxiliar vector to change the numbering ---*/
	iElemDomain = 0; iPointDomain = 0; iPointReal = 0; iPointGhost = nPointDomain;
	for (iPoint = 0; iPoint < geometry->GetnPoint(); iPoint++) 
		Global_to_Local_Point[iPoint] = -1;
	
	
	/*--- Bucle over the original grid to create the point and element structures ---*/
	for (iElem = 0; iElem < geometry->GetnElem(); iElem++) {
		
		/*--- Check if the element belong to the domain ---*/
		ElemIn = false;
		for (iNode = 0; iNode < geometry->elem[iElem]->GetnNodes(); iNode++) {
			iPoint = geometry->elem[iElem]->GetNode(iNode);
			if ( geometry->node[iPoint]->GetColor() == val_domain ) {
				ElemIn = true;
				break;
			}
		}
		
		if (ElemIn) {
			for (iNode = 0; iNode < geometry->elem[iElem]->GetnNodes(); iNode++) {
				iPoint = geometry->elem[iElem]->GetNode(iNode);
				
				if (Global_to_Local_Point[iPoint] == -1) {
					
					if ( geometry->node[iPoint]->GetColor() == val_domain ) iPointDomain = iPointReal;
					else iPointDomain = iPointGhost;
					
					Global_to_Local_Point[iPoint] = iPointDomain;
					for (iDim = 0; iDim < nDim; iDim++)
						coord[iDim] = geometry->node[iPoint]->GetCoord(iDim);
					
					Local_to_Global_Point[iPointDomain] = iPoint;
					if ( nDim == 2 ) node[iPointDomain] = new CPoint(coord[0], coord[1], iPoint, config);
					if ( nDim == 3 ) node[iPointDomain] = new CPoint(coord[0], coord[1], coord[2], iPoint, config);
					
					if ( geometry->node[iPoint]->GetColor() == val_domain ) iPointReal++;
					else iPointGhost++;
				}
				
				vnodes_local[iNode] = Global_to_Local_Point[iPoint];
				
			}
			
			switch(geometry->elem[iElem]->GetVTK_Type()) {
				case TRIANGLE:
					elem[iElemDomain] = new CTriangle(vnodes_local[0], vnodes_local[1], vnodes_local[2], 2);
					nelem_triangle++;
					break;
				case RECTANGLE: 
					elem[iElemDomain] = new CRectangle(vnodes_local[0], vnodes_local[1], vnodes_local[2],
													   vnodes_local[3], 2); 
					nelem_quad++;
					break;
				case TETRAHEDRON: 
					elem[iElemDomain] = new CTetrahedron(vnodes_local[0], vnodes_local[1], vnodes_local[2],
														 vnodes_local[3]); 
					nelem_tetra++;
					break;
				case HEXAHEDRON: 
					elem[iElemDomain] = new CHexahedron(vnodes_local[0], vnodes_local[1], vnodes_local[2],
														vnodes_local[3], vnodes_local[4], vnodes_local[5],
														vnodes_local[6], vnodes_local[7]); 
					nelem_hexa++;
					break;
				case WEDGE: 
					elem[iElemDomain] = new CWedge(vnodes_local[0], vnodes_local[1], vnodes_local[2],
												   vnodes_local[3], vnodes_local[4], vnodes_local[5]);
					nelem_wedge++;
					break;
				case PYRAMID: 
					elem[iElemDomain] = new CPyramid(vnodes_local[0], vnodes_local[1], vnodes_local[2],
													 vnodes_local[3], vnodes_local[4]);
					nelem_pyramid++;
					break;
			}
			iElemDomain++;
		}
	}
	
	nElem_Storage = nelem_triangle*4 + nelem_quad*5 + nelem_tetra*5 + nelem_hexa*9 + nelem_wedge*7 + nelem_pyramid*6; 
	
	SetnElem_Storage(nElem_Storage); SetnElem(nElem); SetnPoint(nPoint);
	
	/*--- Dimensionalization with physical boundaries ---*/
	nMarkerDomain = 0;
	for (iMarker = 0; iMarker < geometry->GetnMarker(); iMarker++) {
    if (config->GetMarker_All_Boundary(iMarker) != SEND_RECEIVE ) {
      MarkerIn = false; nVertexDomain[nMarkerDomain] = 0;
      for (iVertex = 0; iVertex < geometry->GetnElem_Bound(iMarker); iVertex++) {
        VertexIn = false;
        for (iNode = 0; iNode < geometry->bound[iMarker][iVertex]->GetnNodes(); iNode++) {
          vnodes_global[iNode] = geometry->bound[iMarker][iVertex]->GetNode(iNode);
          vnodes_local[iNode] = Global_to_Local_Point[vnodes_global[iNode]];
          if (geometry->node[vnodes_global[iNode]]->GetColor() == val_domain ) VertexIn = true;
        }
        if (VertexIn) { nVertexDomain[nMarkerDomain] ++;  MarkerIn = true; }
      }
      if (MarkerIn) { nMarkerDomain++; }
    }
  }
	
	nDomain = config->GetnDomain();
	
	/*--- It is necesary to add the Send Boundaries ---*/
	for (iDomain = 0; iDomain < nDomain; iDomain++)
		if (geometry->SendDomain[val_domain][iDomain].size() != 0) {
			nVertexDomain[nMarkerDomain] = geometry->SendDomain[val_domain][iDomain].size();
			nMarkerDomain++;
		}
	
	/*--- It is also necesary to add the Receive Boundaries ---*/
	for (iDomain = 0; iDomain < nDomain; iDomain++)
		if (geometry->SendDomain[iDomain][val_domain].size() != 0) {
			nVertexDomain[nMarkerDomain] = geometry->SendDomain[iDomain][val_domain].size();
			nMarkerDomain++;
		}
	
	SetnMarker(nMarkerDomain);
	nElem_Bound = new unsigned long [nMarkerDomain];
	Local_to_Global_Marker = new unsigned short [nMarkerDomain];
	Global_to_Local_Marker = new unsigned short [geometry->GetnMarker()];
	for (iMarker = 0; iMarker < nMarkerDomain; iMarker++) {
		SetnElem_Bound(iMarker, nVertexDomain[iMarker]);
	}
	
	nElem_Bound_Storage = new unsigned long [nMarkerDomain];
	
	bound = new CPrimalGrid**[GetnMarker()];
	for (iMarker = 0; iMarker < GetnMarker(); iMarker++) {
		bound[iMarker] = new CPrimalGrid* [GetnElem_Bound(iMarker)];
	}
	
	/*--- Bucle over the original grid to create the boundaries ---*/
	nMarkerDomain = 0; 
	for (iMarker = 0; iMarker < geometry->GetnMarker(); iMarker++) {
		nelem_edge = 0; nelem_triangle = 0; nelem_quad = 0;
		MarkerIn = false; nVertexDomain[nMarkerDomain] = 0;
		for (iVertex = 0; iVertex < geometry->GetnElem_Bound(iMarker); iVertex++) {
			VertexIn = false; 
			for (iNode = 0; iNode < geometry->bound[iMarker][iVertex]->GetnNodes(); iNode++) {
				vnodes_global[iNode] = geometry->bound[iMarker][iVertex]->GetNode(iNode);
				vnodes_local[iNode] = Global_to_Local_Point[vnodes_global[iNode]];
				if (geometry->node[vnodes_global[iNode]]->GetColor() == val_domain ) VertexIn = true;
			}
			if (VertexIn) { 
				switch(geometry->bound[iMarker][iVertex]->GetVTK_Type()) {
					case LINE:
						bound[nMarkerDomain][nVertexDomain[nMarkerDomain]] = new CLine(vnodes_local[0],vnodes_local[1],2);
						nelem_edge++;
						break;
					case TRIANGLE:
						bound[nMarkerDomain][nVertexDomain[nMarkerDomain]] = new CTriangle(vnodes_local[0],vnodes_local[1],
																						   vnodes_local[2],3);
						nelem_triangle++;						
						break;
					case RECTANGLE: 
						bound[nMarkerDomain][nVertexDomain[nMarkerDomain]] = new CRectangle(vnodes_local[0],vnodes_local[1],
																							vnodes_local[2],vnodes_local[3],3);
						nelem_quad++;
						break;
				}
				nVertexDomain[nMarkerDomain] ++; VertexIn = true; MarkerIn = true; 
				
			}
		}
		if (MarkerIn) {
			nElem_Bound_Storage[nMarkerDomain] = nelem_edge*3+nelem_triangle*4+nelem_quad*5;
			Local_to_Global_Marker[nMarkerDomain] = iMarker;
			Global_to_Local_Marker[iMarker] = nMarkerDomain;
			nMarkerDomain++; 
		}
	}
	
	/*--- Dealing with periodic boundary condition, the number 
	 of nPointDomain must be reduced ---*/
	unsigned long nPointPeriodic = 0;
	for (iPoint = 0; iPoint < nPointDomain; iPoint++) {
		unsigned long GlobalPoint = Local_to_Global_Point[iPoint];
		if (GlobalPoint >= geometry->GetnPointDomain()) {
			nPointPeriodic++;
		}
	}
	
	nPointDomain = nPointDomain - nPointPeriodic;
	nPointGhost = nPoint - nPointDomain;
	
	cout << "Domain "<< val_domain << " has " << nPoint << " points including " << 
	nPointGhost << " ghost points, and " << nPointPeriodic << " periodic points." <<endl;
	
}

CDomainGeometry::~CDomainGeometry(void) {
	
	if (Global_to_Local_Point != NULL) delete [] Global_to_Local_Point;
	if (Local_to_Global_Point != NULL) delete [] Local_to_Global_Point;
	if (Local_to_Global_Marker != NULL) delete [] Local_to_Global_Marker;
	if (Global_to_Local_Marker != NULL) delete [] Global_to_Local_Marker;
}

void CDomainGeometry::SetSendReceive(CGeometry *geometry, CConfig *config, unsigned short val_domain) {
	unsigned short iDomain, nDomain, Counter_Send, Counter_Receive, iMarkerSend, iMarkerReceive;
	unsigned long iVertex, GlobalNode, LocalNode;
	long GlobalNodeAux;
	
	nDomain = config->GetnDomain();
	
	/*--- First compute the Send/Receive bundaries ---*/
	Counter_Send = 0; 	Counter_Receive = 0;
	for (iDomain = 0; iDomain < nDomain; iDomain++)
		if (geometry->SendDomain[val_domain][iDomain].size() != 0) Counter_Send++;
	for (iDomain = 0; iDomain < nDomain; iDomain++)
		if (geometry->SendDomain[iDomain][val_domain].size() != 0) Counter_Receive++;
	
	iMarkerSend = GetnMarker() - Counter_Send - Counter_Receive;
	iMarkerReceive = GetnMarker() - Counter_Receive;
	 
	/*--- First we do the send ---*/
	for (iDomain = 0; iDomain < nDomain; iDomain++) {
		if (geometry->SendDomain[val_domain][iDomain].size() != 0) {
			for (iVertex = 0; iVertex < GetnElem_Bound(iMarkerSend); iVertex++) {
				GlobalNode = geometry->SendDomain[val_domain][iDomain][iVertex];
				LocalNode = Global_to_Local_Point[GlobalNode];
				bound[iMarkerSend][iVertex] = new CVertexMPI(LocalNode,3);
				bound[iMarkerSend][iVertex]->SetRotation_Type(geometry->SendTransf[val_domain][iDomain][iVertex]);
			}
			config->SetMarker_All_SendRecv(iMarkerSend,iDomain+1);
			iMarkerSend++;
		}
	}
	
	/*--- Second we do the receive ---*/
	for (iDomain = 0; iDomain < nDomain; iDomain++) {
		if (geometry->SendDomain[iDomain][val_domain].size() != 0) {
			for (iVertex = 0; iVertex < GetnElem_Bound(iMarkerReceive); iVertex++) {
				GlobalNode = geometry->SendDomain[iDomain][val_domain][iVertex];

				/*--- Dealing with periodic boundary conditions, we must take in account that the donor
				 node is not on the domain, and we must pick the node that is in the domain ---*/
				if (geometry->SendTransf[iDomain][val_domain][iVertex] != 0) {
					if (geometry->PeriodicDomainIndex[GlobalNode] != -1) {
						GlobalNodeAux = geometry->PeriodicDomainIndex[GlobalNode];
						if (Global_to_Local_Point[GlobalNodeAux] != -1)
							GlobalNode = GlobalNodeAux;
					}
				}
				
				LocalNode = Global_to_Local_Point[GlobalNode];
		
				bound[iMarkerReceive][iVertex] = new CVertexMPI(LocalNode,3);
				bound[iMarkerReceive][iVertex]->SetRotation_Type(geometry->SendTransf[iDomain][val_domain][iVertex]);
			}
			config->SetMarker_All_SendRecv(iMarkerReceive,-(iDomain+1));
			iMarkerReceive++;
		}
	}
	
}


void CDomainGeometry::SetMeshFile (CConfig *config, string val_mesh_out_filename) {
	
	unsigned long iElem, iPoint, iElem_Bound;
	unsigned short iMarker, iNodes, iDim, iPeriodic, nPeriodic = 0;
	double *center, *angles, *transl;
	ofstream output_file;
	string Grid_Marker;
	
	char *cstr = new char [val_mesh_out_filename.size()+1];
	strcpy (cstr, val_mesh_out_filename.c_str());
	
	output_file.open(cstr, ios::out);
	
	output_file << "NDIME= " << nDim << endl;
	output_file << "NELEM= " << nElem << endl;
	
	for (iElem = 0; iElem < nElem; iElem++) {
		output_file << elem[iElem]->GetVTK_Type();
		for (iNodes = 0; iNodes < elem[iElem]->GetnNodes(); iNodes++)
			output_file << "\t" << elem[iElem]->GetNode(iNodes);
		output_file << "\t"<<iElem<<endl;	
	}
	
	output_file << "NPOIN= " << nPoint << "\t" << nPointDomain <<endl;
	output_file.precision(15);
	for (iPoint = 0; iPoint < nPoint; iPoint++) {
		for (iDim = 0; iDim < nDim; iDim++)
			output_file << scientific << "\t" << node[iPoint]->GetCoord(iDim) ;
		output_file << "\t" << iPoint << "\t" << Local_to_Global_Point[iPoint] << endl;
	}
	
	output_file << "NMARK= " << nMarker << endl;
	for (iMarker = 0; iMarker < nMarker; iMarker++) {
		
		if (bound[iMarker][0]->GetVTK_Type() != VERTEX) {
			
			Grid_Marker = config->GetMarker_All_Tag(Local_to_Global_Marker[iMarker]);
			output_file << "MARKER_TAG= " << Grid_Marker <<endl;
			output_file << "MARKER_ELEMS= " << nElem_Bound[iMarker]<< endl;

			for (iElem_Bound = 0; iElem_Bound < nElem_Bound[iMarker]; iElem_Bound++) {
				output_file << bound[iMarker][iElem_Bound]->GetVTK_Type() << "\t" ;
				for (iNodes = 0; iNodes < bound[iMarker][iElem_Bound]->GetnNodes()-1; iNodes++)
					output_file << bound[iMarker][iElem_Bound]->GetNode(iNodes) << "\t" ;
				iNodes = bound[iMarker][iElem_Bound]->GetnNodes()-1;
				output_file << bound[iMarker][iElem_Bound]->GetNode(iNodes) << endl;
			}
		}
		
		if (bound[iMarker][0]->GetVTK_Type() == VERTEX) {
			output_file << "MARKER_TAG= SEND_RECEIVE" << endl;
			output_file << "MARKER_ELEMS= " << nElem_Bound[iMarker]<< endl;
			if (config->GetMarker_All_SendRecv(iMarker) > 0) output_file << "SEND_TO= " << config->GetMarker_All_SendRecv(iMarker) << endl;
			if (config->GetMarker_All_SendRecv(iMarker) < 0) output_file << "SEND_TO= " << config->GetMarker_All_SendRecv(iMarker) << endl;

			for (iElem_Bound = 0; iElem_Bound < nElem_Bound[iMarker]; iElem_Bound++) {
				output_file << bound[iMarker][iElem_Bound]->GetVTK_Type() << "\t" ;
				output_file << bound[iMarker][iElem_Bound]->GetNode(0) << "\t";
				output_file << bound[iMarker][iElem_Bound]->GetRotation_Type() << endl;
			}
		}
	}
  
  /*--- Get the total number of periodic transformations ---*/
  nPeriodic = config->GetnPeriodicIndex();
	output_file << "NPERIODIC= " << nPeriodic << endl;
		
	/*--- From iPeriodic obtain the iMarker ---*/
	for (iPeriodic = 0; iPeriodic < nPeriodic; iPeriodic++) {
		
		/*--- Retrieve the supplied periodic information. ---*/
		center = config->GetPeriodicCenter(iPeriodic);
		angles = config->GetPeriodicRotation(iPeriodic);
		transl = config->GetPeriodicTranslate(iPeriodic);		
		
		output_file << "PERIODIC_INDEX= " << iPeriodic << endl;
		output_file << center[0] << "\t" << center[1] << "\t" << center[2] << endl;
		output_file << angles[0] << "\t" << angles[1] << "\t" << angles[2] << endl;
		output_file << transl[0] << "\t" << transl[1] << "\t" << transl[2] << endl;
    
  }
  
	output_file.close();
}

void CDomainGeometry::SetParaView(char mesh_filename[200]) {
	
	ofstream para_file;
	para_file.open(mesh_filename, ios::out);
	
	para_file << "# vtk DataFile Version 2.0" << endl;
	para_file << "Visualization of the volumetric grid" << endl;
	para_file << "ASCII" << endl;
	para_file << "DATASET UNSTRUCTURED_GRID" << endl;
	para_file << "POINTS " << nPoint << " float" << endl;
	
	para_file.precision(15);
	
	for(unsigned long iPoint = 0; iPoint < nPoint; iPoint++) {
		for(unsigned short iDim = 0; iDim < nDim; iDim++)
			para_file << scientific << node[iPoint]->GetCoord(iDim) << "\t";
		if (nDim == 2) para_file << "0.0";
		para_file << "\n";
	}	 
	
	para_file << "CELLS " << nElem << "\t" << nElem_Storage << endl;
	for(unsigned long iElem=0; iElem < nElem; iElem++) {
		para_file << elem[iElem]->GetnNodes() << "\t";
		for(unsigned short iNode=0; iNode < elem[iElem]->GetnNodes(); iNode++)
			para_file << elem[iElem]->GetNode(iNode) << "\t";			
		para_file << "\n";
	}	
	
	para_file << "CELL_TYPES " << nElem << endl;
	for(unsigned long iElem=0; iElem < nElem; iElem++) {
		para_file << elem[iElem]->GetVTK_Type() << endl;
	}
	para_file.close();
}

void CDomainGeometry::SetTecPlot(char mesh_filename[200]) {
	unsigned long iPoint, iElem;
	
	ofstream para_file;
	para_file.open(mesh_filename, ios::out);
	
	para_file << " VARIABLES = \"x\",\"y\" " << endl;
	para_file << " ZONE T = \"Time= 0.0\", N = "<< nPoint <<" , E = "<< nElem <<" , F = FEPOINT, ET = QUADRILATERAL"<< endl;
	
	for(iPoint = 0; iPoint < nPoint; iPoint++)
		para_file << node[iPoint]->GetCoord(0) <<" "<< node[iPoint]->GetCoord(1) << endl;
	
	for(iElem = 0; iElem < nElem; iElem++)
		para_file << elem[iElem]->GetNode(0)+1 <<" "<< 
		elem[iElem]->GetNode(1)+1 <<" "<< elem[iElem]->GetNode(2)+1 <<" "<< 
		elem[iElem]->GetNode(2)+1 << endl;
	
	para_file.close();
}

void CDomainGeometry::SetBoundParaView (CConfig *config, char mesh_filename[200]) {
	ofstream para_file;
	para_file.open(mesh_filename, ios::out);
	
	para_file << "# vtk DataFile Version 2.0" << endl;
	para_file << "Visualization of the surface grid" << endl;
	para_file << "ASCII" << endl;
	para_file << "DATASET UNSTRUCTURED_GRID" << endl;
	para_file << "POINTS " << nPoint << " float" << endl;
	
	for(unsigned long iPoint = 0; iPoint<nPoint; iPoint++) {
		for(unsigned short Coord_i = 0; Coord_i < 3; Coord_i++)
			para_file << node[iPoint]->GetCoord(Coord_i) << "\t";
		para_file << "\n";
	}	 
	
	unsigned long Total_nElem_Bound = 0;
	unsigned long Total_nElem_Bound_Storage = 0;
	for (unsigned short iMarker=0; iMarker < config->GetnMarker_All(); iMarker++) {
		unsigned short Boundary = config->GetMarker_All_Boundary(iMarker);
		if ((Boundary == NO_SLIP_WALL)||(Boundary == EULER_WALL)) {
			Total_nElem_Bound += nElem_Bound[iMarker];
			Total_nElem_Bound_Storage += nElem_Bound_Storage[iMarker];
		}
	}
	
	para_file << "CELLS " << Total_nElem_Bound << "\t" << Total_nElem_Bound_Storage << endl;
	for (unsigned short iMarker=0; iMarker < config->GetnMarker_All(); iMarker++) {
		unsigned short Boundary = config->GetMarker_All_Boundary(iMarker);
		if ((Boundary == NO_SLIP_WALL)||(Boundary == EULER_WALL)) {
			for(unsigned long ielem=0; ielem<nElem_Bound[iMarker]; ielem++) {
				para_file << bound[iMarker][ielem]->GetnNodes() << "\t";
				for(unsigned short inode=0; inode<bound[iMarker][ielem]->GetnNodes(); inode++)
					para_file << bound[iMarker][ielem]->GetNode(inode) << "\t";			
				para_file << "\n";
			}
		}
	}
	
	para_file << "CELL_TYPES " << Total_nElem_Bound << endl;
	for (unsigned short iMarker=0; iMarker < config->GetnMarker_All(); iMarker++) {
		unsigned short Boundary = config->GetMarker_All_Boundary(iMarker);
		if ((Boundary == NO_SLIP_WALL)||(Boundary == EULER_WALL)) {
			for(unsigned long ielem=0; ielem <nElem_Bound[iMarker]; ielem++) {
				para_file << bound[iMarker][ielem]->GetVTK_Type() << endl;
			}
		}
	}
	
	
	
	para_file.close();
}

CPeriodicGeometry::CPeriodicGeometry(CGeometry *geometry, CConfig *config) {
	unsigned long nElem_new, nPoint_new, jPoint, iPoint, iElem, jElem, iVertex, 
	nelem_triangle = 0, nelem_quad = 0, nelem_tetra = 0, nelem_hexa = 0, nelem_wedge = 0, 
	nelem_pyramid = 0, iIndex, newElementsBound = 0;
	unsigned short  iMarker, nPeriodic = 0, iPeriodic;
	double *center, *angles, rotMatrix[3][3] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, 
	translation[3], *trans, theta, phi, psi, cosTheta, sinTheta, cosPhi, sinPhi, cosPsi, sinPsi, 
	dx, dy, dz, rotCoord[3], *Coord_i;
	
	/*--- Compute the number of periodic bc on the geometry ---*/
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if (config->GetMarker_All_Boundary(iMarker) == PERIODIC_BOUNDARY)
			nPeriodic++;
		
	/*--- Write the number of dimensions of the problem ---*/
	nDim = geometry->GetnDim();
	
  /*--- Copy the new boundary element information from the geometry class. 
        Be careful, as these are pointers to vectors/objects. ---*/
  nNewElem_BoundPer = geometry->nNewElem_Bound;
  newBoundPer       = geometry->newBound;
  
  /*--- Count the number of new boundary elements. ---*/
  for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
    newElementsBound += nNewElem_BoundPer[iMarker];
    
	/*--- Bucle over the original grid to perform the dimensionalizaton of the new vectors ---*/
	nElem_new = 0; nPoint_new = 0; 
	for (iPeriodic = 1; iPeriodic <= nPeriodic; iPeriodic++) {
		nElem_new += geometry->PeriodicElem[iPeriodic].size();
		nPoint_new += geometry->PeriodicPoint[iPeriodic][0].size();
	}
	
  cout << "Number of new points: " << nPoint_new << "." << endl;
	cout << "Number of new interior elements: " << nElem_new << "." << endl;
	cout << "Number of new boundary elements added to preexisting markers: " << newElementsBound << "." << endl;
  
	/*--- Create a copy of the original grid ---*/
	elem = new CPrimalGrid*[geometry->GetnElem() + nElem_new];
	for (iElem = 0; iElem < geometry->GetnElem(); iElem ++) {
		switch(geometry->elem[iElem]->GetVTK_Type()) {
			case TRIANGLE:
				elem[iElem] = new CTriangle(geometry->elem[iElem]->GetNode(0), 
											geometry->elem[iElem]->GetNode(1), 
											geometry->elem[iElem]->GetNode(2), 2);
				nelem_triangle++;
				break;
				
			case RECTANGLE:
				elem[iElem] = new CRectangle(geometry->elem[iElem]->GetNode(0), 
											 geometry->elem[iElem]->GetNode(1), 
											 geometry->elem[iElem]->GetNode(2), 
											 geometry->elem[iElem]->GetNode(3), 2);
				nelem_quad++;
				break;
				
			case TETRAHEDRON: 
				elem[iElem] = new CTetrahedron(geometry->elem[iElem]->GetNode(0),
											   geometry->elem[iElem]->GetNode(1),
											   geometry->elem[iElem]->GetNode(2),
											   geometry->elem[iElem]->GetNode(3));
				nelem_tetra++;
				break;
				
			case HEXAHEDRON:
				elem[iElem] = new CHexahedron(geometry->elem[iElem]->GetNode(0),
											  geometry->elem[iElem]->GetNode(1),
											  geometry->elem[iElem]->GetNode(2),
											  geometry->elem[iElem]->GetNode(3),
											  geometry->elem[iElem]->GetNode(4),
											  geometry->elem[iElem]->GetNode(5),
											  geometry->elem[iElem]->GetNode(6),
											  geometry->elem[iElem]->GetNode(7));
				nelem_hexa++;
				break;
				
			case WEDGE:
				elem[iElem] = new CWedge(geometry->elem[iElem]->GetNode(0),
											  geometry->elem[iElem]->GetNode(1),
											  geometry->elem[iElem]->GetNode(2),
											  geometry->elem[iElem]->GetNode(3),
											  geometry->elem[iElem]->GetNode(4),
											  geometry->elem[iElem]->GetNode(5));
				nelem_wedge++;
				break;
				
			case PYRAMID:
				elem[iElem] = new CPyramid(geometry->elem[iElem]->GetNode(0),
										   geometry->elem[iElem]->GetNode(1),
										   geometry->elem[iElem]->GetNode(2),
										   geometry->elem[iElem]->GetNode(3),
										   geometry->elem[iElem]->GetNode(4));
				nelem_pyramid++;
				break;
				
		}
	}
	
	/*--- Create a list with all the points and the new index ---*/
	unsigned long *Index = new unsigned long [geometry->GetnPoint()];
	for (iPoint = 0; iPoint < geometry->GetnPoint(); iPoint ++) Index[iPoint] = 0;
		
	for (iPeriodic = 1; iPeriodic <= nPeriodic; iPeriodic++)
		for (iIndex = 0; iIndex < geometry->PeriodicPoint[iPeriodic][0].size(); iIndex++) {
			iPoint =  geometry->PeriodicPoint[iPeriodic][0][iIndex];
			Index[iPoint] = geometry->PeriodicPoint[iPeriodic][1][iIndex];
		}
	
	for (iMarker = 0; iMarker < geometry->GetnMarker(); iMarker++)
		if (config->GetMarker_All_Boundary(iMarker) == PERIODIC_BOUNDARY)
			for (iVertex = 0; iVertex < geometry->GetnVertex(iMarker); iVertex++) {
				iPoint = geometry->vertex[iMarker][iVertex]->GetNode();
				jPoint = geometry->vertex[iMarker][iVertex]->GetPeriodicPoint();
				Index[iPoint] = jPoint;
			}
	
	/*--- Add the new elements due to the periodic boundary condtion ---*/
	iElem = geometry->GetnElem();
	
	for (iPeriodic = 1; iPeriodic <= nPeriodic; iPeriodic++)
		for (iIndex = 0; iIndex < geometry->PeriodicElem[iPeriodic].size(); iIndex++) {
			jElem = geometry->PeriodicElem[iPeriodic][iIndex];
			
			switch(geometry->elem[jElem]->GetVTK_Type()) {
				case TRIANGLE:
					elem[iElem] = new CTriangle(Index[geometry->elem[jElem]->GetNode(0)], 
												Index[geometry->elem[jElem]->GetNode(1)], 
												Index[geometry->elem[jElem]->GetNode(2)], 2);
					iElem++; nelem_triangle++;
					break;
					
				case RECTANGLE:
					elem[iElem] = new CRectangle(Index[geometry->elem[jElem]->GetNode(0)], 
												 Index[geometry->elem[jElem]->GetNode(1)], 
												 Index[geometry->elem[jElem]->GetNode(2)], 
												 Index[geometry->elem[jElem]->GetNode(3)], 2);
					iElem++; nelem_quad++;
					break;
					
				case TETRAHEDRON: 
					elem[iElem] = new CTetrahedron(Index[geometry->elem[jElem]->GetNode(0)],
												   Index[geometry->elem[jElem]->GetNode(1)],
												   Index[geometry->elem[jElem]->GetNode(2)],
												   Index[geometry->elem[jElem]->GetNode(3)]);
					iElem++; nelem_tetra++;
					break;
					
				case HEXAHEDRON:
					elem[iElem] = new CHexahedron(Index[geometry->elem[jElem]->GetNode(0)],
												  Index[geometry->elem[jElem]->GetNode(1)],
												  Index[geometry->elem[jElem]->GetNode(2)],
												  Index[geometry->elem[jElem]->GetNode(3)],
												  Index[geometry->elem[jElem]->GetNode(4)],
												  Index[geometry->elem[jElem]->GetNode(5)],
												  Index[geometry->elem[jElem]->GetNode(6)],
												  Index[geometry->elem[jElem]->GetNode(7)]);
					iElem++; nelem_hexa++;
					break;
					
				case WEDGE:
					elem[iElem] = new CWedge(Index[geometry->elem[jElem]->GetNode(0)],
											 Index[geometry->elem[jElem]->GetNode(1)],
											 Index[geometry->elem[jElem]->GetNode(2)],
											 Index[geometry->elem[jElem]->GetNode(3)],
											 Index[geometry->elem[jElem]->GetNode(4)],
											 Index[geometry->elem[jElem]->GetNode(5)]);
					iElem++; nelem_wedge++;
					break;
					
				case PYRAMID:
					elem[iElem] = new CPyramid(Index[geometry->elem[jElem]->GetNode(0)],
											   Index[geometry->elem[jElem]->GetNode(1)],
											   Index[geometry->elem[jElem]->GetNode(2)],
											   Index[geometry->elem[jElem]->GetNode(3)],
											   Index[geometry->elem[jElem]->GetNode(4)]);
					iElem++; nelem_pyramid++;
					break;
					
			}
		}
	
	
	nElem_Storage = nelem_triangle*4 + nelem_quad*5 + nelem_tetra*5 + nelem_hexa*9 + nelem_wedge*7 + nelem_pyramid*6;
	nElem = geometry->GetnElem() + nElem_new;
	
	/*--- Add the old points ---*/
	node = new CPoint*[geometry->GetnPoint() + nPoint_new];
	for (iPoint = 0; iPoint < geometry->GetnPoint(); iPoint ++) {
		if (geometry->GetnDim() == 2)
			node[iPoint] = new CPoint(geometry->node[iPoint]->GetCoord(0),
									  geometry->node[iPoint]->GetCoord(1), iPoint, config);
		if (geometry->GetnDim() == 3)
			node[iPoint] = new CPoint(geometry->node[iPoint]->GetCoord(0),
									  geometry->node[iPoint]->GetCoord(1),
									  geometry->node[iPoint]->GetCoord(2), iPoint, config);
	}
		
	/*--- Add the new points due to the periodic boundary condtion ---*/
	for (iPeriodic = 1; iPeriodic <= nPeriodic; iPeriodic++)
		for (iIndex = 0; iIndex < geometry->PeriodicPoint[iPeriodic][0].size(); iIndex++) {
			
			/*--- From iPeriodic obtain the iMarker ---*/
			for (iMarker = 0; iMarker < geometry->GetnMarker(); iMarker++)
				if (iPeriodic == config->GetMarker_All_PerBound(iMarker)) break;
			
			/*--- Retrieve the supplied periodic information. ---*/
			center = config->GetPeriodicRotCenter(config->GetMarker_All_Tag(iMarker));
			angles = config->GetPeriodicRotAngles(config->GetMarker_All_Tag(iMarker));
			trans  = config->GetPeriodicTranslation(config->GetMarker_All_Tag(iMarker));
			
			/*--- Store center - trans as it is constant and will be added on. 
            Note the subtraction, as this is the inverse translation. ---*/
			translation[0] = center[0] - trans[0];
			translation[1] = center[1] - trans[1];
			translation[2] = center[2] - trans[2];
			
			/*--- Store angles separately for clarity. Compute sines/cosines.
            Note the negative sign, as this is the inverse rotation. ---*/
			theta = -angles[0];   
			phi   = -angles[1]; 
			psi   = -angles[2];
			
			cosTheta = cos(theta);  cosPhi = cos(phi);  cosPsi = cos(psi);
			sinTheta = sin(theta);  sinPhi = sin(phi);  sinPsi = sin(psi);
			
			/*--- Compute the rotation matrix. Note that the implicit
			 ordering is rotation about the x-axis, y-axis, then z-axis. ---*/
			rotMatrix[0][0] = cosPhi*cosPsi;
			rotMatrix[1][0] = cosPhi*sinPsi;
			rotMatrix[2][0] = -sinPhi;
			
			rotMatrix[0][1] = sinTheta*sinPhi*cosPsi - cosTheta*sinPsi;
			rotMatrix[1][1] = sinTheta*sinPhi*sinPsi + cosTheta*cosPsi;
			rotMatrix[2][1] = sinTheta*cosPhi;
			
			rotMatrix[0][2] = cosTheta*sinPhi*cosPsi + sinTheta*sinPsi;
			rotMatrix[1][2] = cosTheta*sinPhi*sinPsi - sinTheta*cosPsi;
			rotMatrix[2][2] = cosTheta*cosPhi;
			
			/*--- Retrieve node information for this boundary point. ---*/
			iPoint = geometry->PeriodicPoint[iPeriodic][0][iIndex];
			jPoint = geometry->PeriodicPoint[iPeriodic][1][iIndex];
			Coord_i = geometry->node[iPoint]->GetCoord();
			
			/*--- Get the position vector from rot center to point. ---*/
			dx = Coord_i[0] - center[0]; 
			dy = Coord_i[1] - center[1]; 
      if (nDim == 3) {
        dz = Coord_i[2] - center[2];
      } else {
        dz = 0.0;
      }
			
			/*--- Compute transformed point coordinates. ---*/
			rotCoord[0] = rotMatrix[0][0]*dx + rotMatrix[0][1]*dy + rotMatrix[0][2]*dz + translation[0];
			rotCoord[1] = rotMatrix[1][0]*dx + rotMatrix[1][1]*dy + rotMatrix[1][2]*dz + translation[1];
			rotCoord[2] = rotMatrix[2][0]*dx + rotMatrix[2][1]*dy + rotMatrix[2][2]*dz + translation[2];
			
			/*--- Save the new points with the new coordinates. ---*/
			if (geometry->GetnDim() == 2)
				node[jPoint] = new CPoint(rotCoord[0], rotCoord[1], jPoint, config);
			if (geometry->GetnDim() == 3)
				node[jPoint] = new CPoint(rotCoord[0], rotCoord[1], rotCoord[2], jPoint, config);
			
		}
	
	nPoint = geometry->GetnPoint() + nPoint_new;
	
	/*--- Add the old boundary, reserving space for two new bc (send/recive periodic bc) ---*/
	nMarker = geometry->GetnMarker() + 2;
	nElem_Bound = new unsigned long [nMarker];
	bound = new CPrimalGrid**[nMarker];	
	Tag_to_Marker = new string [MAX_INDEX_VALUE];
	config->SetnMarker_All(nMarker);
	
	/*--- Copy the olf boundary ---*/
	for (iMarker = 0; iMarker < geometry->GetnMarker(); iMarker++) {
		
		bound[iMarker] = new CPrimalGrid* [geometry->GetnElem_Bound(iMarker)];
		
		for (iVertex = 0; iVertex < geometry->GetnElem_Bound(iMarker); iVertex++) {
			if (geometry->bound[iMarker][iVertex]->GetVTK_Type() == LINE)
				bound[iMarker][iVertex] = new CLine(geometry->bound[iMarker][iVertex]->GetNode(0),
													geometry->bound[iMarker][iVertex]->GetNode(1), 2);
			if (geometry->bound[iMarker][iVertex]->GetVTK_Type() == TRIANGLE)
				bound[iMarker][iVertex] = new CTriangle(geometry->bound[iMarker][iVertex]->GetNode(0),
														geometry->bound[iMarker][iVertex]->GetNode(1), 
														geometry->bound[iMarker][iVertex]->GetNode(2), 3);
			if (geometry->bound[iMarker][iVertex]->GetVTK_Type() == RECTANGLE)
				bound[iMarker][iVertex] = new CRectangle(geometry->bound[iMarker][iVertex]->GetNode(0),
														 geometry->bound[iMarker][iVertex]->GetNode(1),
														 geometry->bound[iMarker][iVertex]->GetNode(2),
														 geometry->bound[iMarker][iVertex]->GetNode(3), 3);
		}
		
		nElem_Bound[iMarker] = geometry->GetnElem_Bound(iMarker);
		Tag_to_Marker[iMarker] = geometry->GetMarker_Tag(iMarker);
		
	}
	
	delete [] Index;

}

void CPeriodicGeometry::SetPeriodicBoundary(CGeometry *geometry, CConfig *config) {
	unsigned short iMarker, iPeriodic, nPeriodic = 0, iMarkerSend, iMarkerReceive;
	unsigned long iVertex, Counter_Send = 0, Counter_Receive = 0, iIndex;
	
	/*--- Compute the number of periodic bc on the geometry ---*/
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if (config->GetMarker_All_Boundary(iMarker) == PERIODIC_BOUNDARY)
			nPeriodic++;
	
	/*--- First compute the Send/Receive boundaries, count the number of points ---*/
	Counter_Send = 0; 	Counter_Receive = 0;
	for (iPeriodic = 1; iPeriodic <= nPeriodic; iPeriodic++) {
		if (geometry->PeriodicPoint[iPeriodic][0].size() != 0) 
			Counter_Send += geometry->PeriodicPoint[iPeriodic][0].size();
		if (geometry->PeriodicPoint[iPeriodic][1].size() != 0) 
			Counter_Receive += geometry->PeriodicPoint[iPeriodic][1].size();		
	}

	/*--- Adimensionalization of the new boundaries ---*/
	iMarkerSend = nMarker - 2; iMarkerReceive = nMarker - 1;
	config->SetMarker_All_SendRecv(iMarkerSend,1);
	config->SetMarker_All_SendRecv(iMarkerReceive,-1);
	nElem_Bound[iMarkerSend] = Counter_Send; 
	nElem_Bound[iMarkerReceive] = Counter_Receive;
	bound[iMarkerSend] = new CPrimalGrid* [Counter_Send];
	bound[iMarkerReceive] = new CPrimalGrid* [Counter_Receive];
	
	/*--- First we do the send ---*/
	iVertex = 0;
	for (iPeriodic = 1; iPeriodic <= nPeriodic; iPeriodic++)
		if (geometry->PeriodicPoint[iPeriodic][0].size() != 0)
			for (iIndex = 0; iIndex < geometry->PeriodicPoint[iPeriodic][0].size(); iIndex++) {
				bound[iMarkerSend][iVertex] = new CVertexMPI(geometry->PeriodicPoint[iPeriodic][0][iIndex], 3);
				bound[iMarkerSend][iVertex]->SetRotation_Type(iPeriodic);
				iVertex++;
			}
	
	/*--- Second we do the receive ---*/
	iVertex = 0;
	for (iPeriodic = 1; iPeriodic <= nPeriodic; iPeriodic++)
		if (geometry->PeriodicPoint[iPeriodic][1].size() != 0)
			for (iIndex = 0; iIndex < geometry->PeriodicPoint[iPeriodic][1].size(); iIndex++) {
				bound[iMarkerReceive][iVertex] = new CVertexMPI(geometry->PeriodicPoint[iPeriodic][1][iIndex], 3);
				bound[iMarkerReceive][iVertex]->SetRotation_Type(iPeriodic);
				iVertex++;
			}
	
}

void CPeriodicGeometry::SetMeshFile (CConfig *config, string val_mesh_out_filename) {
	unsigned long iElem, iPoint, iElem_Bound, NewPoints;
	unsigned short iMarker, iNodes, iDim;
  unsigned short iMarkerReceive, iPeriodic, nPeriodic = 0;
	ofstream output_file;
	string Grid_Marker;
	char *cstr;
	double *center, *angles, *transl;
	
	cstr = new char [val_mesh_out_filename.size()+1];
	strcpy (cstr, val_mesh_out_filename.c_str());
	
	/*--- Open .su2 grid file ---*/
	output_file.precision(15);
	output_file.open(cstr, ios::out);
	
	/*--- Write dimension, number of elements and number of points ---*/
	output_file << "NDIME= " << nDim << endl;
	output_file << "NELEM= " << nElem << endl;
	for (iElem = 0; iElem < nElem; iElem++) {
		output_file << elem[iElem]->GetVTK_Type();
		for (iNodes = 0; iNodes < elem[iElem]->GetnNodes(); iNodes++)
			output_file << "\t" << elem[iElem]->GetNode(iNodes);
		output_file << "\t"<<iElem<<endl;	
	}
	
	iMarkerReceive = nMarker - 1;
	NewPoints = nElem_Bound[iMarkerReceive];

	output_file << "NPOIN= " << nPoint << "\t" << nPoint - NewPoints << endl;
	for (iPoint = 0; iPoint < nPoint; iPoint++) {
		for (iDim = 0; iDim < nDim; iDim++)
			output_file << scientific << "\t" << node[iPoint]->GetCoord(iDim) ;
		output_file << "\t" << iPoint <<endl;
	}
	
	output_file << "NMARK= " << nMarker << endl;
	for (iMarker = 0; iMarker < nMarker; iMarker++) {
		if (bound[iMarker][0]->GetVTK_Type() != VERTEX) {
			
			Grid_Marker = config->GetMarker_All_Tag(iMarker);
			output_file << "MARKER_TAG= " << Grid_Marker <<endl;
			output_file << "MARKER_ELEMS= " << nElem_Bound[iMarker] + nNewElem_BoundPer[iMarker] << endl;
			
			for (iElem_Bound = 0; iElem_Bound < nElem_Bound[iMarker]; iElem_Bound++) {
				output_file << bound[iMarker][iElem_Bound]->GetVTK_Type() << "\t" ;
				for (iNodes = 0; iNodes < bound[iMarker][iElem_Bound]->GetnNodes()-1; iNodes++)
					output_file << bound[iMarker][iElem_Bound]->GetNode(iNodes) << "\t" ;
				iNodes = bound[iMarker][iElem_Bound]->GetnNodes()-1;
				output_file << bound[iMarker][iElem_Bound]->GetNode(iNodes) << endl;
			}
      
      /*--- Write any new elements at the end of the list. ---*/
      if (nNewElem_BoundPer[iMarker] > 0) {
        for (iElem_Bound = 0; iElem_Bound < nNewElem_BoundPer[iMarker]; iElem_Bound++) {
          output_file << newBoundPer[iMarker][iElem_Bound]->GetVTK_Type() << "\t" ;          
          for (iNodes = 0; iNodes < newBoundPer[iMarker][iElem_Bound]->GetnNodes()-1; iNodes++)
            output_file << newBoundPer[iMarker][iElem_Bound]->GetNode(iNodes) << "\t" ;
          iNodes = newBoundPer[iMarker][iElem_Bound]->GetnNodes()-1;
          output_file << newBoundPer[iMarker][iElem_Bound]->GetNode(iNodes) << endl;
        }
      }
      
		}
		
		if (bound[iMarker][0]->GetVTK_Type() == VERTEX) {
			output_file << "MARKER_TAG= SEND_RECEIVE" << endl;
			output_file << "MARKER_ELEMS= " << nElem_Bound[iMarker]<< endl;
			if (config->GetMarker_All_SendRecv(iMarker) > 0) output_file << "SEND_TO= " << config->GetMarker_All_SendRecv(iMarker) << endl;
			if (config->GetMarker_All_SendRecv(iMarker) < 0) output_file << "SEND_TO= " << config->GetMarker_All_SendRecv(iMarker) << endl;
			
			for (iElem_Bound = 0; iElem_Bound < nElem_Bound[iMarker]; iElem_Bound++) {
				output_file << bound[iMarker][iElem_Bound]->GetVTK_Type() << "\t" << 
				bound[iMarker][iElem_Bound]->GetNode(0) << "\t" << 
				bound[iMarker][iElem_Bound]->GetRotation_Type() << endl;
			}
		}
	}
	
	/*--- Compute the number of periodic bc on the geometry ---*/
	for (iMarker = 0; iMarker < config->GetnMarker_All(); iMarker++)
		if (config->GetMarker_All_Boundary(iMarker) == PERIODIC_BOUNDARY)
			nPeriodic++;
	
	output_file << "NPERIODIC= " << nPeriodic + 1 << endl;
	
	/*--- Periodic 0 correspond with no movement of the surface ---*/
	output_file << "PERIODIC_INDEX= 0" << endl;
	output_file << "0.000000000000000e+00" << "\t" << "0.000000000000000e+00" << "\t" << "0.000000000000000e+00" << endl;
	output_file << "0.000000000000000e+00" << "\t" << "0.000000000000000e+00" << "\t" << "0.000000000000000e+00" << endl;
	output_file << "0.000000000000000e+00" << "\t" << "0.000000000000000e+00" << "\t" << "0.000000000000000e+00" << endl;
	
	/*--- From iPeriodic obtain the iMarker ---*/
	for (iPeriodic = 1; iPeriodic <= nPeriodic; iPeriodic++) {
		for (iMarker = 0; iMarker < nMarker; iMarker++)
			if (iPeriodic == config->GetMarker_All_PerBound(iMarker)) break;
		
		/*--- Retrieve the supplied periodic information. ---*/
		center = config->GetPeriodicRotCenter(config->GetMarker_All_Tag(iMarker));
		angles = config->GetPeriodicRotAngles(config->GetMarker_All_Tag(iMarker));
		transl = config->GetPeriodicTranslation(config->GetMarker_All_Tag(iMarker));
		
		output_file << "PERIODIC_INDEX= " << iPeriodic << endl;
		output_file << center[0] << "\t" << center[1] << "\t" << center[2] << endl;
		output_file << angles[0] << "\t" << angles[1] << "\t" << angles[2] << endl;
		output_file << transl[0] << "\t" << transl[1] << "\t" << transl[2] << endl;
		
	}
	

	output_file.close();
	
}

void CPeriodicGeometry::SetParaView(char mesh_filename[200]) {
	
	ofstream para_file;
	para_file.open(mesh_filename, ios::out);
	
	para_file << "# vtk DataFile Version 2.0" << endl;
	para_file << "Visualization of the volumetric grid (with periodic boundary condition)" << endl;
	para_file << "ASCII" << endl;
	para_file << "DATASET UNSTRUCTURED_GRID" << endl;
	para_file << "POINTS " << nPoint << " float" << endl;
	
	para_file.precision(15);
	
	for(unsigned long iPoint = 0; iPoint < nPoint; iPoint++) {
		for(unsigned short iDim = 0; iDim < nDim; iDim++)
			para_file << scientific << node[iPoint]->GetCoord(iDim) << "\t";
		if (nDim == 2) para_file << "0.0";
		para_file << "\n";
	}	 
	
	para_file << "CELLS " << nElem << "\t" << nElem_Storage << endl;
	for(unsigned long iElem=0; iElem < nElem; iElem++) {
		para_file << elem[iElem]->GetnNodes() << "\t";
		for(unsigned short iNode=0; iNode < elem[iElem]->GetnNodes(); iNode++) {
			if (iNode == elem[iElem]->GetnNodes()-1) para_file << elem[iElem]->GetNode(iNode) << "\n";			
			else para_file << elem[iElem]->GetNode(iNode) << "\t";			
		}
	}	
	
	para_file << "CELL_TYPES " << nElem << endl;
	for(unsigned long iElem=0; iElem < nElem; iElem++) {
		para_file << elem[iElem]->GetVTK_Type() << endl;
	}
	para_file.close();
	
}
