#include "DUGKSDeclaration.h"
#include <iostream>
using std::cout;
using std::endl;

void P_Inlet_4_Boundary()
{
	for(int k = 0;k < P_InletFaceNum;++k)
		for(int i = 0;i < DV_Qu;++i)
		for(int j = 0;j < DV_Qv;++j)
		{
			//!momentum
			#ifdef _ARK_MOMENTUM_FLIP
			P_InletShadowCA[k].f.BarP[i][j] = P_InletShadowCA[k].Cell_C[0]->f.BarP[i][j];
			#endif
//isothermal flip
			#ifndef _ARK_ISOTHERMAL_FLIP
			P_InletShadowCA[k].g.BarP[i][j] = P_InletShadowCA[k].Cell_C[0]->g.BarP[i][j];
			#endif
		}
}
void P_Outlet_5_Boundary()
{
	for(int k = 0;k < P_OutletFaceNum;++k)
		for(int i = 0;i < DV_Qu;++i)
		for(int j = 0;j < DV_Qv;++j)
		{
			//!momentum
			#ifdef _ARK_MOMENTUM_FLIP
			P_OutletShadowCA[k].f.BarP[i][j] = P_OutletShadowCA[k].Cell_C[0]->f.BarP[i][j];
			#endif
//isothermal flip
			#ifndef _ARK_ISOTHERMAL_FLIP
			P_OutletShadowCA[k].g.BarP[i][j] = P_OutletShadowCA[k].Cell_C[0]->g.BarP[i][j];
			#endif
		}
}
void ExtrapolationfBP(Cell_2D *shadowCell,Cell_2D const*neighb,Cell_2D const*nextNeighb)
{
	// shadowCell->MsQ().Phi = 2*neighb->MsQ().Phi - nextNeighb->MsQ().Phi;
	// shadowCell->MsQ().Rho = 2*neighb->MsQ().Rho - nextNeighb->MsQ().Rho;
	shadowCell->MsQ() = 2.0*neighb->MsQ() - nextNeighb->MsQ();
	shadowCell->MsQ().U = -neighb->MsQ().U;
	shadowCell->MsQ().V = -neighb->MsQ().V;
	shadowCell->MsQ().Lambda = -neighb->MsQ().Lambda;
	shadowCell->MsQ().Mu = -neighb->MsQ().Mu;
	LoopVS(DV_Qu,DV_Qv)
	{
		#ifdef _ARK_ALLENCAHN_FLIP
		shadowCell->h.BarP[i][j] = 2*neighb->h.BarP[i][j] - nextNeighb->h.BarP[i][j];
		#endif
		//
		//!momentum
		#ifdef _ARK_MOMENTUM_FLIP
		shadowCell->f.BarP[i][j] = 2*neighb->f.BarP[i][j] - nextNeighb->f.BarP[i][j];
		#endif
		//
		#ifndef _ARK_ISOTHERMAL_FLIP
		shadowCell->g.BarP[i][j] = 2*neighb->g.BarP[i][j] - nextNeighb->g.BarP[i][j];
		#endif
	}
	
}
void WallShadowC_fBP(Cell_2D &shadowCell)
{
// used for GradfBP
	unsigned const TOP = top,BOTTOM = bottom,LEFT = left,RIGHT = right;
	#ifdef _CARTESIAN_MESH_FLIP
		if(TOP == shadowCell.zone)
		{
			ExtrapolationfBP(&shadowCell,shadowCell.ShadowC,shadowCell.ShadowC->Cell_C[3]);
		}
		else if(BOTTOM == shadowCell.zone)
		{
			ExtrapolationfBP(&shadowCell,shadowCell.ShadowC,shadowCell.ShadowC->Cell_C[1]);
		}
		else if(RIGHT == shadowCell.zone)
		{
			ExtrapolationfBP(&shadowCell,shadowCell.ShadowC,shadowCell.ShadowC->Cell_C[2]);
		}
		else if(LEFT == shadowCell.zone)
		{
			ExtrapolationfBP(&shadowCell,shadowCell.ShadowC,shadowCell.ShadowC->Cell_C[0]);
		}
	#endif
		//
		// #ifdef _ARK_ALLENCAHN_FLIP
		// shadowCell.MsQ()= cell->MsQ();
		// #endif
		// //
		// shadowCell.MsQ().U = -cell->MsQ().U;//Non-Equilibrium Extrapolation
		// shadowCell.MsQ().V = -cell->MsQ().V;
		// //
		// Update_phi_Eq(shadowCell);
	#ifndef _CARTESIAN_MESH_FLIP
		for(int i = 0;i < DV_Qu;++i)
		for(int j = 0;j < DV_Qv;++j)
		{
			#ifdef _ARK_ALLENCAHN_FLIP
			shadowCell.h.BarP[i][j] = shadowCell.h.Eq[i][j]
			+ cell->h.aBP*(cell->h.Tilde[i][j] - cell->h.Eq[i][j]);
			#endif
			//!momentum
			#ifdef _ARK_MOMENTUM_FLIP
			shadowCell.f.BarP[i][j] = shadowCell.f.Eq[i][j]
			+ cell->f.aBP*(cell->f.Tilde[i][j] - cell->f.Eq[i][j]);
			#endif
			//isothermal flip
			#ifndef _ARK_ISOTHERMAL_FLIP	
			shadowCell.g.BarP[i][j] = shadowCell.g.Eq[i][j]
			+ cell->g.aBP*(cell->g.Tilde[i][j] - cell->g.Eq[i][j]);
			#endif
		}
	#endif
}
extern void Update_phiFlux_h(Face_2D& face);
//
void Wall_3_BB(Face_2D &face)
{
	using D2Q9::_BB;
	LoopVS(DV_Qu,DV_Qv)
	{
		if(face.xi_n_dS[i][j] < 0)
		{
			int jj = _BB[j];
			#ifdef _ARK_ALLENCAHN_FLIP
			face.h.hDt[i][j] = face.h.hDt[i][jj];
			#endif
			//!momentum
			#ifdef _ARK_MOMENTUM_FLIP
			face.f.hDt[i][j] = face.f.hDt[i][jj];
			#endif
			//!isothemal
			#ifndef _ARK_ISOTHERMAL_FLIP
			face.g.hDt[i][j] = face.g.hDt[i][jj];
			#endif
		}
	}
}
void Wall_3_NEE(Face_2D &face)
{
	Cell_2D &cell = *face.owner;
	face.MsQh().Phi = cell.MsQ().Phi
					+ cell.MsQ().Phi_x*(face.xf - cell.xc)
					+ cell.MsQ().Phi_y*(face.yf - cell.yc);
	face.MsQh().Rho = cell.MsQ().Rho
					+ cell.MsQ().Rho_x*(face.xf - cell.xc)
					+ cell.MsQ().Rho_y*(face.yf - cell.yc);
	face.MsQh().p = cell.MsQ().p;
	// face.MsQh().U = 0.0;
	// face.MsQh().V = 0.0;
	Update_phi_Eqh(face);
	//
	#ifdef _ARK_ALLENCAHN_FLIP
	double hNEq = 2.0*cell.h.tau/(2.0*cell.h.tau + ::dt);
	#endif
	//!momentum
	#ifdef _ARK_MOMENTUM_FLIP
	double fNEq = 2.0*cell.f.tau/(2.0*cell.f.tau + ::dt);
	#endif
	//
	#ifndef _ARK_ISOTHERMAL_FLIP	
	double gNEq = 2.0*cell.g.tau/(2.0*cell.g.tau + ::dt);
	#endif
	//
	LoopVS(DV_Qu,DV_Qv)
	{
		#ifdef _ARK_ALLENCAHN_FLIP
		face.h.hDt[i][j] = face.h.EqhDt[i][j]
		+ hNEq*(cell.h.Tilde[i][j] - cell.h.Eq[i][j] + hDt*cell.h.So[i][j]);
		#endif
	//!momentum
		#ifdef _ARK_MOMENTUM_FLIP
		face.f.hDt[i][j] = face.f.EqhDt[i][j]
		+ fNEq*(cell.f.Tilde[i][j] - cell.f.Eq[i][j] + hDt*cell.f.So[i][j]);
		#endif
	//isothermal flip
		#ifndef _ARK_ISOTHERMAL_FLIP	
		face.g.hDt[i][j] = face.g.EqhDt[i][j]
		+ gNEq*(cell.g.Tilde[i][j] - cell.gEq[i][j] + hDt*cell.g.So[i][j]);
		#endif
	}
	Wall_3_BB(face);
}
void fluxCheck(Face_2D const* faceptr)
{
	Face_2D const &face = *faceptr;
	#ifdef _ARK_ALLENCAHN_FLIP
	double hfluxSum = 0.0;
	#endif
	//!momentum
	#ifdef _ARK_MOMENTUM_FLIP
	double ffluxSum = 0.0;
	#endif
	LoopVS(DV_Qu,DV_Qv)
	{
		#ifdef _ARK_ALLENCAHN_FLIP
		hfluxSum += face.h.hDt[i][j];
		#endif
		//!momentum
		#ifdef _ARK_MOMENTUM_FLIP
		ffluxSum += face.f.hDt[i][j];
		#endif
	}
	#ifdef _ARK_ALLENCAHN_FLIP
	if(!EqualZero(hfluxSum))
	{
		cout <<"hfluxSum of Wall Boundary is nonzero"<<endl;
		cout <<"xf : "<<face.xf<<fs<<"yf : "<<face.yf<<fs<<"hfluxSum : "<<hfluxSum<<endl;
		getchar();
	}
	#endif
	//!momentum
	#ifdef _ARK_MOMENTUM_FLIP
	if(!EqualZero(ffluxSum))
	{
		
		cout <<"ffluxSum of Wall Boundary is nonzero"<<endl;
		cout <<"xf : "<<face.xf<<fs<<"yf : "<<face.yf<<fs<<"ffluxSum : "<<ffluxSum<<endl;
		getchar();
	}
	#endif
}