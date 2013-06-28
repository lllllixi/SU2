/*!
 * \file solution_adjoint_turbulent.cpp
 * \brief Main subrotuines for solving adjoint problems (Euler, Navier-Stokes, etc.).
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

#include "../include/solution_structure.hpp"

CAdjTurbSolution::CAdjTurbSolution(void) : CSolution() {}

CAdjTurbSolution::CAdjTurbSolution(CGeometry *geometry, CConfig *config) : CSolution() {
	unsigned long iPoint, nPoint = geometry->GetnPoint();
	unsigned short nMarker, iDim, iVar;

	nDim = geometry->GetnDim();
	nMarker = config->GetnMarker_All(); 
	Gamma = config->GetGamma();
	Gamma_Minus_One = Gamma - 1.0;

	/*--- Dimension of the problem --> dependent of the turbulent model ---*/
	switch (config->GetKind_Turb_Model()) {	
		case SA :
			nVar = 1;		
			break;
		case SA_COMP :
			nVar = 1;		
			break;
		case SST :
			nVar = 2;		
			break;
	}

	Residual   = new double [nVar];
	Residual_i = new double [nVar];
	Residual_j = new double [nVar];
	Residual_Max = new double [nVar];

	Solution   = new double [nVar];
	Solution_i = new double [nVar];
	Solution_j = new double [nVar];

	/*--- Define some auxiliar vector related with the geometry ---*/
	Vector_i = new double[nDim]; Vector_j = new double[nDim];

	/*--- Define some auxiliar vector related with the flow solution ---*/
	FlowSolution_i = new double [nDim+2]; FlowSolution_j = new double [nDim+2];

	/*--- Point to point Jacobians ---*/
	Jacobian_ii = new double* [nVar];
	Jacobian_ij = new double* [nVar];
	Jacobian_ji = new double* [nVar];
	Jacobian_jj = new double* [nVar];
	for (unsigned short iVar = 0; iVar < nVar; iVar++) {
		Jacobian_ii[iVar] = new double [nVar];
		Jacobian_ij[iVar] = new double [nVar];
		Jacobian_ji[iVar] = new double [nVar];
		Jacobian_jj[iVar] = new double [nVar];
	}

	/*--- Initialization of the structure of the whole Jacobian ---*/
	Initialize_Jacobian_Structure(geometry, config);
	xsol = new double [nPoint*nVar];
	rhs  = new double [nPoint*nVar];

	/*--- Computation of gradients by least squares ---*/
	if ((config->GetKind_Gradient_Method() == LEAST_SQUARES) || 
		(config->GetKind_Gradient_Method() == WEIGHTED_LEAST_SQUARES)) {
		/*--- S matrix := inv(R)*traspose(inv(R)) ---*/
		Smatrix = new double* [nDim]; 
		for (iDim = 0; iDim < nDim; iDim++)
			Smatrix[iDim] = new double [nDim];
		/*--- c vector := transpose(WA)*(Wb) ---*/
		cvector = new double* [nVar+1]; 
		for (iVar = 0; iVar < nVar+1; iVar++)
			cvector[iVar] = new double [nDim];
	}
	
	/*--- Far-Field values and initizalization ---*/
	node = new CVariable* [nPoint];	
	bool restart = config->GetRestart();

	if (!restart) {	
		PsiNu_Inf = 0.0;
		for (iPoint = 0; iPoint < nPoint; iPoint++)
			node[iPoint] = new CAdjTurbVariable(PsiNu_Inf, nDim, nVar, config);
	}
	else {
		string copy, mesh_filename;
		char buffer[50], cstr[200];
		mesh_filename = config->GetSolution_AdjFileName();
		copy.assign(mesh_filename);
		copy.erase (copy.end()-4, copy.end());
		strcpy (cstr, copy.c_str()); 
		if (config->GetKind_ObjFunc() == DRAG_COEFFICIENT) sprintf (buffer, "_cd.dat"); 
		if (config->GetKind_ObjFunc() == LIFT_COEFFICIENT) sprintf (buffer, "_cl.dat");
		if (config->GetKind_ObjFunc() == SIDEFORCE_COEFFICIENT) sprintf (buffer, "_csf.dat"); 
		if (config->GetKind_ObjFunc() == PRESSURE_COEFFICIENT) sprintf (buffer, "_cp.dat"); 
		if (config->GetKind_ObjFunc() == MOMENT_X_COEFFICIENT) sprintf (buffer, "_cmx.dat"); 
		if (config->GetKind_ObjFunc() == MOMENT_Y_COEFFICIENT) sprintf (buffer, "_cmy.dat"); 
		if (config->GetKind_ObjFunc() == MOMENT_Z_COEFFICIENT) sprintf (buffer, "_cmz.dat"); 
		if (config->GetKind_ObjFunc() == EFFICIENCY) sprintf (buffer, "_eff.dat"); 
		if (config->GetKind_ObjFunc() == ELECTRIC_CHARGE) sprintf (buffer, "_ec.dat");
		if (config->GetKind_ObjFunc() == EQUIVALENT_AREA) sprintf (buffer, "_ea.dat"); 
		if (config->GetKind_ObjFunc() == NEARFIELD_PRESSURE) sprintf (buffer, "_nfp.dat"); 
    if (config->GetKind_ObjFunc() == FORCE_X_COEFFICIENT) sprintf (buffer, "_cfx.dat"); 
		if (config->GetKind_ObjFunc() == FORCE_Y_COEFFICIENT) sprintf (buffer, "_cfy.dat"); 
		if (config->GetKind_ObjFunc() == FORCE_Z_COEFFICIENT) sprintf (buffer, "_cfz.dat"); 
    if (config->GetKind_ObjFunc() == THRUST_COEFFICIENT) sprintf (buffer, "_ct.dat"); 
    if (config->GetKind_ObjFunc() == TORQUE_COEFFICIENT) sprintf (buffer, "_cq.dat");
    if (config->GetKind_ObjFunc() == FIGURE_OF_MERIT) sprintf (buffer, "_merit.dat");
    if (config->GetKind_ObjFunc() == FREESURFACE) sprintf (buffer, "_fs.dat");
		strcat(cstr, buffer);
		ifstream restart_file;
		restart_file.open(cstr, ios::in);

		unsigned long index;
		double dull_val;
		string text_line;

		for (iPoint = 0; iPoint < nPoint; iPoint++) {
			getline(restart_file,text_line);
			istringstream point_line(text_line);
			if (nDim == 2) point_line >> index >> dull_val >> dull_val >> dull_val >> dull_val >> Solution[0];
			if (nDim == 3) point_line >> index >> dull_val >> dull_val >> dull_val >> dull_val >> dull_val >> Solution[0];
			node[iPoint] = new CAdjTurbVariable(Solution[0], nDim, nVar, config);
		}
		restart_file.close();
	}
}

CAdjTurbSolution::~CAdjTurbSolution(void) { }

void CAdjTurbSolution::BC_NS_Wall(CGeometry *geometry, CSolution **solution_container, CConfig *config, unsigned short val_marker) {

	unsigned long Point, iVertex; 

	for (iVertex = 0; iVertex<geometry->nVertex[val_marker]; iVertex++) {
		Point = geometry->vertex[val_marker][iVertex]->GetNode();
		Solution[0] = 0.0;
		node[Point]->SetSolution_Old(Solution);
		node[Point]->SetResidualZero();
//		node[Point]->SetTruncationErrorZero();
		Jacobian.DeleteValsRowi(Point); // & includes 1 in the diagonal
	}
}

/* void CAdjTurbSolution::BC_Far_Field(CGeometry *geometry, CSolution **solution_container, CNumerics *solver, CConfig *config, unsigned short val_marker) {

	unsigned long Point, iVertex;
	unsigned short iDim;
	double **Grad_i, *Normal, flux, normal2, Vol_i;

	// This tries to impose a zero-flux BC on the far-field by using an approximation of the Green-Gauss theorem

	for (iVertex = 0; iVertex < geometry->nVertex[val_marker]; iVertex++) {

		Point = geometry->vertex[val_marker][iVertex]->GetNode();

		Grad_i = node[Point]->GetGradient();
		Normal = geometry->vertex[val_marker][iVertex]->GetNormal();
		Vol_i = geometry->node[Point]->GetVolume();

		flux = 0;
		normal2 = 0;
		for (iDim = 0; iDim < nDim; iDim++) {
			normal2 += Normal[iDim]*Normal[iDim];
			flux -= Grad_i[0][iDim]*Normal[iDim];
		}

		Solution[0] = node[Point]->GetSolution(0) - 2.0*Vol_i*flux/normal2;
		node[Point]->SetSolution_Old(Solution);
		node[Point]->SetResidualZero();
//		node[Point]->SetTruncationErrorZero();
		Jacobian.DeleteValsRowi(Point); // & includes 1 in the diagonal
	}
}*/

void CAdjTurbSolution::BC_Far_Field(CGeometry *geometry, CSolution **solution_container, CNumerics *solver, CConfig *config, unsigned short val_marker) {
	
	unsigned long Point, iVertex;
	
	for (iVertex = 0; iVertex < geometry->nVertex[val_marker]; iVertex++) {
		
		Point = geometry->vertex[val_marker][iVertex]->GetNode();
		
		/*--- Set Conservative variables (for convection) ---*/
		double* U_i = solution_container[FLOW_SOL]->node[Point]->GetSolution();
		solver->SetConservative(U_i, NULL); 
				
		/*--- Turbulent adjoint variables w/o reconstruction ---*/
		double* TurbPsi_i = node[Point]->GetSolution();
		solver->SetTurbAdjointVar(TurbPsi_i, NULL);
		
		/*--- Set Normal ---*/
		solver->SetNormal(geometry->vertex[val_marker][iVertex]->GetNormal());
		
		/*--- Add Residuals and Jacobians ---*/
		solver->SetResidual(Residual, Jacobian_ii, NULL, config);
		node[Point]->AddResidual(Residual);
		Jacobian.AddBlock(Point, Point, Jacobian_ii);
	}
}

void CAdjTurbSolution::Preprocessing(CGeometry *geometry, CSolution **solution_container, CConfig *config, unsigned short iRKStep) {

	for (unsigned long iPoint = 0; iPoint < geometry->GetnPoint(); iPoint++)
		node[iPoint]->SetResidualZero(); // Inicialize the residual vector
	Jacobian.SetValZero();

	/*--- Computation of gradients of the different variables ---*/
	switch (config->GetKind_Gradient_Method()) {
		case GREEN_GAUSS : 
			SetSolution_Gradient_GG(geometry); 
			solution_container[ADJFLOW_SOL]->SetSolution_Gradient_GG(geometry); 
			break;
		case LEAST_SQUARES : case WEIGHTED_LEAST_SQUARES : 
			SetSolution_Gradient_LS(geometry, config);
			solution_container[ADJFLOW_SOL]->SetSolution_Gradient_LS(geometry, config);
			break;
	}

}

void CAdjTurbSolution::Upwind_Residual(CGeometry *geometry, CSolution **solution_container, CNumerics *solver, CConfig *config, unsigned short iMesh) {

	unsigned long iEdge, iPoint, jPoint;
	double *U_i, *U_j, *TurbPsi_i, *TurbPsi_j;
	double **TurbVar_Grad_i, **TurbVar_Grad_j;
	double *Limiter_i = NULL, *Limiter_j = NULL, **Gradient_i, **Gradient_j, Project_Grad_i, Project_Grad_j;
	unsigned short iDim, iVar;
	bool high_order_diss = (config->GetKind_Upwind_AdjTurb() == SCALAR_UPWIND_2ND);

	if (high_order_diss) { 
		if (config->GetKind_Gradient_Method() == GREEN_GAUSS) solution_container[FLOW_SOL]->SetSolution_Gradient_GG(geometry);
		if ((config->GetKind_Gradient_Method() == LEAST_SQUARES) || 
			(config->GetKind_Gradient_Method() == WEIGHTED_LEAST_SQUARES)) solution_container[FLOW_SOL]->SetSolution_Gradient_LS(geometry, config);
		if (config->GetKind_SlopeLimit() == VENKATAKRISHNAN) SetSolution_Limiter(geometry, config);
	}

	for (iEdge = 0; iEdge < geometry->GetnEdge(); iEdge++) {
		// Points in edge
		iPoint = geometry->edge[iEdge]->GetNode(0);
		jPoint = geometry->edge[iEdge]->GetNode(1);

		// Conservative variables w/o reconstruction
		U_i = solution_container[FLOW_SOL]->node[iPoint]->GetSolution();
		U_j = solution_container[FLOW_SOL]->node[jPoint]->GetSolution();
		solver->SetConservative(U_i, U_j);

		// Turbulent adjoint variables w/o reconstruction
		TurbPsi_i = node[iPoint]->GetSolution();
		TurbPsi_j = node[jPoint]->GetSolution();
		solver->SetTurbAdjointVar(TurbPsi_i, TurbPsi_j);

		// Gradient of turbulent variables w/o reconstruction
		TurbVar_Grad_i = solution_container[TURB_SOL]->node[iPoint]->GetGradient();
		TurbVar_Grad_j = solution_container[TURB_SOL]->node[jPoint]->GetGradient();
		solver->SetTurbVarGradient(TurbVar_Grad_i, TurbVar_Grad_j);

		if (high_order_diss) {
			/*--- Conservative solution using gradient reconstruction ---*/
			for (iDim = 0; iDim < nDim; iDim++) {
				Vector_i[iDim] = 0.5*(geometry->node[jPoint]->GetCoord(iDim) - geometry->node[iPoint]->GetCoord(iDim));
				Vector_j[iDim] = 0.5*(geometry->node[iPoint]->GetCoord(iDim) - geometry->node[jPoint]->GetCoord(iDim));
			}
			Gradient_i = solution_container[FLOW_SOL]->node[iPoint]->GetGradient(); 
			Gradient_j = solution_container[FLOW_SOL]->node[jPoint]->GetGradient(); 
			for (iVar = 0; iVar < solution_container[FLOW_SOL]->GetnVar(); iVar++) {
				Project_Grad_i = 0; Project_Grad_j = 0;
				for (iDim = 0; iDim < nDim; iDim++) {
					Project_Grad_i += Vector_i[iDim]*Gradient_i[iVar][iDim];
					Project_Grad_j += Vector_j[iDim]*Gradient_j[iVar][iDim];
				}
				FlowSolution_i[iVar] = U_i[iVar] + Project_Grad_i; 
				FlowSolution_j[iVar] = U_j[iVar] + Project_Grad_j;
			}
			solver->SetConservative(FlowSolution_i, FlowSolution_j);

			/*--- Adjoint turbulent variables using gradient reconstruction ---*/
			Gradient_i = node[iPoint]->GetGradient(); 
			Gradient_j = node[jPoint]->GetGradient(); 
			if (config->GetKind_SlopeLimit() != NONE) {
				Limiter_i = node[iPoint]->GetLimiter();				
				Limiter_j = node[jPoint]->GetLimiter();
			}
			for (iVar = 0; iVar < nVar; iVar++) {
				Project_Grad_i = 0; Project_Grad_j = 0;
				for (iDim = 0; iDim < nDim; iDim++) {
					Project_Grad_i += Vector_i[iDim]*Gradient_i[iVar][iDim];
					Project_Grad_j += Vector_j[iDim]*Gradient_j[iVar][iDim];
				}
				if (config->GetKind_SlopeLimit() == NONE) {
					Solution_i[iVar] = TurbPsi_i[iVar] + Project_Grad_i;
					Solution_j[iVar] = TurbPsi_j[iVar] + Project_Grad_j;
				}
				else {
					Solution_i[iVar] = TurbPsi_i[iVar] + Project_Grad_i*Limiter_i[iVar];
					Solution_j[iVar] = TurbPsi_j[iVar] + Project_Grad_j*Limiter_j[iVar];
				}
			}
			solver->SetTurbVar(Solution_i, Solution_j);
		}

		// Set normal vectors and length
		solver->SetNormal(geometry->edge[iEdge]->GetNormal());

		solver->SetResidual(Residual_i, Residual_j, Jacobian_ii, Jacobian_ij, Jacobian_ji, Jacobian_jj, config);
		// Add and Subtract Residual
		node[iPoint]->AddResidual(Residual_i);
		node[jPoint]->AddResidual(Residual_j);
		Jacobian.AddBlock(iPoint,iPoint,Jacobian_ii);
		Jacobian.AddBlock(iPoint,jPoint,Jacobian_ij);
		Jacobian.AddBlock(jPoint,iPoint,Jacobian_ji);
		Jacobian.AddBlock(jPoint,jPoint,Jacobian_jj);
	}
}

void CAdjTurbSolution::Viscous_Residual(CGeometry *geometry, CSolution **solution_container, CNumerics *solver, CConfig *config, 
																				unsigned short iMesh, unsigned short iRKStep) {
	
	unsigned long iEdge, iPoint, jPoint;
	double *Coord_i, *Coord_j;
	bool implicit = (config->GetKind_TimeIntScheme_AdjTurb() == EULER_IMPLICIT);

	if ((config->Get_Beta_RKStep(iRKStep) != 0) || implicit) {
		for (iEdge = 0; iEdge < geometry->GetnEdge(); iEdge++) {
			
			/*--- Points in edge ---*/
			iPoint = geometry->edge[iEdge]->GetNode(0);
			jPoint = geometry->edge[iEdge]->GetNode(1);
			
			/*--- Points coordinates, and set normal vectors and length ---*/
			Coord_i = geometry->node[iPoint]->GetCoord();
			Coord_j = geometry->node[jPoint]->GetCoord();
			solver->SetCoord(Coord_i, Coord_j);
			solver->SetNormal(geometry->edge[iEdge]->GetNormal());
			
			/*--- Conservative variables w/o reconstruction, turbulent variables w/o reconstruction, 
			 and turbulent adjoint variables w/o reconstruction ---*/
			solver->SetConservative(solution_container[FLOW_SOL]->node[iPoint]->GetSolution(), solution_container[FLOW_SOL]->node[jPoint]->GetSolution());
			solver->SetTurbVar(solution_container[TURB_SOL]->node[iPoint]->GetSolution(), solution_container[TURB_SOL]->node[jPoint]->GetSolution());
			solver->SetTurbAdjointVar(node[iPoint]->GetSolution(), node[jPoint]->GetSolution());
			
			/*--- Viscosity ---*/
			solver->SetLaminarViscosity(solution_container[FLOW_SOL]->node[iPoint]->GetLaminarViscosity(), 
																	solution_container[FLOW_SOL]->node[jPoint]->GetLaminarViscosity());
			
			/*--- Turbulent adjoint variables w/o reconstruction ---*/
			solver->SetTurbAdjointGradient(node[iPoint]->GetGradient(), node[jPoint]->GetGradient());
			
			// ATTENTION: CHOOSE ONE OF THE FOLLOWING FORMS TO COMPUTE THE RESIDUAL
			
			// Add and Subtract Residual (CONSERVATIVE FORM)
			solver->SetResidual(Residual, Jacobian_ii, Jacobian_jj, config);
			node[iPoint]->AddResidual(Residual);
			node[jPoint]->SubtractResidual(Residual);
			Jacobian.AddBlock(iPoint,iPoint,Jacobian_ii);
			Jacobian.AddBlock(iPoint,jPoint,Jacobian_jj);
			Jacobian.SubtractBlock(jPoint,iPoint,Jacobian_ii);
			Jacobian.SubtractBlock(jPoint,jPoint,Jacobian_jj);
			
			/*		// Add and Subtract Residual (NON-CONSERVATIVE FORM)
			 solver->SetResidual(Residual_i, Residual_j, Jacobian_ii, Jacobian_ij, Jacobian_ji, Jacobian_jj, config);
			 node[iPoint]->AddResidual(Residual_i);
			 node[jPoint]->AddResidual(Residual_j);
			 Jacobian.AddBlock(iPoint,iPoint,Jacobian_ii);
			 Jacobian.AddBlock(iPoint,jPoint,Jacobian_ij);
			 Jacobian.AddBlock(jPoint,iPoint,Jacobian_ji);
			 Jacobian.AddBlock(jPoint,jPoint,Jacobian_jj);*/
		}
	}
}

void CAdjTurbSolution::Source_Residual(CGeometry *geometry, CSolution **solution_container, CNumerics *solver,
											   CConfig *config, unsigned short iMesh) {

	unsigned long iPoint;
	double *U_i, *TurbVar_i;
	double **TurbVar_Grad_i, *TurbPsi_i, **PsiVar_Grad_i; // Gradients

	for (iPoint = 0; iPoint < geometry->GetnPointDomain(); iPoint++) { 

		// Conservative variables w/o reconstruction
		U_i = solution_container[FLOW_SOL]->node[iPoint]->GetSolution();
		solver->SetConservative(U_i, NULL);

		// Gradient of primitive variables w/o reconstruction 
		solver->SetPrimVarGradient(solution_container[FLOW_SOL]->node[iPoint]->GetGradient_Primitive(), NULL);

		// Laminar viscosity of the fluid
		solver->SetLaminarViscosity(solution_container[FLOW_SOL]->node[iPoint]->GetLaminarViscosity(), 0.0);

		// Turbulent variables w/o reconstruction
		TurbVar_i = solution_container[TURB_SOL]->node[iPoint]->GetSolution();
		solver->SetTurbVar(TurbVar_i, NULL); 

		// Gradient of Turbulent Variables w/o reconstruction
		TurbVar_Grad_i = solution_container[TURB_SOL]->node[iPoint]->GetGradient();
		solver->SetTurbVarGradient(TurbVar_Grad_i, NULL);

		// Turbulent adjoint variables w/o reconstruction
		TurbPsi_i = node[iPoint]->GetSolution();
		solver->SetTurbAdjointVar(TurbPsi_i, NULL);

		// Gradient of Adjoint flow variables w/o reconstruction 
		// (for non-conservative terms depending on gradients of flow adjoint vars.)
		PsiVar_Grad_i = solution_container[ADJFLOW_SOL]->node[iPoint]->GetGradient();
		solver->SetAdjointVarGradient(PsiVar_Grad_i, NULL);

		// Set volume and distances to the surface
		solver->SetVolume(geometry->node[iPoint]->GetVolume());
		solver->SetDistance(geometry->node[iPoint]->GetWallDistance(), 0.0);

		// Add and Subtract Residual
		solver->SetResidual(Residual, Jacobian_ii, NULL, config);
		node[iPoint]->AddResidual(Residual);
		Jacobian.AddBlock(iPoint, iPoint, Jacobian_ii);
	}
}

void CAdjTurbSolution::SourceConserv_Residual(CGeometry *geometry, CSolution **solution_container, CNumerics *solver,
											   CConfig *config, unsigned short iMesh) {

	unsigned long iEdge, iPoint, jPoint;
	double *TurbPsi_i, *TurbPsi_j;
	double **TurbVar_Grad_i, **TurbVar_Grad_j;

	for (iEdge = 0; iEdge < geometry->GetnEdge(); iEdge++) {
		// Points in edge
		iPoint = geometry->edge[iEdge]->GetNode(0);
		jPoint = geometry->edge[iEdge]->GetNode(1);

		// Gradient of turbulent variables w/o reconstruction
		TurbVar_Grad_i = solution_container[TURB_SOL]->node[iPoint]->GetGradient();
		TurbVar_Grad_j = solution_container[TURB_SOL]->node[jPoint]->GetGradient();
		solver->SetTurbVarGradient(TurbVar_Grad_i, TurbVar_Grad_j);

		// Turbulent adjoint variables w/o reconstruction
		TurbPsi_i = node[iPoint]->GetSolution();
		TurbPsi_j = node[jPoint]->GetSolution();
		solver->SetTurbAdjointVar(TurbPsi_i, TurbPsi_j);

		// Set normal vectors and length
		solver->SetNormal(geometry->edge[iEdge]->GetNormal());

		// Add and Subtract Residual
		solver->SetResidual(Residual, Jacobian_ii, Jacobian_jj, config);
		node[iPoint]->AddResidual(Residual);
		node[jPoint]->SubtractResidual(Residual);
		Jacobian.AddBlock(iPoint,iPoint,Jacobian_ii);
		Jacobian.AddBlock(iPoint,jPoint,Jacobian_jj);
		Jacobian.SubtractBlock(jPoint,iPoint,Jacobian_ii);
		Jacobian.SubtractBlock(jPoint,jPoint,Jacobian_jj);
	}
}

void CAdjTurbSolution::ImplicitEuler_Iteration(CGeometry *geometry, CSolution **solution_container, CConfig *config) {
	unsigned short iVar;
	unsigned long iPoint, total_index;
	double Delta, Delta_flow, *local_Residual;//, *local_TruncationError;

	/*--- Set maximum residual to zero ---*/
	for (iVar = 0; iVar < nVar; iVar++)
		SetRes_Max(iVar,0.0);

	/*--- Build implicit system ---*/
	for (iPoint = 0; iPoint < geometry->GetnPoint(); iPoint++) {
		local_Residual = node[iPoint]->GetResidual();

		/*--- Modify matrix diagonal to ensure diagonal dominance ---*/
		Delta_flow = geometry->node[iPoint]->GetVolume()/(solution_container[FLOW_SOL]->node[iPoint]->GetDelta_Time());
		Delta = Delta_flow;
		Jacobian.AddVal2Diag(iPoint,Delta);

		for (iVar = 0; iVar < nVar; iVar++) {
			total_index = iPoint*nVar+iVar;

			/*--- Right hand side of the system (-Residual) and initial guess (x = 0) ---*/
			rhs[total_index] = -local_Residual[iVar];
			xsol[total_index] = 0.0;

#ifdef NO_MPI
			/*--- Compute the norm-2 of the residual ---*/
			if (fabs(rhs[total_index]) > GetRes_Max(iVar))
				SetRes_Max(iVar, fabs(rhs[total_index]));
#endif
			
		}
	}

	/*--- Solve the system ---*/
	Jacobian.LU_SGSIteration(rhs, xsol, geometry, config);

	/*--- Update solution (system written in terms of increments) ---*/
	for (iPoint = 0; iPoint < geometry->GetnPoint(); iPoint++) {
		for (iVar = 0; iVar < nVar; iVar++)
			node[iPoint]->AddSolution(iVar,xsol[iPoint*nVar+iVar]);
	}
}
