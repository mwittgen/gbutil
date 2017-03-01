// Define linear algebra classes using either TMV or Eigen.
// Each package's classes are mapped to common names by deriving
// new generic wrapper classes from the package-specific ones:
// linalg::Vector<T> / Matrix<T> is a dynamic-size object of data type T
// linalg::SVector<T,N> / SMatrix<T,N1,N2> is a fixed-sized object

// At least one of USE_TMV or USE_EIGEN must exist, and TMV takes precendence
// A compiler error will be generated in this file if neither is defined,
// so you don't have to keep checking that in other files.

// Define USE_MKL to have Eigen use Intel MKL calls.

/***
In either case the dynamic/static sized Vector/Matrix become available.
These classes will be capable of the following common ops:
* Element access through expected methods.  Eigen is range checking unless
  NDEBUG is set, or EIGEN_NO_DEBUG.
* Matrix/vector/scalar arithmetic with overloads.  Don't expect much in
  the way of type promotion in such cases.
* Vector dot products via v.dot(w) or v.transpose() * w.  Favor these over
  v*w, which only TMV overloads to dot product.  Note Eigen dot() conjugates
  v, as does our overload on TMV.
* Vector outer product via v.outer(w).  May not be as efficient as TMV or Eigen native
  methods, but be careful that v * w.transpose() will yield an inner product in TMV.
* Dimensions via rows(), cols(), size(), also TMV-style nrows/cols(), row/colsize()
* transpose(), conjugate(), adjoint() methods giving views
* transpose/conjugate/adjointInPlace() or tranpose/conjugat/adjointSelf() methods
* Initialization to constant values by additional argument of constructor.
* setZero(), setIdentity()
* The TMV real/imagPart() views and Eigen's real/imag() version are put into
  macros v.REAL, v.IMAG, etc.
* Anything else that happens to have identical syntax in the two packages.

GOTCHAS:
* Eigen distinguishes row from column vectors, TMV does not.  This means many
  valid operations in one scheme are ill-defined or wrong in another.  In particular,
  the v.dot(w) method is preferred, although v.transpose() * w works too.  For complex
  vectors, these DIFFER though in that Eigen does conjugation for dot, so it's equiv
  to v.adjoint() * w.
* Be wary of things like v * m: if v is a column vector, this will fail in Eigen. We
  can write v.transpose() * m and it'll work in both schemes with the linalg classes.
  But it yields a row vector in Eigen, which is not convertible to [S]Vector, which is
  defined as a column vector.
* Both packages use expression classes frequently for arithmetic operations.  Getting them
  to convert back to desired class can be tricky because of the extra layer I've added atop.
* Automatic type conversions are scarce in Eigen and even harder after wrapping.  In particular
  there are no float - double mixed-type operations or assignments except to individual coefficients.
  So always cast scalars to the type of the array in arithmetic or in initializations.
* Remember that both packages have uninitialized data after default constructors.
* The operator*= is unary class member in Eigen but an external binop in TMV (at least for small vectors).
  So if you derive further from the linalg classes and override this or other similar ops, you'll
  need to deal with them differently.
* Matrix decomposition, solution, etc., is totally different in the two packages.  You'll need to 
  #ifdef your own code for these.
**/ 
#ifndef LINEARALGEBRA_H
#define LINEARALGEBRA_H

#include <complex>


#ifdef USE_TMV

#include "TMV.h"
#include "TMV_Sym.h"

// Use macros for real/imag part because I'm too lazy to do the overloading
// for only complex types.
#define REAL realPart()
#define IMAG imagPart()

namespace linalg {
  // Dynamic-length vector
  template <typename T>
  class Vector: public tmv::Vector<T> {
  public:
    // Pass constructors to base class
    typedef Vector Type;
    typedef tmv::Vector<T> Base;
    // Pass constructors to base class
    Vector(int n): Base(n) {}
    Vector(int n, T val): Base(n,val) {}
    // Conversion from base class
    Vector(const Base& v): Base(v) {}
    Vector(const Base&& v): Base(v) {}  // Move constructor from TMV vector
    Type& operator=(const Base& v) {Base::operator=(v); return *this;}
    // Conversions for any type that base class can do:
    template <class Other>
    Vector(const Other& v): Base(v) {}
    template <class Other>
    Type& operator=(const Other& v) {Base::operator=(v); return *this;}

    // Map some Eigen syntax into TMV

    // We'll use column vectors from Eigen, but in TMV
    // row & column vectors are the same.
    Vector& transpose() {return *this;}
    const Vector& transpose() const {return *this;}

    // The adjoint is messier since TMV conjugate returns a tmv::VectorView
    // so an extra copy is incurred by using this.
    Vector& adjoint() {return this->conjugate();}
    const Vector& adjoint() const {return this->conjugate();}

    // "dot" operator is overloaded to op* in TMV
    T dot(const Vector<T>& rhs) const {return this->conjugate() * rhs;}
    // outer product:
    tmv::Matrix<T> outer(const Base& rhs) const {return (*this)^rhs;}
  };

  // Fixed-length vector
  template <typename T, int N>
  class SVector: public tmv::SmallVector<T,N> {
  public:
    // Pass constructors to base class
    typedef SVector Type;
    typedef tmv::SmallVector<T,N> Base;
    // Pass constructors to base class
    SVector() =default;
    SVector(T val): Base(val) {}
    // Conversion from base class
    SVector(const Base& v): tmv::SmallVector<T,N>(v) {}
    SVector(const Base&& v): tmv::SmallVector<T,N>(v) {}
    Type& operator=(const Base& v) {Base::operator=(v); return *this;}
    // Conversions for any type that base class can do:
    template <class Other>
    SVector(const Other& v): Base(v) {}
    template <class Other>
    Type& operator=(const Other& v) {Base::operator=(v); return *this;}

    // Map some Eigen syntax into TMV

    // We'll use column vectors from Eigen, but in TMV
    // row & column vectors are the same.
    SVector& transpose() {return *this;}
    const SVector& transpose() const {return *this;}

    // The adjoint is messier since TMV conjugate returns a tmv::VectorView
    // so an extra copy is incurred by using this.
    SVector& adjoint() {return this->conjugate();}
    const SVector& adjoint() const {return this->conjugate();}

    // "dot" operator is overloaded to op* in TMV
    T dot(const SVector<T,N>& rhs) const {return *this * rhs;}
    // outer product:
    template<int N2>
    tmv::SmallMatrix<T,N,N2> outer(const tmv::SmallVector<T,N>& rhs) const {return (*this)^rhs;}
  };
    
  // Dynamic-size matrix
  template <typename T>
  class Matrix: public tmv::Matrix<T> {
  public:
    typedef Matrix Type;
    typedef tmv::Matrix<T> Base;
    // Pass constructors to base class
    Matrix(int n1, int n2): Base(n1,n2) {}
    Matrix(int n1, int n2, T val): Base(n1,n2,val) {}
    // Conversion from base class
    Matrix(const Base& m): Base(m) {}
    Matrix(const Base&& m): Base(m) {}  // Move constructor from TMV
    // TMV Specialization: Matrix(const tmv::AssignableToMatrix<T>& m): Base(m) {}
    Type& operator=(const Base& m) {Base::operator=(m); return *this;}

    // Conversions for any type that base class can do:
    template <class Other>
    Matrix(const Other& m): Base(m) {}
    template <class Other>
    Type& operator=(const Other& m) {Base::operator=(m); return *this;}

    // Map some Eigen syntax into TMV

    Type& setIdentity() {return Base::setToIdentity();}
    Type& transposeInPlace() {return Base::transposeSelf();}
    Type& conjugateInPlace() {return Base::conjugateSelf();}
    Type& adjointInPlace() {Base::conjugateSelf(); return Base::transposeSelf();}
    int rows() const {return Base::nrows();}
    int cols() const {return Base::ncols();}
    tmv::VectorView<T>& diagonal() {return Base::diag();}
    tmv::ConstVectorView<T>& diagonal() const {return Base::diag();}
    T determinant() const {return Base::det();}
  };

  // Fixed-size matrix
  template <typename T, int N1, int N2>
  class SMatrix: public tmv::SmallMatrix<T,N1,N2> {
  public:
    typedef SMatrix Type;
    typedef tmv::SmallMatrix<T,N1,N2> Base;
    // Pass constructors to base class
    SMatrix() =default;
    SMatrix(T val): Base(val) {}
    // Conversion from base class
    SMatrix(const Base& m): Base(m) {}
    SMatrix(const Base&& m): Base(m) {}  // Move constructor from TMV
    // TMV specialization: SMatrix(const tmv::SmallMatrixComposite<T,N1,N2>& m): Base(m) {}
    Type& operator=(const Base& m) {Base::operator=(m); return *this;}

    // Conversions for any type that base class can do:
    template <class Other>
    SMatrix(const Other& m): Base(m) {}
    template <class Other>
    Type& operator=(const Other& m) {Base::operator=(m); return *this;}

    // Map some Eigen syntax into TMV

    Type& setIdentity() {return Base::setToIdentity();}
    Type& transposeInPlace() {return Base::transposeSelf();}
    Type& conjugateInPlace() {return Base::conjugateSelf();}
    Type& adjointInPlace() {Base::conjugateSelf(); return Base::transposeSelf();}
    int rows() const {return Base::nrows();}
    int cols() const {return Base::ncols();}
    tmv::VectorView<T>& diagonal() {return Base::diag();}
    tmv::ConstVectorView<T>& diagonal() const {return Base::diag();}
    T determinant() const {return Base::det();}
  };

} // namespace linalg  

#elif defined USE_EIGEN

#ifdef USE_MKL
#define EIGEN_USE_MKL_ALL
#endif
#include "Eigen/Dense"
#include "Eigen/LU"     // Provides inverse() function
  
// Use macros for real/imag part because I'm too lazy to do the overloading
// for only complex types.
#define REAL real()
#define IMAG imag()

namespace linalg {
  // Dynamic-length vector
  template <typename T>
  class Vector: public Eigen::Matrix<T,Eigen::Dynamic,1> {
  public:
    typedef Vector Type;
    typedef Eigen::Matrix<T,Eigen::Dynamic,1> Base;
    // Pass constructors to base class
    Vector(int n): Base(n) {}
    Vector(int n, T val): Base(Base::Constant(n,val)) {}
    // Conversion from base class
    Vector(const Base& v): Base(v) {}
    Vector(const Base&& v): Base(v) {}  // Move constructor from TMV vector
    template <class Other>
    Vector(const Other& o): Base(o) {}
    template <class Other>
    Type& operator=(const Other& o) {Base::operator=(o); return *this;}
    
    // Map some TMV syntax into Eigen
    Eigen::Block<Base> subVector(int i1, int i2) {return Base::block(i1,0,i2-i1,1);}
    Eigen::Block<const Base> subVector(int i1, int i2) const {return Base::block(i1,0,i2-i1,1);}

    // Define an outer product
    Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> outer(const Base& rhs) {return *this * rhs.transpose();}
  };

  // Fixed-length vector
  template <typename T, int N>
  class SVector: public Eigen::Matrix<T,N,1> {
  public:
    typedef SVector Type;
    typedef Eigen::Matrix<T,N,1> Base;
    // Pass constructors to base class
    SVector() =default;
    SVector(T val): Base(Base::Constant(val)) {}
    // Conversion from base class
    SVector(const Base& v): Base(v) {}
    SVector(const Base&& v): Base(v) {}  // Move constructor from TMV vector
    template <class Other>
    SVector(const Other& o): Base(o) {}
    template <class Other>
    Type& operator=(const Other& o) {Base::operator=(o); return *this;}
    
    // Map some TMV syntax into Eigen
    Eigen::Block<Base> subVector(int i1, int i2) {return Base::block(i1,0,i2-i1,1);}
    Eigen::Block<const Base> subVector(int i1, int i2) const {return Base::block(i1,0,i2-i1,1);}
    // Define an outer product
    template <int N2>
    Eigen::Matrix<T,N,N2> outer(const Eigen::Matrix<T,N2,1>& rhs) {return *this * rhs.transpose();}
  };
  
  // Dynamic-size matrix
  template <typename T>
  class Matrix: public Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> {
  public:
    typedef Matrix Type;
    typedef Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> Base;
    // Pass constructors to base class
    Matrix(int n1, int n2): Base(n1,n2) {}
    Matrix(int n1, int n2, T val): Base(Base::Constant(n1,n2,val)) {}
    // Conversion from base class
    Matrix(const Base& v): Base(v) {}
    Matrix(const Base&& v): Base(v) {}  // Move constructor from TMV vector
    template <class Other>
    Matrix(const Other& o): Base(o) {}
    template <class Other>
    Type& operator=(const Other& o) {Base::operator=(o); return *this;}
    
    // Map some TMV syntax into Eigen
    Eigen::Block<Base> subMatrix(int i1, int i2, int j1, int j2) {return Base::block(i1,j1,i2-i1,j2-j1);}
    Eigen::Block<const Base> subMatrix(int i1, int i2, int j1, int j2) const {return Base::block(i1,j1,i2-i1,j2-j1);}
    int nrows() const {return Base::rows();}
    int ncols() const {return Base::cols();}
    int colsize() const {return Base::rows();}
    int rowsize() const {return Base::cols();}
    T det() const {return Base::determinant();}
    Type& setToIdentity() {Base::setIdentity(); return *this;}
    // diag()
  };

  // Fixed-size matrix
  template <typename T, int N1, int N2>
  class SMatrix: public Eigen::Matrix<T,N1,N2> {
  public:
    typedef SMatrix Type;
    typedef Eigen::Matrix<T,N1,N2> Base;
    // Pass constructors to base class
    SMatrix() =default;
    SMatrix(T val): Base(Base::Constant(val)) {}
    // Conversion from base class
    SMatrix(const Base& v): Base(v) {}
    SMatrix(const Base&& v): Base(v) {}  // Move constructor from TMV vector
    template <class Other>
    SMatrix(const Other& o): Base(o) {}
    template <class Other>
    Type& operator=(const Other& o) {Base::operator=(o); return *this;}
    
    // Map some TMV syntax into Eigen
    Eigen::Block<Base> subMatrix(int i1, int i2, int j1, int j2) {return Base::block(i1,j1,i2-i1,j2-j1);}
    Eigen::Block<const Base> subMatrix(int i1, int i2, int j1, int j2) const {return Base::block(i1,j1,i2-i1,j2-j1);}
    int nrows() const {return Base::rows();}
    int ncols() const {return Base::cols();}
    int colsize() const {return Base::rows();}
    int rowsize() const {return Base::cols();}
    T det() const {return Base::determinant();}
    Type& setToIdentity() {Base::setIdentity(); return *this;}
    // diag()
  };
} // end namespace linalg

#else
#error Either USE_TMV or USE_EIGEN must be specified
#endif

namespace linalg {
  typedef Vector<std::complex<double>> CVector;
  typedef Vector<double> DVector;
  typedef Vector<float>  FVector;
  typedef Vector<int>    IVector; 

  typedef Matrix<std::complex<double>> CMatrix; 
  typedef Matrix<double> DMatrix; 
  typedef Matrix<float> FMatrix; 

  typedef SVector<double,2> DVector2;
  typedef SMatrix<double,2,2> DMatrix22;
  typedef SVector<float,2> FVector2;
  typedef SMatrix<float,2,2> FMatrix22;

} // end namespace linalg


#endif  // LINEARALGEBRA_H
