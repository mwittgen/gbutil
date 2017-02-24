// Short header file that includes TMV and sets up some typedefs to use
#ifndef USE_TMV
#define USE_TMV

#include "TMV.h"
#include "TMV_Sym.h"
typedef tmv::Vector<double> DVector;
typedef tmv::VectorView<double> DVectorView;
typedef tmv::Matrix<double> DMatrix; 
typedef tmv::MatrixView<double> DMatrixView; 
typedef tmv::Vector<int>    IVector; 
typedef tmv::Matrix<int>    IMatrix; 
typedef tmv::Vector<std::complex<double> > CVector;
typedef tmv::Matrix<std::complex<double> > CMatrix;
typedef tmv::SymMatrix<double> DSymMatrix;
typedef tmv::SmallVector<double,2> DVector2;
typedef tmv::SmallMatrix<double,2,2> DMatrix22;
typedef tmv::LowerTriMatrix<double> DLowMatrix;

typedef tmv::SymMatrix<float> FSymMatrix;
typedef tmv::SmallVector<float,2> FVector2;
typedef tmv::SmallMatrix<float,2,2> FMatrix22;
typedef tmv::LowerTriMatrix<float> FLowMatrix;

#endif
