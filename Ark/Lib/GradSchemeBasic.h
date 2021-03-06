int const

apsi = 4, 

bpsi = 1;
//
double const 

_2dx = 2.0*DeltaX,

_2dy = 2.0*DeltaY;

void LSCellMatrix(Cell_2D* const &Center,int k,
					const double &neighbour_xc, double const &neighbour_yc)
{
	double dx = neighbour_xc - Center->xc, dy = neighbour_yc - Center->yc;
//
	SetZero(dx);
	SetZero(dy);
//-----------------LeastSquare Left Matrix----------------------
	double DistanceP2P = dx*dx + dy*dy;
	Center->LS_M[0][0] += dx*dx/DistanceP2P;
	Center->LS_M[0][1] += dx*dy/DistanceP2P;
	Center->LS_M[1][0] += dy*dx/DistanceP2P;
	Center->LS_M[1][1] += dy*dy/DistanceP2P;
//-----------------LeastSquare Right Matrix---------------------
	Center->wdx_C[k] = dx/DistanceP2P;
	Center->wdy_C[k] = dy/DistanceP2P;
}

void InverseMatrix_2_2(double (&LS_M)[2][2])
{
	double a[2][2],A;
	A = LS_M[0][0] * LS_M[1][1] - LS_M[0][1] * LS_M[1][0];
	if(0.0 == A)
	{
		cout <<"Singular Maxtrix : " <<__FILE__<<"  "<<__LINE__<<"  "<<__func__<<endl;
		getchar();
		return;
	}
	a[0][0] = LS_M[1][1]/A;
	a[1][1] = LS_M[0][0]/A;
	a[0][1] = -LS_M[0][1]/A;
	a[1][0] = -LS_M[1][0]/A;
	for(int i = 0;i < 2;++i)
		for(int j = 0;j < 2;++j)
			LS_M[i][j] = a[i][j];
}
void Grad_LSMatrix()
{
	printSplitLine();
	cout <<"LeastSquare Matrix Construction..."<<endl;
	Cell_2D* neighbour = nullptr;
	Cell_2D* center = nullptr;
	for(int i = 0;i != Cells;++i)
	{
		center = &CellArray[i];
		for(int k = 0;k != center->celltype;++k)
		{
			neighbour = center->Cell_C[k];
			if(neighbour != nullptr)
			{
				LSCellMatrix(center,k,neighbour->xc,neighbour->yc);
			}
			else
			{
				cout << "CellArray : " << i <<"neighbour cell invalid : "<<endl;
				getchar();
			}
		}
		InverseMatrix_2_2(center->LS_M);
	}
	cout <<"LeastSquare Matrix Construction Done" << endl;
	printSplitLine(nl);
}
void update_DVDF_Grad4points(Cell_2D *cellptr,Cell_2D::DVDF Cell_2D::*dvdf)
{
	LoopVS(DV_Qu,DV_Qv)
	{
		(cellptr->*dvdf).BarP_x[i][j]
		= 
		(
			((cellptr->Cell_C[0]->*dvdf).BarP[i][j])
		  -	((cellptr->Cell_C[2]->*dvdf).BarP[i][j])
		)/(_2dx);
		//
		(cellptr->*dvdf).BarP_y[i][j]
		=
		(
			((cellptr->Cell_C[1]->*dvdf).BarP[i][j])
		  -	((cellptr->Cell_C[3]->*dvdf).BarP[i][j])
		)/(_2dy);
	}
}
void update_DVDF_Grad6points(Cell_2D *cellptr,Cell_2D::DVDF Cell_2D::*dvdf)
{
	LoopVS(DV_Qu,DV_Qv)
	{
		(cellptr->*dvdf).BarP_x[i][j]
		=
		(
			apsi*
	  	  	(
	  	    ((cellptr->Cell_C[0]->*dvdf).BarP[i][j]) - ((cellptr->Cell_C[2]->*dvdf).BarP[i][j])
	  	  	)
	  	  + bpsi*
	  	  	(
	  			((cellptr->Cell_Diag[0]->*dvdf).BarP[i][j]) - ((cellptr->Cell_Diag[1]->*dvdf).BarP[i][j])
	  		+   ((cellptr->Cell_Diag[3]->*dvdf).BarP[i][j]) - ((cellptr->Cell_Diag[2]->*dvdf).BarP[i][j])
	  	  	)
		)/(_2dx*6);
	//
		(cellptr->*dvdf).BarP_y[i][j] = 
		(
			apsi*
		    (
		      ((cellptr->Cell_C[1]->*dvdf).BarP[i][j]) - ((cellptr->Cell_C[3]->*dvdf).BarP[i][j])
		    )
		  + bpsi*
		    (
		  		((cellptr->Cell_Diag[0]->*dvdf).BarP[i][j]) - ((cellptr->Cell_Diag[3]->*dvdf).BarP[i][j])
		  	+   ((cellptr->Cell_Diag[1]->*dvdf).BarP[i][j]) - ((cellptr->Cell_Diag[2]->*dvdf).BarP[i][j])
		  	)
		)/(_2dy*6);
	}
}
void update_DVDF_Grad_LS(Cell_2D *center,Cell_2D::DVDF Cell_2D::*dvdf)
{
	LoopVS(DV_Qu,DV_Qv)
	{
		double Sum_wdxdBP = 0.0;
		double Sum_wdydBP = 0.0;
		for(int nf = 0;nf < center->celltype;++nf)
		{
			Cell_2D *neighbour = center->Cell_C[nf];
			//
			Sum_wdxdBP +=
			center->wdx_C[nf]*((neighbour->*dvdf).BarP[i][j] - (center->*dvdf).BarP[i][j]);
			// 
			Sum_wdydBP +=
			center->wdy_C[nf]*((neighbour->*dvdf).BarP[i][j] - (center->*dvdf).BarP[i][j]);
		}
		//
		(center->*dvdf).BarP_x[i][j]
		=
		center->LS_M[0][0]*Sum_wdxdBP + center->LS_M[0][1]*Sum_wdydBP;
		//
		(center->*dvdf).BarP_y[i][j]
		= 
		center->LS_M[1][0]*Sum_wdxdBP + center->LS_M[1][1]*Sum_wdydBP;
	}
}
double update_MQ_x(Cell_2D const *cellptr,double const MacroQuantity::*member)
{
	return
	(	apsi*
	    (
	      (cellptr->Cell_C[0]->msq->*member) - (cellptr->Cell_C[2]->msq->*member)
	    )
	  + bpsi*
	    (
	  		(cellptr->Cell_Diag[0]->msq->*member) - (cellptr->Cell_Diag[1]->msq->*member)
	  	+   (cellptr->Cell_Diag[3]->msq->*member) - (cellptr->Cell_Diag[2]->msq->*member)
	  	)
	)/(_2dx*6);
}
double update_MQ_y(Cell_2D const *cellptr,double const MacroQuantity::*member)
{
	return
	(	apsi*
	    (
	      (cellptr->Cell_C[1]->MsQ().*member) - (cellptr->Cell_C[3]->MsQ().*member)
	    )
	  + bpsi*
	    (
	  		(cellptr->Cell_Diag[0]->MsQ().*member) - (cellptr->Cell_Diag[3]->MsQ().*member)
	  	+   (cellptr->Cell_Diag[1]->MsQ().*member) - (cellptr->Cell_Diag[2]->MsQ().*member)
	  	)
	)/(_2dy*6);
}
double update_MQ_x(Face_2D const *faceptr,double const MacroQuantity::*member)
{
	return
	(	apsi*
		(
			(faceptr->faceFaces[1]->msqh->*member) - (faceptr->faceFaces[5]->msqh->*member)
		)
	+	bpsi*
		(
			(faceptr->faceFaces[2]->msqh->*member) - (faceptr->faceFaces[4]->msqh->*member)
		+	(faceptr->faceFaces[8]->msqh->*member) - (faceptr->faceFaces[6]->msqh->*member)
		)
	)/(_2dx*6);
}
double update_MQ_y(Face_2D const *faceptr,double const MacroQuantity::*member)
{
	return
	(	apsi*
		(
			(faceptr->faceFaces[3]->msqh->*member) - (faceptr->faceFaces[7]->msqh->*member)
		)
	+	bpsi*
		(
			(faceptr->faceFaces[2]->msqh->*member) - (faceptr->faceFaces[8]->msqh->*member)
		+	(faceptr->faceFaces[4]->msqh->*member) - (faceptr->faceFaces[6]->msqh->*member)
		)
	)/(_2dy*6);
}
//!momentum
#ifdef _ARK_MOMENTUM_FLIP
void LeastSquareDebug()
{
	for(int i = 0;i < Cells;++i)
	{
		Cell_2D *center = &CellArray[i], *neighbour = nullptr;
		for(int m = 0;m < DV_Qu;++m)
		for(int n = 0;n < DV_Qv;++n)
		{
			center->f.BarP[m][n] = -1.0;
			for(int Iface = 0;Iface < center->celltype;++Iface)
			{
			neighbour = center->Cell_C[Iface];
				if(neighbour->xc == center->xc && neighbour->yc > center->yc)
				{
					neighbour->f.BarP[m][n] = 1.5;
				}
				else if(neighbour->xc == center->xc && neighbour->yc < center->yc)
				{
					neighbour->f.BarP[m][n] = 0.0;
				}
				else if(neighbour->yc == center->yc && neighbour->xc > center->xc)
				{
					neighbour->f.BarP[m][n] = 5;
				}
				else if(neighbour->yc == center->yc && neighbour->xc < center->xc)
				{
					neighbour->f.BarP[m][n] = 2;
				}
				else
				{
					cout << "neither xc nor yc is the same;"<<endl;
					_PRINT_ERROR_MSG_FLIP
					getchar();
				}
			}
			double Sum_wdxdfBP = 0.0;
			double Sum_wdydfBP = 0.0;
			for(int Iface = 0;Iface < center->celltype;++Iface)
			{
				neighbour = center->Cell_C[Iface];
				Sum_wdxdfBP += center->wdx_C[Iface]*(neighbour->f.BarP[m][n] - center->f.BarP[m][n]);
				Sum_wdydfBP += center->wdy_C[Iface]*(neighbour->f.BarP[m][n] - center->f.BarP[m][n]);
			}
			center->f.BarP_x[m][n] = center->LS_M[0][0]*Sum_wdxdfBP + center->LS_M[0][1]*Sum_wdydfBP;
			center->f.BarP_y[m][n] = center->LS_M[1][0]*Sum_wdxdfBP + center->LS_M[1][1]*Sum_wdydfBP;
			if(CellArray[i].f.BarP_x[m][n] != 3.0*0.5*NL || CellArray[i].f.BarP_y[m][n] != 1.5*0.5*NL)
			{
				cout <<"CellArray : "<<i<<" ---------"<<endl;
				cout <<"m = "<<m <<"  "<<CellArray[i].f.BarP_x[m][n]
					<<"    "<<CellArray[i].f.BarP_y[m][n]<<endl;
				getchar();
			}
		}
	}
}
#endif