/*
==============================================================================
KratosIncompressibleFluidApplication
A library based on:
Kratos
A General Purpose Software for Multi-Physics Finite Element Analysis
Version 1.0 (Released on march 05, 2007).

Copyright 2007
Pooyan Dadvand, Riccardo Rossi
pooyan@cimne.upc.edu
rrossi@cimne.upc.edu
- CIMNE (International Center for Numerical Methods in Engineering),
Gran Capita' s/n, 08034 Barcelona, Spain


Permission is hereby granted, free  of charge, to any person obtaining
a  copy  of this  software  and  associated  documentation files  (the
"Software"), to  deal in  the Software without  restriction, including
without limitation  the rights to  use, copy, modify,  merge, publish,
distribute,  sublicense and/or  sell copies  of the  Software,  and to
permit persons to whom the Software  is furnished to do so, subject to
the following condition:

Distribution of this code for  any  commercial purpose  is permissible
ONLY BY DIRECT ARRANGEMENT WITH THE COPYRIGHT OWNERS.

The  above  copyright  notice  and  this permission  notice  shall  be
included in all copies or substantial portions of the Software.

THE  SOFTWARE IS  PROVIDED  "AS  IS", WITHOUT  WARRANTY  OF ANY  KIND,
EXPRESS OR  IMPLIED, INCLUDING  BUT NOT LIMITED  TO THE  WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT  SHALL THE AUTHORS OR COPYRIGHT HOLDERS  BE LIABLE FOR ANY
CLAIM, DAMAGES OR  OTHER LIABILITY, WHETHER IN AN  ACTION OF CONTRACT,
TORT  OR OTHERWISE, ARISING  FROM, OUT  OF OR  IN CONNECTION  WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

==============================================================================
*/

//
//   Project Name:        Kratos
//   Last Modified by:    $Author: kazem, pavel $
//   Date:                $Date: 2009-01-21 14:14:49 $
//   Revision:            $Revision: 1.4 $
//
//


#if !defined(KRATOS_ASGS_3D_COMP_ENR_H_INCLUDED )
#define  KRATOS_ASGS_3D_COMP_ENR_H_INCLUDED


// System includes


// External includes
#include "boost/smart_ptr.hpp"


// Project includes
#include "includes/define.h"
#include "includes/element.h"
#include "includes/ublas_interface.h"
#include "includes/variables.h"
#include "includes/serializer.h"


namespace Kratos
{

///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{

///@}
///@name  Enum's
///@{

///@}
///@name  Functions
///@{

///@}
///@name Kratos Classes
///@{

/// Short class definition.
/** Detail class definition.
*/
class ASGS3D_COMP_ENR
    : public Element
{
public:
    ///@name Type Definitions
    ///@{

    /// Counted pointer of Fluid2DASGS
    KRATOS_CLASS_POINTER_DEFINITION(ASGS3D_COMP_ENR);

    ///@}
    ///@name Life Cycle
    ///@{

    /// Default constructor.
    ASGS3D_COMP_ENR(IndexType NewId, GeometryType::Pointer pGeometry);
    ASGS3D_COMP_ENR(IndexType NewId, GeometryType::Pointer pGeometry,  PropertiesType::Pointer pProperties);

    /// Destructor.
    virtual ~ASGS3D_COMP_ENR();


    ///@}
    ///@name Operators
    ///@{


    ///@}
    ///@name Operations
    ///@{

    Element::Pointer Create(IndexType NewId, NodesArrayType const& ThisNodes,  PropertiesType::Pointer pProperties) const;

    void CalculateLocalSystem(MatrixType& rLeftHandSideMatrix, VectorType& rRightHandSideVector, ProcessInfo& rCurrentProcessInfo);

    void CalculateRightHandSide(VectorType& rRightHandSideVector, ProcessInfo& rCurrentProcessInfo);
    //virtual void CalculateLeftHandSide(MatrixType& rLeftHandSideMatrix, ProcessInfo& rCurrentProcessInfo);

    void EquationIdVector(EquationIdVectorType& rResult, ProcessInfo& rCurrentProcessInfo);

    void GetDofList(DofsVectorType& ElementalDofList,ProcessInfo& CurrentProcessInfo);

//	  void InitializeSolutionStep(ProcessInfo& CurrentProcessInfo);

    void Calculate( const Variable<array_1d<double,3> >& rVariable,
                    array_1d<double,3>& Output,
                    const ProcessInfo& rCurrentProcessInfo);

    void GetFirstDerivativesVector(Vector& values, int Step = 0);
    void GetSecondDerivativesVector(Vector& values, int Step = 0);

//      void CalculateDampingMatrix(MatrixType& rDampingMatrix, ProcessInfo& rCurrentProcessInfo);
    void CalculateLocalVelocityContribution(MatrixType& rDampingMatrix,VectorType& rRightHandSideVector,ProcessInfo& rCurrentProcessInfo);


    ///@}
    ///@name Access
    ///@{


    ///@}
    ///@name Inquiry
    ///@{


    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    virtual std::string Info() const
    {
        return "ASGS3D_COMP_ENR #" ;
    }

    /// Print information about this object.
    virtual void PrintInfo(std::ostream& rOStream) const
    {
        rOStream << Info() << Id();
    }

    /// Print object's data.
//      virtual void PrintData(std::ostream& rOStream) const;


    ///@}
    ///@name Friends
    ///@{


    ///@}

protected:
    ///@name Protected static Member Variables
    ///@{


    ///@}
    ///@name Protected member Variables
    ///@{

    virtual void CalculateResidual(const MatrixType& K, VectorType& F);

    virtual void CalculateAdvMassStblTerms(MatrixType& M,const boost::numeric::ublas::bounded_matrix<double,4,3>& DN_DX, const array_1d<double,4>& N, const double thawone,const double volume, const double density);
    virtual void CalculateGradMassStblTerms(MatrixType& M,const boost::numeric::ublas::bounded_matrix<double,4,3>& DN_DX, const array_1d<double,4>& N, const double thawone,const double volume, const double density);
    //Pavel
    virtual void CalculateEnrichmentTerms(MatrixType& DampingMatrix, VectorType& rRightHandSideVector, const double dt);
    virtual void CalculateEnrichmentOperators(boost::numeric::ublas::bounded_matrix<double, 1, 12 > & Dstar, boost::numeric::ublas::bounded_matrix<double, 12, 1 > & Gstar,  boost::numeric::ublas::bounded_matrix<double, 1, 4 > & Lap_star, boost::numeric::ublas::bounded_matrix<double, 1, 4 > & N_Nstar, double & f_star, double & Lap, double & Mstar, const double delta_t );
    virtual void FinalizeSolutionStep( ProcessInfo& CurrentProcessInfo );
    virtual void FinalizeNonLinearIteration( ProcessInfo& CurrentProcessInfo );
	
    virtual void EvaluateAtGaussPoint(double& rResult, const Variable< double >& rVariable, const array_1d< double, 4 >& N, const int n_step );
    virtual void GetValueOnIntegrationPoints( const Variable<double>& rVariable, std::vector<double>& rValues, const ProcessInfo& rCurrentProcessInfo );
    //virtual double EvaluateSoundVelAtGaussPoint(const double distance_at_gauss_point, const double density, const double pressure, const double density_n, const double pressure_n);
    ///@}
    ///@name Protected Operators
    ///@{

    ASGS3D_COMP_ENR() : Element() {}

    ///@}
    ///@name Protected Operations
    ///@{


    ///@}
    ///@name Protected  Access
    ///@{


    ///@}
    ///@name Protected Inquiry
    ///@{


    ///@}
    ///@name Protected LifeCycle
    ///@{


    ///@}


    ///@name Static Member Variables
    ///@{

    ///@}
    ///@name Member Variables
    ///@{

    ///@}
    ///@name Private Operators
    ///@{
    virtual void CalculateMassContribution(MatrixType& K, const array_1d<double,4>& partition_N, const double volume, const double density, const double sound_vel);

    //Pavel: below is changed for several gauss points
    virtual void CalculateViscousTerm(MatrixType& K,const boost::numeric::ublas::bounded_matrix<double,4,3>& DN_DX, const double partition_volume, const double nu, const double rho);
    //Pavel: below is changed for several gauss points
    virtual void CalculateAdvectiveTerm(MatrixType& K,const boost::numeric::ublas::bounded_matrix<double,4,3>& DN_DX, const array_1d<double,4>& partition_N, const double partition_volume, const double density);

    //Pavel: below is changed for several gauss points
    virtual void CalculatePressureTerm(MatrixType& K,const boost::numeric::ublas::bounded_matrix<double,4,3>& DN_DX, const array_1d<double,4>& partition_N,const double partition_volume, const double density, const double sound_vel);

    //Pavel: below is changed for several gauss points
    virtual void CalculateAdvStblAllTerms(MatrixType& K,VectorType& F,const boost::numeric::ublas::bounded_matrix<double,4,3>& DN_DX,const array_1d<double,4>& N, const double tau, const double volume, const double density);

    virtual void CalculateGradStblAllTerms(MatrixType& K,VectorType& F,const boost::numeric::ublas::bounded_matrix<double,4,3>& DN_DX, const array_1d<double,4>& N, const double tau, const double volume, const double density);
    virtual void AddBodyForceAndMomentum(VectorType& F, const array_1d<double,4>& N, const double volume, const double density);

    //Pavel: below is changed for several gauss points
    virtual void CalculateTau(const array_1d<double,4>& N, double& thawone, double& tautwo, const double time,const double partition_volume, const double density, const double viscosity);
	virtual void CalculateDivStblTerm(MatrixType& K, const boost::numeric::ublas::bounded_matrix<double,4,3>& DN_DX, const double tautwo, const double Volume, const double density);
    virtual void CalculateMassMatrix(MatrixType& rMassMatrix, ProcessInfo& rCurrentProcessInfo);
	virtual void ChangeLumpedToConsistPressMassMatrix(VectorType& rRightHandSideVector, const double sound_vel, const double Volume, const double delta_t, const double density);
private:
    ///@}

private:
    ///@name Private Operations
    ///@{


    ///@}
    ///@name Private  Access
    ///@{


    ///@}
    ///@name Serialization
    ///@{
    friend class Serializer;


    virtual void save(Serializer& rSerializer) const
    {
        KRATOS_SERIALIZE_SAVE_BASE_CLASS(rSerializer, Element);
    }

    virtual void load(Serializer& rSerializer)
    {
        KRATOS_SERIALIZE_LOAD_BASE_CLASS(rSerializer, Element);
    }

    ///@}

    ///@name Private Inquiry
    ///@{


    ///@}
    ///@name Un accessible methods
    ///@{

    /// Assignment operator.
    //Fluid2DASGS& operator=(const Fluid2DASGS& rOther);

    /// Copy constructor.
    //Fluid2DASGS(const Fluid2DASGS& rOther);


    ///@}

}; // Class Fluid2DASGS

///@}

///@name Type Definitions
///@{


///@}
///@name Input and output
///@{


/// input stream function
/*  inline std::istream& operator >> (std::istream& rIStream,
				    Fluid2DASGS& rThis);
*/
/// output stream function
/*  inline std::ostream& operator << (std::ostream& rOStream,
				    const Fluid2DASGS& rThis)
    {
      rThis.PrintInfo(rOStream);
      rOStream << std::endl;
      rThis.PrintData(rOStream);

      return rOStream;
    }*/
///@}

}  // namespace Kratos.

#endif // KRATOS_ASGS_3D_COMP_ENR_H_INCLUDED  defined 


